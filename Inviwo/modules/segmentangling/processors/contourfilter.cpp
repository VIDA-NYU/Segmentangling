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
    , _contourNegative("identifierBufferNegative")
{
    addPort(_contour);
    _contourNegative.setOptional(true);
    addPort(_contourNegative);

    //this->dataFormat_ = DataUInt32::get();
    this->dataFormat_ = DataVec2UInt32::get();
    //this->dataFormat_ = DataFloat32::get();
}

const ProcessorInfo ContourFilter::getProcessorInfo() const {
    return processorInfo_;
}

void ContourFilter::preProcess(TextureUnitContainer& cont) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _contour.getData()->ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _contour.getData()->ssbo);

    shader_.setUniform("hasNegativeData", _contourNegative.hasData());
    if (_contourNegative.hasData()) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _contourNegative.getData()->ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _contourNegative.getData()->ssbo);
    }

    GLuint tex = volume_->getRepresentation<VolumeGL>()->getTexture()->getID();
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void ContourFilter::postProcess() {
    volume_->dataMap_.dataRange = dvec2(0, _contour.getData()->nFeatures);
    volume_->dataMap_.valueRange = dvec2(0, _contour.getData()->nFeatures);
}

}  // namespace
