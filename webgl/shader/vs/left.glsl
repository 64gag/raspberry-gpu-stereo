precision lowp float;

uniform vec2 texelsize;
uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc;
varying vec2 tc_l;

void main(void) 
{
	vec4 pos = vertex;
	tc = pos.xy;
	tc_l = tc + vec2(-1.0, 0.0) * texelsize;
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
