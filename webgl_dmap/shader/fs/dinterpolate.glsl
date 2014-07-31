precision mediump float;

uniform sampler2D tex;

varying vec2 tc_ul;
varying vec2 tc_uc;
varying vec2 tc_ur;
varying vec2 tc_cl;
varying vec2 tc_cc;
varying vec2 tc_cr;
varying vec2 tc_dl;
varying vec2 tc_dc;
varying vec2 tc_dr;

void main(void)
{
	gl_FragColor = ( texture2D(tex, tc_ul) + texture2D(tex, tc_uc) + texture2D(tex, tc_ur) + 
			 texture2D(tex, tc_cl) + texture2D(tex, tc_cc) + texture2D(tex, tc_cr) +
			 texture2D(tex, tc_dl) + texture2D(tex, tc_dc) + texture2D(tex, tc_dr) ) * 2.0 / 9.0;
}
