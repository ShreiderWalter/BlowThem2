/*
This file is part of Caelum.
See http://www.ogre3d.org/wiki/index.php/Caelum 

Copyright (c) 2006-2007 Caelum team. See Contributors.txt for details.

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

// Get how much of a certain point on the moon is seen (or not) because of the phase.
// uv is the rect position on moon; as seen from the earth.
// phase ranges from 0 to 2

float MoonPhaseFactor(vec2 uv, float phase)
{
   float alpha = 1.0;

   float srefx = uv.x - 0.5;
   float refx = abs(uv.x - 0.5);
   float refy = abs(uv.y - 0.5);
   float refxfory = sqrt(0.25 - refy * refy);
   float xmin = -refxfory;
   float xmax = refxfory;
   float xmin1 = (xmax - xmin) * (phase / 2.0) + xmin;
   float xmin2 = (xmax - xmin) * phase + xmin;
   if (srefx < xmin1)
   {
      alpha = 0.0;
   }
   else
   {
      if (srefx < xmin2 && xmin1 != xmin2)
      {
         alpha = (srefx - xmin1) / (xmin2 - xmin1);
      }
   }

   return alpha;
}

uniform float phase;
uniform sampler2D moonDisc;

void main()
{
    gl_FragColor = texture2D(moonDisc, gl_TexCoord[0].xy);
    float alpha = MoonPhaseFactor(gl_TexCoord[0].xy, phase);

    // Get luminance from the texture.
    float lum = dot(gl_FragColor.rgb, vec3(0.3333, 0.3333, 0.3333));
    gl_FragColor.a = min(gl_FragColor.a, lum * alpha);
    gl_FragColor.rgb /= lum;
}
