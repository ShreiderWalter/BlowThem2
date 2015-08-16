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

// Caelum Sky Dome fragment program in GLSL

//varying float incidenceAngleCos; using gl_TexCoord[1].x
//varying float yValue; using gl_TexCoord[1].y 
//varying float normalY; using gl_TexCoord[1].z
uniform vec4 hazeColour;
uniform float offset;
uniform sampler1D atmRelativeDepth;
uniform sampler2D gradientsMap;

// Complile time constants
const float sunlightScatteringFactor = 0.001;
const float sunlightScatteringLossFactor = 0.7;
const float atmLightAbsorptionFactor = 0.1;

const float fogDensity = 15.0;
const float invHazeHeight = 100.0;

void main()
{
	// Calculate fogExp
	float zValue = pow( clamp(1.0 - gl_TexCoord[1].z, 0.0, 1.0), invHazeHeight);
	float haze = 1.0 - clamp( pow(2.71828, -zValue * fogDensity), 0.0, 1.0);
//end do haze

	// Pass the colour
	gl_FragColor = texture2D(gradientsMap, gl_TexCoord[0].xy +
			vec2(offset, 0.0)) * gl_Color;

	// Sunlight inscatter
	if(gl_TexCoord[1].x > 0.0)
	{
		vec4 sunColour = vec4(3.0, 3.0, 3.0, 1.0);
		float incidenceAngleCosClamped = clamp(gl_TexCoord[1].x, 0.0, 1.0);
		
		// Calculate sunlight inscatter
		float scatteredSunlight = pow(incidenceAngleCosClamped,
				log(sunlightScatteringFactor * 0.5) / log(0.5));
		float absorption = clamp(atmLightAbsorptionFactor *
				(1.0 - texture1D(atmRelativeDepth, gl_TexCoord[1].y).r), 0.0, 1.0);
		sunColour = sunColour * (1.0 - absorption) * vec4(0.9, 0.5, 0.09, 1.0);
		vec4 sunlightInscatter = sunColour * scatteredSunlight;
		
		gl_FragColor.rgb += sunlightInscatter.rgb * (1.0 - sunlightScatteringLossFactor);
	}

//do haze
	gl_FragColor *= (1.0 - haze) + vec4(hazeColour.xyz, 1.0) * haze;
//end do haze
}
