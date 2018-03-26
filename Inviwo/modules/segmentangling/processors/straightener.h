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
    void eventStartDiffusion();
    void eventSelectPoint();
    void eventReset();
    void eventPreviousParameter();
    void eventNextParameter();
    void eventPreviousLevelset();
    void eventNextLevelset();
    void diffusionDistances();
    void updateConstraints();

    bool isReadyToComputeDiffusion() const;

    std::shared_ptr<BasicMesh> createOutputSurfaceMesh(const Eigen::MatrixXd& TV);



    //
    // Handling the state
    //
    enum class SelectionState {
        None = 0,
        Front,
        Back
    };

    SelectionState _currentSelectionState = SelectionState::Front;

    std::string selectionStateToString(SelectionState s);
    void updateSelectionStateString();
    StringProperty _selectionStateString;



    //
    // Input states
    //
    struct InputParams {
        int frontVertexId = -1;
        int backVertexId = -1;
        std::vector<float> levelSetOrientations;

        std::vector<float>::iterator currentLevelset = levelSetOrientations.end();
    };

    bool isInputParamEmpty(const InputParams& i) const;

    std::vector<InputParams> _inputParameters;
    std::vector<InputParams>::iterator _currentInputParameter;

    void updateInputParameterString();
    StringProperty _inputParameterSelection;


    void updateDiffusionReadyString();
    StringProperty _diffusionReadyString;


    //
    // Ports
    //
    VolumeInport _inport;
    MeshOutport _meshOutport;
    std::shared_ptr<BasicMesh> _outputSurfaceMesh;
    std::atomic_bool _isOutputMeshDirty = false;
    MeshOutport _frontSelectionMesh;
    MeshOutport _backSelectionMesh;
    struct {
        MeshOutport isoValues;
        std::shared_ptr<BasicMesh> isoMesh;

        MeshOutport skeletonVertices;
        std::shared_ptr<BasicMesh> skeletonMesh;
    } _debug;
    //ImageOutport _imageOutport;



    //
    // Properties
    //

    // Input
    StringProperty _filename;
    bool _filenameDirty = false;
    ButtonProperty _reload;

    
    // Output 
    IntVec2Property _windowSize;
    struct {
        FloatVec3Property normal;
        FloatVec3Property position;
    } _levelsetPlane;


    // Internal
    CameraProperty _camera;


    // Testing
    void createDebugMeshes();

    BoolProperty _debugOnlyEndAndTets;
    IntProperty _nBones;
    FloatProperty _sphereRadius;
    


    //
    // Mouse events
    //
    EventProperty _eventPositionUpdate;



    //
    // Keyboard events
    //
    EventProperty _eventStartDiffusion;
    EventProperty _eventSelectPoint;
    EventProperty _eventReset;
    EventProperty _eventPreviousInputParameter;
    EventProperty _eventNextInputParameter;
    EventProperty _eventPreviousLevelSet;
    EventProperty _eventNextLevelSet;

    //
    // SLIM related members
    //
    Eigen::MatrixXd _TVOriginal;
    Eigen::MatrixXd _TV;
    Eigen::MatrixXi _TF;
    Eigen::MatrixXi _TT;
    Eigen::MatrixXd _TFn;
    Eigen::MatrixXd _texCoords;

    Eigen::VectorXd _isoValues;

    Eigen::MatrixXd _slimDataOutput;
    std::mutex _slimDataMutex;

    bool _isSlimRunning;
    std::thread _slimThread;
    bool _isConstraintsChanged;
    std::mutex _meshLock;
    ConstraintState _constraintState;
    std::mutex _constraintsLock;

    
    //
    // Across-frames members
    //
    int _currentHoverVertexId;


    //
    // Internal members
    //
    bool _isFirstFrame = true;
    BoolProperty _isDebugging;
};

} // namespace

#endif // __AB_STRAIGHTENER_H__
