//============================================================================
// Name        : Dip1.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description : 
//============================================================================

#include "Dip1.h"

// function that performs some kind of (simple) image processing
/*
img	input image
return	output image
*/
Mat Dip1::doSomethingThatMyTutorIsGonnaLike(Mat& img){
  
	// TO DO !!!
    
    //OBJECTIVE: detect egdes of image
    Mat src, src_gray;
    Mat grad;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    
    src = img.clone();
    //remove noise via blurring
    GaussianBlur(src, src, Size(3,3), 0, 0, BORDER_DEFAULT);
    
    //convert it to gray
    cvtColor(src, src_gray, CV_BGR2GRAY);
    
    //generate grad_x and grad_y
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    
    Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_x, abs_grad_x );
    
    Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_y, abs_grad_y );
    
    //total Gradient (approximate)
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
    
    //OBJECTIVE: add watermark to image whilst maintaining colors
    float alpha = 0.1;
    Mat imgLog = imread("OpenCVlogo.png");
    Mat imgRet = grad.clone();
    cv::Vec3b pixelVal;
    cv::Vec3b pixelLogVal;
    
    for (int ii = 1; ii <= imgLog.rows; ii++)
    {
        for (int jj = 1; jj <= imgLog.cols; jj++)
        {

            pixelVal = grad.at<cv::Vec3b>(grad.rows - ii, grad.cols - jj);
            pixelLogVal = imgLog.at<cv::Vec3b>(imgLog.rows - ii, imgLog.cols - jj);

            if (pixelLogVal != cv::Vec3b(0,0,0))
            {

                pixelVal[0] = alpha * pixelVal[0] + (1 - alpha) * pixelLogVal[0];
                pixelVal[1] = alpha * pixelVal[1] + (1 - alpha) * pixelLogVal[1];
                pixelVal[2] = alpha * pixelVal[2] + (1 - alpha) * pixelLogVal[2];
            }
            
            imgRet.at<cv::Vec3b>(imgRet.rows - ii, imgRet.cols - jj) = pixelVal;
        }
    }
    
    return imgRet;

}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function loads input image, calls processing function, and saves result
/*
fname	path to input image
*/
void Dip1::run(string fname){

	// window names
	string win1 = string ("Original image");
	string win2 = string ("Result");
  
	// some images
	Mat inputImage, outputImage;
  
	// load image
	cout << "load image" << endl;
	inputImage = imread( fname );
	cout << "done" << endl;
	
	// check if image can be loaded
	if (!inputImage.data){
	    cout << "ERROR: Cannot read file " << fname << endl;
	    cout << "Press enter to continue..." << endl;
	    cin.get();
	    exit(-1);
	}

	// show input image
	namedWindow( win1.c_str() );
	imshow( win1.c_str(), inputImage );
	
	// do something (reasonable!)
	outputImage = doSomethingThatMyTutorIsGonnaLike( inputImage );
	
	// show result
	namedWindow( win2.c_str() );
	imshow( win2.c_str(), outputImage );
	
	// save result
	imwrite("result.jpg", outputImage);
	
	// wait a bit
	waitKey(0);

}

// function loads input image and calls the processing functions
// output is tested on "correctness" 
/*
fname	path to input image
*/
void Dip1::test(string fname){

	// some image variables
	Mat inputImage, outputImage;
  
	// load image
	inputImage = imread( fname );

	// check if image can be loaded
	if (!inputImage.data){
	    cout << "ERROR: Cannot read file " << fname << endl;
	    cout << "Continue with pressing enter..." << endl;
	    cin.get();
	    exit(-1);
	}

	// create output
	outputImage = doSomethingThatMyTutorIsGonnaLike( inputImage );
	// test output
	test_doSomethingThatMyTutorIsGonnaLike(inputImage, outputImage);

}

// function loads input image and calls processing function
// output is tested on "correctness" 
/*
inputImage	input image as used by doSomethingThatMyTutorIsGonnaLike()
outputImage	output image as created by doSomethingThatMyTutorIsGonnaLike()
*/
void Dip1::test_doSomethingThatMyTutorIsGonnaLike(Mat& inputImage, Mat& outputImage){

	// ensure that input and output have equal number of channels
	if ( (inputImage.channels() == 3) and (outputImage.channels() == 1) )
		cvtColor(inputImage, inputImage, CV_BGR2GRAY);

	// split (multi-channel) image into planes
	vector<Mat> inputPlanes, outputPlanes;
	split( inputImage, inputPlanes );
	split( outputImage, outputPlanes );

	// number of planes (1=grayscale, 3=color)
	int numOfPlanes = inputPlanes.size();

	// calculate and compare image histograms for each plane
	Mat inputHist, outputHist;
	// number of bins
	int histSize = 100;
	float range[] = { 0, 256 } ;
	const float* histRange = { range };
	bool uniform = true; bool accumulate = false;
	double sim = 0;
	for(int p = 0; p < numOfPlanes; p++){
		// calculate histogram
		calcHist( &inputPlanes[p], 1, 0, Mat(), inputHist, 1, &histSize, &histRange, uniform, accumulate );
		calcHist( &outputPlanes[p], 1, 0, Mat(), outputHist, 1, &histSize, &histRange, uniform, accumulate );
		// normalize
		inputHist = inputHist / sum(inputHist).val[0];
		outputHist = outputHist / sum(outputHist).val[0];
		// similarity as histogram intersection
		sim += compareHist(inputHist, outputHist, CV_COMP_INTERSECT);
	}
	sim /= numOfPlanes;

	// check whether images are to similar after transformation
	if (sim >= 0.8)
		cout << "Warning: The input and output image seem to be quite similar (similarity = " << sim << " ). Are you sure your tutor is gonna like your work?" << endl;

}
