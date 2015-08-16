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

// Caelum Sky Dome vertex program in GLSL

//varying float incidenceAngleCos; using gl_TexCoord[1].x
//varying float yValue; using gl_TexCoord[1].y 
//varying float normalY; using gl_TexCoord[1].z
uniform vec3 sunDirection;

void main()
{
	vec3 newSunDirection = normalize(sunDirection);
	vec3 normal = normalize(gl_Normal);
	float cosine = dot(-newSunDirection, normal);
	gl_TexCoord[1].z = -normal.y;
	
	gl_TexCoord[1].x = -cosine;
	gl_TexCoord[1].y = -newSunDirection.y;

	gl_Position = ftransform();
	gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0);
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
