precision mediump float;

uniform sampler2D tex;

varying vec2 tc_ul;
varying vec2 tc_uc;
varying vec2 tc_ur;
varying vec2 tc_cl;
varying vec2 tc;
varying vec2 tc_cr;
varying vec2 tc_dl;
varying vec2 tc_dc;
varying vec2 tc_dr;

void main(void)
{
	gl_FragColor = abs( 	texture2D(tex, tc_uc) +
				texture2D(tex, tc_cl) - 4.0*texture2D(tex, tc) +texture2D(tex, tc_cr) +
				texture2D(tex, tc_dc) );
}
