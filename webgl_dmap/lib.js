var buffer;
var screen_w;
var screen_h;

// Following loadShaders() function is from: http://webreflection.blogspot.fr/2010/09/fragment-and-vertex-shaders-my-way-to.html
function loadShaders(e,a,r){function t(){var t=this,n=t.i;if(4==t.readyState){if(a[n]=e.createShader("fs"==a[n].slice(0,2)?e.FRAGMENT_SHADER:e.VERTEX_SHADER),e.shaderSource(a[n],t.responseText),e.compileShader(a[n]),!e.getShaderParameter(a[n],e.COMPILE_STATUS))throw e.getShaderInfoLog(a[n]);!--d&&"function"==typeof r&&r(a)}}for(var n,a=[].concat(a),o=!!r,s=a.length,d=s;s--;)(n=new XMLHttpRequest).i=s,n.open("get",loadShaders.base+a[s]+".glsl",o),o&&(n.onreadystatechange=t),n.send(null),t.call(n);return a}loadShaders.base="shader/";

function main()
{
  /* Get left image */
  var image_left = new Image();
  image_left.onload = function() {
	/* Now get the right one */
	var image_right = new Image();
	image_right.onload = function() {
		/* Ready to go! */
		render(image_left, image_right);
	}
	image_right.src = "img_right.bmp";
  }
  image_left.src = "img_left.bmp";
}

function buildProgram(gl, vs, fs)
{
	var p = gl.createProgram();
	/* console.log(p); */
	gl.attachShader(p, vs);
	gl.attachShader(p, fs);  
	gl.linkProgram(p);

	return p;
}

function DrawTextureRect(gl, texture, target, x0, y0, x1, y1, proggy)
{
	gl.bindTexture(gl.TEXTURE_2D, texture);
	gl.useProgram(proggy);

	if (target){
		gl.bindFramebuffer(gl.FRAMEBUFFER, target);
		gl.viewport (0, 0, target.width, target.height);
	}

	/* Get locations */
	var _vertex = gl.getAttribLocation(proggy, "vertex");
	var _offset = gl.getUniformLocation(proggy, "offset");
	var _scale = gl.getUniformLocation(proggy, "scale");
	var _texelsize = gl.getUniformLocation(proggy, "texelsize");
	var _tex = gl.getUniformLocation(proggy, "tex");

	/* Set the uniforms */
	gl.uniform2f(_offset, x0, y0);
	gl.uniform2f(_scale, x1-x0, y1-y0);
	gl.uniform2f(_texelsize, 1.0/texture.width, 1.0/texture.height);
	gl.uniform1i(_tex, 0);


	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	gl.vertexAttribPointer(_vertex, 4, gl.FLOAT, false, 16, 0);
	gl.enableVertexAttribArray(_vertex);

	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	gl.bindFramebuffer(gl.FRAMEBUFFER, null);

	if(target){
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.viewport(0, 0, screen_w, screen_h);
	}
}

function DrawDisparityRect(gl, tex_left, tex_right, tex_dmap, target, x0, y0, x1, y1, proggy, d)
{
	gl.bindFramebuffer(gl.FRAMEBUFFER, target);
	gl.viewport (0, 0, target.width, target.height);

	gl.useProgram(proggy);

	/* Get locations */
	var _vertex = gl.getAttribLocation(proggy, "vertex");
	var _offset = gl.getUniformLocation(proggy, "offset");
	var _scale = gl.getUniformLocation(proggy, "scale");
	var _texelsize = gl.getUniformLocation(proggy, "texelsize");
	var _d = gl.getUniformLocation(proggy, "d");
	var _tex_left = gl.getUniformLocation(proggy, "tex_left");
	var _tex_right = gl.getUniformLocation(proggy, "tex_right");
	var _tex_dmap = gl.getUniformLocation(proggy, "tex_dmap");

	/* Set the uniforms */
	gl.uniform2f(_offset, x0, y0);
	gl.uniform2f(_scale, x1-x0, y1-y0);
	gl.uniform2f(_texelsize, 1.0/tex_left.width, 1.0/tex_left.height);  
	gl.uniform1f(_d, d);
	gl.uniform1i(_tex_left, 0);
	gl.uniform1i(_tex_right, 1);
	gl.uniform1i(_tex_dmap, 2);

	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, tex_left);
	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, tex_right);
	gl.activeTexture(gl.TEXTURE2);
	gl.bindTexture(gl.TEXTURE_2D, tex_dmap);
	gl.activeTexture(gl.TEXTURE0);

	gl.vertexAttribPointer(_vertex, 4, gl.FLOAT, false, 16, 0);
	gl.enableVertexAttribArray(_vertex);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	gl.bindFramebuffer(gl.FRAMEBUFFER, null);

	gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	gl.viewport(0, 0, screen_w, screen_h);
}

function render(image_left, image_right)
{
	/* Set up the "screen" canvas */
	var canvObj=document.getElementById("screen");
	var w = image_left.width;
	var h = image_left.height;
	screen_w = w * cols; canvObj.width = screen_w;
	screen_h = h * rows; canvObj.height = screen_h;

	/* Get the Webgl instance */
	try {
		var gl = canvObj.getContext("experimental-webgl", { preserveDrawingBuffer: true });
	} catch (e) {
		alert("No Webgl support!");
		return false;
	};

	/* Create the buffer for the quad */
	buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([0.0, 0.0, 1.0, 1.0,
		1.0, 0.0, 1.0, 1.0,
		0.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0
		]), gl.STATIC_DRAW);

	/* Get and load shaders */
	var v_shaders = loadShaders(gl, vs_paths);
	var f_shaders = loadShaders(gl, fs_paths);

	/* Create programs, textures and framebuffers */
	var programs = [];
	var textures = [];	
	var framebuffers = [];

	/* Programs */
	for (var i = 0; i < PROGRAMS.length; i++) {
		var program = buildProgram(gl, v_shaders[PROGRAMS[i].VS], f_shaders[PROGRAMS[i].FS]);
		programs.push(program);
	}

	/* Textures and framebuffers */
	for (var i = 0; i < _FBS.length; i++) {
		/* Texture */
		var texture = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

		if(i == 0){
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image_left);
			texture.width = image_left.width;
		}else if(i == 1){
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image_right);
			texture.width = image_right.width;
		}else{
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, image_left.width/_FBS[i].WDIV, image_left.height/_FBS[i].WDIV, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);		
			texture.width = image_left.width/_FBS[i].WDIV;
		}
		texture.height = image_left.height

		/* Framebuffer */
		var framebuffer = gl.createFramebuffer();
		framebuffer.width = image_left.width/_FBS[i].WDIV;
		framebuffer.height = image_left.height/_FBS[i].WDIV;
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
		gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);

		/* Add to array */
		textures.push(texture);
		framebuffers.push(framebuffer);
	}
	console.time('Drawing time');
	/* Method-specific drawing... */
	Draw(gl, textures, framebuffers, programs);
	console.timeEnd('Drawing time');

	/* Draw the grid */
//	var tex = rows * cols - 1;
	var tex = 0;
	var grid_col_size = 2.0 / cols;
	var grid_row_size = 2.0 / rows;

	for(var row = 0; row < rows; row++){
//		for(var col = cols - 1; col >= 0; col--){
		for(var col = 0; col < cols; col++){
			var colx = -1.0 + col * grid_col_size;
			var rowy = -1.0 + row * grid_row_size;
			DrawTextureRect(gl, textures[tex++], null, colx, rowy, colx + grid_col_size, rowy + grid_row_size, programs[0]);
		}
	}
}
