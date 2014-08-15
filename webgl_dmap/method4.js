var w = 256;
var h = 256;

var left_index = 0;
var right_index = 1;
var stereo_index = 2;
var smallest_index = 3;
var dmap_index = smallest_index + pyramid;

/* These will be filled later */
var _texelsizes = [];
var _k;

var _VS = {
	SIMPLE: 0,
	CROP: 1,
	INTERPOLATE: 2,
	FLIP: 3,
	LEFT: 4,
	DIAGS: 5,
	RIGHT8: 6
};
var vs_paths = [ "vs/simple", "vs/crop", "vs/interpolate_xy", "vs/flip", "vs/left", "vs/diagonals", "vs/line8" ];

var _FS = {
	SIMPLE: 0,
	PACK: 1,
	DMAP0: 2,
	DINTERPOLATE: 3,
	DMAPN: 4,
	CONTRAST: 5,
	DILATE: 6,
	MAX8: 7,
	FLIP: 8
};
var fs_paths = [ "fs/simple", "fs/channelpack", "fs/dmap0", "fs/dinterpolate", "fs/dmapN", "fs/contrast", "fs/dilate", "fs/max8" ];

var PROGRAMS = [
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},
	{VS: _VS.CROP,		FS: _FS.PACK},
	{VS: _VS.LEFT,		FS: _FS.DMAP0},
	{VS: _VS.INTERPOLATE,	FS: _FS.DINTERPOLATE},
	{VS: _VS.SIMPLE,	FS: _FS.DMAPN},
	{VS: _VS.SIMPLE,	FS: _FS.CONTRAST},
	{VS: _VS.DIAGS,		FS: _FS.DILATE},
	{VS: _VS.RIGHT8,	FS: _FS.MAX8},
	{VS: _VS.FLIP,		FS: _FS.SIMPLE}

];

var FBS = [
	{WDIV: 1, HDIV: 1},	/* O_L: 0 */
	{WDIV: 1, HDIV: 1},	/* O_R: 1 */
	{WDIV: 1, HDIV: 1}	/* Cropped */
];

for(var i = pyramid; i >= 0; i--){
	var div = Math.pow(2, i);
	var fbObj = {WDIV: div, HDIV: div}; /* Pyramid of disparity maps */
	FBS.push(fbObj);
}

var fbObj = {WDIV: 16, HDIV: 16};	/* Filtered disparity map */
FBS.push(fbObj);
var fbObj = {WDIV: 128, HDIV: 16};	/* Horizontal maximum */
FBS.push(fbObj);
var fbObj = {WDIV: 128, HDIV: 128};	/* Vertical maximum */
FBS.push(fbObj);

/* Controls of what you see */
var dtex = [10];
//var dtex = [2, 3, 4, 5, 6, 7, 8, 9, 10];
var cols = 1;
var rows = Math.ceil(dtex.length/cols);

function renderInit(gl, textures, framebuffers, programs)
{
	/* Set the uniforms of all programs */
	initialValues = [	{LVALUE: "offset", VALUE: [-1.0, -1.0]},
				{LVALUE: "scale", VALUE: [2.0, 2.0]},
				{LVALUE: "dmap", VALUE: 1},
				{LVALUE: "range", VALUE: [w/textures[0].width, h/textures[0].height]},
				{LVALUE: "from", VALUE: [(textures[0].width - w) / 2 / textures[0].width, 
							(textures[0].height - h) / 2 / textures[0].height]},
				{LVALUE: "G", VALUE: 1}		];

	for(var p = 0; p < programs.length; p++){
		gl.useProgram(programs[p]);
		setVariables(gl, programs[p], initialValues);
		/* Get texelsizes' locations */
		var _texelsize = gl.getUniformLocation(programs[p], "texelsize");
		_texelsizes.push(_texelsize);
	}

	/* Set uniforms that does not change */
	gl.useProgram(programs[_FS.PACK]);
	gl.uniform2f(_texelsizes[_FS.PACK], 1.0/textures[left_index].width, 1.0/textures[left_index].height);
	gl.useProgram(programs[_FS.DMAP0]);
	gl.uniform2f(_texelsizes[_FS.DMAP0], 1.0/textures[smallest_index].width, 1.0/textures[smallest_index].height);
	gl.useProgram(programs[_FS.DILATE]);
	gl.uniform2f(_texelsizes[_FS.DILATE], 1.0/128.0, 1.0/128.0);

	/* Get locations of changing uniforms */
	_k = gl.getUniformLocation(programs[_FS.CONTRAST], "k");
}

function render()
{
	/* Capture POT region and pack to a single texture */
	gl.useProgram(programs[_FS.PACK]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[stereo_index]);
	gl.viewport (0, 0, framebuffers[stereo_index].width, framebuffers[stereo_index].height);
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, textures[left_index]);
	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, textures[right_index]);
	gl.activeTexture(gl.TEXTURE0);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Generate the mipmaps */
	gl.bindTexture(gl.TEXTURE_2D, textures[stereo_index]);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
	gl.hint(gl.GENERATE_MIPMAP_HINT, gl.FASTEST);
	gl.generateMipmap(gl.TEXTURE_2D);

	/* Initialize to white */
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[smallest_index]);
	gl.clear(gl.COLOR_BUFFER_BIT);

	/* Lowest resolution pass */
	gl.useProgram(programs[_FS.DMAP0]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[smallest_index]);
	gl.viewport (0, 0, framebuffers[smallest_index].width, framebuffers[smallest_index].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[stereo_index]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Grow the previous disparity map, calculate new disparities, loop */
	for(var level = 0; level < pyramid; level++){
		/* Grow the previous */
		var grow_index = smallest_index + level;
		var grown_index = grow_index + 1;
		gl.useProgram(programs[_FS.DINTERPOLATE]);
		gl.uniform2f(_texelsizes[_FS.DINTERPOLATE], 1.0/textures[grow_index].width, 1.0/textures[grow_index].height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[grown_index]);
		gl.viewport (0, 0, framebuffers[grown_index].width, framebuffers[grown_index].height);
		gl.bindTexture(gl.TEXTURE_2D, textures[grow_index]);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

		/* Calculate new disparities */
		gl.useProgram(programs[_FS.DMAPN]);
		gl.uniform2f(_texelsizes[_FS.DMAPN], 1.0/textures[grown_index].width, 1.0/textures[grown_index].height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[grown_index]);
		gl.viewport (0, 0, framebuffers[grown_index].width, framebuffers[grown_index].height);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, textures[stereo_index]);
		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(gl.TEXTURE_2D, textures[grown_index]);
		gl.activeTexture(gl.TEXTURE0);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	/* Increase contrast for visualization */
	gl.useProgram(programs[_FS.CONTRAST]);
	for(var level = 0; level <= pyramid; level++){
		var contrast_index = smallest_index + level;
		gl.uniform1f(_k, 4*(pyramid-level+1));
		gl.uniform2f(_texelsizes[_FS.CONTRAST], 1.0/textures[contrast_index].width, 1.0/textures[contrast_index].height);
		runShader(gl, textures[contrast_index], framebuffers[contrast_index]);
	}

	/* Filter to remove noise from inaccurate match */
	gl.useProgram(programs[_FS.DILATE]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 1]);
	gl.viewport (0, 0, framebuffers[dmap_index + 1].width, framebuffers[dmap_index + 1].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[dmap_index]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Calculate the maximum by row of 8 elements */
	gl.useProgram(programs[_FS.MAX8]);
	gl.uniform2f(_texelsizes[_FS.MAX8], 1.0/textures[dmap_index + 1].width, 0.0);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 2]);
	gl.viewport (0, 0, framebuffers[dmap_index + 2].width, framebuffers[dmap_index + 2].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[dmap_index + 1]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Calculate the maximum by column of 8 elements */
	gl.uniform2f(_texelsizes[_FS.MAX8], 0.0, 1.0/textures[dmap_index + 2].height);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 3]);
	gl.viewport (0, 0, framebuffers[dmap_index + 3].width, framebuffers[dmap_index + 3].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[dmap_index + 2]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Render the grid */
	renderGrid();

	window.requestAnimationFrame(render);
}
