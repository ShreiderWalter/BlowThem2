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

// Caelum Sky Dome Haze fragment program in GLSL

varying float haze;
varying vec2 sunlight;
uniform vec4 fogColour;
uniform sampler1D atmRelativeDepth;

float fogExp(float z, float density)
{
	return 1.0 - clamp( pow(2.71828, -z * density), 0.0, 1.0);
}

void main()
{
	float incidenceAngleCos = sunlight.x;
	float y = sunlight.y;

	vec4 sunColour = vec4(3.0, 2.5, 1.0, 1.0);

    // Factor determining the amount of light lost due to absorption
	float atmLightAbsorptionFactor = 0.1; 
	float fogDensity = 15.0;

	float newHaze = fogExp(haze * 0.005, atmLightAbsorptionFactor);

	// Haze amount calculation
	float invHazeHeight = 100.0;
	float hazeAbsorption = fogExp( pow(1.0 - y, invHazeHeight), fogDensity);

	vec4 hazeColour;
	hazeColour = fogColour;
	if(incidenceAngleCos > 0.0)
	{
		float incidenceAngleCosClamped = clamp(incidenceAngleCos, 0.0, 1.0);
		
        // Factor determining the amount of scattering for the sun light
		float sunlightScatteringFactor = 0.1;
        // Factor determining the amount of sun light intensity lost due to scattering
		float sunlightScatteringLossFactor = 0.3;	

		// Calculate sunlight inscatter
		float absorption = (1.0 - texture1D(atmRelativeDepth, y).r) * hazeAbsorption;
		absorption = clamp(absorption, 0.0, 1.0);
		float scatteredSunlight = pow(incidenceAngleCosClamped,
				log(sunlightScatteringFactor * 0.5) / log(0.5));
		sunColour = sunColour * (1.0 - absorption) * vec4(0.9, 0.5, 0.09, 1.0);
		vec4 sunlightInscatter = sunColour * scatteredSunlight;
		
		vec4 sunlightInscatterColour = sunlightInscatter *
				(1.0 - sunlightScatteringLossFactor);
		hazeColour.rgb =
                hazeColour.rgb * (1.0 - sunlightInscatterColour.a) +
                sunlightInscatterColour.rgb * sunlightInscatterColour.a * newHaze;
	}

	gl_FragColor = hazeColour;
	gl_FragColor.a = newHaze;
}
