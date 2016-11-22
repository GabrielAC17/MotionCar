#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//GPIO related
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//Motor 1 front and back
#define m1f 1
#define m1b 2

//Motor 2 front and back
#define m2f 3
#define m2b 4

#define ENTRADA 1
#define SAIDA	0
#define INPUT 	1
#define OUTPUT	0
#define HIGH	1
#define LOW	0

bool GPIOExport(int pino);
bool GPIODirection(int pino, int direcao);
bool GPIOWrite(int pino, int valor);
bool GPIORead(int pino);
bool GPIOUnexport(int pino);

using namespace cv;
using namespace std;

 int main( int argc, char** argv )
 {
 	GPIOExport(m1f);
	GPIODirection(m1f, OUTPUT);
	
	GPIOExport(m1b);
	GPIODirection(m1b, OUTPUT);
	
	GPIOExport(m2f);
	GPIODirection(m2f, OUTPUT);
	
	GPIOExport(m2b);
	GPIODirection(m2b, OUTPUT);
 
	VideoCapture cap(0); //capture the video from webcam

    if ( !cap.isOpened() )  // if not success, exit program
    {
         cout << "Cannot open the web cam" << endl;
         return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 219; 
	int iHighS = 255;

	int iLowV = 67;
	int iHighV = 255;

	
	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);
	

	/*
	int iLastX = -1; 
	int iLastY = -1;
	*/

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp); 

	int picCols = imgTmp.cols;
	cout<< "Border X maximum size: " << picCols<< endl;

	//Create a black image with the size as the camera output
	Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;
 

    while (true)
    {
    	Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      
		//morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

		//morphological closing (removes small holes from the foreground)
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
		if (dArea > 10000)
		{
			//calculate the position of the ball
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;        
        		cout<<dArea<<endl;
			if (posX >=0 && posX <= (picCols/3))
			{
				cout<<"Go left"<<endl;
				GPIOWrite(m1f,HIGH);
				GPIOWrite(m1b,LOW);
				GPIOWrite(m2f,LOW);
				GPIOWrite(m2b,LOW);
				
				//Draw a red line from the previous point to the current point
				//line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255), 2);
			}
			
			else if (posX >=(2*picCols/3) && posX <= picCols)
			{
				cout<<"Go right"<<endl;
				GPIOWrite(m1f,LOW);
				GPIOWrite(m1b,LOW);
				GPIOWrite(m2f,HIGH);
				GPIOWrite(m2b,LOW);
			}
			
			else
			{
				if (dArea < 500000)
				{
					cout<<"Forward"<<endl;
					GPIOWrite(m1f,HIGH);
					GPIOWrite(m1b,LOW);
					GPIOWrite(m2f,HIGH);
					GPIOWrite(m2b,LOW);
				}
				
				else if (dArea > 2000000)
				{
					cout<<"Back"<<endl;
					GPIOWrite(m1f,LOW);
					GPIOWrite(m1b,HIGH);
					GPIOWrite(m2f,LOW);
					GPIOWrite(m2b,HIGH);
				}
				else {
					cout<<"Stop"<<endl;
					GPIOWrite(m1f,LOW);
					GPIOWrite(m1b,LOW);
					GPIOWrite(m2f,LOW);
					GPIOWrite(m2b,LOW);
				}
				
			}

			/*
			iLastX = posX;
			iLastY = posY;
			*/
  		}

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //show the original image

        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break; 
        }
    }
    
	GPIOUnexport(m1f);
	GPIOUnexport(m1b);
	GPIOUnexport(m2f);
	GPIOUnexport(m2b);

   return 0;
}

bool GPIOExport(int pino)
{
	char 	buffer[3];
	int 	arquivo;

	arquivo = open("/sys/class/gpio/export", O_WRONLY);
	if( arquivo == -1 )
		return false;
	
	snprintf( buffer, 3, "%d", pino );
	if( write( arquivo, buffer, 3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIOUnexport(int pino)
{
	char 	buffer[3];
	int 	arquivo;

	arquivo = open("/sys/class/gpio/unexport", O_WRONLY);
	if( arquivo == -1 )
		return false;
	
	snprintf( buffer, 3, "%d", pino );
	if( write( arquivo, buffer, 3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIODirection(int pino, int direcao)
{
	char 	caminho[35];
	int	arquivo;

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/direction", pino);
	arquivo = open(caminho, O_WRONLY);
	if( arquivo == -1 )
		return false;

	if( write( arquivo, (direcao == ENTRADA)?"in":"out", (direcao == ENTRADA)?2:3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIOWrite(int pino, int valor)
{
	char 	caminho[35];
	int	arquivo;

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
	arquivo = open(caminho, O_WRONLY);
	if( arquivo == -1 )
		return false;

	if( write( arquivo, (valor == HIGH)?"1":"0", 1 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIORead(int pino){
	char 	caminho[35];
	int		arquivo;
	char 	retorno[2];

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
	arquivo = open(caminho, O_RDONLY);
	if( arquivo == -1 )
		return false;

	if( read(arquivo,retorno,2) == -1 )
	{
		close(arquivo);
		return false;
	}

	close(arquivo);
	return retorno[0]-'0';
}
