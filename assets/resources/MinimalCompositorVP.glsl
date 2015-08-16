//
// This file is part of Caelum.
// See http://www.ogre3d.org/wiki/index.php/Caelum 
// 
// Copyright (c) 2008 Caelum team. See Contributors.txt for details.
// 
// Caelum is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Caelum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with Caelum. If not, see <http://www.gnu.org/licenses/>.
//

// Fixed function does not always work.
// This is a the minimal compositor VP required.
void main()
{
    // Use standard transform.
    gl_Position = ftransform();

    // Convert to image-space
    vec4 in_pos = gl_Vertex;
    in_pos.xy = sign(in_pos.xy);
    gl_TexCoord[0].xy = (vec2(in_pos.x, -in_pos.y) + 1.0) * 0.5;    
}
