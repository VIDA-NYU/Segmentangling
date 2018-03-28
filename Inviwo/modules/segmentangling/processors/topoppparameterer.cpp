#pragma optimize ("", off)

#include "topoppparameterer.h"

namespace inviwo {

TopoPPParameterer::TopoPPParameterer()
    : Processor()
    , _volumeFile("_volumeFile", "Volume File")
    , _loadDataset("_loadDataset", "Load Dataset")
    , _threshold("_threshold", "Threshold", 0.25f, 0.f, 1.f)
    , _dilation("_dilation", "Dilation", 3, 0, 16)
    , _componentCoefficient("_componentCoefficient", "Component Coefficient", 0.5f, 0.f, 1.f)
    , _eventStartDiffusion("_eventStartDiffusion", "Start Straightening", [](Event*) {}, IvwKey::Space)
    , _eventSelectPoint("_eventSelectPoint", "Select Point", [](Event*) {}, IvwKey::Tab)
    , _eventReset("_eventReset", "Reset", [](Event*) {}, IvwKey::F1)
    , _eventPreviousInputParameter(
        "_eventPreviousInputParameter", 
        "Previous input parameter",
        [](Event*) {},
        IvwKey::A
    )
    , _eventNextInputParameter(
        "_eventNextInputParameter",
        "Next input parameter",
        [](Event*) {},
        IvwKey::D
    )
{
    addProperty(_volumeFile);
    addProperty(_threshold);
    addProperty(_dilation);
    addProperty(_loadDataset);
    addProperty(_componentCoefficient);

    addProperty(_eventStartDiffusion);
    addProperty(_eventSelectPoint);
    addProperty(_eventReset);
    addProperty(_eventPreviousInputParameter);
    addProperty(_eventNextInputParameter);
}

void TopoPPParameterer::process() {}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo TopoPPParameterer::processorInfo_ {
    "bock.topopp.parameterer",  // Class identifier
    "TopoPPParameterer",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};


const ProcessorInfo TopoPPParameterer::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)