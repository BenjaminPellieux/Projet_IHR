#ifndef _MYUTILS_H_
#define _MYUTILS_H_

#define YOLOCONFIDENCE 0.5
#define MAXBODYAREA 0.40
#define MINBODYAREA 0.05
#define NEUTRALPOS 14.9
#define MAXSPEED 17
#define MINSPEED 13
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