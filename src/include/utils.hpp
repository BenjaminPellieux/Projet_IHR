#ifndef _MYUTILS_H_
#define _MYUTILS_H_

enum class Status {
    FOLLOW,
    STOP,
    DANCE,
    PARKING,
    ERROR
};


enum class Movement {
    HANDS_UP, // DANCE
    HAND_RIGHT, // STOP
    HAND_LEFT, // FOLLOW
    TILT_RIGHT, //PARKING
    TILT_LEFT, // TBD
    NONE,
};

#endif