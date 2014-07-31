precision mediump float;

uniform sampler2D tex_left;
uniform sampler2D tex_right;
uniform sampler2D tex_dmap;
uniform vec2 texelsize;

varying vec2 tcoord;

#define PIX_LEFT	0
#define PIX_CENTER	1
#define PIX_RIGHT	2
#define PIX_SIM		3

void main(void)
{
	mat4 right;
	vec4 current = texture2D(tex_dmap, tcoord);
	vec4 left = texture2D(tex_left, tcoord);

//	float d = floor(current.r * 255.0 + 0.5);
	float d = current.r * 255.0;

	right[PIX_LEFT] = texture2D(tex_right, tcoord - vec2((d - 1.0) * texelsize.x, 0.0));
	right[PIX_CENTER] = texture2D(tex_right, tcoord - vec2(d * texelsize.x, 0.0));
	right[PIX_RIGHT] = texture2D(tex_right, tcoord - vec2((d + 1.0) * texelsize.x, 0.0));

	right[PIX_SIM] = abs(left.r - vec4(right[PIX_LEFT].r, right[PIX_CENTER].r, right[PIX_RIGHT].r, 1.0));

	if(right[PIX_SIM].r < right[PIX_SIM].b){
		if(right[PIX_SIM].r < right[PIX_SIM].g){
			d -= 1.0;
		}
	}else{
		if(right[PIX_SIM].b < right[PIX_SIM].g){
			d += 1.0;
		}
	}

	gl_FragColor = vec4(d / 255.0, d / 255.0, d / 255.0, 1.0);
//	gl_FragColor.rgb = gl_FragColor.rgb * 20.0;
}
