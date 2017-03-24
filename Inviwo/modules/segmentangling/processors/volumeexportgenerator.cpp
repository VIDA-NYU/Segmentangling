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
    , _outport("_outport")
    , _featherFactor("_featherFactor", "Feathering", 0.f, 0.f, 0.01f)
    , _shouldOverwriteFiles("_shouldOverwriteFiles", "Should Overwrite files", true)
    , _basePath("_basePath", "Save Base Path")
    , _saveVolumes("_saveVolumes", "Save Volumes")
    , _saveVolumesFlag(false)
{
    addPort(_inportData);
    addPort(_inportIdentifiers);
    addPort(_inportFeatureMapping);
    addPort(_outport);

    //addProperty(_featherFactor);
    addProperty(_shouldOverwriteFiles);
    addProperty(_basePath);
    _saveVolumes.onChange([this]() { _saveVolumesFlag = true; });
    addProperty(_saveVolumes);
}

const ProcessorInfo VolumeExportGenerator::getProcessorInfo() const {
    return processorInfo_;
}

#pragma optimize("", off)

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




    // Lambda expression to create the volumes from the convex hulls
    auto createVolumes = [&featureInfos, &identifierData, &idMapping](uint32_t iFeature, Volume* volume) {
        VolumeRAM* rep = volume->getEditableRepresentation<VolumeRAM>();
        uint8_t* data = reinterpret_cast<uint8_t*>(rep->getData());

        for (size_t x = 0; x < rep->getDimensions().x; ++x) {
            for (size_t y = 0; y < rep->getDimensions().y; ++y) {
                for (size_t z = 0; z < rep->getDimensions().z; ++z) {
                    const uint64_t idx = VolumeRAM::posToIndex({ x, y, z }, rep->getDimensions());

                    if (featureInfos[iFeature].usingConvexHull) {
                        double pt[3] = {
                            static_cast<double>(x) / static_cast<double>(rep->getDimensions().x),
                            static_cast<double>(y) / static_cast<double>(rep->getDimensions().y),
                            static_cast<double>(z) / static_cast<double>(rep->getDimensions().z)
                        };

                        double dist;
                        boolT isOutside;

                        featureInfos[iFeature].convexHullMutex.lock();
                        qh_findbestfacet(featureInfos[iFeature].convexHull.qh(), pt, qh_False, &dist, &isOutside);
                        featureInfos[iFeature].convexHullMutex.unlock();

                        if (isOutside) {
                            // Remove all the voxels that are outside of the convex hull
                            data[idx] = 0;
                        }
                        else {
                            const uint32_t id = identifierData[idx];
                            const uint32_t feature = idMapping[id];
                            if (feature != iFeature && feature != uint32_t(-1)) {
                                // If the voxel is not a feature, we can remove it
                                data[idx] = 0;
                            }
                        }
                    }
                    else {
                        // We are not using the convex hull, so we just filter out the
                        // feature indentifiers
                        const uint32_t id = identifierData[idx];
                        const uint32_t feature = idMapping[id];
                        if (feature != iFeature) {
                            data[idx] = 0;
                        }
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
    auto saveVolumes = [&featureInfos, this](size_t iFeature){
        const std::string fileName = _basePath.get() + "__" + std::to_string(iFeature) + ".dat";
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

    _outport.setData(featureInfos[0].volume);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

#pragma optimize("", on)

}  // namespace
