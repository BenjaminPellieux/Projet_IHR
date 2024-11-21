#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;
using namespace cv::dnn;

// Charger les noms des classes
vector<string> loadClassNames(const string& filePath) {
    vector<string> classNames;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Erreur : Impossible d'ouvrir " << filePath << endl;
        return classNames;
    }
    string line;
    while (getline(file, line)) {
        classNames.push_back(line);
    }
    return classNames;
}

// Dessiner un rectangle autour des détections
void drawBoundingBox(Mat& frame, const Rect& box, const string& label) {
    rectangle(frame, box, Scalar(0, 255, 0), 2);
    putText(frame, label, Point(box.x, box.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2);
}

int main() {
    // Chemins vers les fichiers YOLO
    string modelConfig = "model/yolov4-tiny.cfg";  // Chemin vers le fichier cfg
    string modelWeights = "model/yolov4-tiny.weights";  // Chemin vers le fichier weights
    string classFile = "model/coco.names";  // Chemin vers le fichier des classes

    // Charger le modèle
    Net net = readNetFromDarknet(modelConfig, modelWeights);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    // Charger les noms des classes
    vector<string> classNames = loadClassNames(classFile);
    if (classNames.empty()) {
        cerr << "Erreur : Impossible de charger les noms des classes." << endl;
        return -1;
    }

    // Initialiser la webcam
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Erreur : Impossible d'ouvrir la caméra !" << endl;
        return -1;
    }

    // Créer une fenêtre unique
    namedWindow("Détection d'humains", WINDOW_NORMAL);

    // Paramètres pour YOLO
    float confidenceThreshold = 0.5;  // Confiance minimale pour garder une détection
    float nmsThreshold = 0.4;  // Seuil de suppression non maximale

    while (true) {
        Mat frame;
        cap >> frame; // Capture une image
        if (frame.empty()) {
            cerr << "Erreur : Frame vide capturée !" << endl;
            break;
        }

        // Prétraitement pour YOLO
        Mat blob;
        blobFromImage(frame, blob, 1 / 255.0, Size(416, 416), Scalar(), true, false);
        net.setInput(blob);

        // Faire des prédictions
        vector<Mat> detections;
        net.forward(detections, net.getUnconnectedOutLayersNames());

        // Parcourir les résultats
        vector<Rect> boxes;
        vector<int> classIds;
        vector<float> confidences;

        for (auto& detection : detections) {
            for (int i = 0; i < detection.rows; i++) {
                Mat scores = detection.row(i).colRange(5, detection.cols);
                Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if (confidence > confidenceThreshold) {
                    int centerX = (int)(detection.at<float>(i, 0) * frame.cols);
                    int centerY = (int)(detection.at<float>(i, 1) * frame.rows);
                    int width = (int)(detection.at<float>(i, 2) * frame.cols);
                    int height = (int)(detection.at<float>(i, 3) * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    boxes.push_back(Rect(left, top, width, height));
                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                }
            }
        }

        // Appliquer la suppression non maximale
        vector<int> indices;
        dnn::NMSBoxes(boxes, confidences, confidenceThreshold, nmsThreshold, indices);

        for (int idx : indices) {
            Rect box = boxes[idx];
            string label = format("%s: %.2f", classNames[classIds[idx]].c_str(), confidences[idx]);
            drawBoundingBox(frame, box, label);
        }

        // Afficher le flux vidéo dans une seule fenêtre
        imshow("Détection d'humains", frame);

        // Quitter si 'q' est pressé
        char key = (char)waitKey(1);
        if (key == 'q' || key == 27) { // 'q' ou Échap pour quitter
            break;
        }
    }

    // Libérer les ressources
    cap.release();
    destroyAllWindows();

    return 0;
}
