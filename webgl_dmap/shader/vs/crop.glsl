uniform vec2 offset;
uniform vec2 scale;

attribute vec4 vertex;

varying vec2 tc;

void main(void) 
{
	vec4 pos = vertex;
	tc = pos.xy * vec2(0.6667, 0.8889) + vec2(0.1667, 0.0556);
	pos.xy = pos.xy * scale + offset;
	gl_Position = pos;
}

/* Get a centered 256x256 texture from any image size */
