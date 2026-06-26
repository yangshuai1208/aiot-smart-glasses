#ifndef COMMAND_H
#define COMMAND_H

#include "gesture.h"

typedef enum
{
    COMMAND_NONE=0,
    COMMAND_OPEN,
    COMMAND_RELEASE,
    COMMAND_GRAB,
    COMMAND_STOP
}command_type_t;

command_type_t command_from_gesture(gesture_type_t gesture);
const char*command_to_string(command_type_t command);


#endif