precision mediump float;

uniform sampler2D tex;

varying vec2 tc_l;
varying vec2 tc_c;
varying vec2 tc_r;
varying vec2 tc_rr;

void main(void)
{
	gl_FragColor = (texture2D(tex, tc_l) + 3.0*texture2D(tex, tc_c) + 3.0*texture2D(tex, tc_r) + texture2D(tex, tc_rr))/8.0;
}
