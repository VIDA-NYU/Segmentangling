#include "segmentationidraycaster.h"


#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

const ProcessorInfo SegmentationIdRaycaster::processorInfo_{
    "bock.segmentationidraycaster",  // Class identifier
    "Segmentation ID Raycaster",            // Display name
    "Volume Rendering",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

SegmentationIdRaycaster::SegmentationIdRaycaster()
    : Processor()
    , _shader("segmentationraycaster.frag", false)
    , _volumePort("volume")
    , _segmentationPort("segmentation")
    , _contour("contour")
    , _entryPort("entry")
    , _exitPort("exit")
    , _backgroundPort("bg")
    , _outport("outport")
    , _performFeatureLookup("performFeatureLookup", "Lookup feature from segmentation", true)
    , _colorById("colorById", "Color By ID", true)
    , _filterById("filterById", "Filter by Identifier", true)
    , _id("id", "Identifier", -1, -1, std::numeric_limits<int>::max(), 1)
    , _transferFunction("transferFunction", "Transfer function", TransferFunction(), &_volumePort)
    , _channel("channel", "Render Channel")
    , _raycasting("raycaster", "Raycasting")
    , _camera("camera", "Camera")
    , _lighting("lighting", "Lighting", &_camera)
    , _positionIndicator("positionindicator", "Position Indicator")
    , _toggleShading("toggleShading", "Toggle Shading", [this](Event* e) { toggleShading(e); },
                     IvwKey::L)
{
                     
    _shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(_volumePort, "VolumePortGroup");
    addPort(_segmentationPort, "VolumePortGroup");
    addPort(_contour);
    addPort(_entryPort, "ImagePortGroup1");
    addPort(_exitPort, "ImagePortGroup1");
    addPort(_outport, "ImagePortGroup1");
    addPort(_backgroundPort,"ImagePortGroup1");

    _backgroundPort.setOptional(true);

    _channel.addOption("Channel 1", "Channel 1", 0);
    _channel.setSerializationMode(PropertySerializationMode::All);
    _channel.setCurrentStateAsDefault();

    //_volumePort.onChange(this, &SegmentationIdRaycaster::onVolumeChange);
    _backgroundPort.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    _backgroundPort.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    // change the currently selected channel when a pre-computed gradient is selected
    _raycasting.gradientComputationMode_.onChange([this]() {
        if (_channel.size() == 4) {
            if (_raycasting.gradientComputationMode_.isSelectedIdentifier("precomputedXYZ")) {
                _channel.set(3);
            } else if (_raycasting.gradientComputationMode_.isSelectedIdentifier(
                           "precomputedYZW")) {
                _channel.set(0);
            }
        }
    });

    addProperty(_channel);
    addProperty(_transferFunction);
    addProperty(_raycasting);
    addProperty(_camera);
    addProperty(_lighting);
    addProperty(_positionIndicator);
    addProperty(_toggleShading);
    addProperty(_colorById);
    addProperty(_filterById);
    addProperty(_id);
    addProperty(_performFeatureLookup);
}

const ProcessorInfo SegmentationIdRaycaster::getProcessorInfo() const {
    return processorInfo_;
}

void SegmentationIdRaycaster::initializeResources() {
    utilgl::addShaderDefines(_shader, _raycasting);
    utilgl::addShaderDefines(_shader, _camera);
    utilgl::addShaderDefines(_shader, _lighting);
    utilgl::addShaderDefines(_shader, _positionIndicator);
    utilgl::addShaderDefinesBGPort(_shader, _backgroundPort);
    _shader.build();
}

//void SegmentationIdRaycaster::onVolumeChange() {
//    if (_volumePort.hasData()) {
//        size_t channels = _volumePort.getData()->getDataFormat()->getComponents();
//
//        if (channels == _channel.size()) return;
//
//        std::vector<OptionPropertyIntOption> channelOptions;
//        for (size_t i = 0; i < channels; i++) {
//            channelOptions.emplace_back("Channel " + toString(i+1), "Channel " + toString(i+1),
//                                        static_cast<int>(i));
//        }
//        _channel.replaceOptions(channelOptions);
//        _channel.setCurrentStateAsDefault();
//    }
//}

void SegmentationIdRaycaster::process() {
    if (_volumePort.isChanged()) {
        auto newVolume = _volumePort.getData();

        if (newVolume->hasRepresentation<VolumeGL>()) {
            _loadedVolume= newVolume;
        } else {
            dispatchPool([this, newVolume]() {
                RenderContext::getPtr()->activateLocalRenderContext();
                newVolume->getRep<kind::GL>();
                glFinish();
                dispatchFront([this, newVolume]() {
                    _loadedVolume= newVolume;
                    invalidate(InvalidationLevel::InvalidOutput);
                });
            });
        }
    }

    if (_segmentationPort.isChanged()) {
        auto newVolume = _segmentationPort.getData();

        if (newVolume->hasRepresentation<VolumeGL>()) {
            _loadedSegmentationVolume = newVolume;
        }
        else {
            dispatchPool([this, newVolume]() {
                RenderContext::getPtr()->activateLocalRenderContext();
                newVolume->getRep<kind::GL>();
                glFinish();
                dispatchFront([this, newVolume]() {
                    _loadedSegmentationVolume = newVolume;
                    invalidate(InvalidationLevel::InvalidOutput);
                });
            });
        }
    }

    const auto& contourInformation = _contour.getData();
    _id.setMaxValue(contourInformation->nFeatures - 1);

    if (!_loadedVolume) return;
    if (!_loadedVolume->hasRepresentation<VolumeGL>()) {
        LogWarn("No GL rep !!!");
        return;
    }

    if (!_loadedSegmentationVolume) return;
    if (!_loadedSegmentationVolume->hasRepresentation<VolumeGL>()) {
        LogWarn("No GL rep segmentation !!!");
        return;
    }

    utilgl::activateAndClearTarget(_outport);
    _shader.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(_shader, units, *_loadedVolume, "volume");
    utilgl::bindAndSetUniforms(_shader, units, *_loadedSegmentationVolume, "segmentationVolume");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, contourInformation->ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, contourInformation->ssbo);

    utilgl::bindAndSetUniforms(_shader, units, _transferFunction);
    utilgl::bindAndSetUniforms(_shader, units, _entryPort, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(_shader, units, _exitPort, ImageType::ColorDepth);

    if(_backgroundPort.isConnected()){
        utilgl::bindAndSetUniforms(_shader, units, _backgroundPort, ImageType::ColorDepthPicking);
    }
    utilgl::setUniforms(_shader, _outport, _camera, _lighting, _raycasting, _positionIndicator,
        _channel, _id, _filterById, _colorById, _performFeatureLookup);

    utilgl::singleDrawImagePlaneRect();

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

void SegmentationIdRaycaster::toggleShading(Event*) {
    if (_lighting.shadingMode_.get() == ShadingMode::None) {
        _lighting.shadingMode_.set(ShadingMode::Phong);
    } else {
        _lighting.shadingMode_.set(ShadingMode::None);
    }
}

// override to do member renaming.
void SegmentationIdRaycaster::deserialize(Deserializer& d) {
    util::renamePort(d, {{&_entryPort, "entry-points"}, {&_exitPort, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace
