var w = 256;
var h = 256;

/* These will be filled later */
var _texelsizes = [];

var _VS = {
	CROP: 0,
	FLIP: 1
};
var vs_paths = [ "vs/crop", "vs/flip" ];

var _FS = {
	SIMPLE: 0,
	FLIP: 1
};
var fs_paths = [ "fs/simple" ];

var PROGRAMS = [
	{VS: _VS.CROP,		FS: _FS.SIMPLE},
	{VS: _VS.FLIP,		FS: _FS.SIMPLE}
];

var FBS = [
	{WDIV: 1, HDIV: 1},	/* O_L: 0 */
	{WDIV: 1, HDIV: 1},	/* O_R: 1 */
];

for(var i = pyramid; i >= 0; i--){
	var fbObj = {WDIV: Math.pow(2, i), HDIV: Math.pow(2, i)};
	FBS.push(fbObj);
	FBS.push(fbObj);
}

/* Controls of what you see */
var dtex = [];

for(var ti = 0; ti < 2; ti++){
	for(var tj = 1; tj <= pyramid; tj++){
		dtex.push(2*tj + ti);
	}
}

var rows = 2;
var cols = Math.ceil(dtex.length/rows);

function renderInit(gl, textures, framebuffers, programs)
{
	/* Set the uniforms of all programs */
	initialValues = [	{LVALUE: "offset", VALUE: [-1.0, -1.0]},
				{LVALUE: "range", VALUE: [w/textures[0].width, h/textures[0].height]},
				{LVALUE: "from", VALUE: [(textures[0].width - w) / 2 / textures[0].width, 
							(textures[0].height - h) / 2 / textures[0].height]},
				{LVALUE: "scale", VALUE: [2.0, 2.0]},
				{LVALUE: "tex_right", VALUE: 1},
				{LVALUE: "tex_dmap", VALUE: 2}
		];

	for(var p = 0; p < programs.length; p++){
		gl.useProgram(programs[p]);
		setVariables(gl, programs[p], initialValues);
		/* Get texelsizes' locations */
		var _texelsize = gl.getUniformLocation(programs[p], "texelsize");
		_texelsizes.push(_texelsize);
	}
}

function render(gl, textures, framebuffers, programs)
{
	/* Draw onto a square */
	gl.useProgram(programs[_FS.SIMPLE]);
	gl.uniform2f(_texelsizes[_FS.SIMPLE], 1.0/textures[0].width, 1.0/textures[0].height);
	runShader(gl, textures[0], framebuffers[textures.length - 2]);
	runShader(gl, textures[1], framebuffers[textures.length - 1]);

	/* Generate the mipmaps */
	gl.bindTexture(gl.TEXTURE_2D, textures[textures.length - 2]);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
	gl.hint(gl.GENERATE_MIPMAP_HINT, gl.FASTEST);
	gl.generateMipmap(gl.TEXTURE_2D);

	gl.bindTexture(gl.TEXTURE_2D, textures[textures.length - 1]);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
	gl.hint(gl.GENERATE_MIPMAP_HINT, gl.FASTEST);
	gl.generateMipmap(gl.TEXTURE_2D);

	var index = 2;

	for(var level = pyramid; level > 0; level--){
		gl.useProgram(programs[_FS.SIMPLE]);
		gl.uniform2f(_texelsizes[_FS.SIMPLE], 1.0/textures[index].width, 1.0/textures[index].height);
		runShader(gl, textures[textures.length - 2], framebuffers[index++]);
		runShader(gl, textures[textures.length - 1], framebuffers[index++]);
	}

	/* Render the grid */
	renderGrid();

	window.requestAnimationFrame(render);
}
