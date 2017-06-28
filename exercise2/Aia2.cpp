//============================================================================
// Name        : Aia2.cpp
// Author      : Ronny Haensch
// Version     : 1.0
// Copyright   : -
// Description : 
//============================================================================

#include "Aia2.h"

// calculates the contour line of all objects in an image
/*
img			the input image
objList		vector of contours, each represented by a two-channel matrix
thresh		threshold used to binarize the image
k			number of applications of the erosion operator
*/
void Aia2::getContourLine(const Mat& img, vector<Mat>& objList, int thresh, int k){
  Mat dst;
  threshold(img, dst, thresh, 255, THRESH_BINARY_INV);
  erode(dst, dst, Mat(), Point(-1, -1), k);
  findContours(dst, objList, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);  
}

// calculates the (unnormalized!) fourier descriptor from a list of points
/*
contour		1xN 2-channel matrix, containing N points (x in first, y in second channel)
out		fourier descriptor (not normalized)
*/
Mat Aia2::makeFD(const Mat& contour){
  Mat src, dst;
  contour.convertTo(src, CV_32F);
  dft(src, dst);
  return dst;
}

// normalize a given fourier descriptor
/*
fd		the given fourier descriptor
n		number of used frequencies (should be even)
out		the normalized fourier descriptor
*/
//int scale_index = 0;
Mat Aia2::normFD(const Mat& fd, int n){
  Mat fdc = fd.clone();
  
  //plotFD(fdc, "fd not normalized", 0);
  
  // translation invariance: set F[0] to zero
  fdc.at<Vec2f>(0) = Vec2f(0,0);
  //plotFD(fdc, "fd translation invariant", 0);
  
  // scale invariance: divide by magnitude of F[1]
  // TODO: magnitude of F[1] is zero (and magnitude of F[1] is zero or almost zero))
    float mag_chosen;
    int lastindex = fdc.rows-1;
    float mag_f1 = sqrt(pow(fdc.at<Vec2f>(1)[0],2) + pow(fdc.at<Vec2f>(1)[1],2));
    float mag_f1_neg = sqrt(pow(fdc.at<Vec2f>(lastindex)[0],2) + pow(fdc.at<Vec2f>(lastindex)[1],2));
    if (mag_f1 > mag_f1_neg) {
        mag_chosen = mag_f1;;
    }
    else {
      mag_chosen = mag_f1_neg;
    }
  fdc /= mag_chosen;
  //plotFD(fdc, "fd translation and scale invariant", 0);
  
  // rotation invariance: set F[i] := magnitude(F[i])
  Mat planes[2];
  split(fdc, planes);
  cartToPolar(planes[0], planes[1], planes[0], planes[1]);
  //plotFD(planes[0], "fd translation, scale, and rotation invariant", 0);
  
  // smaller sensitivity for details
  // wrt negative/positive/frequency: the corresponding angles are in planes[1]
  // I just used a blur filter to try and remove higher frequencies
  Mat nfd;
  //nfd = planes[0](Rect(0, 0, 1, n));
  //plotFD(nfd, "fd translation, scale, and rotation invariant, smaller sensitivity", 0);
  vconcat(planes[0](Rect(0, 0, 1, n/2)), planes[0](Rect(0,planes[0].rows-n/2,1,n/2)), nfd);
//  cout << planes[0] << endl;
//  cout << "************************" << endl;
//  cout << planes[0].cols << " " << planes[0].rows << endl;
//  cout << "************************" << endl;
//  cout << planes[0](Rect(0, 0, 1, n/2)) << endl;
//  cout << planes[0](Rect(0, planes[0].rows - n/2, 1, n/2));


  return nfd;
}

// plot fourier descriptor
/*
fd	the fourier descriptor to be displayed
win	the window name
dur	wait number of ms or until key is pressed
*/
void Aia2::plotFD(const Mat& fd, string win, double dur){
  
  // if fd is 1 channel, add second channel
  Mat fd_2c = fd;
  if (fd.channels() == 1) {
    vector<Mat> chnls;
    Mat secchan = Mat::zeros(fd.size(), CV_32F);
    chnls.push_back(fd);
    chnls.push_back(secchan);
    merge(chnls, fd_2c);
  }

  // retrieve pixel coordinates
  Mat pc;
  dft(fd_2c, pc, DFT_INVERSE);

  // shift to non-negative coordinates
  double min_x, max_x, min_y, max_y;  
  Mat planes[2];
  split(pc, planes);
  minMaxIdx(planes[0], &min_x, &max_x, NULL, NULL);
  minMaxIdx(planes[1], &min_y, &max_y, NULL, NULL); 
  pc -= Vec2f(min_x, min_y);
  max_x -= min_x;
  max_y -= min_y;

  // rescale to fit into 200x200 image
  pc /= max(max_x, max_y);
  pc *= 199;

  // initialize black image
  Mat img = Mat(200, 200, CV_8UC1, Scalar(0));

  // set pixel coordinates to white
  for (int i=0; i<pc.rows; i++) {
    Vec2f coordinates = pc.at<Vec2f>(i);
    int x = round(coordinates[0]);
    int y = round(coordinates[1]);
    img.at<uchar>(x,y) = 255;      
  }

  // show image
  // showImage(img, win, dur);
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function loads input image, calls processing functions, and saves result
// in particular extracts FDs and compares them to templates
/*
img			path to query image
template1	path to template image of class 1
template2	path to template image of class 2
*/
void Aia2::run(string img, string template1, string template2){

	// process image data base
	// load image as gray-scale, paths in argv[2] and argv[3]
	Mat exC1 = imread( template1, 0);
	Mat exC2  = imread( template2, 0);
	if ( (!exC1.data) || (!exC2.data) ){
	    cout << "ERROR: Cannot load class examples in\n" << template1 << "\n" << template2 << endl;
	    cerr << "Press enter to continue..." << endl;
	    cin.get();
	    exit(-1);
	}

	// parameters
	// these two will be adjusted below for each image individually
	int binThreshold;				// threshold for image binarization
	int numOfErosions;				// number of applications of the erosion operator
	// these two values work fine, but might be interesting for you to play around with them
	int steps = 32;					// number of dimensions of the FD
	double detThreshold = 0.01;		// threshold for detection

	// get contour line from images
	vector<Mat> contourLines1;
	vector<Mat> contourLines2;
	// TO DO !!!
	// --> Adjust threshold and number of erosion operations
	binThreshold = 155;
	numOfErosions = 6;
	getContourLine(exC1, contourLines1, binThreshold, numOfErosions);
	int mSize = 0, mc1 = 0, mc2 = 0, i = 0;
	for(vector<Mat>::iterator c = contourLines1.begin(); c != contourLines1.end(); c++,i++){
		if (mSize<c->rows){
			mSize = c->rows;
			mc1 = i;
		}
	}

	getContourLine(exC2, contourLines2, binThreshold, numOfErosions);
	for(vector<Mat>::iterator c = contourLines2.begin(); c != contourLines2.end(); c++, i++){
		if (mSize<c->rows){
			mSize = c->rows;
			mc2 = i;
		}
	}
	// calculate fourier descriptor
	Mat fd1 = makeFD(contourLines1.at(mc1));
	Mat fd2 = makeFD(contourLines2.at(mc2));

	// normalize  fourier descriptor
	Mat fd1_norm = normFD(fd1, steps);
	Mat fd2_norm = normFD(fd2, steps);

	// process query image
	// load image as gray-scale, path in argv[1]
	Mat query = imread( img, 0);
	if (!query.data){
	    cerr << "ERROR: Cannot load query image in\n" << img << endl;
	    cerr << "Press enter to continue..." << endl;
	    cin.get();
	    exit(-1);
	}
	
	// get contour lines from image
	vector<Mat> contourLines;
	// TO DO !!!
	// --> Adjust threshold and number of erosion operations
	binThreshold = 130;
	numOfErosions = 4;
	getContourLine(query, contourLines, binThreshold, numOfErosions);
	
	cout << "Found " << contourLines.size() << " object candidates" << endl;

	// just to visualize classification result
	Mat result(query.rows, query.cols, CV_8UC3);
	vector<Mat> tmp;
	tmp.push_back(query);
	tmp.push_back(query);
	tmp.push_back(query);
	merge(tmp, result);

	// loop through all contours found
	i = 1;
	for(vector<Mat>::iterator c = contourLines.begin(); c != contourLines.end(); c++, i++){

	    cout << "Checking object candidate no " << i << " :\t";
	  
		// color current object in yellow
	  	Vec3b col(0,255,255);
	    for(int p=0; p < c->rows; p++){
			result.at<Vec3b>(c->at<Vec2i>(p)[1], c->at<Vec2i>(p)[0]) = col;
	    }
	    //showImage(result, "result", 0);
	    
	    // if fourier descriptor has too few components (too small contour), then skip it (and color it in blue)
	    if (c->rows < steps){
			cout << "Too less boundary points (" << c->rows << " instead of " << steps << ")" << endl;
			col = Vec3b(255,0,0);
	    }else{
			// calculate fourier descriptor
			Mat fd = makeFD(*c);
			// normalize fourier descriptor
			Mat fd_norm = normFD(fd, steps);
			// compare fourier descriptors
			double err1 = norm(fd_norm, fd1_norm)/steps;
			double err2 = norm(fd_norm, fd2_norm)/steps;
			// if similarity is too small, then reject (and color in cyan)
			if (min(err1, err2) > detThreshold){
				cout << "No class instance ( " << min(err1, err2) << " )" << endl;
				col = Vec3b(255,255,0);
			}else{
				// otherwise: assign color according to class
				if (err1 > err2){
					col = Vec3b(0,0,255);
					cout << "Class 2 ( " << err2 << " )" << endl;
				}else{
					col = Vec3b(0,255,0);
					cout << "Class 1 ( " << err1 << " )" << endl;
				}
			}
		}
		// draw detection result
	    for(int p=0; p < c->rows; p++){
			result.at<Vec3b>(c->at<Vec2i>(p)[1], c->at<Vec2i>(p)[0]) = col;
	    }
	    // for intermediate results, use the following line
	    //showImage(result, "result", 0);

	}
	// save result
	imwrite("result.png", result);
	// show final result
	showImage(result, "result", 0);
}

// shows the image
/*
img	the image to be displayed
win	the window name
dur	wait number of ms or until key is pressed
*/
void Aia2::showImage(const Mat& img, string win, double dur){
  
    // use copy for normalization
    Mat tempDisplay = img.clone();
    if (img.channels() == 1) normalize(img, tempDisplay, 0, 255, CV_MINMAX);
    // create window and display omage
    namedWindow( win.c_str(), CV_WINDOW_AUTOSIZE );
    imshow( win.c_str(), tempDisplay );
    // wait
    if (dur>=0) waitKey(dur);
    
}

// function loads input image and calls processing function
// output is tested on "correctness" 
void Aia2::test(void){
	test_getContourLine();
	test_makeFD();
	test_normFD();
	
}

void Aia2::test_getContourLine(void){

	vector<Mat> objList;
	Mat img(100, 100, CV_8UC1, Scalar(255));
	Mat roi(img, Rect(40,40,20,20));
	roi.setTo(0);
	getContourLine(img, objList, 128, 1);
	Mat cline(68,1,CV_32SC2);
	int k=0;
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(41,i);
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(i,58);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(58, i);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(i,41);
  if ( sum(cline != objList.at(0)).val[0] != 0 ){
		cout << "There might be a problem with Aia2::getContourLine(..)!" << endl;
		cin.get();
	}
}

void Aia2::test_makeFD(void){

	Mat cline(68,1,CV_32SC2);
	int k=0;
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(41,i);
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(i,58);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(58, i);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(i,41);
	
	Mat fd = makeFD(cline);
	if (fd.rows != cline.rows){
		cout << "There is be a problem with Aia2::makeFD(..):" << endl;
		cout << "\tThe number of frequencies does not match the number of contour points" << endl;
		cin.get();
		exit(-1);
	}
	if (fd.channels() != 2){
		cout << "There is be a problem with Aia2::makeFD(..):" << endl;
		cout << "\tThe fourier descriptor is supposed to be a two-channel, 1D matrix" << endl;
		cin.get();
		exit(-1);
	}
}

void Aia2::test_normFD(void){

	double eps = pow(10,-3);

	Mat cline(68,1,CV_32SC2);
	int k=0;
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(41,i);
	for(int i=41; i<58; i++) cline.at<Vec2i>(k++) = Vec2i(i,58);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(58, i);
	for(int i=58; i>41; i--) cline.at<Vec2i>(k++) = Vec2i(i,41);
	
	Mat fd = makeFD(cline);
	Mat nfd = normFD(fd, 32);
	if (nfd.channels() != 1){
		cout << "There is be a problem with Aia2::normFD(..):" << endl;
		cout << "\tThe normalized fourier descriptor is supposed to be a one-channel, 1D matrix" << endl;
		cin.get();
		exit(-1);
	}
	if (abs(nfd.at<float>(0)) > eps){
		cout << "There is be a problem with Aia2::normFD(..):" << endl;
		cout << "\tThe F(0)-component of the normalized fourier descriptor F is supposed to be 0" << endl;
		cin.get();
		exit(-1);
	}
	if ((abs(nfd.at<float>(1)-1.) > eps) && (abs(nfd.at<float>(nfd.rows-1)-1.) > eps)){
		cout << "There is be a problem with Aia2::normFD(..):" << endl;
		cout << "\tThe F(1)-component of the normalized fourier descriptor F is supposed to be 1" << endl;
		cout << "\tBut what if the unnormalized F(1)=0?" << endl;
		cin.get();
		exit(-1);
	}
	if (nfd.rows != 32){
		cout << "There is be a problem with Aia2::normFD(..):" << endl;
		cout << "\tThe number of components does not match the specified number of components" << endl;
		cin.get();
		exit(-1);
	}
}
