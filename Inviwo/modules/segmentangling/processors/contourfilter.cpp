#include "contourfilter.h"


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

const ProcessorInfo ContourFilter::processorInfo_{
    "bock.contourfilter",  // Class identifier
    "Filter Volume By Contour Tree",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

ContourFilter::ContourFilter()
    : VolumeGLProcessor("contourfilter.frag")
    , _contour("identifierBuffer")
{
    addPort(_contour, "ContourGroup");
}

const ProcessorInfo ContourFilter::getProcessorInfo() const {
    return processorInfo_;
}

void ContourFilter::preProcess(TextureUnitContainer& cont) {
    GLuint ssbo = *(_contour.getData());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

}  // namespace
