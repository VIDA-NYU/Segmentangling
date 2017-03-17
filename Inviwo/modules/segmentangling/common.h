#ifndef __AB_COMMON_H__
#define __AB_COMMON_H__

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

struct ContourInformation {
    uint32_t nFeatures;
    GLuint ssbo;
};

using ContourInport = DataInport<ContourInformation>;
using ContourOutport = DataOutport<ContourInformation>;

} // namespace inviwo

#endif // __AB_COMMON_H__