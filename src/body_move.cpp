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
    analyzePose();
    std::string gesture_string = this->getMovementAsString();
    

    {
        std::lock_guard<std::mutex> lock(frameMutex);
        drawKeypoints(displayFrame);
        cv::putText(displayFrame, "Geste : " + gesture_string, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 2);
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

void PoseNet::setGesture(Movement newMove){
    gesture = newMove;
}

void PoseNet::analyzePose() {
    // Detect hands raised
    if ((keypoints[9].y > 0 && keypoints[5].y > 0 && keypoints[9].y < keypoints[5].y) &&
        (keypoints[10].y > 0 && keypoints[6].y > 0 && keypoints[10].y < keypoints[6].y)) {
        this->setGesture(Movement::HANDS_UP);  // Both hands raised

    // Detect left hand raised
    } else if (keypoints[9].y > 0 && keypoints[5].y > 0 && keypoints[9].y < keypoints[5].y) {
        this->setGesture(Movement::HAND_LEFT);

    // Detect right hand raised
    } else if (keypoints[10].y > 0 && keypoints[6].y > 0 && keypoints[10].y < keypoints[6].y) {
        this->setGesture(Movement::HAND_RIGHT);

    // Detect tilt right
    } else if (keypoints[5].x > 0 && keypoints[6].x > 0 &&
               keypoints[11].x > 0 && keypoints[12].x > 0 && 
               (keypoints[5].y - keypoints[6].y) > 50) {  // Right shoulder significantly higher
        this->setGesture(Movement::TILT_RIGHT);

    // Detect tilt left
    } else if (keypoints[5].x > 0 && keypoints[6].x > 0 &&
               keypoints[11].x > 0 && keypoints[12].x > 0 && 
               (keypoints[6].y - keypoints[5].y) > 50) {  // Left shoulder significantly higher
        this->setGesture(Movement::TILT_LEFT);

    // No specific gesture detected
    } else {
        this->setGesture(Movement::NONE);
    }
}

std::string PoseNet::getMovementAsString(){
    switch (gesture) {
        case Movement::HANDS_UP: return "Deux mains levee";
        case Movement::HAND_RIGHT: return "Main droite levee";
        case Movement::HAND_LEFT: return "Main gauche levee";
        case Movement::NONE: return "Aucun movement";
        default: return "UNKNOWN";
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

Movement PoseNet::getGesture(){
    return gesture;
}
