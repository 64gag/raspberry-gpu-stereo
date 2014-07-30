precision mediump float;

uniform sampler2D tex;

varying vec2 tcoord;

void main(void)
{
	gl_FragColor = texture2D(tex, tcoord);
}
