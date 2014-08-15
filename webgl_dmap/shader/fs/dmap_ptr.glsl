precision mediump float;

uniform sampler2D tex_left;
uniform sampler2D tex_right;
uniform sampler2D tex_dmap;
uniform vec2 texelsize;

varying vec2 tc;

#define PIX_LEFT	0
#define PIX_CENTER	1
#define PIX_RIGHT	2
#define PIX_SIM		3

void main(void)
{
	mat4 right;
	vec4 current = texture2D(tex_dmap, tc);
	vec4 left = texture2D(tex_left, tc);
	vec3 sim;

	float d = current.r * 255.0;

	vec3 sampled = vec3(texture2D(tex_right, tc - vec2((d - 1.0) * texelsize.x, 0.0)).r,
				texture2D(tex_right, tc - vec2(d * texelsize.x, 0.0)).r,
				texture2D(tex_right, tc - vec2((d + 1.0) * texelsize.x, 0.0)).r);
	
	sim = ceil(abs(left.r - sampled)*150.0) + vec3(0.0, 1.0, 2.0) / 255.0;

	d = current.r + fract(min(min(sim.r, sim.g), sim.b)) - 1.0/255.0;
	gl_FragColor = vec4(d, d, d, 1.0);
}
