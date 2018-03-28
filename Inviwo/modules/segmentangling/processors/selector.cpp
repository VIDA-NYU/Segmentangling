#pragma optimize ("", off)

#include "selector.h"

namespace inviwo {

Selector::Selector()
    : Processor()
    , _preprocessedVertexIn("_preprocessedVertexIn")
    , _preprocessedTetIndexIn("_preprocessedTetIndexIn")
    , _computedVertexIn("_computedVertexIn")
    , _computedTetIndexIn("_computedTetIndexIn")
    , _vertexOutport("_vertexOutport")
    , _tetIndexOutport("_tetIndexOutport")
    , _useComputedData("_useComputedData", "Use computed Data")
{
    _preprocessedVertexIn.setOptional(true);
    addPort(_preprocessedVertexIn);
    _preprocessedTetIndexIn.setOptional(true);
    addPort(_preprocessedTetIndexIn);

    _computedVertexIn.setOptional(true);
    addPort(_computedVertexIn);
    _computedTetIndexIn.setOptional(true);
    addPort(_computedTetIndexIn);

    addPort(_vertexOutport);
    addPort(_tetIndexOutport);

    addProperty(_useComputedData);
}

void Selector::process() {
    if (!_preprocessedVertexIn.hasData() && !_preprocessedTetIndexIn.hasData() &&
        !_computedVertexIn.hasData() && !_computedTetIndexIn.hasData())
    {
        return;
    }

    if (_useComputedData) {
        if (!_computedVertexIn.hasData() && !_computedTetIndexIn.hasData()) {
            return;
        }
        _vertexOutport.setData(_computedVertexIn.getData());
        _tetIndexOutport.setData(_computedTetIndexIn.getData());
    }
    else {
        if (!_preprocessedVertexIn.hasData() && !_preprocessedTetIndexIn.hasData()) {
            return;
        }

        _vertexOutport.setData(_preprocessedVertexIn.getData());
        _tetIndexOutport.setData(_preprocessedTetIndexIn.getData());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo Selector::processorInfo_ {
    "bock.selector",  // Class identifier
    "Selector",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};


const ProcessorInfo Selector::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)