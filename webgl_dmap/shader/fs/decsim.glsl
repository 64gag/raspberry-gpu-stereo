precision mediump float;

uniform sampler2D tex;

varying vec2 tcoord;

void main(void)
{
	vec4 encoded = texture2D(tex, tcoord);
	encoded.rb = encoded.gg;

	gl_FragColor = encoded; /* Decoded */
}
