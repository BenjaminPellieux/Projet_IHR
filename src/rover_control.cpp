#include "include/main.hpp"


RoverControl::RoverControl(int fwdPin, int turnPin){
    this->fwdPin = fwdPin;
    this->turnPin = turnPin;
    Yval = fwdIdle; Xval = trnIdle; 

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

void RoverControl::updateControl(std::pair<int, int>  target) {
    // Map target vector to speed and direction
    
    // Map Yval (forward/backward)
    if (target.second > 50) {
        Yval = std::min(1000, (int) fwdIdle + (target.second * 500)); // Scale and cap
    } else if (target.second < 0) {
        Yval = std::max(1,  (int) fwdIdle + (target.second * 500)); // Scale and cap
    } else {
        Yval = fwdIdle; // Neutral position
    }

    // Map Xval (turning)
    if (target.first > 0) {
        Xval = std::min(1000, (int) trnIdle + (target.first * 500)); // Scale and cap
    } else if (target.first < 0) {
        Xval = std::max(1, (int) trnIdle + (target.first * 500)); // Scale and cap
    } else {
        Xval = trnIdle; // Neutral position
    }

    // Apply updated values to motors
    applyValues();
}

int RoverControl::mapValue(float value, float inMin, float inMax, int outMin, int outMax) {
        return static_cast<int>((value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
}

void RoverControl::stopRover() {
    softPwmWrite(fwdPin, 0); // Neutral signal for stopping
    softPwmWrite(turnPin, 0);
    pwmWrite(fwdPin, 0);     // Optional: explicitly stop PWM signal
    pwmWrite(turnPin, 0);

    std::cout << "[INFO] Motors stopped completely." << std::endl;
}

void RoverControl::applyValues() {
    std::cout<<"[DEBUG] Tval: "<<Yval <<" Xval: "<<Xval<<std::endl;
    //softPwmWrite(fwdPin,mapValue(Yval, 1.0f, 1000.0f, 5, 25));  // Map to servo range
    int dutyCycle = mapValue(Yval, 1.0f, 1000.0f, 5, 10);

      // Debug output
    std::cout << "[DEBUG] Yval: " << Yval 
              << " | Duty Cycle: " << dutyCycle 
              << "%" << std::endl;
        
    softPwmWrite(fwdPin,dutyCycle);  // Map to servo range

    
    // softPwmWrite(turnPin, mapValue(Xval, 1.0f, 1000.0f, 179, 0)); // Map to servo range}
}