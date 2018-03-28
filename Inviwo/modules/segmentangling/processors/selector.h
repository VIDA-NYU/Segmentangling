#ifndef __AB_SELECTOR_H__
#define __AB_SELECTOR_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/segmentangling/common.h>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API Selector : public Processor {
public:
    Selector();
    virtual ~Selector() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VertexInport _preprocessedVertexIn;
    TetIndexInport _preprocessedTetIndexIn;

    VertexInport _computedVertexIn;
    TetIndexInport _computedTetIndexIn;

    VertexOutport _vertexOutport;
    TetIndexOutport _tetIndexOutport;

    BoolProperty _useComputedData;
};

} // namespace

#endif // __AB_SELECTOR_H__
