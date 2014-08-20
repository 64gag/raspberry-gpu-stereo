precision lowp float;

uniform sampler2D R;
uniform sampler2D G;
uniform vec2 texelsize;

varying vec2 tc;

void main(void)
{
	gl_FragColor = vec4(texture2D(R, tc).r, texture2D(G, tc).r, 0.0, 1.0);
}
