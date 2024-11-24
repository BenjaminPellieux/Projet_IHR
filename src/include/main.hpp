#ifndef _MYMAIN_H_
#define _MYMAIN_H_

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <wiringPi.h>
#include <softPwm.h>

#include "utils.hpp"

extern std::mutex frameMutex;   // Déclaration globale
extern bool disableDisplay;



// Classe pour gérer les threads
class ThreadManager {
public:
    template<typename Function, typename... Args>
    void runThread(Function&& func, Args&&... args) {
        std::thread th(std::forward<Function>(func), std::forward<Args>(args)...);
        threads.emplace_back(std::move(th));
    }

    void waitForThreads() {
        for (auto& th : threads) {
            if (th.joinable()) th.join();
        }
        threads.clear();
    }

private:
    std::vector<std::thread> threads;
};

// Classe pour gérer le modèle PoseNet
class PoseNet {
public:
    PoseNet(const std::string& modelPath);

    // Process a frame, detect pose, analyze gestures, and draw on the frame
    void processFrame(const cv::Mat& frame, cv::Mat& displayFrame);
    Movement getGesture();

private:
    cv::dnn::Net net;  // PoseNet model
    std::vector<cv::Point2f> keypoints;  // Detected keypoints
    std::vector<std::pair<int, int>> skeleton;  // Pose skeleton connections
    Movement gesture;

    // Helper methods
    void detectKeypoints(const cv::Mat& frame);
    void analyzePose();
    void setGesture(Movement newMove);


    void drawKeypoints(cv::Mat& frame);
    std::string getMovementAsString();
};


// Classe pour gérer le modèle YOLO
class YoloNet {
public:
    YoloNet(const std::string& cfgPath, const std::string& weightsPath);
    void detectHumans(const cv::Mat& frame, cv::Mat& displayFrame);
    void changeOrigin(const cv::Mat& frame);
    void drawBodyBox(cv::Mat& displayFrame, float bestConfidence, cv::Rect bestBox);
    std::pair<int, int> getBody();

private:
    cv::dnn::Net net;
    cv::Rect body;
};



class Rover {
    public:

        Rover();
        void updateStatusfromMove(Movement gesture, std::pair<int, int> target);
        std::pair<int, int> getTarget();

    private:

        void setStatus(Status newStatus);
        Status getStatus();
        std::string getStatusAsString();

        // Get the status as a string for easier display
        std::pair<int, int> target; 
        Status status;  // Private member to hold the rover's status
        


};


class RoverControl {
    public:
        RoverControl(int fwdPin, int turnPin);
        void initialize();
        void updateControl(std::pair<int, int> target);

    private:
        int fwdPin;
        int turnPin;
        int Yval;
        int Xval;
        const int fwdIdle = 500; // Neutral position for forward/backward
        const int trnIdle = 500; // Neutral position for turning
        void applyValues();
};



#endif
