#include "stdio.h"
#include "stdlib.h"
#include "xil_printf.h"
#include "math.h"
#include "rgb_image.h"
#include <xtime_l.h>
#include "ximageproc.h"

#define PI 3.141592654
#define HEIGHT 400
#define WIDTH 300


void grayscale(int *image_in);
void gaussian_blur(int *image_in);
void threshold(int *image_in);
void print_img(int *image_in);
void canny(int *image_in);
void ip_handle(int *image_in_addr,int *image_out_addr);


int * image_insw=(int *)(0x05110000);
int * image_out_blur=(int *)(0x07110000);
int * image_out_gr=(int *)(0x03110000);
int * image_out_blurhw = (int *)(0x01110000);

static int image_out_th[HEIGHT*WIDTH];
static int canny_out[HEIGHT*WIDTH];

XImageproc Img,Img_in,Img_out;
XImageproc_Config Img_Ctrl={0,XPAR_IMAGEPROC_0_S_AXI_CTRL_BASEADDR};



int main(int argc,char **argv) {
	XTime tStart, tEnd,tStart1, tEnd1;
	    double  ElapsedTime;

	static int image_in[HEIGHT*WIDTH*3];
	memcpy(image_in,rgb_image,sizeof(image_in));


	grayscale(image_in);

/////// software///////////////////////////////////////////////////////////

	 XTime_GetTime(&tStart1);
	 gaussian_blur(image_out_gr);
	 XTime_GetTime(&tEnd1);

///////// hardware/////////////////////////////////////////////////////////

	 XTime_GetTime(&tStart);
	 ip_handle(image_out_gr,image_out_blurhw);
	 XTime_GetTime(&tEnd);


	canny(image_out_blurhw);
	threshold(image_out_blurhw);
//	print_img(canny_out);

	 printf("Software Output took %llu clock cycles.\n", 2*(tEnd1 - tStart1));
	 ElapsedTime = 1.0 * (tEnd1 - tStart1) / (COUNTS_PER_SECOND);
	 printf("Software Output took %.2f s.\n",ElapsedTime);


	 printf("Hardware Output took %llu clock cycles.\n", 2*(tEnd - tStart));
	 ElapsedTime = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND);
	 printf("Hardware Output took %.2f s.\n",ElapsedTime);



	return 0;
}
void ip_handle(int *image_in_addr,int *image_out_addr){

    XImageproc_CfgInitialize(&Img,&Img_Ctrl); 			 //Initialize the IP

	XImageproc_Set_image_in(&Img,(u32)image_in_addr);	 //Set Address of input image
	XImageproc_Set_image_out(&Img,(u32)image_out_addr);  //Set Address of output image

	XImageproc_Start(&Img);								 //Start the IP

	while(!XImageproc_IsDone(&Img));					 //Wait until IP is done


}



void grayscale(int *image_in){

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

}

void threshold(int *image_in){

	int threshold = 130;
	int max_value = 255;
	for(int i =0;i<HEIGHT*WIDTH;i++){
		if(image_in[i]>threshold){
			image_out_th[i]=0;
		}
		else {
			image_out_th[i]=max_value;
		}
	}
}

void gaussian_blur(int *image_in){
int arr1[HEIGHT][WIDTH];
int zpadded_img4[HEIGHT+4][WIDTH+4];

int tmp=0;
	 int	kernel[5][5]={{1,  4,  6,  4,  1},
			 	 	 	  {4, 16, 24, 16,  4},
						  {6, 24, 36, 24,  6},
						  {4, 16, 24, 16,  4},
						  {1,  4,  6,  4,  1}};

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
			image_out_blur[WIDTH*i+j]=arr1[i][j];
		}
	}


}

void canny(int *image_in){

	uint8_t arr1[HEIGHT][WIDTH];
	int8_t Ix[HEIGHT][WIDTH];
	int8_t Iy[HEIGHT][WIDTH];
	uint8_t G[HEIGHT][WIDTH];
	int16_t theta[HEIGHT][WIDTH];
	int tmp1=0, tmp2=0;

	int8_t Kx[3][3]={{-1,0,1},
					 {-2,0,2},
					 {-1,0,1}};


	int8_t Ky[3][3]={{1,2,1},
					 {0,0,0},
					 {-1,-2,-1}};


	//ONE DIMENSION TO TWO DIMENSION
	int k=0;
	int SIZE=HEIGHT*WIDTH;
	for(int i=0;i<HEIGHT;i++){
		if(k==SIZE)
			break;
		for(int j=0;j<WIDTH;j++){
			arr1[i][j]=(uint8_t)image_in[k];
			k++;
		}
	}
	//ZERO PADDING////////////////////////////////////////////////////////////////////

	uint8_t zpadded_img[HEIGHT+2][WIDTH+2];

	for(int i=0;i<HEIGHT+2;i++){
	    for(int j=0;j<WIDTH+2;j++){
	        if(i==0||i==HEIGHT+1||j==0||j==WIDTH+1){
	        	zpadded_img[i][j]=0;
	        }
	        else{
	        	zpadded_img[i][j]=arr1[i-1][j-1];
	        }
	    }
	}
	//-X & -Y CONVOLUTION
	for(int i=1;i<HEIGHT+1;i++){
		for(int j=1;j<WIDTH+1;j++){
			for(int a=i-1,m=0;a<i+2;a++,m++){
				for(int b=j-1,n=0;b<j+2;b++,n++){
					tmp1+=Kx[m][n]*zpadded_img[a][b];
					tmp2+=Ky[m][n]*zpadded_img[a][b];
				}
			}
			Ix[i-1][j-1]=tmp1;
			Iy[i-1][j-1]=tmp2;
			tmp1=0;
			tmp2=0;
		}
	}
	/*
	//TWO DIMENSION TO ONE DIMENSION X
	for(int i=0;i<800;i++){
		for(int j=0;j<600;j++){
			image_out_Ix[600*i+j]=Ix[i][j];
		}
	}*/
	/*
	//TWO DIMENSION TO ONE DIMENSION Y
	for(int i=0;i<800;i++){
		for(int j=0;j<600;j++){
			image_out_Iy[600*i+j]=Iy[i][j];
		}
	}*/

	//GRADIENT AND ANGLE
	float val=180.0/PI;
	for(int i=0;i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++){
			G[i][j]=sqrt(pow(Ix[i][j],2)+pow(Iy[i][j],2));

			theta[i][j]=atan((double)Iy[i][j]/(double)Ix[i][j])*val;

			if(theta[i][j]<=-67.5||theta[i][j]>67.5)
				theta[i][j]=90;
			else if(theta[i][j]<=-22.5)
				theta[i][j]=135;
			else if(theta[i][j]<=22.5)
				theta[i][j]=0;
			else if(theta[i][j]<=67.5)
				theta[i][j]=45;
		}
	}

	//ZERO PADDING
	for(int i=0;i<HEIGHT+2;i++){
		for(int j=0;j<WIDTH+2;j++){
			if(i==0||i==HEIGHT+1||j==0||j==WIDTH+1){
				zpadded_img[i][j]=0;
			}
			else{
				zpadded_img[i][j]=G[i-1][j-1];
			}
		}
	}
	//NON-MAXIMUM SUPPRESSION
	int max=0;
	for(int i=1;i<HEIGHT+1;i++){
		for(int j=1;j<WIDTH+1;j++){
			switch(theta[i][j]){
				case 90:
					if(zpadded_img[i][j]<zpadded_img[i-1][j] && zpadded_img[i][j]<zpadded_img[i+1][j])
						arr1[i-1][j-1]=0;
					else
						arr1[i-1][j-1]=zpadded_img[i][j];
					break;
				case 135:
					if(zpadded_img[i][j]<zpadded_img[i-1][j-1] && zpadded_img[i][j]<zpadded_img[i+1][j+1])
						arr1[i-1][j-1]=0;
					else
						arr1[i-1][j-1]=zpadded_img[i][j];
					break;
				case 0:
					if(zpadded_img[i][j]<zpadded_img[i][j-1] && zpadded_img[i][j]<zpadded_img[i][j+1])
						arr1[i-1][j-1]=0;
					else
						arr1[i-1][j-1]=zpadded_img[i][j];
					break;
				case 45:
					if(zpadded_img[i][j]<zpadded_img[i-1][j+1] && zpadded_img[i][j]<zpadded_img[i+1][j-1])
						arr1[i-1][j-1]=0;
					else
						arr1[i-1][j-1]=zpadded_img[i][j];
					break;
			}
			if(max<arr1[i-1][j-1])
				max=arr1[i-1][j-1];
		}
	}
	//DOUBLE THRESHOLD
	float hightratio=0.14;
	float lowtratio=0.03;
	int strong=255;
	int weak=25;
	float hight=max*hightratio;
	float lowt=hight*lowtratio;

	for(int i=0;i<HEIGHT;i++){
			for(int j=0;j<WIDTH;j++){
				if(arr1[i][j]>=(int)hight)
					arr1[i][j]=strong;
				else if (arr1[i][j]>=(int)lowt)
					arr1[i][j]=weak;
				else
					arr1[i][j]=0;
			}
	}
	//EDGE TRACKING BY HYSTERESIS
	for(int i=1;i<HEIGHT-1;i++){
		for(int j=1;j<WIDTH-1;j++){
			if(arr1[i][j]==weak){
				if(arr1[i-1][j-1]==strong || arr1[i-1][j]==strong || arr1[i-1][j+1]==strong ||
						arr1[i][j-1]==strong || arr1[i][j+1]==strong || arr1[i+1][j-1]==strong ||
						arr1[i+1][j]==strong || arr1[i+1][j+1]==strong)
					theta[i][j]=strong;
				else
					theta[i][j]=0;
			}
		}
	}
	/*
	//SCALING
	for(int i=0;i<800;i++){
		for(int j=0;j<600;j++){
			G[i][j]=G[i][j]*255/max;
		}
	}*/
	//TWO DIMENSION TO ONE DIMENSION
	for(int i=0;i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++){
			canny_out[WIDTH*i+j]=(int)arr1[i][j];
		}
	}
}

void print_img(int *image_in){

	for(int i=0;i<HEIGHT*WIDTH;i=i+WIDTH){
		for(int j=0;j<WIDTH;j++){
	       xil_printf("%d ",image_in[i+j]);
	    }
	    xil_printf("\n");
	}

}
