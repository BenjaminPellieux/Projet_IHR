#include "include/main.hpp"

std::mutex frameMutex;  // Define the global mutex
bool disableDisplay = false;

int main(int argc, char** argv) {

    // Check for command-line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--no-display" || arg == "-nd") {
            disableDisplay = true;
        }
    }

    int8_t newPriority = -10; // Valeur entre -20 (priorité maximale) et 19 (priorité minimale)

    if (setpriority(PRIO_PROCESS, getpid(), newPriority) == 0) {
        std::cout << "Priorité du processus définie à " << static_cast<int>(newPriority) << std::endl;
    } else {
        perror("Erreur lors de la définition de la priorité");
        return EXIT_FAILURE;
    }

    PoseNet poseNet("model/Posenet-Mobilenet.onnx");
    YoloNet yoloNet("model/yolov4-tiny.cfg", "model/yolov4-tiny.weights");
    // RoverControl roverControl();
    Rover roverStatus(13, 12);
    // Use GPIO pins 1 and 2 for forward/turn

    if (!disableDisplay) {
        cv::namedWindow("Détection combinée", cv::WINDOW_NORMAL);
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Erreur : Impossible d'ouvrir la caméra !" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat frame, displayFrame;
    int frameCount = 0;
    double totalProcessingTime = 0.0;
    auto startTime = std::chrono::steady_clock::now();
    while (true) {

        auto frameStart = std::chrono::steady_clock::now(); // Temps de début pour une image

        cap >> frame;
        if (frame.empty()) break;

        frame.copyTo(displayFrame);
        auto poseNetFuture = std::async(std::launch::async, &PoseNet::processFrame, &poseNet, std::ref(frame), std::ref(displayFrame), std::ref(roverStatus));
        auto yoloNetFuture = std::async(std::launch::async, &YoloNet::detectHumans, &yoloNet, std::ref(frame), std::ref(displayFrame), std::ref(roverStatus));

        // Attendre que les deux tâches soient terminées
        poseNetFuture.get();
        yoloNetFuture.get();
        //myRover.updateStatusfromMove(poseNet.getGesture(), yoloNet.getBody());

        // std::pair<int, int> target = roverStatus.getTarget();
        // if ((roverStatus.getStatus() == Status::FOLLOW)){
        //     //threadManager.runThread(&RoverControl::updateControl, &roverControl, std::ref(target));
        //     roverControl.updateControl(target, frame);
        // }else {
        //     roverControl.stopRover();            
        // }

        auto frameEnd = std::chrono::steady_clock::now(); // Utilisation cohérente de steady_clock
        double frameProcessingTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();
        totalProcessingTime += frameProcessingTime;
        frameCount++;

        // Afficher FPS toutes les 30 images
        if (frameCount % 30 == 0) {
            auto currentTime = std::chrono::steady_clock::now();
            double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            double fps = frameCount / elapsedTime;

            std::cout << "Temps par image : " << frameProcessingTime << " ms, FPS : " << fps << std::endl;
        }

        if (!disableDisplay) {
            cv::imshow("Détection combinée", displayFrame);
            if (cv::waitKey(1) == 'q') break;
        }
    }


  // Calculer les statistiques finales
    double averageProcessingTime = totalProcessingTime / frameCount;
    std::cout << "Temps moyen par image : " << averageProcessingTime << " ms" << std::endl;
    std::cout << "Images traitées : " << frameCount << std::endl;


    cap.release();
    if (!disableDisplay) {
        cv::destroyAllWindows();
    }

    return 0;
}
