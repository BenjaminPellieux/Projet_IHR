#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <thread>
#include <mutex>

// Classes pour ASL et YOLO
const std::vector<std::string> classesASL = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N",
    "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y"
};
const std::vector<std::string> classNamesYOLO = {"person"};  // YOLO détecte uniquement les personnes

// Mutex pour synchronisation des threads
std::mutex frameMutex;

// Prétraitement de l'image pour ASL
void detectASL(const cv::Mat& frame, cv::dnn::Net& netASL, cv::Mat& displayFrame) {
    cv::Mat roi = frame(cv::Rect(350, 50, 250, 250));  // Région d'intérêt (ROI) pour la détection ASL
    cv::Mat gray, blurred;
    cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(7, 7), 0);

    // Prétraiter l'image
    cv::Mat blob = cv::dnn::blobFromImage(blurred, 1.0 / 255.0, cv::Size(28, 28), cv::Scalar(), true, false);
    netASL.setInput(blob);

    // Faire une prédiction
    cv::Mat output = netASL.forward();
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(output, 0, &confidence, 0, &classIdPoint);
    int predictedClass = classIdPoint.x;

    // Afficher les résultats sur l'image
    std::string predictedLetter = classesASL[predictedClass];
    std::string text = "Lettre : " + predictedLetter + " (" + std::to_string(confidence * 100).substr(0, 5) + "%)";
    std::lock_guard<std::mutex> lock(frameMutex);
    cv::putText(displayFrame, text, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
    cv::rectangle(displayFrame, cv::Rect(350, 50, 250, 250), cv::Scalar(0, 255, 0), 2);
}

// Détection de personnes avec YOLO
void detectHumans(const cv::Mat& frame, cv::dnn::Net& netYOLO, const std::vector<std::string>& classNames, cv::Mat& displayFrame) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
    netYOLO.setInput(blob);

    // Faire des prédictions
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

    std::lock_guard<std::mutex> lock(frameMutex);
    for (int idx : indices) {
        cv::Rect box = boxes[idx];
        std::string label = classNames[classIds[idx]] + ": " + std::to_string(confidences[idx] * 100).substr(0, 5) + "%";
        cv::rectangle(displayFrame, box, cv::Scalar(255, 0, 0), 2);
        cv::putText(displayFrame, label, cv::Point(box.x, box.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
    }
}

int main() {
    // Charger les modèles
    cv::dnn::Net netASL = cv::dnn::readNetFromONNX("model/cnn156_fixed.onnx");
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

        // Copie pour affichage
        frame.copyTo(displayFrame);

        // Lancer les threads
        std::thread threadASL(detectASL, std::ref(frame), std::ref(netASL), std::ref(displayFrame));
        std::thread threadYOLO(detectHumans, std::ref(frame), std::ref(netYOLO), std::ref(classNamesYOLO), std::ref(displayFrame));

        threadASL.join();
        threadYOLO.join();

        // Afficher la vidéo dans une seule fenêtre
        cv::imshow("Détection combinée", displayFrame);

        // Quitter avec 'q'
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
