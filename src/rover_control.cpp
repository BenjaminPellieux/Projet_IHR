#include "main.hpp"


Rover::Rover(){
    status = Status::STOP;
    target = (cv::Point) {0, 0};
}

void Rover::setStatus(Status newStatus) {
    status = newStatus;
}

Status Rover::getStatus(){
    return status;
}


std::string Rover::getStatusAsString() {
    switch (status) {
        case Status::FOLLOW: return "FOLLOW";
        case Status::STOP: return "STOP";
        case Status::DANCE: return "DANCE";
        case Status::PARKING: return "PARKING";
        case Status::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Rover::updateStatusfromMove(Movement gesture, cv::Point new_target){
    switch (gesture) {
        case Movement::HANDS_UP: setStatus(Status::DANCE); break;
        case Movement::HAND_RIGHT: setStatus(Status::STOP); break;
        case Movement::HAND_LEFT: setStatus(Status::FOLLOW); break;
        case Movement::TILT_RIGHT: setStatus(Status::PARKING); break;
        case Movement::TILT_LEFT: setStatus(Status::ERROR); break;
        default: status = status; break;
    }
    target = new_target;

    std::cout<<"INFO: Status: " << getStatusAsString() 
             << "\nTarget: x" << target.x 
             << "y" << target.y << std::endl;
}