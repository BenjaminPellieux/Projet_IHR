#include "include/main.hpp"


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

void Rover::updateStatusfromMove(Movement gesture, cv::Point new_target) {
    Status oldStatus = status; // Sauvegarder l'ancien statut
    cv::Point oldTarget = target; // Sauvegarder l'ancien point cible

    // Mettre à jour le statut en fonction du geste détecté
    switch (gesture) {
        case Movement::HANDS_UP: setStatus(Status::DANCE); break;
        case Movement::HAND_RIGHT: setStatus(Status::STOP); break;
        case Movement::HAND_LEFT: setStatus(Status::FOLLOW); break;
        case Movement::TILT_RIGHT: setStatus(Status::PARKING); break;
        case Movement::TILT_LEFT: setStatus(Status::ERROR); break;
        default: break; // Pas de changement
    }

    target = new_target;

    // Imprimer uniquement si le statut ou la cible a changé
    if (status != oldStatus || target != oldTarget) {
        std::cout << "INFO: Status: " << getStatusAsString() 
                  << "\nTarget: x" << target.x 
                  << " y" << target.y << std::endl;
    }
}