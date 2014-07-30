var cols = 2;
var rows = 4;

var _VS = {
	SIMPLE: 0,
	KERNEL: 1,
	FLIP: 2,
	RESIZE: 3
};
var vs_paths = [ "vs/simple", "vs/kernel", "vs/flip" ];

var _FS = {
	SIMPLE: 0,
	EDGE: 1,
	BLUR: 2,
	DMAP: 3,
	WHITE: 4,
	DECDMAP: 5,
	DECSIM: 6,
};
var fs_paths = [ "fs/simple", "fs/edge"+e, "fs/blur"+b, "fs/dmap", "fs/white", "fs/decdmap", "fs/decsim" ];

var PROGRAMS = [
	{VS: _VS.FLIP,		FS: _FS.SIMPLE},	/* 0 */
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},	/* 1 */
	{VS: _VS.KERNEL,	FS: _FS.EDGE},		/* 2 */
	{VS: _VS.KERNEL,	FS: _FS.BLUR},		/* 3 */
	{VS: _VS.SIMPLE,	FS: _FS.DMAP},		/* 4 */
	{VS: _VS.SIMPLE,	FS: _FS.WHITE},		/* 5 */
	{VS: _VS.SIMPLE,	FS: _FS.DECDMAP},	/* 6 */
	{VS: _VS.SIMPLE,	FS: _FS.DECSIM},	/* 7 */
];

var _FBS = [
	{WDIV: 1},	/* IRAW_L: 		0 */
	{WDIV: 1},	/* IRAW_R: 		1 */
	{WDIV: 1},	/* EDGE_L: 		2 */
	{WDIV: 1},	/* EDGE_R: 		3 */
	{WDIV: 1},	/* SWIN_L: 		4 */
	{WDIV: 1},	/* SWIN_R: 		5 */
	{WDIV: 1},	/* DECDMAP_L: 		6 */
	{WDIV: 1},	/* DECSIM_L: 		7 */
	{WDIV: 1},	/* DMAP_L: 		8 */
];

function Draw(gl, textures, framebuffers, programs, w, h)
{
	/* Edge detect */
	DrawTextureRect(gl, textures[0], framebuffers[2], -1.0, -1.0, 1.0, 1.0, programs[2]);
	DrawTextureRect(gl, textures[1], framebuffers[3], -1.0, -1.0, 1.0, 1.0, programs[2]);

	/* Support window */
	DrawTextureRect(gl, textures[2], framebuffers[4], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[3], framebuffers[5], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* Initialize disparity map to white */
	DrawTextureRect(gl, textures[0], framebuffers[6], -1.0, -1.0, 1.0, 1.0, programs[5]);
	DrawTextureRect(gl, textures[0], framebuffers[7], -1.0, -1.0, 1.0, 1.0, programs[5]);
	DrawTextureRect(gl, textures[0], framebuffers[8], -1.0, -1.0, 1.0, 1.0, programs[5]);

	for(var x = 0; x < p; x++){
		DrawDisparityRect(gl, textures[4], textures[5], textures[8], framebuffers[8], -1.0, -1.0, 1.0, 1.0, programs[4], x/w);
	}

	/* Decode disparity map */
	DrawTextureRect(gl, textures[8], framebuffers[6], -1.0, -1.0, 1.0, 1.0, programs[6]);
	/* Decode similarity */
	DrawTextureRect(gl, textures[8], framebuffers[7], -1.0, -1.0, 1.0, 1.0, programs[7]);

}
