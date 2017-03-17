#ifndef __AB_VOLUMESLICEOVERLAY_H__
#define __AB_VOLUMESLICEOVERLAY_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <modules/basegl/processors/volumeslicegl.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API VolumeSliceOverlay : public VolumeSliceGL {
public:
    VolumeSliceOverlay();
    virtual ~VolumeSliceOverlay() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
};

} // namespace

#endif // __AB_VOLUMESLICEOVERLAY_H__
