#include "volumestopper.h"

namespace inviwo {

VolumeStopper::VolumeStopper()
    : Processor()
    , _inport("_inport")
    , _outport("_outport")
    , _passthrough("_passthrough", "Do passthrough")
{
    addPort(_inport);
    addPort(_outport);
    addProperty(_passthrough);

}

void VolumeStopper::process() {
    if (_passthrough) {
        _outport.setData(_inport.getData());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo VolumeStopper::processorInfo_ {
    "bock.volumestopper",  // Class identifier
    "VolumeStopper",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};


const ProcessorInfo VolumeStopper::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)