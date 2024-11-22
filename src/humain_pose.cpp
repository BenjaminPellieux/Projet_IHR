#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>

cv::dnn::Net loadPoseNet() {
    std::string modelPath = "model/Posenet-Mobilenet.onnx";
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);

    if (net.empty()) {
        std::cerr << "Erreur : Impossible de charger le modèle PoseNet." << std::endl;
        exit(-1);
    }
    return net;
}



std::vector<cv::Point2f> detectPoseKeypoints(cv::dnn::Net& net, const cv::Mat& frame) {
    // Redimensionner et prétraiter l'image
    cv::Mat inputBlob = cv::dnn::blobFromImage(frame, 1.0 / 255.0, cv::Size(257, 257), cv::Scalar(), true, false);
    net.setInput(inputBlob);

    // Obtenir la sortie
    cv::Mat output = net.forward();

    // Traiter les points clés
    std::vector<cv::Point2f> keypoints;
    int H = output.size[2];
    int W = output.size[3];

    for (int i = 0; i < 17; i++) { // PoseNet détecte 17 points clés
        cv::Mat heatMap(H, W, CV_32F, output.ptr(0, i));

        cv::Point maxLoc;
        double conf;
        cv::minMaxLoc(heatMap, 0, &conf, 0, &maxLoc);

        if (conf > 0.5) { // Seuil de confiance
            keypoints.emplace_back((maxLoc.x * frame.cols) / W, (maxLoc.y * frame.rows) / H);
        } else {
            keypoints.emplace_back(-1, -1); // Point non détecté
        }
    }

    return keypoints;
}




bool isHandRaised(const std::vector<cv::Point2f>& keypoints) {
    if (keypoints[9].y > 0 && keypoints[5].y > 0) { // Poignet droit et épaule droite détectés
        return keypoints[9].y < keypoints[5].y;    // Poignet au-dessus de l'épaule
    }
    return false;
}


bool isLeaning(const std::vector<cv::Point2f>& keypoints) {
    if (keypoints[5].x > 0 && keypoints[6].x > 0) { // Épaules gauche et droite détectées
        return std::abs(keypoints[5].y - keypoints[6].y) > 50; // Différence de hauteur significative
    }
    return false;
}



void drawKeypoints(cv::Mat& frame, const std::vector<cv::Point2f>& keypoints) {
    for (int i = 0; i != (int) keypoints.size(); i++){
        const auto& point = keypoints[i];
        if (point.x > 0 && point.y > 0) {
            cv::circle(frame, point, 5, cv::Scalar(0, 255, 0), -1);
            cv::putText(frame, std::to_string(i), point, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        }
    }
}

void displayGesture(cv::Mat& frame, const std::string& gesture) {
    cv::putText(frame, gesture, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
}



int main() {
    cv::dnn::Net poseNet = loadPoseNet();
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Erreur : Impossible d'ouvrir la caméra." << std::endl;
        return -1;
    }

    cv::namedWindow("PoseNet Gestures", cv::WINDOW_NORMAL);

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) break;

        // Détecter les points clés
        std::vector<cv::Point2f> keypoints = detectPoseKeypoints(poseNet, frame);

        // Vérifier les gestes
        std::string gesture = "Aucun";
        if (isHandRaised(keypoints)) gesture = "Main levee";
        if (isLeaning(keypoints)) gesture = "Inclinaison";

        // Dessiner les points clés et afficher le geste
        drawKeypoints(frame, keypoints);
        displayGesture(frame, gesture);

        cv::imshow("PoseNet Gestures", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
