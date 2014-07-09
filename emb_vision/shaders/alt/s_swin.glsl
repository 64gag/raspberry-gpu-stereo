/* 5x5 window */
varying vec2 tcoord;
uniform sampler2D tex;
uniform vec2 texelsize;

void main(void) 
{

	vec4 color = vec4(0);

	vec4 cols_sum_l = vec4(0.0);
	vec4 cols_sum_c = vec4(0.0);
	vec4 cols_sum_r = vec4(0.0);


	for(int yoffset = -2; yoffset <= 2; yoffset++) {
		cols_sum_l += texture2D(tex, tcoord + vec2(-1.0, yoffset) * texelsize);
		cols_sum_c += texture2D(tex, tcoord + vec2(0.0, yoffset) * texelsize);
		cols_sum_r += texture2D(tex, tcoord + vec2(1.0, yoffset) * texelsize);
	}

	color.r = cols_sum_l.b + cols_sum_l.a + cols_sum_c.r + cols_sum_c.g + cols_sum_c.b;
	color.g = color.r - cols_sum_l.b + cols_sum_c.a;
	color.b = color.g - cols_sum_l.a + cols_sum_r.r;
	color.a = color.b - cols_sum_c.r + cols_sum_r.g;

	color /= 25.0;

    gl_FragColor = color;
}
