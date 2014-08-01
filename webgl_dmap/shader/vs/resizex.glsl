uniform vec2 texelsize;
uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc_l;
varying vec2 tc_c;
varying vec2 tc_r;
varying vec2 tc_rr;
void main()
{
	vec4 pos = vertex;

	tc_c.xy = vec2(pos.x, pos.y);

	tc_l = tc_c + vec2(-1.0, 0.0) * texelsize;
	tc_r = tc_c + vec2(1.0, 0.0) * texelsize;
	tc_rr = tc_c + vec2(2.0, 0.0) * texelsize;

	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
