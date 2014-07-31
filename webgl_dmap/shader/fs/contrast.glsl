precision mediump float;

uniform sampler2D tex;
uniform float k;

varying vec2 tcoord;

void main(void)
{
	vec4 current = texture2D(tex, tcoord);
	current.r = current.r * k;
	current.gb = current.rr;

	gl_FragColor = current; /* Amplified */
}
