#include "volumesliceoverlay.h"

namespace inviwo {

const ProcessorInfo VolumeSliceOverlay::processorInfo_{
    "bock.volumesliceoverlay",  // Class identifier
    "Volume Slice Overlay",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

VolumeSliceOverlay::VolumeSliceOverlay()
    : VolumeSliceGL()
{
    //shader_ = Shader("standard.vert", "volumesliceoverlay.frag", false);
}

const ProcessorInfo VolumeSliceOverlay::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace
