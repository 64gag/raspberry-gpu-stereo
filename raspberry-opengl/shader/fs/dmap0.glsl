precision lowp float;

uniform sampler2D stereo;

varying vec2 tc;
varying vec2 tc_l;

void main(void)
{
	vec4 origin = texture2D(stereo, tc);
	float matching = origin.r;
	vec2 candidates = vec2(texture2D(stereo, tc_l).g, origin.g);
	mediump vec2 sim = ceil(abs(matching - candidates) * 150.0);
	sim.r += 0.0039;

	gl_FragColor = vec4(fract(min(sim.r, sim.g)), 0.0, 0.0, 1.0);
}
