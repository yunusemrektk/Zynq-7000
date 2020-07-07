#include <stdint.h>
#include "stdio.h"

#define HEIGHT 400
#define WIDTH 300

static uint8_t arr1[HEIGHT][WIDTH];
static uint8_t zpadded_img4[HEIGHT+4][WIDTH+4];

void imageproc(int image_in[HEIGHT*WIDTH],int image_out[HEIGHT*WIDTH]){
#pragma HLS INTERFACE m_axi depth=120000 port=image_out offset=slave bundle=OUTPUT
#pragma HLS INTERFACE m_axi depth=120000 port=image_in offset=slave bundle=INPUT
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL
	///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////GAUSSIAN BLUR/////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

		int tmp=0;
		int	kernel[5][5]={
				{1, 4, 6, 4,1},
				{4,16,24,16,4},
				{6,24,36,24,6},
				{4,16,24,16,4},
				{1, 4, 6, 4,1}
		};

		//ONE DIMENSION TO TWO DIMENSION
		int k=0;
		for(int i=0;i<HEIGHT;i++){
			for(int j=0;j<WIDTH;j++){
				arr1[i][j]=image_in[k];
				k++;
			}
		}

		//ZERO PADDING
		for(int i=0;i<HEIGHT+4;i++){
		    for(int j=0;j<WIDTH+4;j++){
		    	if(i==0||i==1||i==HEIGHT+2||i==HEIGHT+3||j==0||j==1||j==WIDTH+2||j==WIDTH+3){
			        zpadded_img4[i][j]=0;
		    	}
		    	else{
		    		zpadded_img4[i][j]=arr1[i-2][j-2];
		    	}
		    }
		}

		//CONVOLUTION WITH 5X5 PASCAL KERNEL
		for(int i=2;i<HEIGHT+2;i++){
			for(int j=2;j<WIDTH+2;j++){
				for(int a=i-2,m=0;a<i+3;a++,m++){
					for(int b=j-2,n=0;b<j+3;b++,n++){
						tmp+=kernel[m][n]*zpadded_img4[a][b];
					}
				}
				arr1[i-2][j-2]=tmp/256;//arr1 is the result of the gaussian blur
				tmp=0;
			}
		}

		//TWO DIMENSION TO ONE DIMENSION
		for(int i=0;i<HEIGHT;i++){
			for(int j=0;j<WIDTH;j++){
				image_out[WIDTH*i+j]=arr1[i][j];
			}
		}
}

