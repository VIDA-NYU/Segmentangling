#pragma optimize ("", off)

#include "tetmesher.h"

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

#include <utils/utils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <modules/segmentangling/util/defer.h>

#include <igl/copyleft/marching_cubes.h>
#include <igl/components.h>
#include <igl/writeOFF.h>

#include <TetWild.h>

namespace inviwo {

TetMesher::TetMesher()
    : Processor()
    , _inport("volumeInport")
    , _triangleVertexOutport("_triangleVertexOutport")
    , _triangleIndexOutport("_triangleIndexOutport")
    , _vertexOutport("_vertexOutport")
    , _tetIndexOutport("_tetIndexOutport")
    //, _volumeFilename("_volumeFilename", "Volume Filename")
    , _componentCutoff("_componentCutoff", "Component Cutoff Ratio", 0.75f, 0.f, 1.f)
    , _action("_action", "Go")
{
    addPort(_inport);
    addPort(_triangleVertexOutport);
    addPort(_triangleIndexOutport);

    addPort(_vertexOutport);
    addPort(_tetIndexOutport);


    //addProperty(_volumeFilename);

    _action.onChange([this]() { action(); });
    addProperty(_action);


    _componentCutoff.onChange([this]() { updateFilter(); });
    addProperty(_componentCutoff);
}

void TetMesher::process() {
    if (_isFirstFrame) {
        _triangleVertexOutport.setData(std::make_shared<Eigen::MatrixXd>());
        _triangleIndexOutport.setData(std::make_shared<Eigen::MatrixXi>());
        _vertexOutport.setData(std::make_shared<Eigen::MatrixXd>());
        _tetIndexOutport.setData(std::make_shared<Eigen::MatrixXi>());

        _isFirstFrame = false;
    }
}

std::tuple<Eigen::MatrixXd, Eigen::MatrixXi> TetMesher::marchingCubes(const VolumeRAM& v) {
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;

    //
    // Marching cubes
    //
    LogInfo("Compute marching cubes");
    Eigen::MatrixXd GP(
        (v.getDimensions().x + 2) * (v.getDimensions().y + 2) * (v.getDimensions().z + 2),
        3
    );
    Eigen::VectorXd SV(GP.rows());

    int readcount = 0;
    //int appendcount = 0;
    for (int zi = 0; zi < v.getDimensions().z + 2; zi++) {
        for (int yi = 0; yi < v.getDimensions().y + 2; yi++) {
            for (int xi = 0; xi < v.getDimensions().x + 2; xi++) {
                if (xi == 0 || yi == 0 || zi == 0 || xi == (v.getDimensions().x + 1) ||
                    yi == (v.getDimensions().y + 1) || zi == (v.getDimensions().z + 1))
                {
                    SV[readcount] = -1.0;
                }
                else {
                    // We subtract 1 to account for the boundary
                    SV[readcount] = v.getAsDouble({ xi - 1, yi - 1, zi - 1 });
                    //SV[readcount] = double(data[appendcount]);
                    //appendcount += 1;
                }
                GP.row(readcount) = Eigen::RowVector3d(xi, yi, zi);
                readcount += 1;
            }
        }
    }
    getProgressBar().updateProgress(0.3f);

    //delete data;

    //datfile.m_bb_min = Eigen::RowVector3d(1.0, 1.0, 1.0);
    //datfile.m_bb_max = Eigen::RowVector3d(datfile.w, datfile.h, datfile.d);

    //cout << "Running Marching Cubes..." << endl;
    igl::copyleft::marching_cubes(
        SV,
        GP,
        v.getDimensions().x + 2,
        v.getDimensions().y + 2,
        v.getDimensions().z + 2,
        V,
        F
    );

    LogInfo("Finished marching cubes");
    LogInfo("Marching cubes model has " << V.rows() << " vertices and " << F.rows() << " faces");
    getProgressBar().updateProgress(0.6f);

    return { V, F };
}

std::tuple<Eigen::VectorXi, std::vector<int>> TetMesher::findConnectedComponents(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F) {
    //
    // remove_garbage_components
    //
    LogInfo("Computing connected components...");
    Eigen::VectorXi components;
    igl::components(F, components);

    LogInfo("Counting connected components...");
    std::vector<int> component_count;
    component_count.resize(components.maxCoeff());
    for (int i = 0; i < V.rows(); i++) {
        component_count[components[i]] += 1;
    }
    LogInfo("The model has " << component_count.size() <<
        " connected components.");

    return { components, component_count };
}

Eigen::MatrixXi TetMesher::filterConnectedComponents(const Eigen::MatrixXi& F,
        const Eigen::VectorXi& components, const std::vector<int>& componentsCount)
{
    Eigen::MatrixXi newF;


    LogInfo("Finding component with most vertices...");
    int max_component = -1;
    int max_component_count = 0;
    int min_component = -1;
    int min_component_count = std::numeric_limits<int>::max();

    for (int i = 0; i < componentsCount.size(); i++) {
        if (max_component_count < componentsCount[i]) {
            max_component = i;
            max_component_count = componentsCount[i];
        }

        if (min_component_count > componentsCount[i]) {
            min_component = i;
            min_component_count = componentsCount[i];
        }

    }
    LogInfo("Component " << max_component <<
        " has the most vertices with a count of " <<
        max_component_count);

    LogInfo("Component " << min_component <<
        " has the least vertices with a count of " <<
        min_component_count);


    int cutoffComponentCount = int(
        _componentCutoff * (float(max_component_count) - float(min_component_count)) + float(min_component_count)
        );



    LogInfo("Deleting components with count < " << cutoffComponentCount);
    newF.resize(F.rows(), 3);

    int fcount = 0;
    for (int i = 0; i < F.rows(); i++) {
        bool keep = true;
        for (int j = 0; j < 3; j++) {
            if (componentsCount[components[F(i, j)]] <= cutoffComponentCount) {
                keep = false;
                break;
            }
        }
        if (keep) {
            //nKeep++;
            newF.row(fcount++) = F.row(i);
        }
    }
    int nKeep = std::accumulate(
        componentsCount.begin(),
        componentsCount.end(),
        0,
        [cutoffComponentCount](int i, int j) {
        if (j <= cutoffComponentCount) {
            return i;
        }
        else {
            return i + 1;
        }
    }
    );

    LogInfo("Keeping " << nKeep << " out of " << componentsCount.size() << " components");

    LogInfo("Largest component of model has " << fcount << " faces and " <<
        newF.maxCoeff() << " vertices");
    newF.conservativeResize(fcount, 3);

    return newF;
}


void TetMesher::action() {
    std::shared_ptr<const Volume> vol = _inport.getData();

    getProgressBar().show();

    const VolumeRAM* v = vol->getRepresentation<VolumeRAM>();

    Eigen::MatrixXd V;
    std::tie(V, _F) = marchingCubes(*v);

    std::tie(_components, nComponents) = findConnectedComponents(V, _F);

    Eigen::MatrixXi newF = filterConnectedComponents(_F, _components, nComponents);


    //std::sort(component_count.begin(), component_count.end());
    //LogInfo("N Components:");
    //std::for_each(
    //    component_count.begin(),
    //    component_count.end(),
    //    [this](int i) { LogInfo(i); }
    //);


    Eigen::VectorXd V2 = std::move(V.col(2));
    V.col(2) = std::move(V.col(1));
    V.col(1) = std::move(V2);

    std::shared_ptr<Eigen::MatrixXd> TV = std::make_shared<Eigen::MatrixXd>();
    std::shared_ptr<Eigen::MatrixXi> TT = std::make_shared<Eigen::MatrixXi>();
    *TV = std::move(V);
    *TT = std::move(newF);

    _triangleVertexOutport.setData(TV);
    _triangleIndexOutport.setData(TT);

    _vertexOutport.setData(std::make_shared<Eigen::MatrixXd>());
    _tetIndexOutport.setData(std::make_shared<Eigen::MatrixXi>());

    getProgressBar().updateProgress(1.f);


    //dispatchPool([this, V, newF]() {
    //    //
    //    // gtet
    //    //
    //    args.is_quiet = true;
    //    args.max_pass = 5;
    //    args.filter_energy = 200;
    //    args.i_epsilon = 100;
    //    args.i_dd = 100;

    //    std::shared_ptr<Eigen::MatrixXd> TV = std::make_shared<Eigen::MatrixXd>();
    //    std::shared_ptr<Eigen::MatrixXi> TT = std::make_shared<Eigen::MatrixXi>();
    //    TetWild::gtet(
    //        V,
    //        newF,
    //        *TV,
    //        *TT
    //    );

    //    dispatchFront([this, TV, TT]() {
    //        _vertexOutport.setData(TV);
    //        _tetIndexOutport.setData(TT);
    //    });
    //});
}

void TetMesher::updateFilter() {
    if (_F.rows() == 0 || _components.rows() == 0 || nComponents.empty()) {
        return;
    }
    Eigen::MatrixXi newF = filterConnectedComponents(_F, _components, nComponents);

    std::shared_ptr<Eigen::MatrixXi> TT = std::make_shared<Eigen::MatrixXi>();
    *TT = std::move(newF);
    _triangleIndexOutport.setData(TT);
}



//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo TetMesher::processorInfo_ {
    "bock.tetmesher",  // Class identifier
    "TetMesher",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};


const ProcessorInfo TetMesher::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)