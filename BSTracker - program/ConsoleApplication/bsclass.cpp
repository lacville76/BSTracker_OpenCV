// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/objdetect.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>
using namespace cv;
using namespace std;

Mat frame; //aktualna klatka
Mat fgMaskMOG2; //fg maska MOG
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //wejscie z klawiatury



void on_trackbarThreshold(int, void*);

void on_trackbarVideo(int, void*);

Mat OriginalFrame;

const int threshold_slider_max = 255;
const int dilation_slider_max = 30;
const int erosion_slider_max = 50;

int threshold_slider = 128;
int dilation_slider = 20;
int erosion_slider = 40;







VideoCapture capture;

int g_slider_position = 0;

int main(int argc, char* argv[])
{
	
	if (argc != 3) {
		cerr << "Niew³aœciwa lista wejœcia" << endl;
		cerr << "wyjœcie..." << endl;
		return EXIT_FAILURE;
	}

	pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach
	if (strcmp(argv[1], "-vid") == 0) {
	
		processVideo(argv[2]);
	}

	else {
		
		cerr << "Sprawdz parametry." << endl;
		cerr << "Wychodzenie..." << endl;
		return EXIT_FAILURE;
	}

	destroyAllWindows();
	return EXIT_SUCCESS;
}


//wykrywanie kolizji
bool intersection(Point2f o1, Point2f o2, Mat img)
{

	if (o2.x - img.size().width / 2>0 && o1.x - img.size().width / 2<0)
		return true;
	else
		return false;
}

//funkcja g³ówna
void processVideo(char* videoFilename) {
	//inicjalizacja wartoœci odpowiadzialnych za przekszta³cenie obrazu
	int threshold_size = 128;
	int dilation_size = 10;
	int erosion_size = 12;
	//tworzenie okien z maska i wlasciwym obrazem
	namedWindow("Frame");
	namedWindow("FG Mask MOG 2");
	moveWindow("Frame", 100, 100);
	moveWindow("FG Mask MOG 2", 700, 100);
	int count = 0;
	 capture = VideoCapture(videoFilename);
	stringstream ss;
	int frames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	if (frames != 0)
	{
		 createTrackbar("Frames", "Frame", &g_slider_position, frames, on_trackbarVideo);
	}
	string frameNumberString = ss.str();
	char TrackbarName[50];

	
	
	if (!capture.isOpened()) {
		
		cerr << "Nie znaleziono pliku: " << videoFilename << endl;
		exit(EXIT_FAILURE);
	}
	
	while ((char)keyboard != 'q' && (char)keyboard != 27) {
	
		if (!capture.read(frame)) {
			
			g_slider_position = 0;
			processVideo(videoFilename);
		}
		
		OriginalFrame = frame.clone();


		Point lineBeginning = Point(OriginalFrame.size().width / 2, OriginalFrame.size().height);
		Point lineEnding = Point(OriginalFrame.size().width / 2, 0);
		line(OriginalFrame, lineBeginning, lineEnding, CV_RGB(0, 255, 0), 5);
		//ROZMYCIE
		blur(frame, frame, Size(12, 12));
		//aktualizacja modelu t³a
		pMOG2->apply(frame, fgMaskMOG2);

		Mat element = getStructuringElement(cv::MORPH_CROSS,
			cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
			cv::Point(erosion_size, erosion_size));

		//nak³adanie erozji i dylatacji na klatkê

		//MORF
		erode(fgMaskMOG2, fgMaskMOG2, element);  // dilate(image,dst,element);
		dilate(fgMaskMOG2, fgMaskMOG2, Mat(), Point(-1, -1), dilation_size, 1, 1);


		threshold(fgMaskMOG2, fgMaskMOG2, 128, 255, CV_THRESH_BINARY);
		Mat ContourImg;
		//znajdywanie konturów
		ContourImg = fgMaskMOG2.clone();
		

		vector<vector<Point>> contours; // Vector for storing contour
		vector<Vec4i> hierarchy;
		findContours(ContourImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		// iterate through each contour.

		vector< Rect > output;
		vector< vector< Point> >::iterator itc = contours.begin();
		//wykrywanie ludzi i zliczanie ich czyli to co program robi dodatkowo
		while (itc != contours.end()) {

			
			//zamykanie w prostok¹cie
			Rect mr = boundingRect(Mat(*itc));
			//Rect mr1 = Rect(OriginalFrame.size().width / 2-15, 0, 30, OriginalFrame.size().height);
		
			//double minFraction(0.65);
			if (mr.area()>800 &&mr.width<mr.height)
			{
				
				int sensitivity = 4;
				Point center = Point((mr.x + mr.width / 2), (mr.y + mr.height / 2));
				Point center2 = Point((mr.x + mr.width / 2) + sensitivity, (mr.y + mr.height / 2) + sensitivity);
				
				//intersection(center, center2, OriginalFrame)
				rectangle(OriginalFrame, mr, CV_RGB(255, 0, 0));
				//rectangle(OriginalFrame, mr1, CV_RGB(255, 0, 255));
				rectangle(OriginalFrame, center, center2, CV_RGB(0, 255, 0));
				if (intersection(center, center2, OriginalFrame))
				{
					count++;
				
				}
				else
				{
				
				}
			}
		
			++itc;
		}

		rectangle(OriginalFrame, cv::Point(10, 2), cv::Point(200, 20),
			cv::Scalar(255, 255, 255), -1);
		ss << capture.get(CAP_PROP_POS_FRAMES);
		putText(OriginalFrame,"Naliczone osoby:" + to_string(count), cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
		//pokazanie aktualnej klatki i maski mog
		imshow("Frame", OriginalFrame);
		imshow("FG Mask MOG 2", fgMaskMOG2);
	
		keyboard = waitKey(30);
		cvSetTrackbarPos("Frames", "Frame", ++g_slider_position);
	}
	//delete capture object
	
	destroyAllWindows();
	capture.release();
	
	
	//DisplayMainMenu();

}








void on_trackbarThreshold(int threshold_size, void*)
{
	threshold_size = (double)threshold_slider / threshold_slider_max;

}


void on_trackbarVideo(int actualFrame, void *)
{
	actualFrame = g_slider_position;
	capture.set(CV_CAP_PROP_POS_FRAMES, actualFrame);
}

