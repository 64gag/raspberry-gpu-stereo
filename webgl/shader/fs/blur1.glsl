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
	gl_FragColor = ( texture2D(tex, tc_ul) + 2.0*texture2D(tex, tc_uc) + texture2D(tex, tc_ur) +
			 2.0*texture2D(tex, tc_cl) + 4.0*texture2D(tex, tc) + 2.0*texture2D(tex, tc_cr) +
			 texture2D(tex, tc_dl) + 2.0*texture2D(tex, tc_dc) + texture2D(tex, tc_dr)) / 16.0;
}
