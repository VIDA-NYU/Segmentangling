#ifndef __AB_STRAIGHTENER_H__
#define __AB_STRAIGHTENER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>

//#include <modules/segmentangling/util/constraintsstate.h>
#include <deformation_constraints.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <modules/segmentangling/common.h>

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
    //void eventPreviousLevelset();
    //void eventNextLevelset();
    //void diffusionDistances();
    //void updateConstraints();

    void doSlimSetupStuff();

    void clearInputParameters();

    bool isReadyToComputeDiffusion() const;

    std::shared_ptr<BasicMesh> createOutputSurfaceMesh(const Eigen::MatrixXd& TV);
    std::shared_ptr<BasicMesh> createOuterSurfaceMesh(const Eigen::MatrixXd& TV);
    std::shared_ptr<BasicMesh> createMeshFromTriangles();



    //
    // Handling the state
    //
    enum class SelectionState {
        None = 0,
        Front,
        Back
    };
    std::string selectionStateToString(SelectionState s);
    void updateSelectionStateString();
    StringProperty _selectionStateString;



    //
    // Input states
    //
    struct InputParams {
        SelectionState state = SelectionState::Front;

        int frontVertexId = -1;
        int backVertexId = -1;

        float frontAngle = 0.f;
        float backAngle = 0.f;
        //std::vector<float> levelSetOrientations;

        //std::vector<float>::iterator currentLevelset = levelSetOrientations.end();
    };
    std::vector<std::array<int, 2>> pointsFromPoints() const;
    bool isInputParamEmpty(const InputParams& i) const;

    std::vector<InputParams> _inputParameters;
    std::vector<InputParams>::iterator _currentInputParameter;

    void updateInputParameterString();
    StringProperty _inputParameterSelection;


    void updateDiffusionReadyString();
    StringProperty _diffusionReadyString;

    void updateStatusString();
    StringProperty _statusString;
    FloatVec4Property _statusColor;

    void updateInstructionsString();
    StringProperty _instructionsString;


    //
    // Ports
    //
    VolumeInport _inport;
    VolumeOutport _outport;

    VertexInport _trianglesVertexInport;
    TetIndexInport _trianglesTetIndexInport;

    VertexInport _skeletonTetraVertexInport;
    TetIndexInport _skeletonTetraTetIndexInport;

    VertexInport _outerTetraVertexInport;
    TetIndexInport _outerTetraTetIndexInport;


    MeshOutport _meshOutport;
    std::shared_ptr<BasicMesh> _outputSurfaceMesh;
    std::atomic_bool _isOutputMeshDirty = false;

    std::shared_ptr<BasicMesh> _frontSelectionMesh;
    MeshOutport _frontSelectionMeshPort;
    std::shared_ptr<BasicMesh> _backSelectionMesh;
    MeshOutport _backSelectionMeshPort;

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
    //StringProperty _filename;
    //bool _filenameDirty = false;
    //ButtonProperty _reload;

    
    // Output 
    IntVec2Property _windowSize;
    ButtonProperty _rasterizeVolume;
    //struct {
    //    FloatVec3Property normal;
    //    FloatVec3Property position;
    //} _levelsetPlane;


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
    //EventProperty _eventPreviousLevelSet;
    //EventProperty _eventNextLevelSet;

    const Eigen::MatrixXd& currentTV() const;
    const Eigen::MatrixXi& currentTF() const;

    //
    // SLIM related members
    //
    struct {
        Eigen::MatrixXd TVOriginal;
        Eigen::MatrixXd TV;
        Eigen::MatrixXi TF;
        Eigen::MatrixXi TT;
        Eigen::MatrixXd TFn;
        
        Eigen::VectorXi components;
    } _skeletonTetra;

    struct {
        Eigen::MatrixXd TV;
        Eigen::MatrixXi TF;
        Eigen::MatrixXd TFn;
        Eigen::MatrixXi TT;
    } _outerTetra;

    //
    // Surface mesh related members
    // 
    struct {
        Eigen::MatrixXd TV;
        Eigen::MatrixXi TF;
        Eigen::MatrixXd TFn;

        Eigen::VectorXi components;
    } _triangle;

    bool hasTetraMeshes() const;

    //enum class MeshType {
    //    None = 0,
    //    Tetra,
    //    Triangle
    //};
    //MeshType _currentMeshType = MeshType::None;

    Eigen::VectorXd _isoValues;

    Eigen::MatrixXd _slimDataOutput;
    std::mutex _slimDataMutex;

    bool _isSlimRunning;
    std::thread _slimThread;
    bool _isConstraintsChanged = false;
    std::mutex _meshLock;
    DeformationConstraints _deformationConstraints;
    std::atomic_bool _isDeformationConstraintsReady = false;
    std::mutex _constraintsLock;

    
    //
    // Across-frames members
    //
    int _currentHoverVertexId;
    bool _userRequestedSlim = false;
    bool _constraintValidationFailed = false;
    bool _selectedInvalidConstraint = false;
    bool _stopInteraction = false;
    


    //
    // Internal members
    //
    bool _isFirstFrame = true;
    BoolProperty _isDebugging;
};

} // namespace

#endif // __AB_STRAIGHTENER_H__
