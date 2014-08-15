var w = 384;
var h = 288;

/* These will be filled later */
var _texelsizes = [];
var _d;
var _k;

/* _ENUMS */
var _FEATURE = {
	RAW: 0,
	SWIN: 1,
	DMAP: 2
}

var _SIDE = {
	LEFT: 0,
	RIGHT: 1
}

var _VS = {
	SIMPLE: 0,
	KERNEL: 1,
	FLIP: 2,
	INTERPOLATE: 3,
	RESIZE: 4
};
var vs_paths = [ "vs/simple", "vs/kernel", "vs/flip", "vs/interpolate_xy", "vs/resizexy" ];

var _FS = {
	SIMPLE: 0,
	EDGE: 1,
	BLUR: 2,
	DMAP: 3,
	DINTERPOLATE: 4,
	DMAP_PTR: 5,
	CONTRAST: 6,
	RESIZE: 7,
	FLIP: 8
};
var fs_paths = [ "fs/simple", "fs/edge"+edge, "fs/blur"+blur, "fs/dmap", "fs/dinterpolate", "fs/dmap_ptr", "fs/contrast", "fs/resizexy1" ];

var PROGRAMS = [
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},
	{VS: _VS.KERNEL,	FS: _FS.EDGE},
	{VS: _VS.KERNEL,	FS: _FS.BLUR},
	{VS: _VS.SIMPLE,	FS: _FS.DMAP},
	{VS: _VS.INTERPOLATE,	FS: _FS.DINTERPOLATE},
	{VS: _VS.SIMPLE,	FS: _FS.DMAP_PTR},
	{VS: _VS.SIMPLE,	FS: _FS.CONTRAST},
	{VS: _VS.RESIZE,	FS: _FS.RESIZE},
	{VS: _VS.FLIP,		FS: _FS.SIMPLE}
];

var FBS = [
	{WDIV: 1, HDIV: 1},	/* O_L: 0 */
	{WDIV: 1, HDIV: 1},	/* O_R: 1 */
];

for(var i = pyramid; i >= 0; i--){
	var fbObj = {WDIV: Math.pow(2, i), HDIV: Math.pow(2, i)};
	for(var j = 0; j < 6; j++){
		FBS.push(fbObj);
	}
}

/* Controls of what you see */
var dtex = [6*pyramid + 2*_FEATURE.DMAP + _SIDE.LEFT + 3];
//var dtex = [2*_FEATURE.DMAP + _SIDE.LEFT + 2];
//var dtex = [0];
var rows = 1;
var cols = Math.ceil(dtex.length/rows);

function renderInit(gl, textures, framebuffers, programs)
{
	/* Set the uniforms of all programs */
	initialValues = [	{LVALUE: "offset", VALUE: [-1.0, -1.0]},
				{LVALUE: "scale", VALUE: [2.0, 2.0]},
				{LVALUE: "tex", VALUE: 0},
				{LVALUE: "tex_left", VALUE: 0},
				{LVALUE: "tex_right", VALUE: 1},
				{LVALUE: "tex_dmap", VALUE: 2}		];

	/* buffer has the quad coordinates, bind it */
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	
	for(var p = 0; p < programs.length; p++){
		gl.useProgram(programs[p]);
		setVariables(gl, programs[p], initialValues);

		/* Point to the buffer with the quad */
		var _vertex = gl.getAttribLocation(programs[p], "vertex");
		gl.vertexAttribPointer(_vertex, 4, gl.FLOAT, false, 16, 0);
		gl.enableVertexAttribArray(_vertex);

		/* Get texelsizes' locations */
		var _texelsize = gl.getUniformLocation(programs[p], "texelsize");
		_texelsizes.push(_texelsize);
	}

	/* Get locations of changing uniforms */
	_d = gl.getUniformLocation(programs[_FS.DMAP], "d");
	_k = gl.getUniformLocation(programs[_FS.CONTRAST], "k");
}

function render()
{
	console.time('RenderTime');
	/* Initialize to white */
	var dmap_index = 2*_FEATURE.DMAP + _SIDE.LEFT + 2;
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index]);
	gl.clearColor(1.0, 1.0, 1.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT);

	/* Copy the original image to its place at the pyramid */
	var left_index = 6*pyramid + 2*_FEATURE.RAW + _SIDE.LEFT + 2;
	var right_index = 6*pyramid + 2*_FEATURE.RAW + _SIDE.RIGHT + 2;

	gl.useProgram(programs[_FS.SIMPLE]);
	gl.uniform2f(_texelsizes[_FS.SIMPLE], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
	runShader(gl, textures[_SIDE.LEFT], framebuffers[left_index]);
	runShader(gl, textures[_SIDE.RIGHT], framebuffers[right_index]);

	/* Generate the pyramid and calculate the support windows */
	for(var level = pyramid; level > 0; level--){
		left_index = 6*level + 2*_FEATURE.RAW + _SIDE.LEFT + 2;
		right_index = 6*level + 2*_FEATURE.RAW + _SIDE.RIGHT + 2;

		gl.useProgram(programs[_FS.RESIZE]);
		gl.uniform2f(_texelsizes[_FS.RESIZE], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
		runShader(gl, textures[left_index], framebuffers[left_index - 6]);
		runShader(gl, textures[right_index], framebuffers[right_index - 6]);

		gl.useProgram(programs[_FS.BLUR]);
		gl.uniform2f(_texelsizes[_FS.BLUR], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
		runShader(gl, textures[left_index], framebuffers[left_index + 2]);
		runShader(gl, textures[right_index], framebuffers[right_index + 2]);
	}

	/* Last support window... (top of pyramid, out of the loop above) ***USE BLUR PROGRAM*** */
	gl.uniform2f(_texelsizes[_FS.BLUR], 1.0/textures[left_index - 6].width, 1.0/textures[left_index - 6].height);
	runShader(gl, textures[left_index - 6], framebuffers[left_index - 4], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR]);
	runShader(gl, textures[right_index - 6], framebuffers[right_index - 4], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR]);

	/* Lowest resolution passes */
	left_index = 2*_FEATURE.SWIN + _SIDE.LEFT + 2;
	right_index = 2*_FEATURE.SWIN + _SIDE.RIGHT + 2;

	gl.useProgram(programs[_FS.DMAP]);
	gl.uniform2f(_texelsizes[_FS.DMAP], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
	for(var x = d_0; x < d_times; x++){
		gl.uniform1f(_d, x);
		runShaderDisparity(gl, textures[left_index], textures[right_index], textures[dmap_index], framebuffers[dmap_index]);
	}

	/* Grow the previous disparity map, calculate new disparities, loop */
	for(var level = 1; level <= pyramid; level++){
		left_index = 6*level + 2*_FEATURE.SWIN + _SIDE.LEFT + 2;
		right_index = 6*level + 2*_FEATURE.SWIN + _SIDE.RIGHT + 2;
		dmap_index = 6*level + 2*_FEATURE.DMAP + _SIDE.LEFT + 2;

		gl.useProgram(programs[_FS.DINTERPOLATE]);
		gl.uniform2f(_texelsizes[_FS.DINTERPOLATE], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
		runShader(gl, textures[dmap_index - 6], framebuffers[dmap_index]);

		gl.useProgram(programs[_FS.DMAP_PTR]);
		gl.uniform2f(_texelsizes[_FS.DMAP_PTR], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
		runShaderDisparity(gl, textures[left_index], textures[right_index], textures[dmap_index], framebuffers[dmap_index]);
	}

	/* Increase contrast for visualization */
	gl.useProgram(programs[_FS.CONTRAST]);
	for(var level = 0; level <= pyramid; level++){
		gl.uniform1f(_k, 5*(pyramid-level+1));
		dmap_index = 6*level + 2*_FEATURE.DMAP + _SIDE.LEFT + 2;
		gl.uniform2f(_texelsizes[_FS.CONTRAST], 1.0/textures[dmap_index].width, 1.0/textures[dmap_index].height);
		runShader(gl, textures[dmap_index], framebuffers[dmap_index + 1]);
	}

	renderGrid();
	console.timeEnd('RenderTime');
	window.requestAnimationFrame(render);
}
