#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

layout (std430, binding = 0) buffer Contour {
    uint nFeatures;
    uint values[];
} contour;

layout (std430, binding = 1) buffer ContourNegative {
    uint nFeatures;
    uint values[];
} contourNegative;

uniform usampler3D volume;
uniform VolumeParameters volumeParameters;

uniform bool hasNegativeData;

in vec4 texCoord_;

void main() {
    const uint voxel = texture(volume, texCoord_.xyz).r;
    const uint feature = contour.values[voxel];

    // uint fade = 3;
    uint fade = -1;
    if (hasNegativeData) {
        // If we have negative data, we should use it
        const uint negFeature = contourNegative.values[voxel];
        if (negFeature != -1) {
            // The current voxel is a feature that we have already selected
            fade = 0;
        }
    }
    else {
        FragData0 = vec4(feature + 1);
    }

    // FragData0 = vec4(vec3(feature + 1), fade);
    FragData0 = vec4(feature + 1, fade, 0.0, 0.0);
    // 
}
