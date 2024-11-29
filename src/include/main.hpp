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
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <sys/resource.h> // Pour setpriority
#include <unistd.h> // Pour getpid()
#include <future>
#include <cmath>

#include "utils.hpp"

extern std::mutex frameMutex;   // Déclaration globale
extern bool disableDisplay;




class RoverControl {
    public:
        RoverControl(u_int8_t fwdPin, u_int8_t turnPin);
        ~RoverControl();
        void updateControl(std::pair<int, int> target,  cv::Mat& frame);
        void stopRover();
    private:
        u_int8_t fwdPin;
        u_int8_t turnPin;
        float Yval;
        float Xval;
        float mapValue(float value, float inMin, float inMax);

        void applyValues();
};


class Rover {
    public:

        Rover(u_int8_t fwdPin,  u_int8_t turnPin);
        void updateStatusfromMove(Movement gesture, std::pair<int, int> target);
        std::pair<int, int> getTarget();
        Status getStatus();
        void setStatus(Status newStatus);
        void setTarget(std::pair<int, int> newtarget);
        void setTheta(double newangle);

    private:

        RoverControl* roverControl;
        
        std::string getStatusAsString();
        double theta;
        // Get the status as a string for easier display
        std::pair<int, int> target; 
        Status status;  // Private member to hold the rover's status
        


};



// Classe pour gérer le modèle PoseNet
class PoseNet {
public:
    PoseNet(const std::string& modelPath);

    // Process a frame, detect pose, analyze gestures, and draw on the frame
    void processFrame(const cv::Mat& frame, cv::Mat& displayFrame, Rover& roverStatus );
    Movement getGesture();

private:
    cv::dnn::Net net;  // PoseNet model
    std::vector<cv::Point2f> keypoints;  // Detected keypoints
    std::vector<std::pair<int, int>> skeleton;  // Pose skeleton connections
    Movement gesture;

    // Helper methods
    void detectKeypoints(const cv::Mat& frame);
    void analyzePose(Rover& roverStatus);
    void setGesture(Movement newMove);


    void drawKeypoints(cv::Mat& frame);
    std::string getMovementAsString();
};


// Classe pour gérer le modèle YOLO
class YoloNet {
public:
    YoloNet(const std::string& cfgPath, const std::string& weightsPath);
    void detectHumans(const cv::Mat& frame, cv::Mat& displayFrame, Rover& roverStatus );
    void changeOrigin(const cv::Mat& frame);
    void drawBodyBox(cv::Mat& displayFrame, float bestConfidence, cv::Rect bestBox);
    std::pair<int, int> getBody();

private:
    cv::dnn::Net net;
    cv::Rect body;
};




#endif
