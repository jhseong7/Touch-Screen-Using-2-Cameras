/*--------------------------------------------------*/
/* Touch Screen Using 2 Cameras						*/
/* Created on:  25th Dec 2015 by Jong Hyun Seong	*/
/* Last Edited 30th Dec 2015 bt Jong Hyun Seong     */
/* All copyrights reserved							*/
/*--------------------------------------------------*/



/*------------------------------*/
/*			Includes			*/
/*------------------------------*/

#include <cstdio>
#include <cstdlib>
#include <Windows.h>
#include <iostream>
#include <opencv\cv.hpp>
#include <cmath>

/*------------------------------*/
/*		Local Defines			*/
/*------------------------------*/

#define MORPH_SIZE_CAM0_X 1
#define MORPH_SIZE_CAM0_Y 3

#define MORPH_SIZE_CAM1_X 1
#define MORPH_SIZE_CAM1_Y 3

#define CAMERA_0_TAN_BASE 100
#define CAMERA_1_TAN_BASE 100

#define MOUSE_MULTIPLIER_X 10
#define MOUSE_MULTIPLIER_Y 10

using namespace cv;
using namespace std;

/*------------------------------*/
/*	Local Global Variables		*/
/*------------------------------*/

//Camera 0 Parameters
Point ROIOriginStart = { 0, 0 }; 
Point ROIOriginEnd = { 0, 0 };

Rect ROIregion;

bool Start_found = false;
bool End_found = false;

//Camera 1 Parameters
Point ROIOriginStart2 = { 0, 0 };
Point ROIOriginEnd2 = { 0, 0 };

Rect ROIregion2;

bool Start_found2 = false;
bool End_found2 = false;


//Tracker Settings
bool MouseTrackON = false;



/*------------------------------*/
/*	Local Functions				*/
/*------------------------------*/

void ResetRegion(void)
{
	Start_found = false, End_found = false;
	Start_found2 = false, End_found2 = false;

	std::cout << "ROI0, ROI1 has been reset. please designate it again" << std::endl;
}


void MouseCallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN && !Start_found)
	{
		cout << "Start Position of ROI0 designated at (" << x << ", " << y << ")" << endl;
		ROIOriginStart = { x, y };
		Start_found = true;

	}

	else if (event == EVENT_LBUTTONDOWN && Start_found && !End_found)
	{
		cout << "End Position of ROI0 designated at (" << x << ", " << y << ")" << endl;
		ROIOriginEnd = { x, y };

		ROIregion = Rect(ROIOriginStart, ROIOriginEnd);

		End_found = true;
	}

	if (Start_found && !End_found)
	{
		ROIOriginEnd = { x, y };
	}
}

void MouseCallBackFuncCam2(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN && !Start_found2)
	{
		cout << "Start Position of ROI1 designated at (" << x << ", " << y << ")" << endl;
		ROIOriginStart2 = { x, y };
		Start_found2 = true;

	}

	else if (event == EVENT_LBUTTONDOWN && Start_found2 && !End_found2)
	{
		cout << "End Position of ROI1 designated at (" << x << ", " << y << ")" << endl;
		ROIOriginEnd2 = { x, y };

		ROIregion2 = Rect(ROIOriginStart2, ROIOriginEnd2);

		End_found2 = true;
	}

	if (Start_found2 && !End_found2)
	{
		ROIOriginEnd2 = { x, y };
	}
}



Point TrackedPosition(Mat* Background)
{
	int x_size = Background->cols;
	int y_size = Background->rows;

	int min_x, max_x, min_y, max_y;
	
	min_x = x_size;
	min_y = y_size;

	max_x = 0;
	max_y = 0;

	Point Hand_Position;

	for (int x = 0; x < x_size; x++)
	{
		for (int y = 0; y < y_size; y++)
		{
			if (Background->data[y*x_size + x]==255)
			{
				if (x > max_x)
					max_x = x;
				if (x < min_x)
					min_x = x;
				if (y > max_y)
					max_y = y;
				if (y < min_y)
					min_y = y;
			}			
		}
	}


	Hand_Position.x = (min_x + max_x) / 2;
	Hand_Position.y = (min_y + max_y) / 2;

	return Hand_Position;


}


int main(void)
{
	std::cout << "Initializing Cameras" << std::endl;


	VideoCapture Webcam0(0);

	std::cout << "Initialized Camera 0" << std::endl;

	VideoCapture Webcam1(1);

	std::cout << "Initialized Camera 1" << std::endl;

	Mat Frame0, Frame1, MoGframe0, MoGframe1;
	Mat ROI0, ROI1;

	Ptr<BackgroundSubtractorMOG2> pMOG0;
	pMOG0 = createBackgroundSubtractorMOG2();

	Ptr<BackgroundSubtractorMOG2> pMOG1;
	pMOG1 = createBackgroundSubtractorMOG2();

	std::cout << "Initialized Background Subtractors" << std::endl;

	POINT MousePosition;
	
	Point HandPosition0, HandPosition1;

	Point CalculatedPosition;

	Point Offset(2000, 0);

	
	Mat element0 = getStructuringElement(MORPH_RECT, Size(2 * MORPH_SIZE_CAM0_X + 1, 2 * MORPH_SIZE_CAM0_Y + 1), Point(MORPH_SIZE_CAM0_X, MORPH_SIZE_CAM0_Y));
	Mat element1 = getStructuringElement(MORPH_RECT, Size(2 * MORPH_SIZE_CAM1_X + 1, 2 * MORPH_SIZE_CAM1_Y + 1), Point(MORPH_SIZE_CAM1_X, MORPH_SIZE_CAM1_Y));

	Mat ROI_image0, Morphed0, Cropped0;
	Mat ROI_image1, Morphed1, Cropped1;

	if (Webcam0.isOpened() && Webcam1.isOpened())
	{
		Webcam0 >> Frame0;
		Webcam1 >> Frame1;

		std::cout << "All Cameras have been Successfully Initialized" << std::endl;
	}


	int Rows = Frame0.rows;
	int Cols = Frame1.cols;

	//Windows 생성
	namedWindow("Webcam0", CV_WINDOW_AUTOSIZE);
	namedWindow("Webcam1", CV_WINDOW_AUTOSIZE);
	namedWindow("ROI0", CV_WINDOW_AUTOSIZE);
	namedWindow("Morphed Background0", CV_WINDOW_AUTOSIZE);
	namedWindow("ROI1", CV_WINDOW_AUTOSIZE);
	namedWindow("Morphed Background1", CV_WINDOW_AUTOSIZE);

	//Mouse Callback
	setMouseCallback("Webcam0", MouseCallBackFunc, NULL);
	setMouseCallback("Webcam1", MouseCallBackFuncCam2, NULL);


	std::cout << "Starting Program" << std::endl;

	while (Webcam0.isOpened() && Webcam1.isOpened())
	{
		Webcam0 >> Frame0;
		Webcam1 >> Frame1;

		if (Start_found && End_found)
		{
			ROI_image0 = Frame0(ROIregion);

			ROI_image0.copyTo(Cropped0);

			pMOG0->apply(Cropped0, MoGframe0);

			threshold(MoGframe0, MoGframe0, 250, 255, THRESH_BINARY);
			cv::morphologyEx(MoGframe0, Morphed0, MORPH_OPEN, element0);

			HandPosition0 = TrackedPosition(&Morphed0);

			//추적 지점의 X축으로 긴 직선을 그린다
			rectangle(Frame0, Point(ROIOriginStart.x + HandPosition0.x, 0), Point(ROIOriginStart.x + HandPosition0.x, 0) + Point(1, Rows), Scalar(255, 255, 0), -1, 8, 0);

			ostringstream DebugData0;
			
			double Theta0 = (180.0 / 3.141)*atan((double)(ROIOriginStart.x + HandPosition0.x - Cols / 2) / (double)CAMERA_0_TAN_BASE);

			DebugData0 << "Degrees:" << Theta0 << "deg";

			putText(Frame0, DebugData0.str(), Point(30, 30), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1, 8, false);

			//추적 지점의 X축으로 점을 그린다
			rectangle(Frame0, ROIOriginStart + HandPosition0, ROIOriginStart + HandPosition0 + Point(5, 5), Scalar(0, 255, 0), -1, 8, 0);


			imshow("ROI0", Cropped0);
			imshow("Morphed Background0", Morphed0);
		}

		if (Start_found2 && End_found2)
		{
			ROI_image1 = Frame1(ROIregion2);

			ROI_image1.copyTo(Cropped1);

			pMOG1->apply(Cropped1, MoGframe1);

			threshold(MoGframe1, MoGframe1, 250, 255, THRESH_BINARY);
			cv::morphologyEx(MoGframe1, Morphed1, MORPH_OPEN, element1);

			HandPosition1 = TrackedPosition(&Morphed1);

			//추적 지점의 X축으로 긴 직선을 그린다
			rectangle(Frame1, Point(0, ROIOriginStart2.y + HandPosition1.y), Point(0, ROIOriginStart2.y + HandPosition1.y) + Point( Cols, 1), Scalar(255, 255, 0), -1, 8, 0);

			ostringstream DebugData1;

			double Theta1 = (180.0 / 3.141)*atan((double)(ROIOriginStart2.y + HandPosition1.y - Rows / 2) / (double)CAMERA_1_TAN_BASE);

			DebugData1 << "Degrees:" << Theta1 << "deg";

			putText(Frame1, DebugData1.str(), Point(30, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1, 8, false);


			rectangle(Frame1, ROIOriginStart2 + HandPosition1, ROIOriginStart2 + HandPosition1 + Point(5, 5), Scalar(0, 255, 0), -1, 8, 0);


			imshow("ROI1", Cropped1);
			imshow("Morphed Background1", Morphed1);
		}


		//Draw ROI on the frame
		rectangle(Frame0, ROIOriginStart, ROIOriginEnd, Scalar(255, 0, 0), 2, 8, 0);
		rectangle(Frame1, ROIOriginStart2, ROIOriginEnd2, Scalar(255, 0, 0), 2, 8, 0);


		//Update Webcam Frame
		imshow("Webcam0", Frame0);
		imshow("Webcam1", Frame1);


		//Move Windows to 
		moveWindow("Webcam0", 0, 0);
		moveWindow("Webcam1", 0 + Cols, 0);

		moveWindow("ROI0", 0, 0 + Rows + 30);
		moveWindow("ROI1", 2*Cols, 0 );

		moveWindow("Morphed Background0", 0, 0 + Rows + 30);
		moveWindow("Morphed Background1", 2*Cols, Rows + 100);


		//Get Cursor Position with WINAPI
		GetCursorPos(&MousePosition);


		char key;
		//Get Pressed Key
		key = waitKey(1);

		if (key == 'r')
			ResetRegion();
		else if (key == 27)
			break;


		//Mouse Track Option Set------//
		if (key == 'm')
			MouseTrackON = true;
		else if (key == 'n')
			MouseTrackON = false;
		//----------------------------//

		CalculatedPosition = { MOUSE_MULTIPLIER_X*(640 - HandPosition0.x), MOUSE_MULTIPLIER_Y*(HandPosition1.y) };

		if (End_found2 && End_found && MouseTrackON)
			if ( !SetCursorPos(CalculatedPosition.x, CalculatedPosition.y) )
				std::cout << "Set Fail" << std::endl;

			


	}

	std::cout << "Terminating Program" << std::endl;


	return 0;
}
