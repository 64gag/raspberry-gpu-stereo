precision lowp float;

uniform sampler2D tex;

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
	gl_FragColor = max(
	max(max(texture2D(tex, tc), texture2D(tex, tc_p1)), max(texture2D(tex, tc_p2), texture2D(tex, tc_p3))),
	max(max(texture2D(tex, tc_m1), texture2D(tex, tc_m2)), max(texture2D(tex, tc_m3), texture2D(tex, tc_m4)))
	);
}
