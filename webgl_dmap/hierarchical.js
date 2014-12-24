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
	LINE8: 6
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
	ERODE_HOR: 7,
	ERODE_VER: 8,
	FLIP: 9
};
var fs_paths = [ "fs/simple", "fs/channelpack", "fs/dmap0", "fs/dinterpolate", "fs/dmapN", "fs/contrast", "fs/dilate", "fs/erode" ];

var PROGRAMS = [
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},
	{VS: _VS.CROP,		FS: _FS.PACK},
	{VS: _VS.LEFT,		FS: _FS.DMAP0},
	{VS: _VS.INTERPOLATE,	FS: _FS.DINTERPOLATE},
	{VS: _VS.SIMPLE,	FS: _FS.DMAPN},
	{VS: _VS.SIMPLE,	FS: _FS.CONTRAST},
	{VS: _VS.DIAGS,		FS: _FS.DILATE},
	{VS: _VS.LINE8,		FS: _FS.ERODE_HOR},
	{VS: _VS.LINE8,		FS: _FS.ERODE_HOR},
	{VS: _VS.FLIP,		FS: _FS.SIMPLE},
	{VS: _VS.CROP,		FS: _FS.SIMPLE}
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
var fbObj = {WDIV: 1, HDIV: 1};	/* Left and right, cropped */
FBS.push(fbObj);
FBS.push(fbObj);

/* Controls of what you see */
var dtex = [10];
var dtex = [2, 3, 4, 5, 6, 7, 8, 9, 10];
//var dtex = [11, 12, 7];

var cols = 3;
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
	gl.useProgram(programs[_FS.ERODE_HOR]);
	gl.uniform2f(_texelsizes[_FS.ERODE_HOR], 1.0/textures[dmap_index + 1].width, 0.0);
	gl.useProgram(programs[_FS.ERODE_VER]);
	gl.uniform2f(_texelsizes[_FS.ERODE_VER], 0.0, 1.0/textures[dmap_index + 2].height);

	/* Sampler uniforms */
	gl.useProgram(programs[_FS.PACK]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.PACK], "R"), 1);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.PACK], "G"), 2);
	gl.useProgram(programs[_FS.DMAP0]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.DMAP0], "stereo"), 3);
	gl.useProgram(programs[_FS.DMAPN]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.DMAPN], "stereo"), 3);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.DMAPN], "disparity_map"), 0);
	gl.useProgram(programs[_FS.DILATE]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.DILATE], "tex"), 4);
	gl.useProgram(programs[_FS.ERODE_HOR]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.ERODE_HOR], "tex"), 5);
	gl.useProgram(programs[_FS.ERODE_VER]);
	gl.uniform1i(gl.getUniformLocation(programs[_FS.ERODE_VER], "tex"), 6);

	/* Make good use of the texture slots, match above values */
	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, textures[left_index]);
	gl.activeTexture(gl.TEXTURE2);
	gl.bindTexture(gl.TEXTURE_2D, textures[right_index]);
	gl.activeTexture(gl.TEXTURE3);
	gl.bindTexture(gl.TEXTURE_2D, textures[stereo_index]);
	gl.activeTexture(gl.TEXTURE4);
	gl.bindTexture(gl.TEXTURE_2D, textures[dmap_index]);
	gl.activeTexture(gl.TEXTURE5);
	gl.bindTexture(gl.TEXTURE_2D, textures[8]);
	gl.activeTexture(gl.TEXTURE6);
	gl.bindTexture(gl.TEXTURE_2D, textures[9]);

	/* Get locations of changing uniforms */
	_k = gl.getUniformLocation(programs[_FS.CONTRAST], "k");
	disparity2texel = gl.getUniformLocation(programs[_FS.DMAPN], "disparity2texel");

	/* Mipmap filters and hints */
	gl.activeTexture(gl.TEXTURE3);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
	gl.hint(gl.GENERATE_MIPMAP_HINT, gl.FASTEST);
	gl.generateMipmap(gl.TEXTURE_2D); /* This is only here to keep firefox happy... */
	gl.activeTexture(gl.TEXTURE0);
}

function render()
{
	/* Capture POT region and pack to a single texture */
	gl.useProgram(programs[_FS.PACK]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[stereo_index]);
	gl.viewport(0, 0, framebuffers[stereo_index].width, framebuffers[stereo_index].height);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);


//////////////////////
	gl.useProgram(programs[_FS.FLIP+1]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[11]);
	gl.viewport(0, 0, framebuffers[11].width, framebuffers[11].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[0]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[12]);
	gl.viewport(0, 0, framebuffers[12].width, framebuffers[12].height);
	gl.bindTexture(gl.TEXTURE_2D, textures[1]);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
//////////////////////


	/* Generate the mipmaps */
	gl.activeTexture(gl.TEXTURE3);
	gl.generateMipmap(gl.TEXTURE_2D);
	gl.activeTexture(gl.TEXTURE0);

	/* Lowest resolution pass */
	gl.useProgram(programs[_FS.DMAP0]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[smallest_index]);
	gl.viewport(0, 0, framebuffers[smallest_index].width, framebuffers[smallest_index].height);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Grow the previous disparity map, calculate new disparities, loop */
	for(var level = 0; level < pyramid; level++){
		/* Grow the previous */
		var grow_index = smallest_index + level;
		var grown_index = grow_index + 1;
		gl.useProgram(programs[_FS.DINTERPOLATE]);
		gl.uniform2f(_texelsizes[_FS.DINTERPOLATE], 1.0/textures[grow_index].width, 1.0/textures[grow_index].height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[grown_index]);
		gl.viewport(0, 0, framebuffers[grown_index].width, framebuffers[grown_index].height);
		gl.bindTexture(gl.TEXTURE_2D, textures[grow_index]);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

		/* Calculate new disparities */
		gl.useProgram(programs[_FS.DMAPN]);
		gl.uniform1f(disparity2texel, 255.0/textures[grown_index].width);
		gl.bindTexture(gl.TEXTURE_2D, textures[grown_index]);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	/* Increase contrast for visualization */
	gl.useProgram(programs[_FS.CONTRAST]);
	for(var level = 0; level <= pyramid; level++){
		var contrast_index = smallest_index + level;
		gl.uniform1f(_k, 8*(pyramid-level+1));
		gl.uniform2f(_texelsizes[_FS.CONTRAST], 1.0/textures[contrast_index].width, 1.0/textures[contrast_index].height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[contrast_index]);
		gl.viewport (0, 0, framebuffers[contrast_index].width, framebuffers[contrast_index].height);
		gl.bindTexture(gl.TEXTURE_2D, textures[contrast_index]);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	/* Filter to remove noise from inaccurate match */
	gl.useProgram(programs[_FS.DILATE]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 1]);
	gl.viewport(0, 0, framebuffers[dmap_index + 1].width, framebuffers[dmap_index + 1].height);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Calculate the maximum by row of 8 elements */
	gl.useProgram(programs[_FS.ERODE_HOR]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 2]);
	gl.viewport(0, 0, framebuffers[dmap_index + 2].width, framebuffers[dmap_index + 2].height);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Calculate the maximum by column of 8 elements */
	gl.useProgram(programs[_FS.ERODE_VER]);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index + 3]);
	gl.viewport(0, 0, framebuffers[dmap_index + 3].width, framebuffers[dmap_index + 3].height);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	/* Render the grid */
	renderGrid();

	window.requestAnimationFrame(render);
}
