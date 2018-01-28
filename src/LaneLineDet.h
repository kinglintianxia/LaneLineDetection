#ifndef LANELINEDET_H
#define LANELINEDET_H

#include <opencv2/opencv.hpp>
#include <cmath>

// USE_VIDEO  USE_IMAGE
#define USE_IMAGE	//USE_VIDEO  USE_IMAGE
#define Release // Release & Debug

using namespace cv;
using namespace std;

class LaneLineDet
{
public:
    LaneLineDet(std::string param_path);
    void LaneLineDetection( Mat &src);

    int cntNum;
	char Num[3];
private:
    void ImgPreProcess(cv::Mat& input, cv::Mat& output);    // Image preprocess
    void PerspectiveTrans(const Mat &input, Mat &output, bool flag);    // prespective transformation
    void ThresholdOTSU(cv::Mat &src, Mat &dst);
    void HoughLines(cv::Mat &src, cv::Mat &houghImg);
    void ColorFilter(cv::Mat& input, cv::Mat& output);
    void NarrowImg(cv::Mat &input, cv::Mat& output);
    void ShowCntNum(cv::Mat& cnt_img, cv::Mat &Img);
    cv::Mat src_img;		//src Image
//	Mat narrowImg;
    std::string param_file_path;
    int color;
    cv::Mat warp_mat;

    // For threshold
	int g_nThresh;
	int g_nlastThresh;
	float lastProbability;
	float probability;

};



#endif


/****************End of File*************************/
