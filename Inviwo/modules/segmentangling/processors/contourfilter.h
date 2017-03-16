#ifndef __AB_CONTOURFILTER_H__
#define __AB_CONTOURFILTER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

#include "../../ContourTree/TopologicalFeatures.hpp"

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API ContourFilter : public VolumeGLProcessor {
public:
    ContourFilter();
    virtual ~ContourFilter() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;
    
    DataInport<GLuint> _contour;
};

} // namespace

#endif // __AB_CONTOURFILTER_H__
