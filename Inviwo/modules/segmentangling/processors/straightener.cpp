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

#include <glm/gtx/component_wise.hpp>
#include <modules/base/algorithm/meshutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <modules/segmentangling/util/fresh_new_utils.h>

using namespace fresh_prince_of_utils;

namespace inviwo {

    int i = 0;
void Straightener::slimThread() {
    using namespace std;
    using namespace Eigen;

    igl::SLIMData sData;

    //m_ui_state.m_avg_draw_state_update_time = 0.0;
    //m_ui_state.m_avg_slim_time = 0.0;

    cout << "INFO: SLIM Thread: Starting SLIM background thread." << endl;
    while (_isSlimRunning) {

        _constraintsLock.lock();
        if (_isConstraintsChanged) {
            VectorXi slim_b;
            MatrixXd slim_bc;
            _constraintState.slim_constraints(slim_b, slim_bc, _debugOnlyEndAndTets);

            const double soft_const_p = 1e5;
            const MatrixXd TV_0 = _TV;
            sData.exp_factor = 5.0;
            slim_precompute(_TVOriginal, _TT, TV_0, sData, igl::SLIMData::EXP_CONFORMAL,
                slim_b, slim_bc, soft_const_p);
            _isConstraintsChanged = false;
        }
        _constraintsLock.unlock();

        if (_constraintState.num_constraints() == 0) {
            this_thread::yield();
            continue;
        }
        LogInfo("Iteration: " << i);
        ++i;
        //auto slim_start_time = chrono::high_resolution_clock::now();
        igl::slim_solve(sData, 1);

        _drawStateLock.lock();
        _outputSurfaceMesh = createMesh(sData.V_o);
        _drawStateLock.unlock();



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
    , _meshOutport("meshOutport")
    , _imageOutport("imageOutput")
    , _frontSelectionMesh("frontSelectionMesh")
    , _backSelectionMesh("backSelectionMesh")
    , _debugOnlyEndAndTets("onlyEndsAndTets", "_debugOnlyEndAndTets", false)
    , _nBones("_nBones", "Number of Bones", 25, 1, 100)
    , _camera("camera", "Camera")
    , _filename("filename", "Mesh File")
    , _reload("reload", "Reload")
    , _mousePositionTracker("mousePositionTracker", "Mouse Position Tracker",
        [this](Event* e) { eventUpdateMousePos(e); },
        MouseButton::None, MouseState::Move
    )
    , _keyboardPressSelect("keyboardPress", "keyboardPress",
        [this](Event* e) { eventUpdateKeyboard(e); },
        IvwKey::Space
    )
    , _keyboardPressReset("keyboardReset", "keyboardPressReset",
        [this](Event* e) { eventUpdateKeyboardReset(e); },
        IvwKey::F1
    )
{
    addPort(_inport);
    addPort(_meshOutport);
    addPort(_imageOutport);
    addPort(_frontSelectionMesh);
    addPort(_backSelectionMesh);

    addProperty(_debugOnlyEndAndTets);

    addProperty(_camera);
    addProperty(_nBones);

    addProperty(_mousePositionTracker);
    addProperty(_keyboardPressSelect);
    addProperty(_keyboardPressReset);

    addProperty(_filename);
    _filename.onChange([this](){
        _filenameDirty = true;
    });

    addProperty(_reload);
    _reload.onChange([this]() {
        _filenameDirty = true;
    });
    
    _isSlimRunning = true;
    _slimThread = std::thread(&Straightener::slimThread, this);
}

Straightener::~Straightener() {
    _isSlimRunning = false;
    _slimThread.join();
}


void Straightener::process() {
    if (_filenameDirty) {
        std::string filename = _filename;
        load_yixin_tetmesh(filename, _TVOriginal, _TF, _TT);
        _TV = _TVOriginal;
        //edge_endpoints(TV, TT, TEV1, TEV2);

        size3_t dim = _inport.getData()->getDimensions();


        size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;
        
        _TV /= maxDim;


        _texCoords = _TV - Eigen::MatrixXd::Ones(_TV.rows(), _TV.cols());

        igl::per_vertex_normals(_TV, _TF, _TFn); 

        //_texCoords.resize(_TV.rows(), 3);
        //for (int i = 0; i < _TV.rows(); i++) {
        //    // Subtract 1 since we pad the grid with a zero cell all around
        //    _texCoords.row(i) = _TV.row(i) - f.m_bb_min - Eigen::RowVector3d::Ones();
        //}

        _filenameDirty = false;

        _outputSurfaceMesh = createMesh(_TV);
        _meshOutport.setData(_outputSurfaceMesh);
    }

    _drawStateLock.lock();
    _meshOutport.setData(_outputSurfaceMesh);
    _drawStateLock.unlock();


    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    invalidate(InvalidationLevel::InvalidOutput);
}

void Straightener::eventUpdateMousePos(Event* e) {
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

    glm::size2_t viewportSize = _imageOutport.getData()->getDimensions();

    Eigen::Vector4f viewport;
    viewport << 0.f, 0.f, static_cast<float>(viewportSize.x), static_cast<float>(viewportSize.y);

    LogInfo("Viewport: " << viewport);

    LogInfo("Mouse pos: " << pos);

    int fid;
    Eigen::Vector3f bc;


    _drawStateLock.lock();

    _currentVertexId = -1;
    if (igl::unproject_onto_mesh(Eigen::Vector2f(pos.x, pos.y),
        modelView, projMatrix,
        viewport,
        _TV, _TF, fid, bc))
    {
        int max;
        bc.maxCoeff(&max);
        _currentVertexId = _TF(fid, max);
    }

    _drawStateLock.unlock();

    LogInfo("Current vertex id: " << _currentVertexId);

    switch (_currentSelectionState) {
        case SelectionState::None: LogInfo("Current state: None"); break;
        case SelectionState::Front: LogInfo("Current state: Front"); break;
        case SelectionState::Back: LogInfo("Current state: Back"); break;
    }

    LogInfo("Current front id: " << _currentFrontVertexId);
    LogInfo("Current back id: " << _currentBackVertexId);
}

void Straightener::eventUpdateKeyboardReset(Event* e) {
    _currentSelectionState = SelectionState::None;
    _currentFrontVertexId = -1;
    _currentBackVertexId = -1;

    _frontSelectionMesh.setData(nullptr);
    _backSelectionMesh.setData(nullptr);

    _isoValues.resize(0, 0);
    createMesh(_TV);
}

void Straightener::diffusionDistances() {
    using namespace std;
    using namespace Eigen;

    typedef SparseMatrix<double> SparseMatrixXd;

    // Discrete Gradient operator
    SparseMatrixXd G;
    igl::grad(_TV, _TT, G);

    SimplicialLDLT<SparseMatrixXd> solver;

    MatrixXi constraint_indices;
    MatrixXd constraint_values;
    constraint_indices.resize(2, 1);
    constraint_values.resize(2, 1);
    constraint_indices(0, 0) = _currentBackVertexId;
    constraint_indices(1, 0) = _currentFrontVertexId;
    constraint_values(0, 0) = 1.0;
    constraint_values(1, 0) = 0.0;

    igl::harmonic(_TV, _TT, constraint_indices,
        constraint_values, 1, _isoValues);

    scale_zero_one(_isoValues);
    VectorXd g = G * _isoValues;
    Map<MatrixXd> V(g.data(), _TT.rows(), 3);
    V.rowwise().normalize();

    solver.compute(G.transpose()*G);
    _isoValues = solver.solve(G.transpose()*g);
    scale_zero_one(_isoValues);
}

void Straightener::updateConstraints() {
    std::lock_guard<std::mutex> g(_constraintsLock);

    _constraintState.update_bone_constraints(
        _TV, _TT, _isoValues, { _currentFrontVertexId, _currentBackVertexId }, _nBones
    );
    _isConstraintsChanged = true;
}


void Straightener::eventUpdateKeyboard(Event* e) {
    std::lock_guard<std::mutex> g(_drawStateLock);

    switch (_currentSelectionState) {
        case SelectionState::None:
            break;
        case SelectionState::Front:
        {
            _currentFrontVertexId = _currentVertexId;
            _currentSelectionState = SelectionState::Back;

            LogInfo("Selected front: " << _currentFrontVertexId);

            glm::vec3 p = glm::vec3(_TV(_currentFrontVertexId, 0), _TV(_currentFrontVertexId, 1), _TV(_currentFrontVertexId, 2));
            std::shared_ptr<BasicMesh> mesh = meshutil::sphere(p, 0.05f, glm::vec4(0.f, 0.75f, 0.f, 1.f));

            _frontSelectionMesh.setData(mesh);
            break;
        }
        case SelectionState::Back:
        {
            _currentBackVertexId = _currentVertexId;
            _currentSelectionState = SelectionState::None;

            LogInfo("Selected back: " << _currentBackVertexId);

            glm::vec3 p = glm::vec3(_TV(_currentBackVertexId, 0), _TV(_currentBackVertexId, 1), _TV(_currentBackVertexId, 2));
            std::shared_ptr<BasicMesh> mesh = meshutil::sphere(p, 0.05f, glm::vec4(0.75f, 0.f, 0.f, 1.f));
            _backSelectionMesh.setData(mesh);


            diffusionDistances();
            updateConstraints();
            createMesh(_TV);

            break;
        }
    }
}


std::shared_ptr<BasicMesh> Straightener::createMesh(const Eigen::MatrixXd& TV) {
    std::shared_ptr<BasicMesh> mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4());
    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    Eigen::MatrixXd colors;
    if (_isoValues.rows() != 0) {
        igl::colormap(igl::ColorMapType::COLOR_MAP_TYPE_INFERNO, _isoValues, false, colors);
    }

    //_isoValues.rows() == 0

    size_t maxDim = glm::compMax(_inport.getData()->getDimensions()) + 1;
    for (int i = 0; i < TV.rows(); ++i) {
        glm::vec3 p = glm::vec3(TV(i, 0), TV(i, 1), TV(i, 2));

        glm::vec4 c = _isoValues.rows() == 0 ?
            glm::vec4((p * float(maxDim)) / glm::vec3(_inport.getData()->getDimensions()) + glm::vec3(1.f), 1.f) :
            glm::vec4(colors(i, 0), colors(i, 1), colors(i, 2), 1.f);

        mesh->addVertex(
            p, // pos
            glm::vec3(_TFn(i, 0), _TFn(i, 1), _TFn(i, 2)), // normal
            glm::vec3(0.f), // texCoord
            c // color
        );
    }

    for (int i = 0; i < _TF.rows(); ++i) {
        indices->add({
            static_cast<unsigned int>(_TF(i, 0)),
            static_cast<unsigned int>(_TF(i, 1)),
            static_cast<unsigned int>(_TF(i, 2))
        });
    }

    return mesh;
}



const ProcessorInfo Straightener::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)