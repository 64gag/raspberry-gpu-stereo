precision mediump float;

uniform sampler2D tex;

varying vec2 tc_ul;
varying vec2 tc_uc;
varying vec2 tc_ur;
varying vec2 tc_urr;

varying vec2 tc_cl;
varying vec2 tc_cc;
varying vec2 tc_cr;
varying vec2 tc_crr;

varying vec2 tc_dl;
varying vec2 tc_dc;
varying vec2 tc_dr;
varying vec2 tc_drr;

varying vec2 tc_ddl;
varying vec2 tc_ddc;
varying vec2 tc_ddr;
varying vec2 tc_ddrr;

void main(void)
{
	gl_FragColor = ( texture2D(tex, tc_ul) + 3.0*texture2D(tex, tc_uc) + 3.0*texture2D(tex, tc_ur) + texture2D(tex, tc_urr) +
			 3.0*texture2D(tex, tc_cl) + 9.0*texture2D(tex, tc_cc) + 9.0*texture2D(tex, tc_cr) + 3.0*texture2D(tex, tc_crr) +
			 3.0*texture2D(tex, tc_dl) + 9.0*texture2D(tex, tc_dc) + 9.0*texture2D(tex, tc_dr) + 3.0*texture2D(tex, tc_drr) +
			 texture2D(tex, tc_ddl) + 3.0*texture2D(tex, tc_ddc) + 3.0*texture2D(tex, tc_ddr) + texture2D(tex, tc_ddrr)) / 64.0;
}
