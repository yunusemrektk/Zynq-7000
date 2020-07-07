#include "stdio.h"
#include "stdlib.h"
#include "hls_video.h"
#include "rgb_image.h"
#define RGB_IMAGE_MAP
#define HEIGHT 400
#define WIDTH 300
using namespace std;

void imageproc(uint8_t* image_in,uint8_t* image_out);
void grayscale(uint8_t *image_in);
static uint8_t image_out_gr[400*300];


int main(int argc,char **argv) {

static uint8_t image_in[400*300*3];
static uint8_t image_out[400*300];

memcpy(image_in,rgb_image,sizeof(uint8_t)*400*300*3);
grayscale(image_in);
imageproc(image_out_gr,image_out);

return 0;

}
void grayscale(uint8_t *image_in){

	float R_param,G_param,B_param;
	R_param = 0.2989;
	G_param = 0.5870;
	B_param = 0.1140;

	int tmp_r,tmp_g,tmp_b;
	int new_pixel;

	int j=0;
	for(int i =0;i<HEIGHT*WIDTH*3;i=i+3){
		tmp_r=image_in[i];
		tmp_g=image_in[i+1];
		tmp_b=image_in[i+2];
		new_pixel = (tmp_r*R_param)+(tmp_g*G_param)+(tmp_b*B_param);

		image_out_gr[j]=new_pixel;
		j++;
	}
	printf("[");
		for(int i=0;i<HEIGHT*WIDTH;i=i+WIDTH){
			printf("[");
			for(int j=0;j<WIDTH;j++){
		       printf("%d, ",image_out_gr[i+j]);
		    }
		    printf("],\n");
		}
		printf("]");
}
