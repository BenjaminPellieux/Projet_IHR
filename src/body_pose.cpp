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

std::pair<int, int> YoloNet::getBody(){
    return (std::pair<int, int>) {body.x, body.y};
};

void YoloNet::changeOrigin(const cv::Mat& frame)
{
    this->body.y = frame.rows - this->body.y;
    this->body.x -= frame.cols / 2;
}

// Detect only one human (the one with the highest confidence) using YOLO
void YoloNet::detectHumans(const cv::Mat& frame, cv::Mat& displayFrame,  Rover& roverStatus) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
    float frameArea = (float) (frame.cols * frame.rows);
    net.setInput(blob);
    // this->body.x = -1; this->body.y = -1;
    std::vector<cv::Mat> detections;
    net.forward(detections, net.getUnconnectedOutLayersNames());

    cv::Rect bestBox;
    float bestConfidence = 0.0f;

    for (const auto& detection : detections) {
        for (int i = 0; i < detection.rows; ++i) {
            const float* data = detection.ptr<float>(i);
            float confidence = data[4]; // Confidence for the detected object
            if (confidence > YOLOCONFIDENCE ) {     // Threshold for confidence
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);
                cv::Rect box(centerX, centerY, width, height);
                if (confidence > bestConfidence) {
                    bestConfidence = confidence;
                    bestBox = box; // Store the box with the highest confidence
                }
            }
        }
    }
    float report_area = (float) bestBox.area() / frameArea;
    
    //std::cout<<"[DEBUG] Report AREA " << report_area << std::endl;
    if ((bestConfidence) && ((report_area > MINBODYAREA) && (report_area < MAXBODYAREA))){
        this->body = bestBox;
        this->changeOrigin(frame);
        roverStatus.setTarget((std::pair<int, int>) {this->body.x, this->body.y});
        roverStatus.setTheta(std::atan2(this->body.x, this->body.y) * 100);
    }

    

    if(! disableDisplay){
        drawBodyBox(displayFrame, bestConfidence, bestBox);
    }
}

void YoloNet::drawBodyBox(cv::Mat& displayFrame, float bestConfidence, cv::Rect bestBox ){
    std::string label = "BODY: " + std::to_string(bestConfidence * 100).substr(0, 5) + "%";
    cv::rectangle(displayFrame, bestBox, cv::Scalar(255, 0, 0), 2);
    cv::putText(displayFrame, label, cv::Point(bestBox.x, bestBox.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
}
