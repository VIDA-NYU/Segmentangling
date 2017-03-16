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
    , _contourTreeFile("contourTreeFile", "Contour Tree File")
    , _fileIsDirty(false)
    , _dataIsDirty(false)
{
    addPort(_outport, "VolumePortGroup");

    _contourTreeLevel.onChange([&](){ _dataIsDirty = true; });
    addProperty(_contourTreeLevel);

    _contourTreeFile.onChange([&]() { _fileIsDirty = true; });
    addProperty(_contourTreeFile);
}

const ProcessorInfo LoadContourTree::getProcessorInfo() const {
    return processorInfo_;
}

void LoadContourTree::process() {
    if (_fileIsDirty) {
        std::string file = _contourTreeFile;
        tf.loadData(QString::fromStdString(file));
        _dataIsDirty = true;
        _fileIsDirty = false;
    }


    if (_dataIsDirty) {
        GLuint* buffer = new GLuint;
        glGenBuffers(1, buffer);
        LogInfo("Created buffer " << *buffer);


        std::vector<contourtree::Feature> features = tf.getFeatures(-1, _contourTreeLevel);

        contourtree::Feature& f = *std::max_element(
            features.begin(),
            features.end(),
            [](const contourtree::Feature& lhs, const contourtree::Feature& rhs) {
                return *std::max_element(lhs.arcs.begin(), lhs.arcs.end()) < *std::max_element(rhs.arcs.begin(), rhs.arcs.end());
            }
        );

        uint32_t size = *std::max_element(f.arcs.begin(), f.arcs.end());

        // The buffer contains a linearized map from voxel identifier -> feature number
        std::vector<uint32_t> bufferData(size + 1, 0);
        for (size_t i = 0; i < features.size(); ++i) {
            for (uint32_t j : features[i].arcs) {
                bufferData[j] = i;
            }
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, *buffer);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            sizeof(uint32_t) * bufferData.size(),
            bufferData.data(),
            GL_DYNAMIC_COPY
        );

        _outport.setData(buffer);

        _dataIsDirty = false;
    }
}

}  // namespace
