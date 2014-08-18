precision lowp float;

uniform sampler2D stereo;
uniform sampler2D dmap;
uniform vec2 texelsize;

varying vec2 tc;

void main(void)
{
	float matching = texture2D(stereo, tc).r;
	float dfloat = texture2D(dmap, tc).r;
	mediump float d = dfloat * 255.0;

	vec3 candidates = vec3(	texture2D(stereo, tc - vec2((d - 1.0) * texelsize.x, 0.0)).g,
				texture2D(stereo, tc - vec2(d * texelsize.x, 0.0)).g,
				texture2D(stereo, tc - vec2((d + 1.0) * texelsize.x, 0.0)).g);

	mediump vec3 sim = ceil(abs(matching - candidates) * 150.0);
	sim.g += 0.0039;
	sim.b += 0.0078;

	gl_FragColor = vec4(dfloat + fract(min(min(sim.r, sim.g), sim.b)) - 0.0039, 0.0, 0.0, 1.0);
}
