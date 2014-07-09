varying vec2 tcoord;
uniform sampler2D tex;
uniform vec2 texelsize;

void main(void) 
{
	vec4 color;
	float gray_val = 0.0;



	gray_val = mod(tcoord.x, texelsize.x) + 0.001;

	if(gray_val < 0.003125){
		gray_val = texture2D(tex, tcoord).r;
	}else if(gray_val < 0.00625){
		gray_val = texture2D(tex, tcoord).g;
	}else if(gray_val < 0.009375){
		gray_val = texture2D(tex, tcoord).b;
	}else{
		gray_val = texture2D(tex, tcoord).a;
	}

	color.rgb = vec3(gray_val);
	color.a = 1.0;

 gl_FragColor = color;
}
