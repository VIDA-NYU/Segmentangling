#include "volumecollectiongenerator.h"

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>

namespace {
    static const int ModificationAdd = 0;
    static const int ModificationRemove = 1;
} // namespace

namespace inviwo {

void VolumeCollectionGenerator::logStatus(std::string msg) const {
    LogInfo("Status");
    LogInfo(msg);
    LogInfo("_dirty.mapping: " << _dirty.mapping << "\t" << 
        "_dirty.removeVolume: " << _dirty.removeVolume << "\t" <<
        "_currentVolume: " << _currentVolume << "\t" <<
        "_featureToModify: " << _featureToModify << "\t" <<
        "_nVolumes: " << _nVolumes << "\t" <<
        "_modification: " << _modification.get()
    );
}

const ProcessorInfo VolumeCollectionGenerator::processorInfo_{
    "bock.volumecollectiongenerator",  // Class identifier
    "Volume Collection Generator",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

VolumeCollectionGenerator::VolumeCollectionGenerator()
    : Processor()
    , _inport("volumeinport")
    , _inportFeatureMapping("inportfeaturemapping")
    , _outportContour("outportcontour")
    , _currentVolume("currentVolume", "Current volume", 0, 0, 254)
    , _addVolume("addVolume", "Add Volume")
    , _removeVolume("removeVolume", "Remove Volume")
    , _modification("modification", "Modification")
    , _featureToModify("featureToModify", "Feature ID to Modify", 0, 0, 1000)
    , _modify("modify", "Modify")
    , _nVolumes("nVolumes", "Number of Volumes", 15, 1, 1000)
    , _selectVolume1Event("_selectVolume1Event", "Select Volume 1", [this](Event* e) { selectVolume(e, 0); }, IvwKey::Num1, KeyState::Press)
    , _selectVolume2Event("_selectVolume2Event", "Select Volume 2", [this](Event* e) { selectVolume(e, 1); }, IvwKey::Num2, KeyState::Press)
    , _selectVolume3Event("_selectVolume3Event", "Select Volume 3", [this](Event* e) { selectVolume(e, 2); }, IvwKey::Num3, KeyState::Press)
    , _selectVolume4Event("_selectVolume4Event", "Select Volume 4", [this](Event* e) { selectVolume(e, 3); }, IvwKey::Num4, KeyState::Press)
    , _selectVolume5Event("_selectVolume5Event", "Select Volume 5", [this](Event* e) { selectVolume(e, 4); }, IvwKey::Num5, KeyState::Press)
    , _selectVolume6Event("_selectVolume6Event", "Select Volume 6", [this](Event* e) { selectVolume(e, 5); }, IvwKey::Num6, KeyState::Press)
    , _selectVolume7Event("_selectVolume7Event", "Select Volume 7", [this](Event* e) { selectVolume(e, 6); }, IvwKey::Num7, KeyState::Press)
    , _selectVolume8Event("_selectVolume8Event", "Select Volume 8", [this](Event* e) { selectVolume(e, 7); }, IvwKey::Num8, KeyState::Press)
    , _selectVolume9Event("_selectVolume9Event", "Select Volume 9", [this](Event* e) { selectVolume(e, 8); }, IvwKey::Num9, KeyState::Press)
    , _selectVolume10Event("_selectVolume10Event", "Select Volume 10", [this](Event* e) { selectVolume(e, 9); }, IvwKey::Num0, KeyState::Press)
    , _addVolumeEvent("_addVolumeEvent", "Add Volume", [this](Event* e) { addVolumeModification(e); }, IvwKey::F1, KeyState::Press)
    , _removeVolumeEvent("_removeVolumeEvent", "Remove Volume", [this](Event* e) { removeVolumeModification(e); }, IvwKey::F2, KeyState::Press)
    , _trigger("_trigger", "Trigger", [this](Event* e) { _modify.pressButton(); e->markAsUsed(); }, IvwKey::Space, KeyState::Press)
    , _clearAllVolumes("_clearAllVolumes", "Clear all volumes")
    , _slice1Position("_slice1Position", "Slice 1 Position")
    , _slice2Position("_slice2Position", "Slice 2 Position")
    , _slice3Position("_slice3Position", "Slice 3 Position")
    , _lastChangedSlicePosition(_slice1Position)
    , _information(nullptr)
    , _dirty{ false, false, false }
{
    //addPort(_inportIdentifiers);
    addPort(_inport);
    addPort(_inportFeatureMapping);
    addPort(_outportContour);

    //this->dataFormat_ = DataUInt8::get();

    addProperty(_selectVolume1Event);
    addProperty(_selectVolume2Event);
    addProperty(_selectVolume3Event);
    addProperty(_selectVolume4Event);
    addProperty(_selectVolume5Event);
    addProperty(_selectVolume6Event);
    addProperty(_selectVolume7Event);
    addProperty(_selectVolume8Event);
    addProperty(_selectVolume9Event);
    addProperty(_selectVolume10Event);

    addProperty(_addVolumeEvent);
    addProperty(_removeVolumeEvent);

    addProperty(_trigger);

    addProperty(_currentVolume);

    _clearAllVolumes.onChange([this]() { _dirty.clearAllVolumes = true; });
    addProperty(_clearAllVolumes);

    _nVolumes.onChange([this]() {
        _currentVolume.setMaxValue(_nVolumes - 1);
    });
    addProperty(_nVolumes);
    _currentVolume.setMaxValue(_nVolumes - 1);
    //_addVolume.onChange([this]() {
    //    logStatus("beg");
    //    _currentVolume.setMaxValue(_nVolumes - 1);
    //    _nVolumes = _nVolumes + 1;
    //    _currentVolume = _nVolumes.get();
    //    logStatus("end");
    //});
    //addProperty(_addVolume);
    //_removeVolume.onChange([this]() {
    //    logStatus("beg");
    //    _currentVolume = _currentVolume - 1;
    //    _nVolumes = _nVolumes - 1;
    //    _currentVolume.setMaxValue(_nVolumes - 1);
    //    _dirty.removeVolume = true;
    //    //_dirty.mapping = true;
    //    logStatus("end");
    //});
    //addProperty(_removeVolume);
    
    // @FRAGILE:  Sync this with the shader
    _modification.addOption("a", "Add", ModificationAdd);
    _modification.addOption("r", "Remove", ModificationRemove);
    addProperty(_modification);
    addProperty(_featureToModify);
    _modify.onChange([&]() { _dirty.mapping = true; });
    addProperty(_modify);

    _slice1Position.onChange([this]() { _lastChangedSlicePosition = _slice1Position; });
    addProperty(_slice1Position);
    _slice2Position.onChange([this]() { _lastChangedSlicePosition = _slice2Position; });
    addProperty(_slice2Position);
    _slice3Position.onChange([this]() { _lastChangedSlicePosition = _slice3Position; });
    addProperty(_slice3Position);
}

const ProcessorInfo VolumeCollectionGenerator::getProcessorInfo() const {
    return processorInfo_;
}
#pragma optimize("", off)

void VolumeCollectionGenerator::process() {
    if (!_information) {
        _information = std::make_shared<ContourInformation>();

        glGenBuffers(1, &(_information->ssbo));
    }

    if (_dirty.removeVolume) {
        for (uint32_t& m : _mappingData) {
            if (m == _currentVolume) {
                m = uint32_t(-1);
            }
        }
        //_dirty.mapping = true;
        _dirty.removeVolume = false;
    }

    if (_dirty.clearAllVolumes) {
        for (uint32_t& m : _mappingData) {
            m = uint32_t(-1);
        }
        //_dirty.mapping = true;
        _dirty.clearAllVolumes = false;
    }

    if (_mappingData.empty() || _inport.isChanged()) {
        glm::size3_t dim = _inport.getData()->getDimensions();
        _mappingData = std::vector<uint32_t>(dim.x * dim.y * dim.z + 1, uint32_t(-1));
    }
    _mappingData[0] = _nVolumes + 1;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _information->ssbo);
    if (_dirty.mapping) {
        std::vector<Feature> m = *_inportFeatureMapping.getData();
        uint32_t feature = -1;

        bool featureFound = false;
        {
            glm::vec3 p = _lastChangedSlicePosition.get();
            glm::size3_t dim = _inport.getData()->getDimensions();

            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(_inport.getData()->getRepresentation<VolumeRAM>()->getData());

            glm::size3_t ip = {
                size_t(p.x * dim.x),
                size_t(p.y * dim.y),
                size_t(p.z * dim.z)
            };
            glm::u64 idx = VolumeRAM::posToIndex(ip, dim);
            uint32_t index = ptr[idx]; // arcs index

            for (size_t i = 0; i < m.size(); ++i) {
                const Feature& f = m[i];
                auto it = std::find(
                    f.begin(),
                    f.end(),
                    index
                );

                if (it != f.end()) {
                    feature = i;
                    featureFound = true;
                    break;
                }
            }
        }

        LogInfo("Feature found: " << featureFound << "{ " << feature << " }");
        if (featureFound) {
            _featureToModify.setMaxValue(m.size() - 1);

            //std::vector<uint32_t> idx = m[_featureToModify];
            std::vector<uint32_t> idx = m[feature];

            // If we are adding, we want to set the current volume into it, otherwise we replace it with NoVolume := 0
            for (uint32_t i : idx) {
                if (_modification.get() == ModificationAdd) {
                    _mappingData[i + 1] = static_cast<uint32_t>(_currentVolume.get());
                }
                else {
                    _mappingData[i + 1] = uint32_t(-1);
                }
            }

            glBufferData(
                GL_SHADER_STORAGE_BUFFER,
                sizeof(uint32_t) * (_mappingData.size() + 1),
                _mappingData.data(),
                GL_DYNAMIC_COPY
            );
        }
        _dirty.mapping = false;
    }
    glBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        0,
        sizeof(uint32_t),
        _mappingData.data()
    );

    _information->nFeatures = _nVolumes + 1;

    _outportContour.setData(_information);
}
#pragma optimize("", on)

void VolumeCollectionGenerator::selectVolume(Event* e, int volume) {
    _currentVolume = volume;
    e->markAsUsed();
}

void VolumeCollectionGenerator::addVolumeModification(Event* e) {
    _modification.setSelectedValue(ModificationAdd);
    e->markAsUsed();
}

void VolumeCollectionGenerator::removeVolumeModification(Event* e) {
    _modification.setSelectedValue(ModificationRemove);
    e->markAsUsed();
}

}  // namespace
