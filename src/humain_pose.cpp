#include "main.hpp"


PoseNet::PoseNet(const std::string& modelPath) {
    net = cv::dnn::readNetFromONNX(modelPath);
    if (net.empty()) {
        throw std::runtime_error("Erreur : Impossible de charger le modèle PoseNet.");
    }
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // Define the skeleton connections
    skeleton = {{10, 8}, {8, 6}, {6, 5}, {6, 12}, {5, 11}, {5, 7},
                {7, 9}, {11, 12}, {12, 14}, {14, 16}, {11, 13}, {13, 15}
            };
}
void PoseNet::processFrame(const cv::Mat& frame, cv::Mat& displayFrame) {
    detectKeypoints(frame);

    std::string gesture;
    analyzePose(gesture);

    {
        std::lock_guard<std::mutex> lock(frameMutex);
        drawKeypoints(displayFrame);
        cv::putText(displayFrame, "Geste : " + gesture, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 2);
    }
}


void PoseNet::detectKeypoints(const cv::Mat& frame) {
    cv::Mat inputBlob = cv::dnn::blobFromImage(frame, 1.0 / 255.0, cv::Size(257, 257), cv::Scalar(), true, false);
    net.setInput(inputBlob);
    cv::Mat output = net.forward();

    keypoints.clear();
    int H = output.size[2];
    int W = output.size[3];

    for (int i = 0; i < 17; i++) {
        cv::Mat heatMap(H, W, CV_32F, output.ptr(0, i));
        cv::Point maxLoc;
        double conf;
        cv::minMaxLoc(heatMap, 0, &conf, 0, &maxLoc);

        if (conf > 0.5) {
            keypoints.emplace_back((maxLoc.x * frame.cols) / W, (maxLoc.y * frame.rows) / H);
        } else {
            keypoints.emplace_back(-1, -1);
        }
    }
}

void PoseNet::analyzePose(std::string& gesture) {
    if ((keypoints[9].y > 0 && keypoints[5].y > 0 && keypoints[9].y < keypoints[5].y) && 
        (keypoints[10].y > 0 && keypoints[6].y > 0 && keypoints[10].y < keypoints[6].y)) {
        gesture = "Deux mains levees";
    } else if (keypoints[9].y > 0 && keypoints[5].y > 0 && keypoints[9].y < keypoints[5].y) {
        gesture = "Main droite levee";
    } else if (keypoints[10].y > 0 && keypoints[6].y > 0 && keypoints[10].y < keypoints[6].y) {
        gesture = "Main gauche levee";
    } else {
        gesture = "Aucun geste";
    }
}
// Dessine les points clés et les connexions
void PoseNet::drawKeypoints(cv::Mat& frame) {
    for (int i = 0; i != (int) keypoints.size(); i++ ) {
        const auto& point = keypoints[i];
        if (point.x > 0 && point.y > 0) {
            cv::circle(frame, point, 5, cv::Scalar(0, 255, 0), -1);
            cv::putText(frame, std::to_string(i), keypoints[i], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
        }
    }

    for (const auto& connection : skeleton) {
        const auto& p1 = keypoints[connection.first];
        const auto& p2 = keypoints[connection.second];
        if (p1.x > 0 && p1.y > 0 && p2.x > 0 && p2.y > 0) {
            cv::line(frame, p1, p2, cv::Scalar(255, 0, 0), 2);
        }
    }
}

// Charge le modèle YOLO
YoloNet::YoloNet(const std::string& cfgPath, const std::string& weightsPath) {
    net = cv::dnn::readNetFromDarknet(cfgPath, weightsPath);
    if (net.empty()) {
        throw std::runtime_error("Erreur : Impossible de charger le modèle YOLO.");
    }
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

// Détecte les humains avec YOLO
void YoloNet::detectHumans(const cv::Mat& frame, cv::Mat& displayFrame) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
    net.setInput(blob);

    std::vector<cv::Mat> detections;
    net.forward(detections, net.getUnconnectedOutLayersNames());

    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    for (const auto& detection : detections) {
        for (int i = 0; i < detection.rows; ++i) {
            const float* data = detection.ptr<float>(i);
            float confidence = data[4];
            if (confidence > 0.5) {
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);
                boxes.emplace_back(centerX - width / 2, centerY - height / 2, width, height);
                confidences.push_back(confidence);
            }
        }
    }

    // Suppression non maximale
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

    for (int idx : indices) {
        std::string label = "BODY : " + std::to_string(confidences[idx] * 100).substr(0, 5) + "%  Area: " +  std::to_string(( static_cast<float>(boxes[idx].area()) / static_cast<float>(frame.cols * frame.rows)) * 100.f) ;
        cv::rectangle(displayFrame, boxes[idx], cv::Scalar(255, 0, 0), 2);
        cv::putText(displayFrame, label, cv::Point( boxes[idx].x, boxes[idx].y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);    
    }
}