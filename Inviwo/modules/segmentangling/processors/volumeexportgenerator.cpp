#include "volumeexportgenerator.h"

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>

#include "libqhullcpp/Qhull.h"
#include "libqhullcpp/QhullFacetList.h"

using namespace orgQhull;

namespace inviwo {

namespace {
} // namespace

const ProcessorInfo VolumeExportGenerator::processorInfo_{
    "bock.volumeexportgenerator",  // Class identifier
    "Volume Export Generator",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

VolumeExportGenerator::VolumeExportGenerator()
    : Processor()
    , _inportData("volumeinportdata")
    , _inportIdentifiers("volumeinportidentifiers")
    , _inportFeatureMapping("inportfeaturemapping")
    , _inportFullData("inportfulldata")
    , _featherDistance("_featherDistance", "Feathering", 0, 0, 100)
    , _shouldOverwriteFiles("_shouldOverwriteFiles", "Should Overwrite files", true)
    , _basePath("_basePath", "Save Base Path")
    , _saveVolumes("_saveVolumes", "Save Volumes")
    , _saveVolumesFlag(false)
{
    addPort(_inportData);
    addPort(_inportIdentifiers);
    addPort(_inportFeatureMapping);
    _inportFullData.setOptional(true);
    addPort(_inportFullData);

    addProperty(_featherDistance);
    addProperty(_shouldOverwriteFiles);
    addProperty(_basePath);
    _saveVolumes.onChange([this]() { _saveVolumesFlag = true; });
    addProperty(_saveVolumes);
}

const ProcessorInfo VolumeExportGenerator::getProcessorInfo() const {
    return processorInfo_;
}

//#pragma optimize("", off)

void VolumeExportGenerator::process() {
    if (!_saveVolumesFlag || !_inportData.hasData() || !_inportIdentifiers.hasData()) {
        return;
    }
    _saveVolumesFlag = false;

    const Volume& dataVolume = *_inportData.getData();
    const VolumeRAM& identifierVolume = *(_inportIdentifiers.getData()->getRepresentation<VolumeRAM>());
    const uint32_t* identifierData = reinterpret_cast<const uint32_t*>(identifierVolume.getData());

    LogInfo("Inverting mapping information");

    const ContourInformation& features = *_inportFeatureMapping.getData();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, features.ssbo);
    // We jump ahead one because the first value in the ssbo contains the numebr of features
    const uint32_t* idMapping = reinterpret_cast<const uint32_t*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY)) + 1;


    struct InternalFeatureInfo {
        bool isUsed = false;
        Qhull convexHull;
        std::mutex convexHullMutex;
        std::thread thread;
        Volume* volume = nullptr;
        bool usingConvexHull;
        glm::size3_t boundingBoxMin = glm::size3_t(-1);
        glm::size3_t boundingBoxMax = glm::size3_t(0);
    };

    // Some of the features might not contain any information at all, so we filter those
    // in advance for some more rapid saving
    const size_t size = identifierVolume.getDimensions().x * identifierVolume.getDimensions().y * identifierVolume.getDimensions().z;
    std::vector<InternalFeatureInfo> featureInfos(features.nFeatures);
    for (size_t i = 0; i < size; ++i) {
        uint32_t feature = idMapping[i];
        if (feature != uint32_t(-1)) {
            featureInfos[feature].isUsed = true;
        }
    }

    // Populating the data in the feature infos struct
    for (size_t i = 0; i < features.nFeatures; ++i) {
        LogInfo("Using convex hull: " << i << "  " << features.useConvexHull[i] ? "true" : "false");
        featureInfos[i].usingConvexHull = features.useConvexHull[i];
    }


    // Lambda to wait on used feature threads
    auto waitOnThreads = [&featureInfos](){
        for (InternalFeatureInfo& info : featureInfos) {
            if (info.isUsed) {
                info.thread.join();
            }
        }
    };

    // Lambda to construct the convex hull
    auto constructConvexHull = [&featureInfos, &dataVolume, &idMapping, &identifierData, this](size_t iFeature) {
        LogInfo("Starting volume " << iFeature);

        const VolumeRAM* rep = dataVolume.getRepresentation<VolumeRAM>();
        const uint8_t* data = reinterpret_cast<const uint8_t*>(rep->getData());

        std::vector<double> points;

        LogInfo("Creating convex hull " << iFeature);
        for (size_t x = 0; x < rep->getDimensions().x; ++x) {
            for (size_t y = 0; y < rep->getDimensions().y; ++y) {
                for (size_t z = 0; z < rep->getDimensions().z; ++z) {
                    const uint64_t idx = VolumeRAM::posToIndex({ x, y, z }, rep->getDimensions());

                    const uint32_t id = identifierData[idx];

                    if (idMapping[id] == iFeature) {
                        // If it belongs to the feature, we want to throw it into the QHull computation later
                        points.push_back(static_cast<double>(x) / static_cast<double>(rep->getDimensions().x));
                        points.push_back(static_cast<double>(y) / static_cast<double>(rep->getDimensions().y));
                        points.push_back(static_cast<double>(z) / static_cast<double>(rep->getDimensions().z));
                    }
                }
            }
        }

        const char* f = "";
        featureInfos[iFeature].convexHull.runQhull(f, 3, int(points.size() / 3), points.data(), f);
    };
    



    // Construct the convex hulls
    LogInfo("Saving " << features.nFeatures << " volumes");
    for (size_t iFeature = 0; iFeature < features.nFeatures; ++iFeature) {
        if (featureInfos[iFeature].isUsed) {
            featureInfos[iFeature].thread = std::thread(constructConvexHull, iFeature);
        }
    }




    // We need to wait for all threads to finish before we can continue
    waitOnThreads();


    auto neighboringVoxelOffsets = [](int distance) {
        std::vector<glm::ivec3> offsets;
        offsets.reserve(pow(2 * distance + 1, 3));

        offsets.push_back({ 0, 0, 0 });

        for (int x = -distance; x <= distance; ++x) {
            for (int y = -distance; y <= distance; ++y) {
                for (int z = -distance; z <= distance; ++z) {
                    if (x == 0 && y == 0 && z == 0) {
                        // We put (0,0,0) at the top to speed up things later
                        continue;
                    }

                    offsets.push_back({ x, y, z });
                }
            }
        }

        return offsets;
    };

    const std::vector<glm::ivec3> offsets = neighboringVoxelOffsets(_featherDistance);


    // Lambda expression to create the volumes from the convex hulls
    auto createVolumes = [&featureInfos, &identifierData, &idMapping, &offsets](uint32_t iFeature, Volume* volume) {
        VolumeRAM* rep = volume->getEditableRepresentation<VolumeRAM>();
        uint8_t* data = reinterpret_cast<uint8_t*>(rep->getData());



        // Predicate that tests the position against the convex hull
        auto voxelPredicateCH = [&featureInfos, iFeature, &data, &identifierData, &idMapping](const glm::size3_t& pos, const glm::size3_t& dim) {
            const glm::size3_t p = glm::clamp(
                pos,
                glm::size3_t(0),
                dim
            );


            const uint64_t idx = VolumeRAM::posToIndex(p, dim);
            double pt[3] = {
                static_cast<double>(p.x) / static_cast<double>(dim.x),
                static_cast<double>(p.y) / static_cast<double>(dim.y),
                static_cast<double>(p.z) / static_cast<double>(dim.z)
            };

            double dist;
            boolT isOutside;

            featureInfos[iFeature].convexHullMutex.lock();
            qh_findbestfacet(featureInfos[iFeature].convexHull.qh(), pt, qh_False, &dist, &isOutside);
            featureInfos[iFeature].convexHullMutex.unlock();

            if (isOutside) {
                // Remove all the voxels that are outside of the convex hull
                return true;
            }
            else {
                const uint32_t id = identifierData[idx];
                const uint32_t feature = idMapping[id];
                if (feature != iFeature && feature != uint32_t(-1)) {
                    // If the voxel is not a feature, we can remove it
                    return true;
                }
            }
            return false;
        };

        auto voxelPredicateCHNeighbor = [&voxelPredicateCH, &offsets](const glm::size3_t& pos, const glm::size3_t& dim) {
            for (const glm::size3_t& offset : offsets) {
                if (!voxelPredicateCH(pos + offset, dim)) {
                    return false;
                }
            }
            return true;
        };

        // Predicate that tests the position against the feature list
        auto voxelPredicate = [&identifierData, &idMapping, iFeature, &data](const glm::size3_t& pos, const glm::size3_t& dim) {
            // We are not using the convex hull, so we just filter out the
            // feature indentifiers
            const glm::size3_t p = glm::clamp(
                pos,
                glm::size3_t(0),
                dim - glm::size3_t(1)
            );

            const uint64_t idx = VolumeRAM::posToIndex(p, dim);
            const uint32_t id = identifierData[idx];
            const uint32_t feature = idMapping[id];

            if (feature == iFeature) {
                return false;
            }
            else {
                return true;
            }
        };

        auto voxelPredicateNeighbor = [&voxelPredicate, &offsets](const glm::size3_t& pos, const glm::size3_t& dim) {
            for (const glm::size3_t& offset : offsets) {
                if (!voxelPredicate(pos + offset, dim)) {
                    return false;
                }
            }
            return true;
        };


        for (size_t x = 0; x < rep->getDimensions().x; ++x) {
            for (size_t y = 0; y < rep->getDimensions().y; ++y) {
                for (size_t z = 0; z < rep->getDimensions().z; ++z) {
                    const uint64_t idx = VolumeRAM::posToIndex({ x, y, z }, rep->getDimensions());

                    bool removeValue;
                    if (featureInfos[iFeature].usingConvexHull) {
                        removeValue = voxelPredicateCHNeighbor({ x, y, z }, rep->getDimensions());
                    }
                    else {
                        removeValue = voxelPredicateNeighbor({ x, y, z }, rep->getDimensions());
                    }

                    if (removeValue) {
                        data[idx] = 0;
                    }
                    else {
                        featureInfos[iFeature].boundingBoxMin = glm::min(
                            featureInfos[iFeature].boundingBoxMin,
                            { x, y, z }
                        );

                        featureInfos[iFeature].boundingBoxMax = glm::max(
                            featureInfos[iFeature].boundingBoxMax,
                            { x, y, z }
                        );
                    }
                }
            }
        }
    };




    // Construct the volumes from the convex hulls
    for (size_t iFeature = 0; iFeature < features.nFeatures; ++iFeature) {
        if (featureInfos[iFeature].isUsed) {
            featureInfos[iFeature].volume = dataVolume.clone();
            featureInfos[iFeature].thread = std::thread(createVolumes, iFeature, featureInfos[iFeature].volume);
        }
    }
    



    // Lambda expression to save the volumes
    auto saveVolumes = [&featureInfos, this](size_t iFeature) {
        const std::string fileName = _basePath.get() + "__small__" + std::to_string(iFeature) + ".dat";
        LogInfo("Saving volume " << iFeature << ": " << fileName);

        auto factory = getNetwork()->getApplication()->getDataWriterFactory();
        auto writer = factory->template getWriterForTypeAndExtension<Volume>("dat");
        writer->setOverwrite(_shouldOverwriteFiles);
        writer->writeData(featureInfos[iFeature].volume, fileName);
    };




    // Save the volumes
    for (size_t iFeature = 0; iFeature < features.nFeatures; ++iFeature) {
        if (featureInfos[iFeature].isUsed) {
            featureInfos[iFeature].thread.join();
            featureInfos[iFeature].thread = std::thread(
                saveVolumes, iFeature
            );
        }
    }

    waitOnThreads();


    // Save the volumes;  can't do this multithreaded as we would probably run out of memory
    if (_inportFullData.hasData()) {
        //const VolumeRAM& fullRep = *_inportFullData.getData()->getRepresentation<VolumeRAM>();
        for (size_t iFeature = 0; iFeature < features.nFeatures; ++iFeature) {
            if (!featureInfos[iFeature].isUsed) {
                continue;
            }

            const VolumeRAM& fullRep = *_inportFullData.getData()->getRepresentation<VolumeRAM>();
            const uint8_t* fullData = reinterpret_cast<const uint8_t*>(fullRep.getData());

            const VolumeRAM& smallRep = *featureInfos[iFeature].volume->getRepresentation<VolumeRAM>();
            const uint8_t* smallData = reinterpret_cast<const uint8_t*>(smallRep.getData());

            const glm::ivec3 fullBoundingBoxMin = {
                int(floor((double(featureInfos[iFeature].boundingBoxMin.x) / smallRep.getDimensions().x) * fullRep.getDimensions().x)),
                int(floor((double(featureInfos[iFeature].boundingBoxMin.y) / smallRep.getDimensions().y) * fullRep.getDimensions().y)),
                int(floor((double(featureInfos[iFeature].boundingBoxMin.z) / smallRep.getDimensions().z) * fullRep.getDimensions().z))
            };

            const glm::ivec3 fullBoundingBoxMax = {
                int(ceil((double(featureInfos[iFeature].boundingBoxMax.x) / smallRep.getDimensions().x) * fullRep.getDimensions().x)),
                int(ceil((double(featureInfos[iFeature].boundingBoxMax.y) / smallRep.getDimensions().y) * fullRep.getDimensions().y)),
                int(ceil((double(featureInfos[iFeature].boundingBoxMax.z) / smallRep.getDimensions().z) * fullRep.getDimensions().z))
            };
            const glm::ivec3 newSize = fullBoundingBoxMax - fullBoundingBoxMin;



            Volume* newVolume = new Volume(newSize);
            VolumeRAM& newRep = *newVolume->getEditableRepresentation<VolumeRAM>();
            uint8_t* newData = reinterpret_cast<uint8_t*>(newRep.getData());

            std::memset(newData, uint8_t(0), newRep.getNumberOfBytes());



            // For each full voxel, find the covering small voxel
            for (int x = 0; x < newSize.x; ++x) {
#pragma omp parallel for num_threads(10)
                for (int y = 0; y < newSize.y; ++y) {
                    for (int z = 0; z < newSize.z; ++z) {
                        const size3_t fullPos = {
                            x + fullBoundingBoxMin.x,
                            y + fullBoundingBoxMin.y,
                            z + fullBoundingBoxMin.z
                        };

                        const glm::dvec3 p = {
                            double(fullPos.x) / fullRep.getDimensions().x,
                            double(fullPos.y) / fullRep.getDimensions().y,
                            double(fullPos.z) / fullRep.getDimensions().z
                        };

                        const size3_t smallPos = {
                            size_t(p.x * smallRep.getDimensions().x),
                            size_t(p.y * smallRep.getDimensions().y),
                            size_t(p.z * smallRep.getDimensions().z)
                        };


                        const uint64_t smallIdx = VolumeRAM::posToIndex(smallPos, smallRep.getDimensions());
                        if (smallData[smallIdx] != 0) {
                            const uint64_t fullIdx = VolumeRAM::posToIndex(fullPos, fullRep.getDimensions());

                            const uint64_t newIdx = VolumeRAM::posToIndex({x, y, z}, newRep.getDimensions());
                            newData[newIdx] = fullData[fullIdx];
                        }
                    }
                }
            }

            const std::string fileName = _basePath.get() + "__" + std::to_string(iFeature) + ".dat";
            LogInfo("Saving volume " << iFeature << ": " << fileName);

            auto factory = getNetwork()->getApplication()->getDataWriterFactory();
            auto writer = factory->template getWriterForTypeAndExtension<Volume>("dat");
            writer->setOverwrite(_shouldOverwriteFiles);
            writer->writeData(newVolume, fileName);

            delete newVolume;
        }
    }

    // @LEAK:  The volumes are not deleted

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

//#pragma optimize("", on)

}  // namespace
