#ifndef DETECTOR_HPP
#define DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <string>

class Detector {
public:
    // 1. 声明 AprilTag 识别函数
    void detect(const cv::Mat& image, cv::Mat& result, cv::Mat& binary);

    // 2. 声明魔方颜色识别函数
    void detect(const std::string& videoPath);
};

#endif