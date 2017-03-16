#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

layout (binding = 0) buffer Contour {
    uint values[];
};

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;
uniform int kernelSize;

uniform float inv2Sigma;

in vec4 texCoord_;

void main() {
//     int k2 = kernelSize / 2;
//     vec4 value = vec4(0,0,0,0);
//     float tot_weight = 0;
//     for(int z = -k2;z < k2;z++) {
//         for(int y = -k2;y < k2;y++) {
//             for(int x = -k2;x < k2;x++){
//                 float w = 1.0;
// #ifdef GAUSSIAN
//                 vec3 p = vec3(x,y,z)/vec3(k2);
//                 float l = dot(p,p);
//                 w = exp(-l*inv2Sigma);
// #endif
//                 value += w*getVoxel(volume, volumeParameters, texCoord_.xyz + (vec3(x, y, z) * volumeParameters.reciprocalDimensions));
//                 tot_weight += w;
//             }
//         }
//     }
//     vec4 voxel = value / tot_weight;
   
    // FragData0 = voxel;
    FragData0 = vec4(1.0);
}
