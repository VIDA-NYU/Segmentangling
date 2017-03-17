#include "loadcontourtree.h"


#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>

#include "../../ContourTree/TopologicalFeatures.hpp"

#include <algorithm>

namespace inviwo {

const ProcessorInfo LoadContourTree::processorInfo_{
    "bock.loadcontourtree",  // Class identifier
    "Load Contour Tree",            // Display name
    "Volume Loading",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

LoadContourTree::LoadContourTree()
    : Processor()
    , _outport("identifierBuffer")
    , _contourTreeLevel("contourTreeLevel", "Contour Tree Level", 0.f, 0.f, 1.f)
    , _nFeatures("nFeatures", "Number of Features", 0, 0, 10000)
    , _contourTreeFile("contourTreeFile", "Contour Tree File")
    , _fileIsDirty(false)
    , _dataIsDirty(false)
{
    addPort(_outport, "VolumePortGroup");

    _contourTreeLevel.onChange([&](){ _dataIsDirty = true; });
    addProperty(_contourTreeLevel);

    _nFeatures.onChange([&](){ _dataIsDirty = true; });
    addProperty(_nFeatures);

    _contourTreeFile.onChange([&]() { _fileIsDirty = true; });
    addProperty(_contourTreeFile);
}

const ProcessorInfo LoadContourTree::getProcessorInfo() const {
    return processorInfo_;
}

void LoadContourTree::process() {
    if (_fileIsDirty) {
        std::string file = _contourTreeFile;
        tf = contourtree::TopologicalFeatures();
        tf.loadData(QString::fromStdString(file), true);
        _dataIsDirty = true;
        _fileIsDirty = false;
    }


    if (_dataIsDirty) {
        ContourInformation* info = new ContourInformation;
        glGenBuffers(1, &info->ssbo);
        LogInfo("Created buffer " << info->ssbo);


        std::vector<contourtree::Feature> features = tf.getFeatures(-1, _contourTreeLevel);
        //std::vector<contourtree::Feature> features = tf.getFeaturesPart(-1, _contourTreeLevel);
        //std::vector<contourtree::Feature> features = tf.getFeatures(_nFeatures, 0.f);
        //std::vector<contourtree::Feature> features = tf.getFeaturesPart(_nFeatures, 0.f);
        LogInfo("Number of features: " << features.size());
        info->nFeatures = static_cast<uint32_t>(features.size());

        uint32_t size = tf.ctdata.noArcs;

        // Buffer contents:
        // [0]: number of features
        // [...]: A linearized map from voxel identifier -> feature number
        std::vector<uint32_t> bufferData(size + 1 + 1, -1);
        bufferData[0] = static_cast<uint32_t>(features.size());
        for (size_t i = 0; i < features.size(); ++i) {
            for (uint32_t j : features[i].arcs) {
                bufferData[j + 1] = i;
            }
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, info->ssbo);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            sizeof(uint32_t) * bufferData.size(),
            bufferData.data(),
            GL_DYNAMIC_COPY
        );

        _outport.setData(info);

        _dataIsDirty = false;
    }
}

}  // namespace
