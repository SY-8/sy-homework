#include "Detector.hpp"
#include <iostream>

int main() {
    // 你的真实文件路径（绝对路径，直接写死，省得传参）
    // const std::string tagImagePath = "/home/shumiao/下载/6785036f0dbe48ac39f3fe0feffad5cd.jpg";
    // const std::string cubeVideoPath = "/home/shumiao/下载/1da6c4bdaad0682d691b0ee6f814bfff.mp4";
    const std::string tagImagePath = "../data/tag.jpg";
    const std::string cubeVideoPath = "../data/cube.mp4";

    Detector detector;

    // ---------- 第 1 部分：AprilTag ----------
    std::cout << "===== 1. AprilTag 识别 =====\n";
    cv::Mat tagImg = cv::imread(tagImagePath);
    if (tagImg.empty()) {
        std::cerr << "图片读取失败：" << tagImagePath << "\n";
    } else {
        cv::Mat result, binary;
        detector.detect(tagImg, result, binary);

        cv::imshow("Image_raw",    tagImg);   // 原图
        cv::imshow("Image_binary", binary);   // 二值化图
        cv::imshow("result",       result);   // 带识别框的结果
        cv::waitKey(0);                       // 按任意键继续
        cv::destroyAllWindows();
    }

    // ---------- 第 2 部分：魔方视频 ----------
    std::cout << "===== 2. 魔方颜色识别 =====\n";
    detector.detect(cubeVideoPath);

    return 0;
}