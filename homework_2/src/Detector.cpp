#include "Detector.hpp"
#include <iostream>

// ==========================================================
// 1. AprilTag 识别
// ==========================================================
void Detector::detect(const cv::Mat& image, cv::Mat& result, cv::Mat& binary) {
    result = image.clone();
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::QRCodeDetector detector;
    std::vector<cv::Point2f> points;
    std::string info = detector.detectAndDecode(gray, points);

    if (!points.empty()) {
        for (size_t i = 0; i < points.size(); i++) {
            cv::line(result, points[i], points[(i + 1) % points.size()], cv::Scalar(0, 255, 0), 2);
        }
        cv::putText(result, "Tag Detected!", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        std::cout << "[Tag] 识别到了标志！" << std::endl;
    } else {
        cv::putText(result, "No Tag Found", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        std::cout << "[Tag] 没有识别到标志。" << std::endl;
    }
}

// ==========================================================
// 2. 魔方颜色识别（逐帧处理视频）
// ==========================================================
void Detector::detect(const std::string& videoPath) {
    // 打开视频
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "打不开视频：" << videoPath << "\n";
        return;
    }

    // 6 种魔方颜色的 HSV 下限 / 上限
    // OpenCV 的 H 范围是 0~180（不是 0~360）
    // 格式：{"名字", Hmin, Smin, Vmin, Hmax, Smax, Vmax}
    struct ColorRange { std::string name; int hmin, smin, vmin, hmax, smax, vmax; };
    std::vector<ColorRange> ranges = {
        {"Red",    0,   120, 70,  10,  255, 255},  // 红（低角度）
        {"Orange", 10,  120, 70,  25,  255, 255},  // 橙
        {"Yellow", 25,  120, 70,  35,  255, 255},  // 黄
        {"Green",  35,  80,  50,  85,  255, 255},  // 绿
        {"Blue",   90,  80,  50,  130, 255, 255},  // 蓝
        {"White",  0,   0,   180, 180, 40,  255},  // 白（低饱和、高亮度）
    };

    // 每个颜色给一个 BGR 值，用来在结果图上画框和文字
    std::map<std::string, cv::Scalar> bgr = {
        {"Red",    {0,   0,   255}},
        {"Orange", {0,   128, 255}},
        {"Yellow", {0,   255, 255}},
        {"Green",  {0,   255, 0  }},
        {"Blue",   {255, 0,   0  }},
        {"White",  {200, 200, 200}},
    };

    // 创建 3 个窗口（窗口名就是题目要求）
    cv::namedWindow("Image_raw");
    cv::namedWindow("Image_binary");
    cv::namedWindow("result");

    cv::Mat frame;
    while (cap.read(frame)) {                 // 一帧一帧读
        if (frame.empty()) break;

        // ---------- 步骤 A：模糊，减少噪点 ----------
        cv::Mat blurred;
        cv::GaussianBlur(frame, blurred, cv::Size(5, 5), 0);

        // ---------- 步骤 B：BGR 转 HSV ----------
        // HSV 里颜色信息集中在 H 通道，比 RGB 好分割
        cv::Mat hsv;
        cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

        // ---------- 步骤 C：按 6 种颜色分别做掩膜 ----------
        cv::Mat binaryShow;  // 用来显示在 Image_binary 窗口的拼接图
        std::vector<cv::Mat> maskList;

        cv::Mat result = frame.clone();  // 结果图画在副本上

        for (auto& r : ranges) {
            // 生成当前颜色的掩膜：在范围内的像素=255，其余=0
            cv::Mat mask;
            cv::inRange(hsv,
                        cv::Scalar(r.hmin, r.smin, r.vmin),
                        cv::Scalar(r.hmax, r.smax, r.vmax),
                        mask);

            // 红色在 OpenCV 的 H 通道上分两段（0~10 和 170~180），要合并
            if (r.name == "Red") {
                cv::Mat mask2;
                cv::inRange(hsv, cv::Scalar(170, r.smin, r.vmin),
                                  cv::Scalar(180, r.smax, r.vmax), mask2);
                cv::bitwise_or(mask, mask2, mask);
            }

            // 形态学开运算：去掉小噪点
            cv::morphologyEx(mask, mask, cv::MORPH_OPEN,
                             cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

            // 保存掩膜用于窗口拼接（转成 3 通道才能拼到彩色图）
            cv::Mat maskBGR;
            cv::cvtColor(mask, maskBGR, cv::COLOR_GRAY2BGR);
            maskList.push_back(maskBGR);

            // ---------- 步骤 D：在掩膜上找轮廓 ----------
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(mask, contours, cv::RETR_EXTERNAL,
                             cv::CHAIN_APPROX_SIMPLE);

            for (auto& cnt : contours) {
                double area = cv::contourArea(cnt);
                // 太小（噪点）或太大（背景）都跳过
                if (area < 500 || area > 50000) continue;

                cv::Rect box = cv::boundingRect(cnt);
                // 只保留接近正方形的轮廓（魔方色块）
                double ratio = (double)box.width / (box.height + 1e-6);
                if (ratio < 0.6 || ratio > 1.6) continue;

                // ---------- 步骤 E：画框 + 写字 ----------
                cv::rectangle(result, box, bgr[r.name], 2);
                cv::putText(result, r.name,
                            cv::Point(box.x, box.y + 20),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            bgr[r.name], 2);
            }
        }

        // ---------- 步骤 F：把 6 张掩膜拼成一张图给 Image_binary 显示 ----------
        cv::hconcat(maskList, binaryShow);

        // // ---------- 步骤 G：显示三个窗口 ----------
        // 在显示之前加上这三行（随便加在哪里，只要在 imshow 之前就行）
        cv::namedWindow("Image_raw", cv::WINDOW_NORMAL);
        cv::namedWindow("Image_binary", cv::WINDOW_NORMAL);
        cv::namedWindow("result", cv::WINDOW_NORMAL);

        // 你原本的 imshow 保持不变，或者把它改小一点（可选，也可以手动拖拽窗口边缘缩放）
        cv::imshow("Image_raw", frame);
        cv::imshow("Image_binary", binaryShow);
        cv::imshow("result", result);

        // 按 ESC 或 q 退出；30ms 一帧 ≈ 33fps
        int key = cv::waitKey(30);
        if (key == 27 || key == 'q') break;
    }

    cv::destroyAllWindows();
}