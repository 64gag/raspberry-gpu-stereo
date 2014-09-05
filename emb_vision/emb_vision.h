#define CAM 0 

#if CAM
 #define IMG_W 176
 #define IMG_H 144
#else
 #define IMG_W 384
 #define IMG_H 288
#endif

#define IMG_SIZE IMG_W*IMG_H

#define CAM_LEFT 0
#define CAM_RIGHT 1

#define SPI_TX 0
#define SPI_GRAYONLY 0

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
	ARG_DRAWN_TEX,
	ARG_COUNT
};
