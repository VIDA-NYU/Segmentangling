#ifndef __AB_VOLUMESTOPPER_H__
#define __AB_VOLUMESTOPPER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/segmentangling/common.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API VolumeStopper : public Processor {
public:
    VolumeStopper();
    virtual ~VolumeStopper() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VolumeInport _inport;
    VolumeOutport _outport;

    BoolProperty _passthrough;
};

} // namespace

#endif // __AB_VOLUMESTOPPER_H__
