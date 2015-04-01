precision mediump float;

uniform sampler2D tex;

varying vec2 tc_ul;
varying vec2 tc_ur;
varying vec2 tc_dl;
varying vec2 tc_dr;

void main(void)
{
	gl_FragColor = abs( texture2D(tex, tc_ul) - texture2D(tex, tc_ur) -
				texture2D(tex, tc_dl) + texture2D(tex, tc_dr) );
}
