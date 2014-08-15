precision mediump float;

uniform sampler2D tex_left;
uniform sampler2D tex_right;
uniform sampler2D tex_dmap;
uniform float d;
uniform vec2 texelsize;

varying vec2 tc;

void main(void)
{
	vec4 left = texture2D(tex_left, tc);
	vec4 right = texture2D(tex_right, tc - vec2(d * texelsize.x, 0.0));
	vec4 current = texture2D(tex_dmap, tc);

	float similarity = abs(left.r - right.r);

	if(similarity < current.g){
		gl_FragColor = vec4(d / 255.0, similarity, 0.0, 1.0);
	}else{
		gl_FragColor = current;
	}
}
