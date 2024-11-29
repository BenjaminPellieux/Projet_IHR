#include "include/main.hpp"


Rover::Rover(u_int8_t fwdPin, u_int8_t turnPin){
    this->roverControl = new RoverControl(fwdPin, turnPin);
    this->status = Status::STOP;
    this->target = (std::pair<int, int>) {0, 0};
    this->theta = 0;
}

void Rover::setStatus(Status newStatus) {
    this->status = newStatus;
}

Status Rover::getStatus(){
    return status;
}

std::pair<int, int> Rover::getTarget(){
    return target;
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

void Rover::setTarget(std::pair<int, int> newtarget){
    this->target = newtarget;
}

void Rover::setTheta(double newangle){
    this->theta = newangle;    
}

void Rover::move(cv::Mat& frame){
    if (this->status == Status::FOLLOW)
        this->roverControl->updateControl(this->target, this->theta, frame);
    else{
        this->roverControl->stopRover();
    }
}

////////////////////////////////////////
//             Rover Control          //
////////////////////////////////////////

RoverControl::RoverControl(u_int8_t fwdPin, u_int8_t turnPin){
    this->fwdPin = fwdPin;
    this->turnPin = turnPin;
    Yval = NEUTRALPOS; Xval = NEUTRALPOS; 

    wiringPiSetupGpio(); // Initialize wiringPi library
    pinMode(fwdPin, PWM_OUTPUT);
    pinMode(turnPin, PWM_OUTPUT);
    softPwmCreate(fwdPin, 0, 200); // Create PWM for forward/backward control
    softPwmCreate(turnPin, 0, 200); // Create PWM for turning control
    stopRover();
}



RoverControl::~RoverControl() {
    softPwmWrite(fwdPin, 0); // Désactiver le PWM
    softPwmWrite(turnPin, 0);
    std::cout << "Nettoyage des ressources GPIO." << std::endl;
}
void RoverControl::updateControl(std::pair<int, int> target, double theta, cv::Mat& frame) {
    // Map Yval (forward/backward) to the range [15, 18]
    if (target.second > 0) {
        Yval = mapValue(static_cast<float>(target.second), 0.0f, static_cast<float>(frame.rows));
    } else {
        Yval = NEUTRALPOS; // NEUTRALPOS or stop (minimum forward)
    }

    // Map Xval (turning) to the range [12, 18]
    if (theta >  25.0f) {
        // Right turn

        Xval = mapValue(theta, 0.0f, 45.0f);

    } else if (theta < -25.0f) {
        // Left turn
        Xval = abs(mapValue(theta, -45.0f, 0.0f));
    } else {
        Xval = NEUTRALPOS; // NEUTRALPOS turning
    }

    // Apply updated values to motors
    applyValues();
}

// Helper function to map a value from one range to another
float RoverControl::mapValue(float value, float inMin, float inMax) {
    return static_cast<int>((value - inMin) * (MAXSPEED - MINSPEED) / (inMax - inMin) + MINSPEED);
}


void RoverControl::stopRover() {

    // Apply NEUTRALPOS position to stop the rover
    softPwmWrite(fwdPin, NEUTRALPOS); 
    softPwmWrite(turnPin, NEUTRALPOS); 
    //softPwmWrite(turnPin, 15); 

    std::cout << "[INFO] Wheels stopped at NEUTRALPOS position ."<< NEUTRALPOS << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Délai pour permettre au servo de se stabiliser

}

void RoverControl::applyValues() {
    // Map Yval to duty cycle for forward/backward motion
    // int forwardDutyCycle = mapValue(Yval, 1.0f, 1000.0f, 10, 20); 

    // // Map Xval to duty cycle for turning
    // int turnDutyCycle = mapValue(Xval, 1.0f, 1000.0f, 10, 20); 

    // Debugging information
    std::cout << "[DEBUG] Forward Duty Cycle: " << Yval
              << " | Turning Duty Cycle: " << Xval << std::endl;

    // Apply calculated duty cycles
    softPwmWrite(fwdPin, Yval); 
    //softPwmWrite(turnPin, Xval); 
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Délai pour permettre au servo de se stabiliser

}
