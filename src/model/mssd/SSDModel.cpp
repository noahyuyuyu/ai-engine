#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/trace.hpp>
using namespace cv;
using namespace cv::dnn;
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

string CLASSES[] = {"background", "station_m", "station_l_30", "station_l_45", "station_r_30",
	"station_r_45","background", "station_m", "station_l_30", "station_l_45", "station_r_30","background", "station_m", "station_l_30", "station_l_45", "station_r_30"};


int main(int argc, char **argv)
{
    CV_TRACE_FUNCTION();
    String modelTxt = "/workspace/weights/mssd/MobileNetSSD_deploy.prototxt";
    String modelBin = "/workspace/weights/mssd/MobileNetSSD_deploy.caffemodel";
    
    String imageFile = (argc > 1) ? argv[1] : "/workspace/data/dog.jpg";
    Net net = dnn::readNetFromCaffe(modelTxt, modelBin);
    net.setPreferableTarget(1);
    if (net.empty())
    {
        std::cerr << "Can't load network by using the following files: " << std::endl;
        std::cerr << "prototxt:   " << modelTxt << std::endl;
        std::cerr << "caffemodel: " << modelBin << std::endl;
        exit(-1);
    }
    Mat img = imread(imageFile);
    if (img.empty())
    {
        std::cerr << "Can't read image from the file: " << imageFile << std::endl;
        exit(-1);
    }
    
    Mat img2;
    resize(img, img2, Size(300,300));
    Mat inputBlob = blobFromImage(img2, 0.007843, Size(300,300), Scalar(127.5, 127.5, 127.5), false);
    net.setInput(inputBlob, "data");
    double time = (double)getTickCount();
    Mat detection = net.forward("detection_out"); 
    
    Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
    double time_elpased = ((double)getTickCount() - time) / getTickFrequency();
    std::cout<<"time spend: "<<time_elpased<<std::endl;  

    ostringstream ss;
    float confidenceThreshold = 0.2;
    for (int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > confidenceThreshold)
        {
            int idx = static_cast<int>(detectionMat.at<float>(i, 1));
            int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * img.cols);
            int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * img.rows);
            int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * img.cols);
            int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * img.rows);

            Rect object((int)xLeftBottom, (int)yLeftBottom,
                        (int)(xRightTop - xLeftBottom),
                        (int)(yRightTop - yLeftBottom));

            rectangle(img, object, Scalar(0, 255, 0), 2);

            cout << CLASSES[idx] << ": " << confidence << endl;

            ss.str("");
            ss << confidence;
            String conf(ss.str());
            String label = CLASSES[idx] + ": " + conf;
            int baseLine = 0;
            Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            putText(img, label, Point(xLeftBottom, yLeftBottom),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
        }
    }

    imwrite("output.jpg",img);


    return 0;
}
