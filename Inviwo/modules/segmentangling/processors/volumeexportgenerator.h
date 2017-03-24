#ifndef __AB_VOLUMEEXPORTGENERATOR_H__
#define __AB_VOLUMEEXPORTGENERATOR_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/eventproperty.h>

#include <modules/segmentangling/common.h>


namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API VolumeExportGenerator : public Processor {
public:
    VolumeExportGenerator();
    virtual ~VolumeExportGenerator() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    VolumeInport _inportData;
    VolumeInport _inportIdentifiers;
    ContourInport _inportFeatureMapping;

    VolumeInport _inportFullData;

    IntProperty _featherDistance;

    BoolProperty _shouldOverwriteFiles;
    StringProperty _basePath;
    ButtonProperty _saveVolumes;

    bool _saveVolumesFlag;
};

} // namespace

#endif // __AB_VOLUMEEXPORTGENERATOR_H__
