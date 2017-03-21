/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/classification.glsl"

#include "common.hglsl"

uniform isampler3D volume;
uniform VolumeParameters volumeParameters;

uniform mat4 sliceRotation; // Rotates around slice axis (offset to center point)
uniform float slice;

uniform vec4 fillColor;

in vec3 texCoord_;


void main() {
    // Rotate around center and translate back to origin
    vec3 samplePos = (sliceRotation * vec4(texCoord_.x, texCoord_.y, slice, 1.0)).xyz;
  
    const vec4 voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);
    const float feature = voxel.r;
    const float fade = voxel.g;

    if (feature == 0.0) {
        FragData0 = vec4(0.0);
    }
    else {
        FragData0.rgb = colormap(feature).rgb;
        if (fade == 0) {
            // This fragment has been marked as 'selected'
            FragData0.a = 0.2;
        }
        else {
            FragData0.a = 1.0;
        }
    }
}
