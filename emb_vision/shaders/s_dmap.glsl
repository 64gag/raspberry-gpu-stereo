varying vec2 tcoord;
uniform sampler2D tex_left;
uniform sampler2D tex_right;
uniform vec2 texelsize;

#define PIX_LEFT	0
#define PIX_CENTER	1
#define PIX_RIGHT	2
#define PIX_STATE	3

void main(void) 
{
	vec4 color;
	vec4 target;
	mat4 sampled_data;
	mat4 res_r, res_g, res_b, res_a;

	/* Sample data of interest */
	target = texture2D(tex_left, tcoord);
	sampled_data[PIX_LEFT] = texture2D(tex_right, tcoord + vec2(-1.0, 0.0) * texelsize);
	sampled_data[PIX_CENTER] = texture2D(tex_right, tcoord);
	sampled_data[PIX_RIGHT] = texture2D(tex_right, tcoord + vec2(1.0, 0.0) * texelsize);

	/* Compute similarity for R channel */
	res_r = sampled_data - target.r;
	res_r[PIX_LEFT] = abs(res_r[PIX_LEFT]);
	res_r[PIX_CENTER] = abs(res_r[PIX_CENTER]);
	res_r[PIX_RIGHT] = abs(res_r[PIX_RIGHT]);
	res_r[PIX_STATE] = vec4(25.0);

	/* Compute similarity for G channel */
	res_g = sampled_data - target.g;
	res_g[PIX_LEFT] = abs(res_g[PIX_LEFT]);
	res_g[PIX_CENTER] = abs(res_g[PIX_CENTER]);
	res_g[PIX_RIGHT] = abs(res_g[PIX_RIGHT]);

	/* Compute similarity for B channel */
	res_b = sampled_data - target.b;
	res_b[PIX_LEFT] = abs(res_b[PIX_LEFT]);
	res_b[PIX_CENTER] = abs(res_b[PIX_CENTER]);
	res_b[PIX_RIGHT] = abs(res_b[PIX_RIGHT]);

	/* Compute similarity for A channel */
	res_a = sampled_data - target.a;
	res_a[PIX_LEFT] = abs(res_a[PIX_LEFT]);
	res_a[PIX_CENTER] = abs(res_a[PIX_CENTER]);
	res_a[PIX_RIGHT] = abs(res_a[PIX_RIGHT]);

	/* Loop through calculated similarities searching for disparity */
	for(int row = 0; row < 4; row++){
		for(int col = 0; col < 3; col++){
			/* R channel */
			if(res_r[col][row] < res_r[PIX_STATE].r){
				res_r[PIX_STATE].r = res_r[col][row];
				color.r = abs(4.0-float((4*col)+row));
			}

			/* G channel */
			if(res_g[col][row] < res_r[PIX_STATE].g){
				res_r[PIX_STATE].g = res_g[col][row];
				color.g = abs(5.0-float((4*col)+row));
			}

			/* B channel */
			if(res_b[col][row] < res_r[PIX_STATE].b){
				res_r[PIX_STATE].b = res_b[col][row];
				color.b = abs(6.0-float((4*col)+row));
			}

			/* A channel */
			if(res_a[col][row] < res_r[PIX_STATE].a){
				res_r[PIX_STATE].a = res_a[col][row];
				color.a = abs(7.0-float((4*col)+row));
			}
		}
	}

	color /= 7.0;

 gl_FragColor = color;
}
