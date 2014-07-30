uniform vec2 texelsize;
uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc_ul;
varying vec2 tc_uc;
varying vec2 tc_ur;
varying vec2 tc_cl;
varying vec2 tc_cc;
varying vec2 tc_cr;
varying vec2 tc_dl;
varying vec2 tc_dc;
varying vec2 tc_dr;

void main()
{
	vec4 pos = vertex;

	tc_cc.xy = vec2(pos.x, pos.y);

	tc_ul = tc_cc + vec2(-0.5, 1.0) * texelsize;
	tc_uc = tc_cc + vec2(0.0, 1.0) * texelsize;
	tc_ur = tc_cc + vec2(0.5, 1.0) * texelsize;

	tc_cl = tc_cc + vec2(-0.5, 0.0) * texelsize;
	tc_cr = tc_cc + vec2(0.5, 0.0) * texelsize;

	tc_dl = tc_cc + vec2(-0.5, -1.0) * texelsize;
	tc_dc = tc_cc + vec2(0.0, -1.0) * texelsize;
	tc_dr = tc_cc + vec2(0.5, -1.0) * texelsize;

	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
