#ifndef __AB_YIXINLOADER_H__
#define __AB_YIXINLOADER_H__

#include <modules/segmentangling/segmentanglingmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/progressbarowner.h>
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

class IVW_MODULE_SEGMENTANGLING_API YixinLoader : public Processor {
public:
    YixinLoader();
    virtual ~YixinLoader() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    void action();
    
private:
    VertexOutport _vertexOutport;
    TetIndexOutport _tetIndexOutport;
    FileProperty _file;
    ButtonProperty _action;
};

} // namespace

#endif // __AB_YIXINLOADER_H__
