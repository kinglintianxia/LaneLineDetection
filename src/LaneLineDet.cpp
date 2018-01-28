#include "LaneLineDet.h"

// Constructor function
LaneLineDet::LaneLineDet(std::string param_path)
{
    param_file_path = param_path;
    //
    cv::FileStorage fs(param_file_path, cv::FileStorage::READ);
    color = fs["Color"];
    std::string homo_path = fs["homography"];
    cv::FileStorage fo(homo_path, cv::FileStorage::READ);
    fo["homography"] >> warp_mat;
    fo.release();
    fs.release();
    //
	cntNum=0;
	g_nlastThresh=0;
	g_nThresh=0;
	lastProbability=0.0;
	probability=0.0;

}


/**********Image preprocess*******************/
void LaneLineDet::ImgPreProcess(cv::Mat& input, cv::Mat& output)
{
    //To gray Image
    cv::Mat gray_img;
    if (!color)     // gray
        cv::cvtColor(input,gray_img,CV_BGR2GRAY);
    else    // color to hsv
        input.copyTo(gray_img);


    //    //Set ROI Image
    //    ROIImg=src(Rect(0,src.rows/2,src.cols,src.rows/2));	// Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
#ifdef Debug
    cv::imshow("grayImg",gray_img);
#endif
    // Filter
    cv::GaussianBlur(gray_img, gray_img, cv::Size(3,5), 0, 0);
    cv::bilateralFilter(gray_img, output, 5, 5*2, 5/2);
#ifdef Debug
    imshow("blurImg",output);
#endif

}


/*********prespective transformation**************/
void LaneLineDet::PerspectiveTrans(const cv::Mat &input, cv::Mat &output, bool flag)
{

    if (flag)   // warp_mat.inverse
        cv::warpPerspective(input,
                            output,
                            warp_mat,
                            input.size(),
                            cv::WARP_INVERSE_MAP | cv::INTER_LINEAR,
                            cv::BORDER_CONSTANT, cv::Scalar::all(0) // Fill border with black
                            );
    else    // warp_mat
        cv::warpPerspective(input,
                            output,
                            warp_mat,input.size(),
                            cv::INTER_LINEAR,
                            cv::BORDER_CONSTANT, cv::Scalar::all(255) // Fill border with black
                            );  //
#ifdef Debug
    imshow("BirdEyeImg",output);
#endif
}

//  OTSU
void LaneLineDet::ThresholdOTSU(Mat &input, Mat &output)
{
    int height=input.rows;
    int width=input.cols;
//    int start=src.rows*1/3;

    //histogram
    float histogram[256]={0};
    for(int i=0;i<height;i++)
    {
        uchar *p=input.ptr<uchar>(i);		//get the header of row i
//		uchar* p=(uchar*)src.data+src.step*i;  //
        for(int j=0;j<width;j++)	//
        {			//*(p+j)=data[i][j]
            histogram[*p++]++;	//
        }
    }
    //normalize histogram
    int size=height*width;
    for(int i=0;i<256;i++)
    {
        histogram[i]=histogram[i]/size;
    }

    //average pixel value
    float avgValue=0;
    for(int i=0;i<256;i++)
    {
        avgValue+=i*histogram[i];  // u
    }

//  int thresh;
    float maxVariance=0;    // g
    float w=0, u=0;
    // iterator the histogram
    for(int i=0; i<256; i++)
    {
        w += histogram[i];  //   w0
        u += i*histogram[i];  //  u0

        float t = avgValue*w-u;
        float variance = t*t/(w*(1-w));
        if(variance > maxVariance)
        {
            maxVariance = variance;
            g_nThresh = i;

        }
    }
//	cout<<"threshold0="<<g_nThresh<<endl;
    //count the number of white pixels
    int cnt=0;
    for(int i=0;i<output.rows;i++)
    {
        uchar *data=output.ptr<uchar>(i);
        for(int j=0;j<output.cols;j++)
        {
            if(data[j]==255)
                cnt++;
        }
    }

    //±
    probability=(float)cnt/(output.rows*output.cols);
//	cout<<"cnt: "<<cnt<<endl;

    if(g_nlastThresh != 0 && fabs(float(g_nThresh-g_nlastThresh)) >= 8)
        g_nThresh=g_nlastThresh;
    else if(probability >= 0.02 && probability < 0.2)	//
        g_nlastThresh=g_nThresh;

/************************************************************************/
    cv::Mat bin_img;
    threshold(input, bin_img, g_nThresh, 255, CV_THRESH_BINARY);
#ifdef Debug
    cv::imshow("binary_img", bin_img);
#endif
    NarrowImg(bin_img, output);
}

void LaneLineDet::NarrowImg(cv::Mat &input, cv::Mat& output)
{
    // narrow image
    cv::Mat narrow_img(input.rows,input.cols,CV_8UC1);
    narrow_img=Scalar::all(0);

    for(int i=0;i<input.rows;i++)
    {
        int cnt=0;
        uchar *data=input.ptr<uchar>(i);
        uchar *temp=narrow_img.ptr<uchar>(i);
        for(int j=0;j<input.cols;j++)
        {
            if(data[j-1]==255&&data[j]==255)
            {
                cnt++;
                if(data[j+1]!=255)	//²»ÄÜÓÃdata[j+1]==0 !!
                {

                    temp[j-cnt/2-1]=255;
                    temp[j-cnt/2]=255;
                    temp[j-cnt/2+1]=255;
                    cnt=0;

                }

            }

        }
    }

    //
    cv::Mat element = cv::getStructuringElement(MORPH_RECT, cv::Size(5,5));
    cv::morphologyEx(narrow_img, narrow_img, MORPH_CLOSE, element);

#ifdef Debug
    cv::imshow("narrow_img", narrow_img);
#endif
    narrow_img.copyTo(output);
}

// Hough Line
void LaneLineDet::HoughLines(cv::Mat &input, cv::Mat &houghImg)
{
    //Hough Lines
    vector<Vec4i> lines;
    cv::HoughLinesP(input, lines, 1, CV_PI*3/180, 10, 5, 10);
    //create color Image
    houghImg.create(input.rows,input.cols,CV_8UC3);
    houghImg=Scalar::all(255);

    //		//
    for( size_t i = 0; i < lines.size(); i++ )
    {
        //
        float dx=(float)lines[i][2]-lines[i][0];
        float dy=(float)lines[i][3]-lines[i][1];
        double theta=atan2(dy,dx)*180/CV_PI;
        if(fabs(theta)<=30||(lines[i][0]<houghImg.cols/5)||(lines[i][0]>houghImg.cols*4/5))	//
            continue;
        // Draw lines
        line(houghImg,Point(lines[i][0],lines[i][1]),Point(lines[i][2],lines[i][3]),Scalar(0,255,0),6,CV_AA);

    }
#ifdef Debug
    cv::imshow("houghImg",houghImg);
#endif
}

//
void LaneLineDet::ShowCntNum(cv::Mat& cnt_img, cv::Mat &Img)
{
/******Count the Num***************************************************/
    int cnt=0;
    static int lastFlag=0,flag=0;
    //±éÀúROIÇøÓò
    for(int i=cnt_img.rows-10;i>cnt_img.rows-15;--i)
    {
        uchar *data=cnt_img.ptr<uchar>(i);
        for(int j=0;j<cnt_img.cols/2;++j)
        {
            if(data[j]==255)
            {
                cnt++;	//
            }
        }
    }
//	cout<<"cnt:"<<cnt<<endl;
    //
    if(cnt>=20)
    {
        flag=1;
    }
    else
        flag=0;
//	cout<<"lastFlag:"<<lastFlag<<endl;
//	cout<<"flag:"<<flag<<endl;
    //±ÈœÏÇ°ºóÁœÕÅÍŒÆ¬
    if(lastFlag-flag==1)
        cntNum+=1;
    //
    lastFlag=flag;
//	cout<<"cntNum:"<<cntNum<<endl<<endl;
/******Draw Num*********************************************/
    //Calculates the width and height of a text string
    sprintf(Num,"The Num: %d",cntNum);
    Size textsize = getTextSize(Num, FONT_HERSHEY_COMPLEX, 3, 5, 0);
    Point org(Img.cols/2-100, textsize.height);
    int lineType = 8;
    putText( Img, Num, org, FONT_HERSHEY_PLAIN, 3,
                Scalar(255, 0, 0), 5, lineType );
    imshow( "NumImg", Img );

    // test
//	if(cntNum==6)
//		imwrite("6.png",Img);
//	if(cntNum==7)
//		imwrite("7.png",Img);
//	if(cntNum==8)
//		imwrite("8.png",Img);
//    std::cout << "---test1---" << std::endl;

}

// For color images
void LaneLineDet::ColorFilter(cv::Mat& input, cv::Mat& output)
{
    // HSV color space
    cv::Mat hsv_img, yel_img, wht_img;
    cv::cvtColor(input, hsv_img, CV_BGR2HSV);   //H:(0,180), S:[0, 255], V [0, 255]

    cv::inRange(hsv_img, cv::Scalar(20, 80, 80), cv::Scalar(25, 255, 255), yel_img);
    cv::inRange(hsv_img, cv::Scalar(0, 0, 180), cv::Scalar(180, 25, 255), wht_img);
    cv::Mat tmp(wht_img.rows, wht_img.cols, CV_8UC1, cv::Scalar::all(0));

    // merge yellow & white
    for (int i = 0; i  < wht_img.rows; i ++)
    {
        uchar* data_wht = wht_img.ptr<uchar>(i);
        uchar* data_yew = yel_img.ptr<uchar>(i);
        uchar* out = tmp.ptr<uchar>(i);
        for (int j = 0; j < wht_img.cols; j++)
        {
            if ((data_wht[j] == 255) || (data_yew[j] == 255))
                out[j] = 255;
        }
    }
    // narrow image
    NarrowImg(tmp, output);
//    wht_img.copyTo(output, yel_img);
#ifdef Debug
    cv::imshow("hsv_img", hsv_img);
    cv::imshow("yellow_img", yel_img);
    cv::imshow("white_img", wht_img);
    cv::imshow("color_filter", output);
#endif

}

//
void LaneLineDet::LaneLineDetection( cv::Mat &src)
{
    src.copyTo(src_img);
    // Image preprocess
    cv::Mat filtered_img;
    ImgPreProcess(src_img, filtered_img);
    // Perspective transfromation
    cv::Mat pers_img;
    PerspectiveTrans(filtered_img,pers_img, true);
    // For color images
    cv::Mat binary_img;
    if (color)
        ColorFilter(pers_img, binary_img);
    else
    // OTSU
        ThresholdOTSU(pers_img, binary_img);
    // Canny
    cv::Mat canny_img;
    cv::Canny(binary_img, canny_img, g_nThresh, 255);
#ifdef Debug
    cv::imshow("cannyImg", canny_img);
#endif
    // Hough Lines detection
    cv::Mat hough_img;
    HoughLines(canny_img,hough_img);

    // Perspective transfromation
    cv::Mat inv_pers_img;
    PerspectiveTrans(hough_img,inv_pers_img, false);
    cv::imshow("inv_hough", inv_pers_img);
/*****mask******************************************************************/
    cv::Mat dst_img;
    src_img.copyTo(dst_img, inv_pers_img);
    cv::imshow("dstImg",dst_img);

//    ShowCntNum(binary_img, dst_img);

}


/*********************End of File*******************************************************/
