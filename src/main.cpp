#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <thread>
#include <mutex>

// Classes des lettres ASL
const std::vector<std::string> classesASL = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N",
    "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y"
};

// Classes YOLO
const std::vector<std::string> classNamesYOLO = {"person"};  // YOLO detects only people
std::vector<int> asl_find(24);  // Counts for each ASL letter detected
std::mutex frameMutex;          // Mutex for thread synchronization
std::deque<std::string> predictionHistory;  // History for stabilizing predictions
const int historySize = 5;      // Number of frames for prediction stability

bool enableASLDetection = false;  // Flag to enable/disable ASL detection

// Function to preprocess the input frame for ASL detection
cv::Mat preprocessImage(const cv::Mat& frame) {
    cv::Mat gray, equalized, filtered, resized, normalized;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, equalized);
    cv::GaussianBlur(equalized, filtered, cv::Size(5, 5), 0);
    cv::resize(filtered, resized, cv::Size(28, 28));
    resized.convertTo(normalized, CV_32F, 1.0 / 255);
    return cv::dnn::blobFromImage(normalized, 1.0, cv::Size(28, 28), cv::Scalar(), true, false);
}

// Function for skin detection using HSV color space
cv::Mat detectSkin(const cv::Mat& frame) {
    cv::Mat hsv, skinMask;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, 58, 60), cv::Scalar(30, 255, 255), skinMask);
    cv::erode(skinMask, skinMask, cv::Mat(), cv::Point(-1, -1), 2);
    cv::dilate(skinMask, skinMask, cv::Mat(), cv::Point(-1, -1), 2);
    return skinMask;
}

// Function to handle specific ASL letters
void handleASLAction(const std::string& letter) {
    if (letter == "C") {
        std::cout << "Action: Dance" << std::endl;
    } else if (letter == "F") {
        std::cout << "Action: Follow" << std::endl;
    } else if (letter == "H") {
        std::cout << "Action: Halt" << std::endl;
    } else if (letter == "P") {
        std::cout << "Action: Parking" << std::endl;
    }
}

// Function to detect ASL gestures
void detectASL(const cv::Mat& frame, cv::dnn::Net& netASL, cv::Mat& displayFrame) {
    if (!enableASLDetection) return;

    // Define ROI for hand detection
    cv::Rect roi(frame.cols * 0.3, 0, frame.cols * 0.4, frame.rows * 0.5);
    
    cv::Mat handROI = frame(roi);
    cv::flip(handROI, handROI, 1);
    //cv::Mat skinMask = detectSkin(handROI);

    //cv::Mat handSkin; 
    //cv::bitwise_and(handROI, handROI, handSkin, skinMask);
    cv::Mat inputBlob = preprocessImage(handROI);

    netASL.setInput(inputBlob);
    cv::Mat output = netASL.forward();

    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(output, 0, &confidence, 0, &classIdPoint);
    int predictedClass = classIdPoint.x;

    if (confidence > 0.75) {
        std::string predictedLetter = classesASL[predictedClass];
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (predictionHistory.size() >= historySize) {
                predictionHistory.pop_front();
            }
            predictionHistory.push_back(predictedLetter);
        }

        std::map<std::string, int> freq;
        for (const auto& letter : predictionHistory) {
            freq[letter]++;
        }
        std::string stablePrediction = "";
        for (const auto& pair : freq) {
            if (pair.second > historySize / 2) {
                stablePrediction = pair.first;
                break;
            }
        }

        if (!stablePrediction.empty()) {
            handleASLAction(stablePrediction);  // Handle the action for specific letters

            asl_find[predictedClass] += 1;
            std::string text = "Lettre : " + stablePrediction + " (" + std::to_string(confidence * 100).substr(0, 5) + "%)";
            std::lock_guard<std::mutex> lock(frameMutex);
            cv::putText(displayFrame, text, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        }
    }

    cv::rectangle(displayFrame, roi, cv::Scalar(0, 255, 0), 2);
}

// Function to detect humans using YOLO
void detectHumans(const cv::Mat& frame, cv::dnn::Net& netYOLO, cv::Mat& displayFrame) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
    netYOLO.setInput(blob);

    std::vector<cv::Mat> detections;
    netYOLO.forward(detections, netYOLO.getUnconnectedOutLayersNames());

    std::vector<cv::Rect> boxes;
    std::vector<int> classIds;
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
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                boxes.emplace_back(left, top, width, height);
                confidences.push_back(confidence);
                classIds.push_back(static_cast<int>(data[5]));
            }
        }
    }

     // Suppression non maximale
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

    if ((int)indices.size() < 1){
        enableASLDetection = false;
        return;
    }

    for (int idx : indices) {
        cv::Rect box = boxes[idx];
        float area = static_cast<float>(box.area());
        float frameArea = static_cast<float>(frame.cols * frame.rows);
        enableASLDetection = ( area / (float) frameArea > 0.5);  // Enable ASL if body box > 80% of frame
        std::string label = "BODY : " + std::to_string(confidences[idx] * 100).substr(0, 5) + "%  Area: " +  std::to_string((area / (float) frameArea) * 100.f) ;
        cv::rectangle(displayFrame, box, cv::Scalar(255, 0, 0), 2);
        cv::putText(displayFrame, label, cv::Point(box.x, box.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

    }
}

int main() {
    cv::dnn::Net netASL = cv::dnn::readNetFromONNX("model/asl_model.onnx");
    cv::dnn::Net netYOLO = cv::dnn::readNetFromDarknet("model/yolov4-tiny.cfg", "model/yolov4-tiny.weights");

    netYOLO.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    netYOLO.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Erreur : Impossible d'ouvrir la caméra !" << std::endl;
        return -1;
    }
    cv::namedWindow("Détection combinée", cv::WINDOW_NORMAL);
    while (true) {
        cv::Mat frame, displayFrame;
        cap >> frame;

        if (frame.empty()) break;
        frame.copyTo(displayFrame);

        std::thread threadASL(detectASL, std::ref(frame), std::ref(netASL), std::ref(displayFrame));
        std::thread threadYOLO(detectHumans, std::ref(frame), std::ref(netYOLO), std::ref(displayFrame));

        threadASL.join();
        threadYOLO.join();

        cv::imshow("Détection combinée", displayFrame);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
