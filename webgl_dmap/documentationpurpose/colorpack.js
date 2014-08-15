var cols = 1;
var rows = 2;
var w = 256;
var h = 256;

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
	FLIP: 1
};
var vs_paths = [ "vs/simple", "vs/flip" ];

var _FS = {
	PACK: 0,
	FLIP: 1
};
var fs_paths = [ "fs/channelpack", "fs/simple" ];

var PROGRAMS = [
	{VS: _VS.SIMPLE,	FS: _FS.PACK},
	{VS: _VS.FLIP,		FS: _FS.FLIP}
];

var FBS = [
	{WDIV: 1, HDIV: 1},
	{WDIV: 1, HDIV: 1},
	{WDIV: 1, HDIV: 1},
	{WDIV: 1, HDIV: 1},
	{WDIV: 1, HDIV: 1}
];

function renderInit(gl, textures, framebuffers, programs)
{
	/* Set the uniforms of all programs */
	initialValues = [	{LVALUE: "offset", VALUE: [-1.0, -1.0]},
				{LVALUE: "scale", VALUE: [2.0, 2.0]},
				{LVALUE: "tex", VALUE: 0},
				{LVALUE: "R", VALUE: 0},
				{LVALUE: "G", VALUE: 1},
				{LVALUE: "B", VALUE: 2},
				{LVALUE: "A", VALUE: 3},
		];

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
}

function render(gl, textures, framebuffers, programs)
{
    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.enable(gl.BLEND);
	gl.useProgram(programs[_FS.PACK]);
	gl.uniform2f(_texelsizes[_FS.PACK], 1.0/textures[0].width, 1.0/textures[0].height);
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[4]);
	gl.viewport (0, 0, framebuffers[4].width, framebuffers[4].height);

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, textures[0]);
	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, textures[1]);
	gl.activeTexture(gl.TEXTURE2);
	gl.bindTexture(gl.TEXTURE_2D, textures[2]);
	gl.activeTexture(gl.TEXTURE3);
	gl.bindTexture(gl.TEXTURE_2D, textures[3]);
	gl.activeTexture(gl.TEXTURE0);

	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
}
