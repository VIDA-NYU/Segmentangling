#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

layout (std430, binding = 0) buffer Contour {
    uint nFeatures;
    uint values[];
} contour;

uniform usampler3D volume;
uniform VolumeParameters volumeParameters;

in vec4 texCoord_;

void main() {
    const uint voxel = texture(volume, texCoord_.xyz).r;
    const uint feature = contour.values[voxel];
    FragData0 = vec4(feature + 1);
}
