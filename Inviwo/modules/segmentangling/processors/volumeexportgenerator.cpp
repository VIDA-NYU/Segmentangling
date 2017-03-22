#include "volumeexportgenerator.h"

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>

#include "libqhullcpp/Qhull.h"

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
    , _featherFactor("_featherFactor", "Feathering", 0.f, 0.f, 0.001f)
    , _shouldOverwriteFiles("_shouldOverwriteFiles", "Should Overwrite files", true)
    , _basePath("_basePath", "Save Base Path")
    , _saveVolumes("_saveVolumes", "Save Volumes")
    , _saveVolumesFlag(false)
{
    addPort(_inportData);
    addPort(_inportIdentifiers);
    addPort(_inportFeatureMapping);

    addProperty(_featherFactor);
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
        usefulFeature[idMapping[i]] = true;
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
                    if (isOutside && dist > _featherFactor.get()) {
                        data[idx] = 0;
                    }
                }
            }
        }

        const std::string fileName = _basePath.get() + "__" + std::to_string(iFeature) + ".dat";
        LogInfo("Saving volume " << iFeature << ": " << fileName);

        writer->setOverwrite(_shouldOverwriteFiles);
        writer->writeData(v, fileName);
        delete v;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

#pragma optimize("", on)

}  // namespace
