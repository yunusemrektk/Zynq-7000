Team number: xohw20_162
Project name: Webcam-based Real-Time Optical Mark Reader

Date: 27.06.2020
Version of uploaded archive: v1.0

University name: Eskiþehir Technical University
Supervisor name: Ýsmail SAN
Supervisor e-mail: isan@eskisehir.edu.tr 
Participant(s): Yunus Emre KUTUK, Hasan KARACA
Email: yunusemrekutuk@eskisehir.edu.tr
Email: hasan_karaca@eskisehir.edu.tr

Board used: ZedBoard Development Board ZYNQ-7000 Series
Vivado Version: 2017.4
Brief description of project: Accelerating a specific function in OMR project. In our build we have
four functions to process the exam answer sheets and the gaussian blur function has chosen for
acceleration.

Description of archive (explain directory structure, documents and source files):
Instructions to build and test project
Step 1: We have only one C++ main code (main.cc) and one header file (rgb
_image.h) contains array representation of the input image. As we demonstared on the YouTube video,
software and hardware comparisons were implemented in main.cc.

After importing the necessary files from the given folders,
1- You need to desing the block diagram. But first, the custom IP core must bu added
to IP repsitory from the project settings under the flow navigator.
2- Validate the design without any error.
3- Create output products, create HDL wrapper.
5- Write bitsream.
4- Export hardware including the bitstream.
On the SDK side:
1- You need to define header file that contains a represantation of our input image as an
array.
2- Finally you just need to assing memory addresses of the input and output array as
arguments to ip_handle function and it does the rest.

...

Link to YouTube Video(s): https://youtu.be/bGizXE-Kc7k
Link to GitHub: 