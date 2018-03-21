#ifndef __AB_STRAIGHTENER_H__
#define __AB_STRAIGHTENER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>

#include <modules/segmentangling/util/constraintsstate.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <mutex>
#include <thread>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API Straightener : public Processor {
public:
    Straightener();
    virtual ~Straightener();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    
private:
    void slimThread();
    void eventUpdateMousePos(Event* e);
    void eventUpdateKeyboard(Event* e);
    void eventUpdateKeyboardReset(Event* e);
    void diffusionDistances();
    void updateConstraints();

    std::shared_ptr<BasicMesh> createMesh(const Eigen::MatrixXd& TV);

    enum class SelectionState {
        None = 0,
        Front,
        Back
    };

    SelectionState _currentSelectionState = SelectionState::Front;


    VolumeInport _inport;
    MeshOutport _meshOutport;
    MeshOutport _frontSelectionMesh;
    MeshOutport _backSelectionMesh;
    ImageOutport _imageOutport;
    //VolumeOutport _outport;

    BoolProperty _debugOnlyEndAndTets;
    CameraProperty _camera;

    StringProperty _filename;
    IntProperty _nBones;
    ButtonProperty _reload;
    bool _filenameDirty = false;
    
    EventProperty _mousePositionTracker;
    
    EventProperty _keyboardPressSelect;
    EventProperty _keyboardPressReset;


    Eigen::MatrixXd _TVOriginal;
    Eigen::MatrixXd _TV;
    Eigen::MatrixXi _TF;
    Eigen::MatrixXi _TT;
    Eigen::MatrixXd _TFn;
    Eigen::MatrixXd _texCoords;

    Eigen::VectorXd _isoValues;

    bool _isSlimRunning;
    std::thread _slimThread;
    bool _isConstraintsChanged;
    std::mutex _constraintsLock;
    std::mutex _drawStateLock;

    int _currentVertexId;

    int _currentFrontVertexId = -1;
    int _currentBackVertexId = -1;


    ConstraintState _constraintState;

    std::shared_ptr<BasicMesh> _outputSurfaceMesh;
};

} // namespace

#endif // __AB_STRAIGHTENER_H__
