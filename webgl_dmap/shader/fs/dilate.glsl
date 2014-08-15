precision mediump float;

uniform sampler2D stereo;

varying vec2 tc_ul;
varying vec2 tc_ur;
varying vec2 tc_dl;
varying vec2 tc_dr;

void main(void)
{
	vec4 ul = texture2D(stereo, tc_ul);
	vec4 ur = texture2D(stereo, tc_ur);
	vec4 dl = texture2D(stereo, tc_dl);
	vec4 dr = texture2D(stereo, tc_dr);

	gl_FragColor = min(min(ul, ur), min(dl, dr));
}
