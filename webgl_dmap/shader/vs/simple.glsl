uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tcoord;

void main(void) 
{
	vec4 pos = vertex;
	tcoord.xy = vec2(pos.x, pos.y);
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}
