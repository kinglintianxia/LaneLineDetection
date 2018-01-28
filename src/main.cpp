#include "LaneLineDet.h"


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " param_folder.\n";
        exit(1);
    }
    cv::FileStorage fs(std::string(argv[1]), cv::FileStorage::READ);
    int video = fs["Video"];
    std::string file_path = fs["file_path"];
    fs.release();
    // video
    cv::VideoCapture cap(file_path.c_str());
    cv::Mat src_img;
    //
	char filename[100];

    LaneLineDet lane_line_det(argv[1]);

	while(1)
	{
        if (video == 1)
        {
            cap >> src_img;
            // for save
    //        cv::imwrite("../image/video.png", src_img);
            if (src_img.empty())
                   break;
            lane_line_det.LaneLineDetection(src_img);
            int key = waitKey(30)&255;
            if (key == 32)
                key = cv::waitKey(0)&255;
            if (key == 27)
                break;
        }
        else
        {
            static int cnt=250;	// Start image num: 289
            sprintf(filename,"%s/%06d.png", file_path.c_str(), cnt);// 000320.jpg
            src_img=imread(filename,1);    //
            if (src_img.empty())
            {
                std::printf("Load image error, check file folder!\n");
                exit(1);
            }
            double t0=(double)getTickCount();
            lane_line_det.LaneLineDetection(src_img);
            cout<<"The operation Time:"<<(getTickCount()-t0)*1000/getTickFrequency()<<" ms"<<endl;

            // 756
            int key = waitKey(30)&255;
            if(cnt<800 && key != 27)	//
            {
                if (key == 32)
                    cv::waitKey(0);
                cnt++;	//
            }
            else	//
                break;	//
        }
	}
    cv::waitKey(0);
	return 0;
}




/*****************End of File************************************/
