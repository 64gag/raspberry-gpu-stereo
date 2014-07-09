varying vec2 tcoord;
uniform sampler2D tex;
uniform vec2 texelsize;

void main(void) 
{
	vec4 color;

	color.r = texture2D(tex, tcoord).r;
	color.g = texture2D(tex, tcoord + vec2(1, 0) * texelsize).r;
	color.b = texture2D(tex, tcoord + vec2(2, 0) * texelsize).r;
	color.a = texture2D(tex, tcoord + vec2(3, 0) * texelsize).r;

 gl_FragColor = color;
}
