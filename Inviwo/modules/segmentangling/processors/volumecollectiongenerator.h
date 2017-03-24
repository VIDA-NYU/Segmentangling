#ifndef __AB_VOLUMECOLLECTIONGENERATOR_H__
#define __AB_VOLUMECOLLECTIONGENERATOR_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/eventproperty.h>

#include <modules/segmentangling/common.h>


namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API VolumeCollectionGenerator : public Processor {
public:
    VolumeCollectionGenerator();
    virtual ~VolumeCollectionGenerator() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    void selectVolume(Event* e, int volume);
    void addVolumeModification(Event* e);
    void removeVolumeModification(Event* e);
    void toggleConvexHull(Event* e);

    VolumeInport _inport;
    FeatureInport _inportFeatureMapping;
    ContourOutport _outportContour;

    IntProperty _currentVolume;
    ButtonProperty _addVolume;
    ButtonProperty _removeVolume;
    
    OptionPropertyInt _modification;
    StringProperty _modificationText;
    IntProperty _featureToModify;
    ButtonProperty _modify;

    IntProperty _nVolumes;

    EventProperty _selectVolume1Event;
    EventProperty _selectVolume2Event;
    EventProperty _selectVolume3Event;
    EventProperty _selectVolume4Event;
    EventProperty _selectVolume5Event;
    EventProperty _selectVolume6Event;
    EventProperty _selectVolume7Event;
    EventProperty _selectVolume8Event;
    EventProperty _selectVolume9Event;
    EventProperty _selectVolume10Event;
    //EventProperty _selectVolume11Event;
    //EventProperty _selectVolume12Event;
    //EventProperty _selectVolume13Event;
    //EventProperty _selectVolume14Event;
    //EventProperty _selectVolume15Event;

    EventProperty _addVolumeEvent;
    EventProperty _removeVolumeEvent;

    EventProperty _toggleConvexHull;

    EventProperty _trigger;

    ButtonProperty _clearAllVolumes;

    FloatVec3Property _slice1Position;
    FloatVec3Property _slice2Position;
    FloatVec3Property _slice3Position;

    int _lastChangedValue;
    FloatVec3Property& _lastChangedSlicePosition;

    StringProperty _featureNumberFound;
    StringProperty _usingConvexHull;

    std::vector<uint32_t> _mappingData;
    
    std::shared_ptr<ContourInformation> _information;
    //ContourInformation* _information;

    struct {
        bool mapping;
        bool removeVolume;
        bool clearAllVolumes;
    } _dirty;
};

} // namespace

#endif // __AB_VOLUMECOLLECTIONGENERATOR_H__
