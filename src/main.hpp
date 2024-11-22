#ifndef _MYMAIN_H_
#define _MYMAIN_H_

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>


extern std::mutex frameMutex;   // Déclaration globale

enum class Status {
    FOLLOW,
    STOP,
    BLOCK,
    ERROR,
    DANCE
};


enum class Movement {
    HANDS_UP,
    HAND_RIGHT,
    HAND_LEFT,
    TILT_RIGHT,
    TILT_LEFT,
    NONE,
};


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
    void change_origin(const cv::Mat& frame);
    cv::Point getBody();

private:
    cv::dnn::Net net;
    cv::Rect body;
};

class Rover {
    public:

        Rover(Status initialStatus = Status::STOP) : status(initialStatus) {}

        Status getStatus() const {
            return status;
        }
        void setStatus(Status newStatus) {
            status = newStatus;
        }

        // Get the status as a string for easier display
        std::string getStatusAsString() const {
            switch (status) {
                case Status::FOLLOW: return "FOLLOW";
                case Status::STOP: return "STOP";
                case Status::BLOCK: return "BLOCK";
                case Status::ERROR: return "ERROR";
                case Status::DANCE: return "DANCE";
                default: return "UNKNOWN";
            }
        }

    private:
        Status status;  // Private member to hold the rover's status


};


#endif
