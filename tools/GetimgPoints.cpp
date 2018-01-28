#include "iostream"
#include "opencv2/opencv.hpp"

// Global para
cv::Point2f src_pts[4], dst_pts[4];
std::vector<cv::Point> dst_vec;
//call back function
static void onMouse(int event,int x,int y,int ,void *)
{
    //opencv2
    if(event!=CV_EVENT_LBUTTONDOWN)
        return;
    //opencv3
//    if(event!=EVENT_LBUTTONDOWN)
//        return;	//
    cv::Point2f seed=cv::Point(x,y);
    if (dst_vec.size() <= 4)
        dst_vec.push_back(seed);
    std::cout << "Point: (" << seed.x << ", " << seed.y << ")" << std::endl;

}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " imgFolder" << std::endl;
        exit(1);
    }
    cv::Mat img = cv::imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);
    if (img.empty())
    {
        std::cout << "Load image error, check the folder!" << std::endl;
        exit(1);
    }
    cv::Mat tmp_img;
    img.copyTo(tmp_img);
    // Draw lines in image
    for (size_t i = 0; i < img.rows; i += 10)
    {
        cv::line(tmp_img, cv::Point(0, i), cv::Point(tmp_img.cols, i), cv::Scalar(0,0,155));
    }
    // img window
    cv::namedWindow("Get_img_Points", CV_WINDOW_AUTOSIZE);
    // Mouse callback
    cv::setMouseCallback("Get_img_Points", onMouse, 0);

    // Show img
    cv::imshow("Get_img_Points", tmp_img);
    cv::waitKey(0);

    // Find Homography
    // box
    src_pts[0] = cv::Point2f(img.cols/3,img.rows/3);
    src_pts[1] = cv::Point2f(img.cols/3*2,img.rows/3);
    src_pts[2] = cv::Point2f(img.cols/3,img.rows-1);
    src_pts[3] = cv::Point2f(img.cols/3*2, img.rows-1);
    // img corner
//    src_pts[0] = cv::Point2f(0,0);
//    src_pts[1] = cv::Point2f(img.cols-1,0);
//    src_pts[2] = cv::Point2f(0,img.rows-1);
//    src_pts[3] = cv::Point2f(img.cols-1, img.rows-1);
    for (int i = 0; i < dst_vec.size(); i ++)
        dst_pts[i] = dst_vec[i];
    // DRAW THE POINTS in order: B,G,R,YELLOW
    //
    cv::circle(tmp_img, dst_pts[0], 5, cv::Scalar(255, 0, 0), 2);
    cv::circle(tmp_img, dst_pts[1], 5, cv::Scalar(0, 255, 0), 2);
    cv::circle(tmp_img, dst_pts[2], 5, cv::Scalar(0, 0, 255), 2);
    cv::circle(tmp_img, dst_pts[3], 5, cv::Scalar(0, 255, 255), 2);
    // Show img
    cv::imshow("Get_img_Points", tmp_img);
    cv::waitKey(0);
    // Read H
    cv::Mat H;

//    cv::FileStorage fs("/home/king/Documents/ccdc2017/image/homography.xml", cv::FileStorage::READ);
//    fs["homography"] >> H;
//    fs.release();

    // H
    H = cv::getPerspectiveTransform(src_pts, dst_pts);
    // Save H
    cv::FileStorage fo("./homography.xml", cv::FileStorage::WRITE);
    fo << "imageWidth" << 3 << "imageHeight" << 3 << "homography" << H;
    fo.release();

    // output
    std::cout << "Homography matrix:\n";
    for (int i = 0; i < 3; i ++)
    {
        std::cout << H.at<cv::Vec3d>(i) << "\n";
    }

    cv::Mat bird_img;
    double z = H.at<double>(2, 2);
    for (;;)
    {
      // escape key stops
      H.at<double>(2, 2) = z;
      // USE HOMOGRAPHY TO REMAP THE VIEW
      //
      cv::warpPerspective(img,
                          bird_img,
                          H,
                          img.size(),
                          cv::WARP_INVERSE_MAP | cv::INTER_LINEAR,
                          cv::BORDER_CONSTANT, cv::Scalar::all(0) // Fill border with black
                          );
      cv::imshow("bird_eye_img", bird_img);
      int key = cv::waitKey() & 255;
      if (key == 'u')
          z += 0.5;
      if (key == 'd')
          z -= 0.5;
      if (key == 27)    // esc
          break;
    }

    return 0;
}
