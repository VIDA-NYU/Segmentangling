#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

layout (std430, binding = 0) buffer Mapping {
    uint values[];
} mapping;

// layout(r8) readonly uniform imageBuffer mybuffer;

uniform usampler3D volumeIdentifiers;
uniform VolumeParameters volumeIdentifiersParameters;

in vec4 texCoord_;

// @FRAGILE:  Sync this with volumecollectiongenerator.cpp anon namespace
// #define ModificationAdd 0
// #define ModificationRemove 1

// uniform int currentVolume;
// uniform int featureToModify;
// uniform int modification;

void main() {
    const uint idx = texture(volumeIdentifiers, texCoord_.xyz).r;
    FragData0 = vec4(mapping.values[idx]);
    // FragData0 = vec4(2);
}
