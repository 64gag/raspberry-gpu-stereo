uniform vec2 texelsize;
uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc;
varying vec2 tc_p1;
varying vec2 tc_p2;
varying vec2 tc_p3;
varying vec2 tc_m1;
varying vec2 tc_m2;
varying vec2 tc_m3;
varying vec2 tc_m4;

void main(void) 
{
	vec4 pos = vertex;
	tc = pos.xy;
	tc_p1 = tc + vec2(1.0) * texelsize;
	tc_p2 = tc + vec2(2.0) * texelsize;
	tc_p3 = tc + vec2(3.0) * texelsize;
	tc_m1 = tc + vec2(-1.0) * texelsize;
	tc_m2 = tc + vec2(-2.0) * texelsize;
	tc_m3 = tc + vec2(-3.0) * texelsize;
	tc_m4 = tc + vec2(-4.0) * texelsize;
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
