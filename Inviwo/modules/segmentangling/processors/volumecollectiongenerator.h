#ifndef __AB_VOLUMECOLLECTIONGENERATOR_H__
#define __AB_VOLUMECOLLECTIONGENERATOR_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/segmentangling/common.h>


namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API VolumeCollectionGenerator : public VolumeGLProcessor {
public:
    VolumeCollectionGenerator();
    virtual ~VolumeCollectionGenerator() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void logStatus(std::string msg) const;

    virtual void preProcess(TextureUnitContainer &cont) override;
    virtual void postProcess() override;

    VolumeInport _inportIdentifiers;
    FeatureInport _inportFeatureMapping;

    IntProperty _currentVolume;
    ButtonProperty _addVolume;
    ButtonProperty _removeVolume;
    
    OptionPropertyInt _modification;
    IntProperty _featureToModify;
    ButtonProperty _modify;

    IntProperty _nVolumes;

    std::vector<uint32_t> _mappingData;

    GLuint _ssbo;

    struct {
        bool mapping;
        bool removeVolume;
    } _dirty;
};

} // namespace

#endif // __AB_VOLUMECOLLECTIONGENERATOR_H__
