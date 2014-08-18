precision lowp float;

uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc;

void main(void) 
{
	vec4 pos = vertex;
	tc = pos.xy;
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
