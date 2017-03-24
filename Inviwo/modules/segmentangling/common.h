#ifndef __AB_COMMON_H__
#define __AB_COMMON_H__

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

struct ContourInformation {
    uint32_t nFeatures;
    GLuint ssbo;
    std::vector<bool> useConvexHull;
};

using ContourInport = DataInport<ContourInformation>;
using ContourOutport = DataOutport<ContourInformation>;


using Feature = std::vector<uint32_t>;
using FeatureInformation = std::vector<Feature>;
using FeatureInport = DataInport<FeatureInformation>;
using FeatureOutport = DataOutport<FeatureInformation>;

} // namespace inviwo

#endif // __AB_COMMON_H__