precision lowp float;

uniform vec2 offset;
uniform vec2 scale;
uniform vec2 from;
uniform vec2 range;

attribute vec4 vertex;

varying vec2 tc;

void main(void) 
{
	vec4 pos = vertex;
	tc = pos.xy * range + from;
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
