/*	CS585_Lab2
*	CS585 Image and Video Computing Fall 2018
*	Lab 2
*	--------------
*	This program introduces the following concepts:
*		a) Reading a stream of images from a webcamera, and displaying the video
*		b) Skin color detection
*		c) Background differencing
*		d) Visualizing motion history
*	--------------
*/

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

//function declarations

/**
Function that returns the maximum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMax(int a, int b, int c);

/**
Function that returns the minimum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMin(int a, int b, int c);

/**
Function that detects whether a pixel belongs to the skin based on RGB values
@param src The source color image
@param dst The destination grayscale image where skin pixels are colored white and the rest are colored black
*/
void mySkinDetect(Mat& src, Mat& dst);

/**
Function that does frame differencing between the current frame and the previous frame
@param src The current color image
@param prev The previous color image
@param dst The destination grayscale image where pixels are colored white if the corresponding pixel intensities in the current
and previous image are not the same
*/
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst);

/**
Function that accumulates the frame differences for a certain number of pairs of frames
@param mh Vector of frame difference images
@param dst The destination grayscale image to store the accumulation of the frame difference images
*/
void myMotionEnergy(vector<Mat> mh, Mat& dst);


int main()
{

	//----------------
	//a) Reading a stream of images from a webcamera, and displaying the video
	//----------------
	// For more information on reading and writing video: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html
	// open the video camera no. 0
	VideoCapture cap(0);

	// if not successful, exit program
	if (!cap.isOpened())
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	//create a window called "MyVideoFrame0"
	namedWindow("MyVideo0", WINDOW_AUTOSIZE);
	Mat frame0;

	// read a new frame from video
	bool bSuccess0 = cap.read(frame0);

	//if not successful, break loop
	if (!bSuccess0)
	{
		cout << "Cannot read a frame from video stream" << endl;
	}

	//show the frame in "MyVideo" window
	imshow("MyVideo0", frame0);

	//create a window called "MyVideo"
	namedWindow("MyVideo", WINDOW_AUTOSIZE);
	namedWindow("MyVideoMH", WINDOW_AUTOSIZE);
	namedWindow("Skin", WINDOW_AUTOSIZE);
	vector<Mat> myMotionHistory;
	Mat fMH1, fMH2, fMH3;
	fMH1 = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);
	fMH2 = fMH1.clone();
	fMH3 = fMH1.clone();
	myMotionHistory.push_back(fMH1);
	myMotionHistory.push_back(fMH2);
	myMotionHistory.push_back(fMH3);

	Mat tp[4], wbtp[4], fliptp[4];
	tp[0] = imread("Gesture/tp1.png");
	tp[1] = imread("Gesture/tp2.png");
	tp[2] = imread("Gesture/tp3.png");
	tp[3] = imread("Gesture/tp4.png");
	cv::resize(tp[0], tp[0], cv::Size(frame0.cols * 0.45, frame0.rows * 0.5));
	cv::resize(tp[1], tp[1], cv::Size(frame0.cols * 0.45, frame0.rows * 0.5));
	cv::resize(tp[2], tp[2], cv::Size(frame0.cols * 0.45, frame0.rows * 0.5));
	cv::resize(tp[3], tp[3], cv::Size(frame0.cols * 0.45, frame0.rows * 0.5));
	cvtColor(tp[0], tp[0], CV_BGR2GRAY);
	cvtColor(tp[1], tp[1], CV_BGR2GRAY);
	cvtColor(tp[2], tp[2], CV_BGR2GRAY);
	cvtColor(tp[3], tp[3], CV_BGR2GRAY);
	threshold(tp[0], wbtp[0], 20, 255, THRESH_BINARY);
	threshold(tp[1], wbtp[1], 20, 255, THRESH_BINARY);
	threshold(tp[2], wbtp[2], 20, 255, THRESH_BINARY);
	threshold(tp[3], wbtp[3], 20, 255, THRESH_BINARY);
	//cv::Mat flip0;
	//cv::Mat flip1;
	//cv::Mat flip2;
	//cv::Mat flip3;
	cv::flip(wbtp[0], fliptp[0], 1);
	cv::flip(wbtp[1], fliptp[1], 1);
	cv::flip(wbtp[2], fliptp[2], 1);
	cv::flip(wbtp[3], fliptp[3], 1);
	namedWindow("tp1", WINDOW_AUTOSIZE);
	namedWindow("tp2", WINDOW_AUTOSIZE);
	namedWindow("tp3", WINDOW_AUTOSIZE);
	namedWindow("tp4", WINDOW_AUTOSIZE);
	imshow("tp1", wbtp[0]);
	imshow("tp2", wbtp[1]);
	imshow("tp3", wbtp[2]);
	imshow("tp4", wbtp[3]);
	namedWindow("tp1flip", WINDOW_AUTOSIZE);
	namedWindow("tp2flip", WINDOW_AUTOSIZE);
	namedWindow("tp3flip", WINDOW_AUTOSIZE);
	namedWindow("tp4flip", WINDOW_AUTOSIZE);
	imshow("tp1flip", fliptp[0]);
	imshow("tp2flip", fliptp[1]);
	imshow("tp3flip", fliptp[2]);
	imshow("tp4flip", fliptp[3]);

	Mat result;
	int result_cols = frame0.cols - wbtp[0].cols + 1;
	int result_rows = frame0.rows - wbtp[0].rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);


	while (1)
	{
		// read a new frame from video
		Mat frame;
		bool bSuccess = cap.read(frame);

		//if not successful, break loop
		if (!bSuccess)
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}
		//show the frame in "MyVideo" window
		imshow("MyVideo0", frame0);

		// destination frame
		Mat frameDest;
		frameDest = Mat::zeros(frame.rows, frame.cols, CV_8UC1); //Returns a zero array of same size as src mat, and of type CV_8UC1
		//----------------
		//	b) Skin color detection
		//----------------



		mySkinDetect(frame, frameDest);
		imshow("Skin", frameDest);

		//right template
		double max_val_right = 0;
		double RminVal; double RmaxVal; Point RminLoc; Point RmaxLoc;
		int Rpos = -1;
		Point RmatchLoc;
		for (int i = 0; i < 4; i++) {
			matchTemplate(frameDest, wbtp[i], result, CV_TM_CCOEFF_NORMED);
			//normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
			minMaxLoc(result, &RminVal, &RmaxVal, &RminLoc, &RmaxLoc, Mat());
			
			/// Localizing the best match with minMaxLoc
			if (RmaxVal > max_val_right) {
				RmatchLoc = RmaxLoc;
				max_val_right = RmaxVal;
				Rpos = i;
			}
		}
		putText(frame, std::to_string(Rpos), Point(50, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 4, 8);
		//pos = -1;
		max_val_right = 0;
		rectangle(frame, RmatchLoc, Point(RmatchLoc.x + wbtp[0].cols, RmatchLoc.y + wbtp[0].rows), Scalar(255, 0, 0), 2, 8, 0);
		//rectangle(result, matchLoc, Point(matchLoc.x + wbtp[0].cols, matchLoc.y + wbtp[0].rows), Scalar::all(0), 2, 8, 0);
		//right template

		//left template
		double max_val_left = 0;
		double LminVal; double LmaxVal; Point LminLoc; Point LmaxLoc;
		Point LmatchLoc;
		int Lpos = -1;
		for (int i = 0; i < 4; i++) {
			matchTemplate(frameDest, fliptp[i], result, CV_TM_CCOEFF_NORMED);
			//normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

			/// Localizing the best match with minMaxLoc
			minMaxLoc(result, &LminVal, &LmaxVal, &LminLoc, &LmaxLoc, Mat());
			if (LmaxVal > max_val_left) {
				LmatchLoc = LmaxLoc;
				max_val_left = LmaxVal;
				Lpos = i;
			}

		}
		putText(frame, std::to_string(Lpos), Point(100, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 4, 8);
		max_val_left = 0;
		rectangle(frame, LmatchLoc, Point(LmatchLoc.x + fliptp[0].cols, LmatchLoc.y + fliptp[0].rows), Scalar(0, 0, 255), 2, 8, 0);
		//rectangle(result, matchLoc, Point(matchLoc.x + wbtp[0].cols, matchLoc.y + wbtp[0].rows), Scalar::all(0), 2, 8, 0);
		//left template


		//----------------
		//	c) Background differencing
		//----------------


		//call myFrameDifferencing function
		myFrameDifferencing(frame0, frame, frameDest);
		imshow("MyVideo", frameDest);


		myMotionHistory.erase(myMotionHistory.begin());
		myMotionHistory.push_back(frameDest);
		Mat myMH = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);

		//----------------
		//  d) Visualizing motion history
		//----------------

		//call myMotionEnergy function
		myMotionEnergy(myMotionHistory, myMH);


		imshow("MyVideoMH", myMH); //show the frame in "MyVideo" window
		frame0 = frame;
		//wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		if (waitKey(30) == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}

	}
	cap.release();
	return 0;
}

//Function that returns the maximum of 3 integers
int myMax(int a, int b, int c) {
	int m = a;
	(void)((m < b) && (m = b)); //short-circuit evaluation
	(void)((m < c) && (m = c));
	return m;
}

//Function that returns the minimum of 3 integers
int myMin(int a, int b, int c) {
	int m = a;
	(void)((m > b) && (m = b));
	(void)((m > c) && (m = c));
	return m;
}

//Function that detects whether a pixel belongs to the skin based on RGB values
void mySkinDetect(Mat& src, Mat& dst) {
	//Surveys of skin color modeling and detection techniques:
	//Vezhnevets, Vladimir, Vassili Sazonov, and Alla Andreeva. "A survey on pixel-based skin color detection techniques." Proc. Graphicon. Vol. 3. 2003.
	//Kakumanu, Praveen, Sokratis Makrogiannis, and Nikolaos Bourbakis. "A survey of skin-color modeling and detection methods." Pattern recognition 40.3 (2007): 1106-1122.
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			//For each pixel, compute the average intensity of the 3 color channels
			Vec3b intensity = src.at<Vec3b>(i, j); //Vec3b is a vector of 3 uchar (unsigned character)
			int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
			if ((R > 95 && G > 40 && B > 20) && (myMax(R, G, B) - myMin(R, G, B) > 15) && (abs(R - G) > 15) && (R > G) && (R > B)) {
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
}

//Function that does frame differencing between the current frame and the previous frame
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst) {
	//For more information on operation with arrays: http://docs.opencv.org/modules/core/doc/operations_on_arrays.html
	//For more information on how to use background subtraction methods: http://docs.opencv.org/trunk/doc/tutorials/video/background_subtraction/background_subtraction.html
	absdiff(prev, curr, dst);
	Mat gs = dst.clone();
	cvtColor(dst, gs, CV_BGR2GRAY);
	dst = gs > 50;
}

//Function that accumulates the frame differences for a certain number of pairs of frames
void myMotionEnergy(vector<Mat> mh, Mat& dst) {
	Mat mh0 = mh[0];
	Mat mh1 = mh[1];
	Mat mh2 = mh[2];

	for (int i = 0; i < dst.rows; i++) {
		for (int j = 0; j < dst.cols; j++) {
			if (mh0.at<uchar>(i, j) == 255 || mh1.at<uchar>(i, j) == 255 || mh2.at<uchar>(i, j) == 255) {
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
}