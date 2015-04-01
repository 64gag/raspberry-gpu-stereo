#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void){
	unsigned char c;
	unsigned int i, j;
	int height = 384;
	int width = 288;
	int fd;	

	fd = open("left_frame_data.bin", O_RDONLY);

	printf("uint8_t left_frame_data[] = {");
	for(i = 1; i <= height; i++)
	{
		printf("\t");
		for(j = 1; j <= width; j++)
		{
			read(fd, &c, sizeof(c));
			printf("0x%x", c);
			if(i != height || j != width){
				printf(", ");
			}
		}
		printf("\n");
	}
	printf("};\n");


return 0;
}

