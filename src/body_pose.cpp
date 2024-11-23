#include "include/main.hpp"

// Charge le modèle YOLO
YoloNet::YoloNet(const std::string& cfgPath, const std::string& weightsPath) {
    net = cv::dnn::readNetFromDarknet(cfgPath, weightsPath);
    if (net.empty()) {
        throw std::runtime_error("Erreur : Impossible de charger le modèle YOLO.");
    }
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

cv::Point YoloNet::getBody(){
    return (cv::Point) {body.x, body.y};
};

void YoloNet::change_origin(const cv::Mat& frame)
{
    this->body.y = frame.rows - this->body.y;
    this->body.x -= frame.cols / 2;
}

// Detect only one human (the one with the highest confidence) using YOLO
void YoloNet::detectHumans(const cv::Mat& frame, cv::Mat& displayFrame) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
    net.setInput(blob);

    std::vector<cv::Mat> detections;
    net.forward(detections, net.getUnconnectedOutLayersNames());

    cv::Rect bestBox;
    float bestConfidence = 0.0f;

    for (const auto& detection : detections) {
        for (int i = 0; i < detection.rows; ++i) {
            const float* data = detection.ptr<float>(i);
            float confidence = data[4]; // Confidence for the detected object
            if (confidence > 0.5) {     // Threshold for confidence
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);

                cv::Rect box(centerX - width / 2, centerY - height / 2, width, height);

                if (confidence > bestConfidence) {
                    bestConfidence = confidence;
                    bestBox = box; // Store the box with the highest confidence
                }
            }
        }
    }

    // Draw the best box on the display frame
    if (bestConfidence > 0.6) {
        body = bestBox;
        this->change_origin(frame);
        std::string label = "BODY: " + std::to_string(bestConfidence * 100).substr(0, 5) + "%";
        cv::rectangle(displayFrame, bestBox, cv::Scalar(255, 0, 0), 2);
        cv::putText(displayFrame, label, cv::Point(bestBox.x, bestBox.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
    }else{
        body.x = -1; body.y = -1;
    }
}
