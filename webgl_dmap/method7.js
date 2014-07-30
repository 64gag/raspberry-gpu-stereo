var cols = 2;
var rows = 11;

var _VS = {
	SIMPLE: 0,
	KERNEL: 1,
	FLIP: 2,
	INTERPOLATE: 3
};
var vs_paths = [ "vs/simple", "vs/kernel", "vs/flip", "vs/interpolate_xy" ];

var _FS = {
	SIMPLE: 0,
	EDGE: 1,
	BLUR: 2,
	DMAP: 3,
	WHITE: 4,
	DECDMAP: 5,
	DECSIM: 6,
	INTERPOLATE: 7
};
var fs_paths = [ "fs/simple", "fs/edge"+e, "fs/blur"+b, "fs/dmap", "fs/white", "fs/decdmap", "fs/decsim", "fs/interpolate" ];

var PROGRAMS = [
	{VS: _VS.FLIP,		FS: _FS.SIMPLE},	/* 0 */
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},	/* 1 */
	{VS: _VS.KERNEL,	FS: _FS.EDGE},		/* 2 */
	{VS: _VS.KERNEL,	FS: _FS.BLUR},		/* 3 */
	{VS: _VS.SIMPLE,	FS: _FS.DMAP},		/* 4 */
	{VS: _VS.SIMPLE,	FS: _FS.WHITE},		/* 5 */
	{VS: _VS.SIMPLE,	FS: _FS.DECDMAP},	/* 6 */
	{VS: _VS.SIMPLE,	FS: _FS.DECSIM},	/* 7 */
	{VS: _VS.INTERPOLATE,	FS: _FS.INTERPOLATE}	/* 8 */
];

var _FBS = [
	{WDIV: 1},	/* IRAW_L: 		0 */
	{WDIV: 1},	/* IRAW_R: 		1 */
	{WDIV: 1},	/* SWIN_L: 		2 */
	{WDIV: 1},	/* SWIN_R: 		3 */
	{WDIV: 2},	/* H_L: 		4 */
	{WDIV: 2},	/* H_R: 		5 */
	{WDIV: 2},	/* HSW_L: 		6 */
	{WDIV: 2},	/* HSW_R: 		7 */
	{WDIV: 4},	/* HH_L: 		8 */
	{WDIV: 4},	/* HH_R: 		9 */
	{WDIV: 4},	/* HHSW_L: 		10 */
	{WDIV: 4},	/* HHSW_R: 		11 */
	{WDIV: 8},	/* HHH_L: 		12 */
	{WDIV: 8},	/* HHH_R: 		13 */
	{WDIV: 8},	/* HHHSW_L: 		14 */
	{WDIV: 8},	/* HHHSW_R: 		15 */
	{WDIV: 16},	/* HHHH_L: 		16 */
	{WDIV: 16},	/* HHHH_R: 		17 */
	{WDIV: 16},	/* HHHHSW_L: 		18 */
	{WDIV: 16},	/* HHHHSW_R: 		19 */
	{WDIV: 16},	/* HHHHFULL_L: 		20 */
	{WDIV: 8},	/* HHHFULL_L: 		21 */
];

function Draw(gl, textures, framebuffers, programs)
{
	/* Swin for the first */
	DrawTextureRect(gl, textures[0], framebuffers[2], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[1], framebuffers[3], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* Half */
	DrawTextureRect(gl, textures[0], framebuffers[4], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[1], framebuffers[5], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[4], framebuffers[6], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[5], framebuffers[7], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* HHalf */
	DrawTextureRect(gl, textures[0], framebuffers[8], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[1], framebuffers[9], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[8], framebuffers[10], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[9], framebuffers[11], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* HHHalf */
	DrawTextureRect(gl, textures[0], framebuffers[12], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[1], framebuffers[13], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[12], framebuffers[14], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[13], framebuffers[15], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* HHHHalf */
	DrawTextureRect(gl, textures[0], framebuffers[16], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[1], framebuffers[17], -1.0, -1.0, 1.0, 1.0, programs[1]);
	DrawTextureRect(gl, textures[16], framebuffers[18], -1.0, -1.0, 1.0, 1.0, programs[3]);
	DrawTextureRect(gl, textures[17], framebuffers[19], -1.0, -1.0, 1.0, 1.0, programs[3]);

	/* Initialize to white */
	DrawTextureRect(gl, textures[0], framebuffers[20], -1.0, -1.0, 1.0, 1.0, programs[5]);

	/* Lowest resolution passes */
	for(var x = 0; x < p; x++){
		DrawDisparityRect(gl, textures[14], textures[15], textures[20], framebuffers[20], -1.0, -1.0, 1.0, 1.0, programs[4], x);
	}

	/* Grow the disparity map and apply the "interpolation kernel" */
	DrawTextureRect(gl, textures[20], framebuffers[21], -1.0, -1.0, 1.0, 1.0, programs[8]);




	/* Decode disparity map */
//	DrawTextureRect(gl, textures[22], framebuffers[20], -1.0, -1.0, 1.0, 1.0, programs[6]);
	/* Decode similarity */
//	DrawTextureRect(gl, textures[22], framebuffers[21], -1.0, -1.0, 1.0, 1.0, programs[7]);


}
