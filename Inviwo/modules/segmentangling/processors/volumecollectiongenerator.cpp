#include "volumecollectiongenerator.h"

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>

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
    : VolumeGLProcessor("volumecollectiongenerator.frag")
    , _inportIdentifiers("inportidentifiers")
    , _inportFeatureMapping("inportfeaturemapping")
    , _currentVolume("currentVolume", "Current volume", 0, 0, 254)
    , _addVolume("addVolume", "Add Volume")
    , _removeVolume("removeVolume", "Remove Volume")
    , _modification("modification", "Modification")
    , _featureToModify("featureToModify", "Feature ID to Modify", 0, 0, 1000)
    , _modify("modify", "Modify")
    , _nVolumes("nVolumes", "Number of Volumes", 1, 0, 1000)
    , _ssbo(0)
    , _dirty{ false, false }
{
    addPort(_inportIdentifiers);
    addPort(_inportFeatureMapping);

    this->dataFormat_ = DataUInt8::get();

    addProperty(_currentVolume);

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
}

const ProcessorInfo VolumeCollectionGenerator::getProcessorInfo() const {
    return processorInfo_;
}

void VolumeCollectionGenerator::preProcess(TextureUnitContainer& cont) {
    //logStatus("beg preProcess");
    if (_ssbo == 0) {
        glGenBuffers(1, &_ssbo);
    }

    if (_dirty.removeVolume) {
        for (uint32_t& m : _mappingData) {
            if (m == _currentVolume + 1) {
                m = 0;
            }
        }

        _dirty.removeVolume = false;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);
    if (_dirty.mapping) {
        if (inport_.isChanged() || _mappingData.empty()) {
            // The first port is just a dummy port, so if that changes, we need to recreate the data
            glm::size3_t dim = inport_.getData()->getDimensions();
            _mappingData = std::vector<uint32_t>(dim.x * dim.y * dim.z, 0);
        }

        std::vector<Feature> m = *_inportFeatureMapping.getData();
        _featureToModify.setMaxValue(m.size() - 1);

        std::vector<uint32_t> idx = m[_featureToModify];

        // If we are adding, we want to set the current volume into it, otherwise we replace it with NoVolume := 0
        for (uint32_t i : idx) {
            if (_modification.get() == ModificationAdd) {
                if (_mappingData[i] != 0) {
                    LogInfo("ASDASD");
                }
                _mappingData[i] = static_cast<uint32_t>(_currentVolume.get() + 1);
            }
            else {
                _mappingData[i] = 0;
            }
        }

        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            sizeof(uint32_t) * _mappingData.size(),
            _mappingData.data(),
            GL_DYNAMIC_COPY
        );
        _dirty.mapping = false;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssbo);

    utilgl::bindAndSetUniforms(shader_, cont, *_inportIdentifiers.getData(), "volumeIdentifiers");
    utilgl::setUniforms(shader_, _currentVolume, _featureToModify, _modification);

    GLuint tex = volume_->getRepresentation<VolumeGL>()->getTexture()->getID();
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //logStatus("end preProcess");
}

void VolumeCollectionGenerator::postProcess() {
    //volume_->dataMap_.dataRange = dvec2(0, _nVolumes + 1);
    //volume_->dataMap_.valueRange = dvec2(0, _nVolumes + 1);
    volume_->dataMap_.dataRange = dvec2(0, _nVolumes);
    volume_->dataMap_.valueRange = dvec2(0, _nVolumes);
}

}  // namespace
