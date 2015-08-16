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

// Caelum Precipitation fragment program in GLSL

uniform sampler2D scene;
uniform sampler2D samplerPrec;

uniform float intensity;
uniform vec4 ambient_light_colour;

// - - corner
uniform vec4 corner1;
// + - corner
uniform vec4 corner2;
// - + corner
uniform vec4 corner3;
// + + corner
uniform vec4 corner4;

// The x and y coordinal deviations for all 3 layers of precipitation
uniform vec4 deltaX;
uniform vec4 deltaY;

uniform vec4 precColor;
const float R = 0.5;
// Cartesian to cylindrical coordinates
vec2 CylindricalCoordinates(vec4 dir)
{
	vec2 res;
	//cubical root is used to counteract top/bottom circle effect
	dir *= R / pow(length(dir.xz), 0.33);
	res.y = -dir.y;
	res.x = -atan(dir.z, dir.x);
	return res;
}

// Returns alpha value of a precipitation
// view_direction is the direction vector resulting from the eye direction,wind direction and possibly other factors
float Precipitation( vec2 cCoords, float intensity, vec2 delta)
{
	cCoords -= delta;
	vec4 raincol = texture2D(samplerPrec, cCoords);
	return (raincol.g < intensity ? raincol.r : 1.0);
}

//main entry point
void main()
{
      vec4 cor1ToCor3 = mix(corner1, corner3, gl_TexCoord[0].y);
      vec4 cor2ToCor4 = mix(corner2, corner4, gl_TexCoord[0].y);
	vec4 eye = mix( cor1ToCor3 , cor2ToCor4 , gl_TexCoord[0].x );
	
	vec4 scenecol = texture2D(scene, gl_TexCoord[0].xy);
	vec2 cCoords = CylindricalCoordinates(eye);
	float prec1 = Precipitation(cCoords, intensity / 4.0, vec2(deltaX.x, deltaY.x));
	float prec2 = Precipitation(cCoords, intensity / 4.0, vec2(deltaX.y, deltaY.y));
	float prec3 = Precipitation(cCoords, intensity / 4.0, vec2(deltaX.z, deltaY.z));
	float prec = min( min(prec1, prec2), prec3 );
	gl_FragColor = mix(precColor, scenecol, prec);	
}
