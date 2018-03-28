#pragma optimize ("", off)

#include "yixinloader.h"

#include <Eigen/Core>
#include <utils/yixin_loader.h>



namespace inviwo {

YixinLoader::YixinLoader()
    : Processor()
    , _vertexOutport("_vertexOutport")
    , _tetIndexOutport("_tetIndexOutport")
    , _file("_file", "File")
    , _action("_action", "Load file")
{
    addPort(_vertexOutport);
    addPort(_tetIndexOutport);

    addProperty(_file);

    _action.onChange([this]() { action(); });
    addProperty(_action);
}

void YixinLoader::process() {}

void YixinLoader::action() {
    std::shared_ptr<Eigen::MatrixXd> TV = std::make_shared<Eigen::MatrixXd>();
    std::shared_ptr<Eigen::MatrixXi> TT = std::make_shared<Eigen::MatrixXi>();

    Eigen::MatrixXi TF;
    load_yixin_tetmesh(_file, *TV, TF, *TT);

    _vertexOutport.setData(TV);
    _tetIndexOutport.setData(TT);
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Inviiiiiiwo
//////////////////////////////////////////////////////////////////////////////////////////


const ProcessorInfo YixinLoader::processorInfo_ {
    "bock.yixinloader",  // Class identifier
    "YixinLoader",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,             // Code state
    Tags::GL                       // Tags
};


const ProcessorInfo YixinLoader::getProcessorInfo() const {
    return processorInfo_;
}

}  // namespace

#pragma optimize ("", on)