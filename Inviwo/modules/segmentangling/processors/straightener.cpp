#pragma optimize ("", off)

#include "straightener.h"

#include <yixin_loader.h>
#include <Eigen/Core>
#include <igl/slim.h>
#include <igl/per_vertex_normals.h>
#include <igl/unproject_onto_mesh.h>
#include <igl/harmonic.h>
#include <igl/grad.h>
#include <igl/colormap.h>
#include <igl/components.h>

#include <glm/gtx/component_wise.hpp>
#include <modules/base/algorithm/meshutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <utils/utils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <modules/segmentangling/util/defer.h>


namespace {

glm::vec3 eigenToGLM(const Eigen::RowVector3d& rv) {
    return glm::vec3(rv[0], rv[1], rv[2]);
}

glm::vec4 eigenToGLMColor(const Eigen::RowVector3d& rv) {
    return glm::vec4(rv[0], rv[1], rv[2], 1.f);
}


//template <typename T>
//glm::tvec3<T::Scalar> eigenToGLM(const Eigen::PlainObjectBase<T>& rv) {
//    return glm::tvec3<T::Scalar>(
//        rv(0, 0),
//    );
//}

} // namespace

namespace inviwo {

int iterCount = 0;

std::string Straightener::selectionStateToString(SelectionState s) {
    switch (s) {
        case SelectionState::None:
            return "Segment finished";
        case SelectionState::Front:
            return "Select front";
        case SelectionState::Back:
            return "Select back";
        default:
            return "HUH!?";
    }
}

bool Straightener::isInputParamEmpty(const InputParams& i) const {
    return (i.frontVertexId == -1) && (i.backVertexId == -1);
}



void Straightener::slimThread() {
    using namespace std;
    using namespace Eigen;

    //m_ui_state.m_avg_draw_state_update_time = 0.0;
    //m_ui_state.m_avg_slim_time = 0.0;

    igl::SLIMData sData;

    cout << "INFO: SLIM Thread: Starting SLIM background thread." << endl;
    while (_isSlimRunning) {

        _constraintsLock.lock();
        if (_isConstraintsChanged) {
            LogInfo("Yey");
            VectorXi slim_b;
            MatrixXd slim_bc;
            _deformationConstraints.slim_constraints(slim_b, slim_bc, _debugOnlyEndAndTets);

            const double soft_const_p = 1e5;
            const MatrixXd TV_0 = _outerTetra.TV;
            sData.exp_factor = 5.0;
            slim_precompute(
                _outerTetra.TV,
                _outerTetra.TT,
                TV_0,
                sData,
                igl::SLIMData::EXP_CONFORMAL,
                slim_b,
                slim_bc,
                soft_const_p
            );

            _isDeformationConstraintsReady = true;
            _isConstraintsChanged = false;
        }
        _constraintsLock.unlock();

        if (!_isDeformationConstraintsReady) {
            this_thread::yield();
            continue;
        }
        _stopInteraction = true;

        LogInfo("Iteration: " << iterCount);
        ++iterCount;

        igl::slim_solve(sData, 1);

        _slimDataMutex.lock();
        //@TODO(abock): try me
        //_slimDataOutput = std::move(sData.V_o);
        _slimDataOutput = sData.V_o;
        _slimDataMutex.unlock();

        _isOutputMeshDirty = true;


        //auto slim_end_time = chrono::high_resolution_clock::now();
        //m_ui_state.update_slim_ema(chrono::duration<double, milli>(slim_end_time - slim_start_time).count());

        //auto ds_update_start_time = chrono::high_resolution_clock::now();

        //int buffer = (m_current_buf + 1) % 2;
        //DrawState& ds = m_ds[buffer];
        //ds.update_tet_mesh(sData.V_o, TT);
        //ds.update_isovalue(isovals, m_constraints.m_level_set_isovalues[m_ui_state.m_current_level_set]);
        //ds.update_skeleton(isovals, m_ui_state.m_num_skel_verts);
        //ds.update_constraint_points(sData.b, sData.bc);

        //auto ds_update_end_time = chrono::high_resolution_clock::now();
        //m_ui_state.update_draw_state_ema(chrono::duration<double, milli>(ds_update_end_time - ds_update_start_time).count());

        //m_double_buf_lock.lock();
        //m_current_buf = buffer;
        //m_draw_state_changed = true;
        //m_double_buf_lock.unlock();
    }
}

const ProcessorInfo Straightener::processorInfo_{
    "bock.straightener",  // Class identifier
    "Tetmesh Straightener",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};

Straightener::Straightener()
    : Processor()
    , _inport("volumeInport")
    , _trianglesVertexInport("_trianglesVertexInport")
    , _trianglesTetIndexInport("_trianglesTetIndexInport")
    , _skeletonTetraVertexInport("_skeletonTetraVertexInport")
    , _skeletonTetraTetIndexInport("_skeletonTetraTetIndexInport")
    , _outerTetraVertexInport("_outerTetraVertexInport")
    , _outerTetraTetIndexInport("_outerTetraTetIndexInport")
    , _meshOutport("meshOutport")
    //, _imageOutport("imageOutput")
    , _frontSelectionMesh("frontSelectionMesh")
    , _backSelectionMesh("backSelectionMesh")
    , _debug {
        MeshOutport("debug_isoValues"),
        meshutil::sphere({ 0.f, 0.f, 0.f }, 0.f, glm::vec4(0.f)),
        MeshOutport("debug_skeletonVertices"),
        meshutil::sphere({ 0.f, 0.f, 0.f }, 0.f, glm::vec4(0.f))
    }
    //, _debugMeshOutput("debugOutput")
    , _debugOnlyEndAndTets("onlyEndsAndTets", "_debugOnlyEndAndTets", false)
    , _selectionStateString("selectionState", "SelectionState")
    , _nBones("_nBones", "Number of Bones", 25, 1, 100)
    , _camera("camera", "Camera")
    //, _filename("filename", "Mesh File")
    , _sphereRadius("_sphereRadius", "Sphere Radius", 0.00075f, 0.f, 0.015f)
    //, _reload("reload", "Reload")
    , _windowSize("_windowSize", "Window Size")
    //, _levelsetPlane{
    //    FloatVec3Property("levelset_normal", "Normal"),
    //    FloatVec3Property("levelset_position", "Position"),
    //}
    , _eventPositionUpdate("_eventPositionUpdate", "Mouse Position Tracker",
        [this](Event* e) { eventUpdateMousePos(e); },
        MouseButton::None, MouseState::Move
    )
    , _eventStartDiffusion("_eventStartDiffusion", "Start Diffusion",
        [this](Event*) { eventStartDiffusion(); },
        IvwKey::Space
    )
    , _eventSelectPoint("_eventSelectPoint", "keyboardPress",
        [this](Event*) { eventSelectPoint(); },
        IvwKey::Tab
    )
    , _eventReset("_eventReset", "keyboardPressReset",
        [this](Event*) { eventReset(); },
        IvwKey::F1
    )
    , _eventPreviousInputParameter("_eventPreviousInputParameter", "Previous input parameter",
        [this](Event*) { eventPreviousParameter(); },
        IvwKey::A
    )
    , _eventNextInputParameter("_eventNextInputParameter", "Next input parameter",
        [this](Event*) { eventNextParameter(); },
        IvwKey::D
    )
    //, _eventPreviousLevelSet("_eventPreviousLevelSet", "Previous level set",
    //    [this](Event*) { eventPreviousLevelset(); },
    //    IvwKey::W
    //)
    //, _eventNextLevelSet("_eventNextLevelSet", "Previous level set",
    //    [this](Event*) { eventNextLevelset(); },
    //    IvwKey::S
    //)
    , _inputParameters(1)
    , _currentInputParameter(_inputParameters.begin())
    , _inputParameterSelection("_inputParameterSelection", "Input parameter Selection")
    , _isDebugging("_isDebugging", "Debugging")
    , _diffusionReadyString("_diffusionReadyString", "Diffusion Ready")
    , _statusString("_statusString", "Status string")
    , _statusColor("_statusColor", "Status color")
    , _instructionsString("_instructionsString", "Instructions string")
{
    // Ports
    addPort(_inport);
    addPort(_trianglesVertexInport);
    addPort(_trianglesTetIndexInport);
    addPort(_skeletonTetraVertexInport);
    addPort(_skeletonTetraTetIndexInport);
    addPort(_outerTetraVertexInport);
    addPort(_outerTetraTetIndexInport);
    addPort(_meshOutport);
    //addPort(_imageOutport);
    addPort(_frontSelectionMesh);
    addPort(_backSelectionMesh);
    addPort(_debug.isoValues);
    addPort(_debug.skeletonVertices);

    //// Useful properties
    //addProperty(_levelsetPlane.normal);
    //addProperty(_levelsetPlane.position);

    // Internal properties
    _windowSize.setReadOnly(true);
    addProperty(_windowSize);

    //addProperty(_filename);
    //_filename.onChange([this]() {
    //    _filenameDirty = true;
    //});

    //addProperty(_reload);
    //_reload.onChange([this]() {
    //    _filenameDirty = true;
    //});

    addProperty(_isDebugging);
    _sphereRadius.onChange([this]() { createDebugMeshes(); });
    addProperty(_sphereRadius);


    addProperty(_camera);
    addProperty(_nBones);

    addProperty(_debugOnlyEndAndTets);

    // Events
    addProperty(_eventPositionUpdate);
    addProperty(_eventStartDiffusion);
    addProperty(_eventSelectPoint);
    addProperty(_eventReset);
    addProperty(_eventPreviousInputParameter);
    addProperty(_eventNextInputParameter);

    // Output properties
    addProperty(_selectionStateString);
    addProperty(_inputParameterSelection);
    addProperty(_diffusionReadyString);
    addProperty(_statusString);
    addProperty(_statusColor);
    addProperty(_instructionsString);


    // Rest of ctor
    _isSlimRunning = true;
    _slimThread = std::thread(&Straightener::slimThread, this);

    _frontSelectionMesh.setData(std::make_shared<BasicMesh>());
    _backSelectionMesh.setData(std::make_shared<BasicMesh>());
    //_debugMesh = meshutil::sphere({ 0.f, 0.f, 0.f }, 0.f, glm::vec4(0.f));

    updateSelectionStateString();
    updateInputParameterString();
    updateStatusString();
}

Straightener::~Straightener() {
    LogInfo("Stopping SLIM thread");
    _isSlimRunning = false;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    _slimThread.join();
}

void Straightener::process() {
    if (_isFirstFrame) {
        // Update the text that is stored in the workspace

        updateSelectionStateString();
        updateInputParameterString();
        updateDiffusionReadyString();
        updateStatusString();

        _isFirstFrame = false;
    }

    updateInstructionsString();

    if (_trianglesVertexInport.isChanged() || _trianglesTetIndexInport.isChanged()) {
        std::shared_ptr<const Eigen::MatrixXd> v = _trianglesVertexInport.getData();
        std::shared_ptr<const Eigen::MatrixXi> t = _trianglesTetIndexInport.getData();

        if (v->rows() > 0 && t->rows() > 0) {
            // We were passed a surface mesh consisting of triangles
            _triangle.TV = *v;
            _triangle.TF = *t;

            size3_t dim = _inport.getData()->getDimensions();
            size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;
            _triangle.TV /= maxDim;

            igl::per_vertex_normals(_triangle.TV, _triangle.TF, _triangle.TFn);

            _outputSurfaceMesh = createMeshFromTriangles();
            _meshOutport.setData(_outputSurfaceMesh);

            //_currentMeshType = MeshType::Triangle;
        }
    }

    if (_skeletonTetraVertexInport.isChanged() || _skeletonTetraTetIndexInport.isChanged()) {
        std::shared_ptr<const Eigen::MatrixXd> v = _skeletonTetraVertexInport.getData();
        std::shared_ptr<const Eigen::MatrixXi> t = _skeletonTetraTetIndexInport.getData();

        if (v->rows() > 0 || t->rows() > 0) {
            // We were passed a tetrahedral mesh
            _skeletonTetra.TVOriginal = *v;
            _skeletonTetra.TT = *t;

            std::vector<std::array<int, 4>> tris_sorted;
            std::vector<std::array<int, 4>> tris;

            tet_mesh_faces(_skeletonTetra.TT, _skeletonTetra.TF);

            _skeletonTetra.TV = _skeletonTetra.TVOriginal;
            //edge_endpoints(TV, TT, TEV1, TEV2);

            size3_t dim = _inport.getData()->getDimensions();
            size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;

            _skeletonTetra.TVOriginal /= maxDim;
            _skeletonTetra.TV /= maxDim;


            _skeletonTetra.texCoords = _skeletonTetra.TV - Eigen::MatrixXd::Ones(_skeletonTetra.TV.rows(), _skeletonTetra.TV.cols());

            igl::per_vertex_normals(_skeletonTetra.TV, _skeletonTetra.TF, _skeletonTetra.TFn);

            //_texCoords.resize(_TV.rows(), 3);
            //for (int i = 0; i < _TV.rows(); i++) {
            //    // Subtract 1 since we pad the grid with a zero cell all around
            //    _texCoords.row(i) = _TV.row(i) - f.m_bb_min - Eigen::RowVector3d::Ones();
            //}

            _meshLock.lock();
            _outputSurfaceMesh = createOutputSurfaceMesh(_skeletonTetra.TV);
            _meshOutport.setData(_outputSurfaceMesh);
            _meshLock.unlock();

            for (InputParams& i : _inputParameters) {
                i.frontVertexId = nearest_vertex(_skeletonTetra.TV, _triangle.TV.row(i.frontVertexId));
                i.backVertexId = nearest_vertex(_skeletonTetra.TV, _triangle.TV.row(i.backVertexId));
            }

            // We can start if the user has said so and we have an outer tetrahedral mesh
            if (_readyToSLIM && _outerTetra.TV.rows() > 0) {
                diffusionDistances();
                updateConstraints();
                createDebugMeshes();
            }
        }
    }

    if (_outerTetraVertexInport.isChanged() || _outerTetraTetIndexInport.isChanged()) {
        std::shared_ptr<const Eigen::MatrixXd> v = _outerTetraVertexInport.getData();
        std::shared_ptr<const Eigen::MatrixXi> t = _outerTetraTetIndexInport.getData();

        if (v->rows() > 0 || t->rows() > 0) {
            // We were passed a tetrahedral mesh
            _outerTetra.TV = *v;
            _outerTetra.TT = *t;

            tet_mesh_faces(_outerTetra.TT, _outerTetra.TF);

            size3_t dim = _inport.getData()->getDimensions();
            size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;
            _outerTetra.TV /= maxDim;

            igl::per_vertex_normals(_outerTetra.TV, _outerTetra.TF, _outerTetra.TFn);

            _outputSurfaceMesh = createOuterSurfaceMesh(_outerTetra.TV);
            _meshOutport.setData(_outputSurfaceMesh);


            // We are ready to start if the user has said so and we have a skeleton mesh
            if (_readyToSLIM && _skeletonTetra.TV.rows() > 0) {
                diffusionDistances();
                updateConstraints();
                createDebugMeshes();
            }
        }
    }

    updateStatusString();

    //_filenameDirty = false;

    if (_isOutputMeshDirty) {
        LogInfo("Update of outer mesh in progess");
        
        std::lock_guard<std::mutex>g(_slimDataMutex);
        _outputSurfaceMesh = createOuterSurfaceMesh(_slimDataOutput);
        _meshOutport.setData(_outputSurfaceMesh);
        
        invalidate(InvalidationLevel::InvalidOutput);
        _isOutputMeshDirty = false;
    }

    _debug.isoValues.setData(_debug.isoMesh);
    _debug.skeletonVertices.setData(_debug.skeletonMesh);

    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

bool Straightener::isReadyToComputeDiffusion() const {
    // Check that all specified segments have start and end points
    for (const InputParams& i : _inputParameters) {
        if ((i.frontVertexId == -1) || (i.backVertexId == -1)) {
            return false;
        }
    }
    return true;
}

void Straightener::diffusionDistances() {
    using namespace std;
    using namespace Eigen;

    typedef SparseMatrix<double> SparseMatrixXd;

    // Discrete Gradient operator
    SparseMatrixXd G;
    igl::grad(_skeletonTetra.TV, _skeletonTetra.TT, G);

    SimplicialLDLT<SparseMatrixXd> solver;

    MatrixXi constraint_indices;
    MatrixXd constraint_values;
    constraint_indices.resize(2, 1);
    constraint_values.resize(2, 1);
    // OBS(abock): index switch is not an error 
    constraint_indices(0, 0) = _currentInputParameter->backVertexId;
    constraint_indices(1, 0) = _currentInputParameter->frontVertexId;
    constraint_values(0, 0) = 1.0;
    constraint_values(1, 0) = 0.0;

    igl::harmonic(_skeletonTetra.TV, _skeletonTetra.TT, constraint_indices,
        constraint_values, 1, _isoValues);

    scale_zero_one(_isoValues);
    VectorXd g = G * _isoValues;
    Map<MatrixXd> V(g.data(), _skeletonTetra.TT.rows(), 3);
    V.rowwise().normalize();

    solver.compute(G.transpose()*G);
    _isoValues = solver.solve(G.transpose()*g);
    scale_zero_one(_isoValues);
}

void Straightener::updateConstraints() {
    Eigen::VectorXi components;
    igl::components(_skeletonTetra.TT, components);

    std::lock_guard<std::mutex> g(_constraintsLock);

    _deformationConstraints.update_bone_constraints(
        _outerTetra.TV,
        _outerTetra.TT,
        _skeletonTetra.TV,
        _skeletonTetra.TT,
        _isoValues,
        components,
        { { _currentInputParameter->frontVertexId, _currentInputParameter->backVertexId } },
        _nBones
    );
    _isConstraintsChanged = true;
}

std::shared_ptr<BasicMesh> createMesh(const Eigen::MatrixXd& TV, const Eigen::MatrixXi& TF, 
    const Eigen::MatrixXd& TFn, size3_t volumeDim, bool useIsovalues, const Eigen::VectorXd& isoValues = Eigen::VectorXd())
{
    std::shared_ptr<BasicMesh> mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4());
    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    Eigen::MatrixXd colors;
    if (useIsovalues) {
        igl::colormap(igl::ColorMapType::COLOR_MAP_TYPE_INFERNO, isoValues, false, colors);
    }

    //_isoValues.rows() == 0

    size_t maxDim = glm::compMax(volumeDim) + 1;
    for (int i = 0; i < TV.rows(); ++i) {
        glm::vec3 p = glm::vec3(TV(i, 0), TV(i, 1), TV(i, 2));
        glm::vec4 c;
        if (useIsovalues) {
            c = eigenToGLMColor(colors.row(i));
        }
        else {
            c = glm::vec4((p * float(maxDim)) / glm::vec3(volumeDim) + glm::vec3(1.f), 1.f);
        }

        mesh->addVertex(
            p, // pos
            glm::vec3(TFn(i, 0), TFn(i, 1), TFn(i, 2)), // normal
            glm::vec3(0.f), // texCoord
            c // color
        );
    }

    for (int i = 0; i < TF.rows(); ++i) {
        indices->add({
            static_cast<unsigned int>(TF(i, 0)),
            static_cast<unsigned int>(TF(i, 1)),
            static_cast<unsigned int>(TF(i, 2))
        });
    }

    return mesh;
}

std::shared_ptr<BasicMesh> Straightener::createOutputSurfaceMesh(const Eigen::MatrixXd& TV) {
    bool useIsoValues = _isoValues.rows() != 0;
    return createMesh(
        TV,
        _skeletonTetra.TF, _skeletonTetra.TFn,
        _inport.getData()->getDimensions(),
        useIsoValues, _isoValues
    );
}

std::shared_ptr<BasicMesh> Straightener::createOuterSurfaceMesh(const Eigen::MatrixXd& TV) {
    return createMesh(TV, _outerTetra.TF, _outerTetra.TFn, _inport.getData()->getDimensions(), false);
}

std::shared_ptr<BasicMesh> Straightener::createMeshFromTriangles() {
    std::shared_ptr<BasicMesh> mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4());
    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;

    for (int i = 0; i < _triangle.TV.rows(); ++i) {
        glm::vec3 p = glm::vec3(_triangle.TV(i, 0), _triangle.TV(i, 1), _triangle.TV(i, 2));
        glm::vec4 c = glm::vec4((p * float(maxDim)) / glm::vec3(_inport.getData()->getDimensions()) + glm::vec3(1.f), 1.f);
        //glm::vec4(colors(i, 0), colors(i, 1), colors(i, 2), 1.f);

        mesh->addVertex(
            p, // pos
            glm::vec3(_triangle.TFn(i, 0), _triangle.TFn(i, 1), _triangle.TFn(i, 2)), // normal
            glm::vec3(0.f), // texCoord
            c // color
        );
    }

    for (int i = 0; i < _triangle.TF.rows(); ++i) {
        indices->add({
            static_cast<unsigned int>(_triangle.TF(i, 0)),
            static_cast<unsigned int>(_triangle.TF(i, 1)),
            static_cast<unsigned int>(_triangle.TF(i, 2))
        });
    }

    return mesh;
}


void Straightener::createDebugMeshes() {
    _debug.isoMesh = meshutil::sphere({ 0.f, 0.f, 0.f }, 0.f, glm::vec4(0.f));
    //_debugMesh->setModelMatrix(mat4());

    Eigen::MatrixXd colors;
    if (_isoValues.rows() != 0) {
        igl::colormap(igl::ColorMapType::COLOR_MAP_TYPE_INFERNO, _isoValues, false, colors);
    }


    for (int i = 0; i < _skeletonTetra.TV.rows(); ++i) {
        glm::vec3 p = eigenToGLM(_skeletonTetra.TV.row(i));
        glm::vec4 c = _isoValues.rows() == 0 ? glm::vec4(1.f) : eigenToGLMColor(colors.row(i));
        //glm::vec4(colors(i, 0), colors(i, 1), colors(i, 2), 1.f);

        std::shared_ptr<BasicMesh> m = meshutil::sphere(p, _sphereRadius, c);

        _debug.isoMesh->append(m.get());
    }


    _debug.skeletonMesh = meshutil::sphere({ 0.f, 0.f, 0.f }, 0.f, glm::vec4(0.f));
    LogInfo("nBoneConstraints: " << _deformationConstraints.m_bone_constraints_idx.size());

    for (int i : _deformationConstraints.m_bone_constraints_idx) {
        glm::vec3 p = eigenToGLM(_skeletonTetra.TV.row(i));
        glm::vec4 c(0.f, 0.95f, 0.f, 1.f);

        std::shared_ptr<BasicMesh> m = meshutil::sphere(p, 1.5f * _sphereRadius, c);

        _debug.skeletonMesh->append(m.get());
    }


    invalidate(InvalidationLevel::InvalidOutput);
}



//////////////////////////////////////////////////////////////////////////////////////////
//                                Event Handlers
//////////////////////////////////////////////////////////////////////////////////////////


void Straightener::eventUpdateMousePos(Event* e) {
    updateStatusString();

    if (_stopInteraction) {
        return;
    }

    MouseEvent* mouseEvent = static_cast<MouseEvent*>(e);

    glm::dvec2 pos = mouseEvent->pos();

    // @FRAGILE(abock): needs to be the same as in ::createMesh
    glm::mat4 m = glm::mat4();

    glm::mat4 v = _camera.get().getViewMatrix();
    glm::mat4 p = _camera.get().getProjectionMatrix();

    // column-major v row-major (GLM v Eigen)
    glm::mat4 mv = glm::transpose(v * m);
    //glm::mat4 mv = (v * m);

    Eigen::Matrix4f modelView;
    modelView << mv[0][0], mv[0][1], mv[0][2], mv[0][3],
        mv[1][0], mv[1][1], mv[1][2], mv[1][3],
        mv[2][0], mv[2][1], mv[2][2], mv[2][3],
        mv[3][0], mv[3][1], mv[3][2], mv[3][3];

    p = glm::transpose(p);
    Eigen::Matrix4f projMatrix;
    projMatrix << p[0][0], p[0][1], p[0][2], p[0][3],
        p[1][0], p[1][1], p[1][2], p[1][3],
        p[2][0], p[2][1], p[2][2], p[2][3],
        p[3][0], p[3][1], p[3][2], p[3][3];

    glm::size2_t viewportSize = glm::size2_t(_windowSize.get());
    //glm::size2_t viewportSize = _imageOutport.getData()->getDimensions();

    Eigen::Vector4f viewport;
    viewport << 0.f, 0.f, static_cast<float>(viewportSize.x), static_cast<float>(viewportSize.y);

    //LogInfo("Viewport: " << viewport);

    //LogInfo("Mouse pos: " << pos);

    int fid;
    Eigen::Vector3f bc;


    _meshLock.lock();

    _currentHoverVertexId = -1;
    if (igl::unproject_onto_mesh(Eigen::Vector2f(pos.x, pos.y),
        modelView, projMatrix,
        viewport,
        currentTV(), currentTF(), fid, bc))
    {
        int max;
        bc.maxCoeff(&max);
        // HUH!?!  currentTF or currentTV?
        _currentHoverVertexId = currentTF()(fid, max);
    }

    _meshLock.unlock();

    //LogInfo("Current vertex id: " << _currentHoverVertexId);
}

void Straightener::eventStartDiffusion() {
    updateStatusString();

    if (_stopInteraction) {
        return;
    }


    if (isReadyToComputeDiffusion()) {
        _readyToSLIM = true;
        if (hasTetraMesh()) {
            diffusionDistances();
            updateConstraints();
            createDebugMeshes();
        }
    }
}

void Straightener::eventSelectPoint() {
    if (_stopInteraction) {
        return;
    }

    std::lock_guard<std::mutex> g(_meshLock);
    defer { updateDiffusionReadyString(); };

    if (_currentHoverVertexId == -1) {
        return;
    }

    switch (_currentSelectionState) {
        case SelectionState::None:
            break;
        case SelectionState::Front:
        {
            _currentInputParameter->frontVertexId = _currentHoverVertexId;
            _currentSelectionState = SelectionState::Back;

            LogInfo("Selected front: " << _currentInputParameter->frontVertexId);

            glm::vec3 p = glm::vec3(
                currentTV()(_currentInputParameter->frontVertexId, 0),
                currentTV()(_currentInputParameter->frontVertexId, 1),
                currentTV()(_currentInputParameter->frontVertexId, 2)
            );
            std::shared_ptr<BasicMesh> mesh = meshutil::sphere(p, 0.015f, glm::vec4(0.f, 0.75f, 0.f, 1.f));

            _frontSelectionMesh.setData(mesh);
            break;
        }
        case SelectionState::Back:
        {
            _currentInputParameter->backVertexId = _currentHoverVertexId;
            _currentSelectionState = SelectionState::None;

            LogInfo("Selected back: " << _currentInputParameter->backVertexId);

            glm::vec3 p = glm::vec3(
                currentTV()(_currentInputParameter->backVertexId, 0),
                currentTV()(_currentInputParameter->backVertexId, 1),
                currentTV()(_currentInputParameter->backVertexId, 2)
            );
            std::shared_ptr<BasicMesh> mesh = meshutil::sphere(p, 0.015f, glm::vec4(0.75f, 0.f, 0.f, 1.f));
            _backSelectionMesh.setData(mesh);
            break;
        }
    }

    _selectionStateString = selectionStateToString(_currentSelectionState);
}

void Straightener::eventReset() {
    defer { updateDiffusionReadyString(); };

    _currentSelectionState = SelectionState::None;
    _inputParameters.clear();
    _inputParameters.resize(1);
    _currentInputParameter = std::prev(_inputParameters.end());

    _frontSelectionMesh.setData(nullptr);
    _backSelectionMesh.setData(nullptr);

    _isoValues.resize(0, 0);
    hasTetraMesh() ? createOutputSurfaceMesh(_skeletonTetra.TV) : createMeshFromTriangles();
}

void Straightener::eventPreviousParameter() {
    if (_stopInteraction) {
        return;
    }
    defer { updateDiffusionReadyString(); };

    LogInfo("Selecting previous input parameter set");

    if (_currentInputParameter != _inputParameters.begin()) {
        --_currentInputParameter;

        auto oldParameter = std::next(_currentInputParameter);
        if (isInputParamEmpty(*(oldParameter))) {
            _inputParameters.erase(oldParameter);
        }

        updateInputParameterString();
    }
}

void Straightener::eventNextParameter() {
    if (_stopInteraction) {
        return;
    }
    LogInfo("Selecting next input parameter set");

    ++_currentInputParameter;

    // If we are now at the end, we need to add a new input state and repoint
    if (_currentInputParameter == _inputParameters.end()) {
        // We are currently the last input parameter and have to add a new one
        _inputParameters.push_back(InputParams());
        _currentInputParameter = std::prev(_inputParameters.end());
    }

    updateInputParameterString();
    updateDiffusionReadyString();
}

//void Straightener::eventPreviousLevelset() {
    //if (_stopInteraction) {
    //    return;
    //}
    //LogInfo("Selecting previous levelset");

    //if (_currentInputParameter->currentLevelset == _currentInputParameter->levelSetOrientations.begin()) {
    //    // We are at the first level set
    //    return;
    //}

    //--(_currentInputParameter->currentLevelset);

    //updateDiffusionReadyString();
//}

//void Straightener::eventNextLevelset() {
    //if (_stopInteraction) {
    //    return;
    //}
    //LogInfo("Selecting next levelset");

    //if (_currentInputParameter->currentLevelset + 1 == _currentInputParameter->levelSetOrientations.end()) {
    //    // We are at the last level set
    //    return;
    //}

    //++(_currentInputParameter->currentLevelset);

    //updateDiffusionReadyString();
//}

const Eigen::MatrixXd& Straightener::currentTV() const {
    return hasTetraMesh() ? _skeletonTetra.TV : _triangle.TV;
}

const Eigen::MatrixXi& Straightener::currentTF() const {
    return hasTetraMesh() ? _skeletonTetra.TF : _triangle.TF;
}

bool Straightener::hasTetraMesh() const {
    return _skeletonTetra.TVOriginal.rows() > 0 && _outerTetra.TV.rows() > 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  String update methods
//////////////////////////////////////////////////////////////////////////////////////////

void Straightener::updateSelectionStateString() {
    _selectionStateString = selectionStateToString(_currentSelectionState);
}

void Straightener::updateInputParameterString() {
    ptrdiff_t d = std::distance(_inputParameters.begin(), _currentInputParameter);
    _inputParameterSelection =
        "Editing " +
        // Adding one to make it more human readable 1-based indexing
        std::to_string(d + 1) +
        " of " +
        std::to_string(_inputParameters.size()) +
        " segments";
}

void Straightener::updateDiffusionReadyString() {
    _diffusionReadyString = isReadyToComputeDiffusion() ? "Ready" : "";
}

void Straightener::updateStatusString() {
    if (hasTetraMesh() && _readyToSLIM) {
        _statusString = "Straightening";
        _statusColor = glm::vec4(0.f, 0.75f, 0.f, 1.f);
    }

    if (hasTetraMesh() && isReadyToComputeDiffusion() && !_readyToSLIM) {
        _statusString = "Waiting for start (SPACE)";
        _statusColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
        return;
    }

    if (!hasTetraMesh() && _readyToSLIM) {
        _statusString = "Tetrahedralizing...";
        _statusColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
        return;
    }

    _statusString = "";
}

void Straightener::updateInstructionsString() {
    bool b =
        (_trianglesVertexInport.getData()->rows() == 0) &&
        (_trianglesTetIndexInport.getData()->rows() == 0) &&
        (_skeletonTetraVertexInport.getData()->rows() == 0) &&
        (_skeletonTetraTetIndexInport.getData()->rows() == 0);

    if (b) {
        _instructionsString = "Press 'Load Dataset' to start";
    }
    else {
        _instructionsString = "";
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo Straightener::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)