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

#ifdef WIN32
#include <Windows.h>
#endif

namespace {

#ifdef WIN32

HANDLE startGtest(const std::string& filename) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    constexpr const char* gTetApplication = "new_gtet.exe";
    std::string commandline = "--input " + filename + " --ideal-edge-length 10 --epsilon 10 --is-quiet 1";

    char Commandline[256];
    sprintf(
        Commandline,
        "%s --input %s  --ideal-edge-length 10 --epsilon 10 --is-quiet 1",
        "new_gtet.exe",
        filename.c_str()
    );


    BOOL success = CreateProcessA(
        nullptr,
        Commandline,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!success) {
        DWORD errCode = GetLastError();
        char *err;
        if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
            (LPTSTR)&err,
            0,
            NULL))
        {
            return nullptr;
        }

        static char buffer[1024];

        inviwo::LogCentral::getPtr()->log(
            "TetMesher", inviwo::LogLevel::Info,
            inviwo::LogAudience::Developer,
            __FILE__,
            __FUNCTION__,
            __LINE__,
            "Failed to start process"
        );

        inviwo::LogCentral::getPtr()->log(
            "TetMesher", inviwo::LogLevel::Info,
            inviwo::LogAudience::Developer,
            __FILE__,
            __FUNCTION__,
            __LINE__,
            std::string(err)
        );

        LocalFree(err);

        return nullptr;
    }

    return pi.hProcess;
}
#else
void startGtestAndWait(const std::string& filename) {
#error("Implement me")
}
#endif
}

namespace inviwo {

TetMesher::TetMesher()
    : Processor()
    , _inport("volumeInport")
    , _volumeFilename("_volumeFilename", "Volume Filename")
    , _action("_action", "Create volume")
{
    addPort(_inport);

    addProperty(_volumeFilename);

    _action.onChange([this]() { action(); });
    addProperty(_action);
}

void TetMesher::process() {
    while (_hasProcessHandle) {
#ifdef WIN32
        DWORD exitCode;
        BOOL b = GetExitCodeProcess(_processHandle, &exitCode);

        if (!b) {
            DWORD errCode = GetLastError();
            char *err;
            if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                errCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
                (LPTSTR)&err,
                0,
                NULL))
            {
                continue;
            }

            static char buffer[1024];

            inviwo::LogCentral::getPtr()->log(
                "TetMesher", inviwo::LogLevel::Info,
                inviwo::LogAudience::Developer,
                __FILE__,
                __FUNCTION__,
                __LINE__,
                "Failed to get exit code process"
            );

            inviwo::LogCentral::getPtr()->log(
                "TetMesher", inviwo::LogLevel::Info,
                inviwo::LogAudience::Developer,
                __FILE__,
                __FUNCTION__,
                __LINE__,
                err
            );
            continue;
        }

        if (exitCode == STILL_ACTIVE) {
            LogInfo("Generating tetrahedral mesh...");
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        else {
            LogInfo("Exit code: " << exitCode);
            break;
        }
#else
#error("Implement me")
#endif
    }

}

void TetMesher::action() {
    std::shared_ptr<const Volume> vol = _inport.getData();

    const VolumeRAM* v = vol->getRepresentation<VolumeRAM>();

    Eigen::MatrixXd V;
    Eigen::MatrixXi F;

    //
    // Marching cubes
    //
    LogInfo("Compute marching cubes");
    Eigen::MatrixXd GP(
        (vol->getDimensions().x + 2) * (vol->getDimensions().y + 2) * (vol->getDimensions().z + 2),
        3
    );
    Eigen::VectorXd SV(GP.rows());

    int readcount = 0;
    //int appendcount = 0;
    for (int zi = 0; zi < vol->getDimensions().z + 2; zi++) {
        for (int yi = 0; yi < vol->getDimensions().y + 2; yi++) {
            for (int xi = 0; xi < vol->getDimensions().x + 2; xi++) {
                if (xi == 0 || yi == 0 || zi == 0 || xi == (vol->getDimensions().x + 1) ||
                    yi == (vol->getDimensions().y + 1) || zi == (vol->getDimensions().z + 1))
                {
                    SV[readcount] = -1.0;
                }
                else {
                    // We subtract 1 to account for the boundary
                    SV[readcount] = v->getAsDouble({ xi - 1, yi - 1, zi - 1 });
                    //SV[readcount] = double(data[appendcount]);
                    //appendcount += 1;
                }
                GP.row(readcount) = Eigen::RowVector3d(xi, yi, zi);
                readcount += 1;
            }
        }
    }

    //delete data;

    //datfile.m_bb_min = Eigen::RowVector3d(1.0, 1.0, 1.0);
    //datfile.m_bb_max = Eigen::RowVector3d(datfile.w, datfile.h, datfile.d);

    //cout << "Running Marching Cubes..." << endl;
    igl::copyleft::marching_cubes(
        SV,
        GP,
        vol->getDimensions().x + 2,
        vol->getDimensions().y + 2,
        vol->getDimensions().z + 2,
        V,
        F
    );

    LogInfo("Finished marching cubes");
    LogInfo("Marching cubes model has " << V.rows() << " vertices and " << F.rows() << " faces");


    //
    // remove_garbage_components
    //
    Eigen::MatrixXi newF;

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

    LogInfo("Finding component with most vertices...");
    int max_component = -1;
    int max_component_count = 0;
    for (int i = 0; i < component_count.size(); i++) {
        if (max_component_count < component_count[i]) {
            max_component = i;
            max_component_count = component_count[i];
        }
    }
    LogInfo("Component " << max_component <<
        " has the most vertices with a count of " <<
        max_component_count);

    LogInfo("Deleting smaller components...");
    newF.resize(F.rows(), 3);

    int fcount = 0;
    for (int i = 0; i < F.rows(); i++) {
        bool keep = true;
        for (int j = 0; j < 3; j++) {
            if (components[F(i, j)] != max_component) {
                keep = false;
                break;
            }
        }
        if (keep) {
            newF.row(fcount++) = F.row(i);
        }
    }

    LogInfo("Largest component of model has " << fcount << " faces and " <<
        newF.maxCoeff() << " vertices");
    newF.conservativeResize(fcount, 3);



    Eigen::VectorXd V2 = V.col(2);
    V.col(2) = V.col(1);
    V.col(1) = V2;

    std::string outputName = _volumeFilename.get() + ".off";
    igl::writeOFF(outputName, V, newF);


    //
    // gtet
    //

#ifdef WIN32
    _processHandle = startGtest(outputName);
    if (_processHandle) {
        _hasProcessHandle = true;
        invalidate(InvalidationLevel::InvalidOutput);
    }
#else
#error("Implement me")
#endif

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