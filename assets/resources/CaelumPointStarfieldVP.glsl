/*
This file is part of Caelum.
See http://www.ogre3d.org/wiki/index.php/Caelum 

Copyright (c) 2008 Caelum team. See Contributors.txt for details.

Caelum is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Caelum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Caelum. If not, see <http://www.gnu.org/licenses/>.
*/

uniform float mag_scale;
uniform float mag0_size;
uniform float min_size;
uniform float max_size;
// width/height
uniform float aspect_ratio;

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;

    float size = exp(mag_scale * gl_MultiTexCoord0.z) * mag0_size;

    // Fade below minSize.
    float fade = clamp(size / min_size, 0.0, 1.0);
    gl_FrontColor = vec4(vec3(1.0, 1.0, 1.0), fade * fade);

    // Clamp size to range.
    size = clamp(size, min_size, max_size);

    // Splat the billboard on the screen.
    // Clip-space output has w=1 and thus no perspective divide happens.
    gl_Position.xyz /= gl_Position.w;
    gl_Position.w = 1.0;
    gl_Position.xy += gl_MultiTexCoord0.xy * vec2(size, size * aspect_ratio);
}
