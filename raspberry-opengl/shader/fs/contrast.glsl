precision mediump float;

uniform sampler2D tex;
uniform float k;

varying vec2 tc;

void main(void)
{
	vec4 current = texture2D(tex, tc);
	current.r *= k;
	current.gb = current.rr;

	gl_FragColor = current; /* Amplified */
}
