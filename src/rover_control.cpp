#include "include/main.hpp"


RoverControl::RoverControl(int fwdPin, int turnPin){
    this->fwdPin = fwdPin;
    this->turnPin = turnPin;
    Yval = fwdIdle; Xval = trnIdle; 

    wiringPiSetupGpio(); // Initialize wiringPi library
    pinMode(fwdPin, PWM_OUTPUT);
    pinMode(turnPin, PWM_OUTPUT);
    digitalWrite(fwdPin, LOW);
    digitalWrite(turnPin, LOW);
    softPwmCreate(fwdPin, 0, 200); // Create PWM for forward/backward control
    softPwmCreate(turnPin, 0, 200); // Create PWM for turning control
    stopRover();
}


RoverControl::~RoverControl() {
    softPwmWrite(fwdPin, 0); // Désactiver le PWM
    softPwmWrite(turnPin, 0);
    std::cout << "Nettoyage des ressources GPIO." << std::endl;
}
void RoverControl::updateControl(std::pair<int, int> target) {
    // Map Yval (forward/backward)
    if (target.second > 0) {
        // Forward movement
        Yval = std::min(1000, (int)fwdIdle + (target.second * 5)); 
    } else {
        // Stop (neutral)
        Yval = fwdIdle;
    }

    // Map Xval (turning)
    if (target.first > 0) {
        // Right turn
        Xval = std::min(1000, (int)trnIdle + (target.first * 5)); 
    } else if (target.first < 0) {
        // Left turn
        Xval = std::max(1, (int)trnIdle + (target.first * 5));
    } else {
        // Neutral turning
        Xval = trnIdle;
    }

    // Apply updated values to motors
    applyValues();
}

int RoverControl::mapValue(float value, float inMin, float inMax, int outMin, int outMax) {
    return static_cast<int>((value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
}

void RoverControl::stopRover() {
    // Map neutral position (1.5 ms pulse width)
    int neutralDutyCycle = mapValue(fwdIdle, 1.0f, 1000.0f, 75, 150); 

    // Apply neutral position to stop the rover
    softPwmWrite(fwdPin, 14.5); 
    //softPwmWrite(turnPin, 15); 

    std::cout << "[INFO] Wheels stopped at neutral position (1.5 ms)." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Délai pour permettre au servo de se stabiliser

}

void RoverControl::applyValues() {
    // Map Yval to duty cycle for forward/backward motion
    int forwardDutyCycle = mapValue(Yval, 1.0f, 1000.0f, 10, 20); 

    // Map Xval to duty cycle for turning
    int turnDutyCycle = mapValue(Xval, 1.0f, 1000.0f, 10, 20); 

    // Debugging information
    std::cout << "[DEBUG] Forward Duty Cycle: " << Yval / 50 
              << " | Turning Duty Cycle: " << turnDutyCycle << std::endl;

    // Apply calculated duty cycles
    softPwmWrite(fwdPin, Yval / 50); 
    //softPwmWrite(turnPin, turnDutyCycle); 
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Délai pour permettre au servo de se stabiliser

}
