#ifndef __AB_TETMESHER_H__
#define __AB_TETMESHER_H__

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

#include <mutex>
#include <thread>

namespace inviwo {

class IVW_MODULE_SEGMENTANGLING_API TetMesher : public Processor {
public:
    TetMesher();
    virtual ~TetMesher() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

    void action();
    
private:
    VolumeInport _inport;
    
    FileProperty _volumeFilename;
    ButtonProperty _action;

#ifdef WIN32
    HANDLE _processHandle;
    std::atomic_bool _hasProcessHandle = false;
#else
#error("implement me")
#endif
};

} // namespace

#endif // __AB_TETMESHER_H__
