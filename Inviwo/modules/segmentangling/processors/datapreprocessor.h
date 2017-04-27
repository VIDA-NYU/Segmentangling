#ifndef __AB_DATAPREPROCESSOR_H__
#define __AB_DATAPREPROCESSOR_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/segmentangling/common.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API DataPreprocessor : public Processor {
public:
    DataPreprocessor();
    virtual ~DataPreprocessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    FileProperty _baseVolume;

    FileProperty _subsampledVolumeFile;
    FileProperty _fullVolumeFile;
    FileProperty _partVolumeFile;
    StringProperty _contourTreeFile;

    bool _volumeIsDirty;
};

} // namespace

#endif // __AB_DATAPREPROCESSOR_H__
