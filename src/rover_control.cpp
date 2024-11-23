#include "main.hpp"


RoverControl::RoverControl(int fwdPin, int turnPin)
    : fwdPin(fwdPin), turnPin(turnPin), Yval(fwdIdle), Xval(trnIdle) {}

void RoverControl::initialize() {
    wiringPiSetup(); // Initialize wiringPi library
    softPwmCreate(fwdPin, 0, 200); // Create PWM for forward/backward control
    softPwmCreate(turnPin, 0, 200); // Create PWM for turning control
    Yval = fwdIdle; // Set initial values to neutral
    Xval = trnIdle;
}

void RoverControl::updateControl(const Rover& rover) {
    // Map target vector to speed and direction
    auto target = rover.getTarget();

    // Map Yval (forward/backward)
    if (target.y > 0) {
        Yval = std::min(1000, fwdIdle + static_cast<int>(target.y * 500)); // Scale and cap
    } else if (target.y < 0) {
        Yval = std::max(1, fwdIdle + static_cast<int>(target.y * 500)); // Scale and cap
    } else {
        Yval = fwdIdle; // Neutral position
    }

    // Map Xval (turning)
    if (target.x > 0) {
        Xval = std::min(1000, trnIdle + static_cast<int>(target.x * 500)); // Scale and cap
    } else if (target.x < 0) {
        Xval = std::max(1, trnIdle + static_cast<int>(target.x * 500)); // Scale and cap
    } else {
        Xval = trnIdle; // Neutral position
    }

    // Apply updated values to motors
    applyValues();
}

void RoverControl::applyValues() {
    softPwmWrite(fwdPin, map(Yval, 1, 1000, 0, 179)); // Map to servo range
    softPwmWrite(turnPin, map(Xval, 1, 1000, 179, 0)); // Map to servo range
}