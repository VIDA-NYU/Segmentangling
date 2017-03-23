#include "volumeexportgenerator.h"

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>

#include "libqhullcpp/Qhull.h"
#include "libqhullcpp/QhullFacetList.h"

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

    // Some of the features might not contain any information at all, so we filter those
    // in advance for some more rapid saving
    const size_t size = identifierVolume.getDimensions().x * identifierVolume.getDimensions().y * identifierVolume.getDimensions().z;
    std::vector<bool> usefulFeature(features.nFeatures, false);
    for (size_t i = 0; i < size; ++i) {
        uint32_t feature = idMapping[i];
        if (feature != uint32_t(-1)) {
            usefulFeature[feature] = true;
        }
    }

    auto factory = getNetwork()->getApplication()->getDataWriterFactory();
    auto writer = factory->template getWriterForTypeAndExtension<Volume>("dat");

    LogInfo("Saving " << features.nFeatures << " volumes");
    for (size_t iFeature = 0; iFeature < features.nFeatures; ++iFeature) {
        if (!usefulFeature[iFeature]) {
            LogInfo("Skippping empty feature " << iFeature);
            continue;
        }
        LogInfo("Starting volume " << iFeature);

        Volume* v = dataVolume.clone();
        VolumeRAM* rep = v->getEditableRepresentation<VolumeRAM>();
        uint8_t* data = reinterpret_cast<uint8_t*>(rep->getData());

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

        using namespace orgQhull;
        const char* f = "";
        Qhull hull(f, 3, points.size() / 3, points.data(), f);


        LogInfo("Creating volume " << iFeature);
        for (size_t x = 0; x < rep->getDimensions().x; ++x) {
            for (size_t y = 0; y < rep->getDimensions().y; ++y) {
                for (size_t z = 0; z < rep->getDimensions().z; ++z) {
                    const uint64_t idx = VolumeRAM::posToIndex({ x, y, z }, rep->getDimensions());

                    double pt[3] = {
                        static_cast<double>(x) / static_cast<double>(rep->getDimensions().x),
                        static_cast<double>(y) / static_cast<double>(rep->getDimensions().y),
                        static_cast<double>(z) / static_cast<double>(rep->getDimensions().z)
                    };

                    double dist;
                    boolT isOutside;
                    qh_findbestfacet(hull.qh(), pt, qh_False, &dist, &isOutside);

                    // Remove all the voxels that are outside of the convex hull
                    //if (isOutside && dist > _featherFactor.get()) {
                    if (isOutside) {
                        data[idx] = 0;
                    }



                }
            }
        }

        // Finding the dominant axis
        //std::vector<QhullFacet> list = hull.facetList().toStdVector();
        //using FacetPair = std::pair<size_t, size_t>;
        //std::vector<FacetPair> facetPairs;
        //for (size_t i = 0; i < list.size(); ++i) {
        //    for (size_t j = i; j < list.size(); ++j) {
        //        facetPairs.emplace_back(i, j);
        //    }
        //}

        //std::sort(
        //    facetPairs.begin(),
        //    facetPairs.end(),
        //    [&list](const FacetPair& lhs, const FacetPair& rhs) {
        //        glm::dvec3 lhsFirst = glm::make_vec3(list[lhs.first].getCenter().coordinates());
        //        glm::dvec3 lhsSecond = glm::make_vec3(list[lhs.second].getCenter().coordinates());
        //        double lhsLength = glm::distance(lhsFirst, lhsSecond);
        //         
        //        glm::dvec3 rhsFirst = glm::make_vec3(list[rhs.first].getCenter().coordinates());
        //        glm::dvec3 rhsSecond = glm::make_vec3(list[rhs.second].getCenter().coordinates());
        //        double rhsLength = glm::distance(rhsFirst, rhsSecond);

        //        // Put the longest length first in the vector
        //        return lhsLength > rhsLength;
        //    }
        //);

        //glm::dvec3 z =
        //    glm::make_vec3(list[facetPairs[0].second].getCenter().coordinates()) -
        //    glm::make_vec3(list[facetPairs[0].first].getCenter().coordinates());

        //glm::dvec3 second = 
        //    glm::make_vec3(list[facetPairs[1].second].getCenter().coordinates()) -
        //    glm::make_vec3(list[facetPairs[1].first].getCenter().coordinates());

        //glm::dvec3 y = glm::cross(z, second);
        //glm::dvec3 x = -glm::cross(z, y);

        //glm::dmat3 rotMat = {
        //    x.x, y.x, z.x,
        //    x.y, y.y, z.y,
        //    x.z, y.z, z.z
        //};

        //Volume* vClone = v->clone();
        //const uint8_t* srcData = reinterpret_cast<const uint8_t*>(vClone->getRepresentation<VolumeRAM>()->getData());
        //LogInfo("Rotating volume");
        //for (size_t x = 0; x < rep->getDimensions().x; ++x) {
        //    for (size_t y = 0; y < rep->getDimensions().y; ++y) {
        //        for (size_t z = 0; z < rep->getDimensions().z; ++z) {
        //            const glm::dvec3 posOld = glm::dvec3(x, y, z);

        //            const uint64_t idxOld = VolumeRAM::posToIndex({ x, y, z }, rep->getDimensions());

        //            const glm::dvec3 posNew = rotMat * posOld;
        //            const glm::size3_t posNewClamped = glm::clamp(
        //                glm::size3_t(posNew),
        //                glm::size3_t(0),
        //                rep->getDimensions()
        //            );

        //            const uint64_t idxNew = VolumeRAM::posToIndex({ posNewClamped.x, posNewClamped.y, posNewClamped.z }, rep->getDimensions());

        //            data[idxNew] = srcData[idxOld];
        //        }
        //    }
        //}


        const std::string fileName = _basePath.get() + "__" + std::to_string(iFeature) + ".dat";
        LogInfo("Saving volume " << iFeature << ": " << fileName);

        writer->setOverwrite(_shouldOverwriteFiles);
        writer->writeData(v, fileName);

        _outport.setData(v);
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

#pragma optimize("", on)

}  // namespace
