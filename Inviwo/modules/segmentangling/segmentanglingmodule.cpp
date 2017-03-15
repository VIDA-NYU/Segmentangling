#include <modules/segmentangling/segmentanglingmodule.h>

#include <modules/opengl/shader/shadermanager.h>

namespace inviwo {

SegmentanglingModule::SegmentanglingModule(InviwoApplication* app)
    : InviwoModule(app, "Segmentanlging")
{
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // registerProcessor<ClusterExport>();
}

}  // namespace
