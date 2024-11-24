#include "include/main.hpp"


RoverControl::RoverControl(int fwdPin, int turnPin)
    : fwdPin(fwdPin), turnPin(turnPin), Yval(fwdIdle), Xval(trnIdle) {}

void RoverControl::initialize() {
    wiringPiSetupGpio(); // Initialize wiringPi library
    softPwmCreate(fwdPin, 0, 200); // Create PWM for forward/backward control
    softPwmCreate(turnPin, 0, 200); // Create PWM for turning control
    Yval = fwdIdle; // Set initial values to neutral
    Xval = trnIdle;
}

void RoverControl::updateControl(std::pair<int, int>  target) {
    // Map target vector to speed and direction
    
    // Map Yval (forward/backward)
    if (target.second > 0) {
        Yval = std::min(1000, fwdIdle + static_cast<int>(target.second * 500)); // Scale and cap
    } else if (target.second < 0) {
        Yval = std::max(1, fwdIdle + static_cast<int>(target.second * 500)); // Scale and cap
    } else {
        Yval = fwdIdle; // Neutral position
    }

    // Map Xval (turning)
    if (target.first > 0) {
        Xval = std::min(1000, trnIdle + static_cast<int>(target.first * 500)); // Scale and cap
    } else if (target.first < 0) {
        Xval = std::max(1, trnIdle + static_cast<int>(target.first * 500)); // Scale and cap
    } else {
        Xval = trnIdle; // Neutral position
    }

    // Apply updated values to motors
    applyValues();
}

int mapValue(int value, int inMin, int inMax, int outMin, int outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

void RoverControl::applyValues() {
    softPwmWrite(fwdPin, mapValue(Yval, 1, 1000, 0, 179));  // Map to servo range
    softPwmWrite(turnPin, mapValue(Xval, 1, 1000, 179, 0)); // Map to servo range
}
