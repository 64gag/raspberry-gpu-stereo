#define CAM 1 

#if CAM
 #define TEX_W 176
 #define TEX_H 144
#else
 #define TEX_W 320
 #define TEX_H 240
#endif

#define TEX_SIZE TEX_W*TEX_H
#define GRID_COLS 4
#define GRID_ROWS 2
#define TEX_GRID -1

#define CAM_LEFT 0
#define CAM_RIGHT 1

#define SPI_TX 0
#define SPI_GRAYONLY 0

enum tex_enum_t{
	TEX_LIRAW,
	TEX_RIRAW,
	TEX_LHENC,
	TEX_RHENC,
	TEX_LSWIN,
	TEX_RSWIN,
	TEX_DMAP,
	TEX_HDEC
};

enum thread_enum_t {
	THREAD_CAPTURE,
	THREAD_DEPTH,
	THREAD_COUNT,
	THREAD_LANE
};

enum arguments_t {
	ARG_SPI_DIV_L,
	ARG_SPI_DIV_R,
	ARG_TRAN_DELAY_L,
	ARG_TRAN_DELAY_R,
	ARG_FRAME_DELAY,
	ARG_COUNT
};
