#include "include/main.hpp"

std::mutex frameMutex;  // Define the global mutex

int main(int argc, char** argv) {
    bool disableDisplay = false;

    // Check for command-line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--no-display" || arg == "-nd") {
            disableDisplay = true;
        }
    }

    PoseNet poseNet("model/Posenet-Mobilenet.onnx");
    YoloNet yoloNet("model/yolov4-tiny.cfg", "model/yolov4-tiny.weights");
    Rover* myRover = new Rover();

    if (!disableDisplay) {
        cv::namedWindow("Détection combinée", cv::WINDOW_NORMAL);
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Erreur : Impossible d'ouvrir la caméra !" << std::endl;
        return -1;
    }

    cv::Mat frame, displayFrame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        frame.copyTo(displayFrame);
        ThreadManager threadManager;

        threadManager.runThread(&PoseNet::processFrame, &poseNet, std::ref(frame), std::ref(displayFrame));
        threadManager.runThread(&YoloNet::detectHumans, &yoloNet, std::ref(frame), std::ref(displayFrame));

        threadManager.waitForThreads();

        myRover->updateStatusfromMove(poseNet.getGesture(), yoloNet.getBody());

        if (!disableDisplay) {
            cv::imshow("Détection combinée", displayFrame);
            if (cv::waitKey(1) == 'q') break;
        }
    }

    cap.release();
    if (!disableDisplay) {
        cv::destroyAllWindows();
    }

    return 0;
}
