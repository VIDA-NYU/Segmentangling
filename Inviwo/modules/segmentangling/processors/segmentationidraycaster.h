#ifndef __AB_SEGMENTATIONIDRAYCASTER_H__
#define __AB_SEGMENTATIONIDRAYCASTER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/segmentangling/common.h>
#include <inviwo/core/interaction/pickingmapper.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API SegmentationIdRaycaster : public Processor {
public:
    SegmentationIdRaycaster();
    virtual ~SegmentationIdRaycaster() = default;

    virtual void initializeResources() override;

    // override to do member renaming.
    virtual void deserialize(Deserializer& d) override;
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    void eventUpdateMousePos(Event* e);


    //void onVolumeChange();
    void toggleShading(Event*);
    
    Shader _shader;
    VolumeInport _volumePort;
    VolumeInport _segmentationPort;
    ContourInport _contour;
    ContourInport _contourNegative;
    std::shared_ptr<const Volume> _loadedVolume;
    std::shared_ptr<const Volume> _loadedSegmentationVolume;
    ImageInport _entryPort;
    ImageInport _exitPort;
    ImageInport _backgroundPort;
    ImageOutport _outport;

    TransferFunctionProperty _transferFunction;
    OptionPropertyInt _channel;

    SimpleRaycastingProperty _raycasting;
    CameraProperty _camera;
    SimpleLightingProperty _lighting;

    VolumeIndicatorProperty _positionIndicator;
    
    BoolProperty _colorById;
    BoolProperty _filterById;
    IntProperty _id;

    BoolProperty _enablePicking;
    EventProperty _mousePositionTracker;
    IntProperty _selectedFeature;

    EventProperty _toggleShading;
};

} // namespace

#endif // __AB_SEGMENTATIONIDRAYCASTER_H__
