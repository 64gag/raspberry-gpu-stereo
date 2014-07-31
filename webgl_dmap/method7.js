var pyramid = 4;

var cols = 2;
var rows = 3 * pyramid + 4;

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
	INTERPOLATE: 3
};
var vs_paths = [ "vs/simple", "vs/kernel", "vs/flip", "vs/interpolate_xy" ];

var _FS = {
	SIMPLE: 0,
	EDGE: 1,
	BLUR: 2,
	DMAP: 3,
	DINTERPOLATE: 4,
	DMAP_PTR: 5,
	CONTRAST: 6
};
var fs_paths = [ "fs/simple", "fs/edge"+e, "fs/blur"+b, "fs/dmap", "fs/dinterpolate", "fs/dmap_ptr", "fs/contrast" ];

var PROGRAMS = [
	{VS: _VS.FLIP,		FS: _FS.SIMPLE},
	{VS: _VS.SIMPLE,	FS: _FS.SIMPLE},
	{VS: _VS.KERNEL,	FS: _FS.EDGE},
	{VS: _VS.KERNEL,	FS: _FS.BLUR},
	{VS: _VS.SIMPLE,	FS: _FS.DMAP},
	{VS: _VS.INTERPOLATE,	FS: _FS.DINTERPOLATE},
	{VS: _VS.SIMPLE,	FS: _FS.DMAP_PTR},
	{VS: _VS.SIMPLE,	FS: _FS.CONTRAST}
];

var FBS = [
	{WDIV: 1},	/* O_L: 0 */
	{WDIV: 1},	/* O_R: 1 */
];

for(var d = pyramid; d >= 0; d--){
	var fbObj = {WDIV: Math.pow(2, d)};
	for(var i = 0; i < 6; i++){
		FBS.push(fbObj);
	}
}

function Draw(gl, textures, framebuffers, programs)
{
	/* Initialize to white */
	var dmap_index = 2*_FEATURE.DMAP + _SIDE.LEFT + 2;
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[dmap_index]);
	gl.clearColor(1.0, 1.0, 1.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT);
	gl.bindFramebuffer(gl.FRAMEBUFFER, null);

	/* Copy the original image to its place at the pyramid */
	var left_index = 6*pyramid + 2*_FEATURE.RAW + _SIDE.LEFT + 2;
	var right_index = 6*pyramid + 2*_FEATURE.RAW + _SIDE.RIGHT + 2;
	DrawTextureRect(gl, textures[_SIDE.LEFT], framebuffers[left_index], -1.0, -1.0, 1.0, 1.0, programs[_FS.SIMPLE + 1]);
	DrawTextureRect(gl, textures[_SIDE.RIGHT], framebuffers[right_index], -1.0, -1.0, 1.0, 1.0, programs[_FS.SIMPLE + 1]);

	/* Generate the pyramid and calculate the support windows */
	for(var level = pyramid; level > 0; level--){
		left_index = 6*level + 2*_FEATURE.RAW + _SIDE.LEFT + 2;
		right_index = 6*level + 2*_FEATURE.RAW + _SIDE.RIGHT + 2;
		DrawTextureRect(gl, textures[left_index], framebuffers[left_index - 6], -1.0, -1.0, 1.0, 1.0, programs[_FS.SIMPLE + 1]);
		DrawTextureRect(gl, textures[right_index], framebuffers[right_index - 6], -1.0, -1.0, 1.0, 1.0, programs[_FS.SIMPLE + 1]);
		DrawTextureRect(gl, textures[left_index], framebuffers[left_index + 2], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR + 1]);
		DrawTextureRect(gl, textures[right_index], framebuffers[right_index + 2], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR + 1]);
	}

	/* Last support window... (top of pyramid, out of the loop above) */
	DrawTextureRect(gl, textures[left_index - 6], framebuffers[left_index - 4], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR + 1]);
	DrawTextureRect(gl, textures[right_index - 6], framebuffers[right_index - 4], -1.0, -1.0, 1.0, 1.0, programs[_FS.BLUR + 1]);

	/* Lowest resolution passes */
	left_index = 2*_FEATURE.SWIN + _SIDE.LEFT + 2;
	right_index = 2*_FEATURE.SWIN + _SIDE.RIGHT + 2;
	for(var x = 0; x < 2; x++){
		DrawDisparityRect(gl, textures[left_index], textures[right_index], textures[dmap_index], framebuffers[dmap_index], -1.0, -1.0, 1.0, 1.0, programs[_FS.DMAP + 1], x);
	}

	/* Grow the previous disparity map, calculate new disparities, loop */
	for(var level = 1; level <= pyramid; level++){
		left_index = 6*level + 2*_FEATURE.SWIN + _SIDE.LEFT + 2;
		right_index = 6*level + 2*_FEATURE.SWIN + _SIDE.RIGHT + 2;
		dmap_index = 6*level + 2*_FEATURE.DMAP + _SIDE.LEFT + 2;
		DrawTextureRect(gl, textures[dmap_index - 6], framebuffers[dmap_index], -1.0, -1.0, 1.0, 1.0, programs[_FS.DINTERPOLATE + 1]);
		DrawDisparityRect2(gl, textures[left_index], textures[right_index], textures[dmap_index], framebuffers[dmap_index], -1.0, -1.0, 1.0, 1.0, programs[_FS.DMAP_PTR + 1]);
	}

	/* Increase contrast for visualization */
	for(var level = 0; level <= pyramid; level++){
		dmap_index = 6*level + 2*_FEATURE.DMAP + _SIDE.LEFT + 2;
		DrawTextureRect2(gl, textures[dmap_index], framebuffers[dmap_index + 1], -1.0, -1.0, 1.0, 1.0, programs[_FS.CONTRAST + 1], 5*(pyramid-level+1));
	}
}
