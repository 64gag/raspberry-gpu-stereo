uniform vec2 texelsize;
uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

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

void main()
{
	vec4 pos = vertex;

	tc_cc.xy = vec2(pos.x, pos.y);

	tc_ul = tc_cc + vec2(-1.0, 1.0) * texelsize;
	tc_uc = tc_cc + vec2(0.0, 1.0) * texelsize;
	tc_ur = tc_cc + vec2(1.0, 1.0) * texelsize;
	tc_urr = tc_cc + vec2(2.0, 1.0) * texelsize;

	tc_cl = tc_cc + vec2(-1.0, 0.0) * texelsize;
	tc_cr = tc_cc + vec2(1.0, 0.0) * texelsize;
	tc_crr = tc_cc + vec2(2.0, 0.0) * texelsize;

	tc_dl = tc_cc + vec2(-1.0, -1.0) * texelsize;
	tc_dc = tc_cc + vec2(0.0, -1.0) * texelsize;
	tc_dr = tc_cc + vec2(1.0, -1.0) * texelsize;
	tc_drr = tc_cc + vec2(2.0, -1.0) * texelsize;

	tc_ddl = tc_cc + vec2(-1.0, -2.0) * texelsize;
	tc_ddc = tc_cc + vec2(0.0, -2.0) * texelsize;
	tc_ddr = tc_cc + vec2(1.0, -2.0) * texelsize;
	tc_ddrr = tc_cc + vec2(2.0, -2.0) * texelsize;

	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
