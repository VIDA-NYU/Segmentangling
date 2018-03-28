#ifndef __AB_TOPOPPPARAMETERER_H__
#define __AB_TOPOPPPARAMETERER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API TopoPPParameterer : public Processor {
public:
    TopoPPParameterer();
    virtual ~TopoPPParameterer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    FileProperty _volumeFile;
    ButtonProperty _loadDataset;
    FloatProperty _threshold;
    IntProperty _dilation;
    FloatProperty _componentCoefficient;

    EventProperty _eventStartDiffusion;
    EventProperty _eventSelectPoint;
    EventProperty _eventReset;
    EventProperty _eventPreviousInputParameter;
    EventProperty _eventNextInputParameter;

};

} // namespace

#endif // __AB_TOPOPPPARAMETERER_H__
