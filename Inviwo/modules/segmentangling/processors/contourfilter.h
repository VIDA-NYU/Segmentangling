#ifndef __AB_CONTOURFILTER_H__
#define __AB_CONTOURFILTER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

#include <modules/segmentangling/common.h>

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
    virtual void postProcess() override;
    
    ContourInport _contour;
    ContourInport _contourNegative;
};

} // namespace

#endif // __AB_CONTOURFILTER_H__
