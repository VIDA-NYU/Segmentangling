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
    , _buffer(0)
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
        if (_buffer == 0) {
            glGenBuffers(1, &_buffer);
            LogInfo("Created buffer " << _buffer);

        }


        std::vector<contourtree::Feature> features = tf.getFeatures(-1, _contourTreeLevel);

        // Buffer data layout:
        // uint32:  Number of features
        // per feature
        //   uint32:  Number of identifiers
        //   list of uint32 as identifiers


        // Required size:

        uint32_t requiredSize =  (1 + features.size() + std::accumulate(
            features.begin(),
            features.end(),
            uint32_t(0), 
            [](uint32_t a, const contourtree::Feature& f) {
                return a + static_cast<uint32_t>(f.arcs.size());
            }
        ));

        LogInfo("Creating buffer of size " << sizeof(uint32_t) * requiredSize);
        std::vector<uint32_t> buffer;
        buffer.reserve(requiredSize);
        
        buffer.push_back(static_cast<uint32_t>(features.size()));
        for (const contourtree::Feature& f : features) {
            buffer.push_back(static_cast<uint32_t>(f.arcs.size()));
            buffer.insert(
                buffer.end(),
                std::make_move_iterator(f.arcs.begin()),
                std::make_move_iterator(f.arcs.end())
            );
        }
        
        glNamedBufferData(
            _buffer,
            sizeof(uint32_t) * buffer.size(),
            buffer.data(),
            GL_STATIC_READ
        );
        _dataIsDirty = false;
    }
}

}  // namespace
