precision mediump float;

uniform sampler2D tex;
uniform vec2 texelsize;

varying vec2 tc;

void main(void) 
{
	vec4 color;

	color.r = texture2D(tex, tc).r;

	color.g = texture2D(tex, tc + vec2(0.0, 1.0) * texelsize).r;
	color.b = texture2D(tex, tc + vec2(0.0, 2.0) * texelsize).r;
	color.a = texture2D(tex, tc + vec2(0.0, 3.0) * texelsize).r;

 gl_FragColor = color;
}
