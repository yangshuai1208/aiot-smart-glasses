#include "command.h"

command_type_t command_from_gesture(gesture_type_t gesture)
{
    switch(gesture){
    case GESTURE_LEFT:
         return COMMAND_OPEN;
        
    case GESTURE_RIGHT:
         return COMMAND_RELEASE;

    case GESTURE_NOD:
        return COMMAND_GRAB;
    
    case GESTURE_SHAKE:
        return COMMAND_NONE;
    
    case GESTURE_NONE:
    default:
        return COMMAND_NONE;
    }
}
const char*command_to_string(command_type_t command)
{
    switch (command)
    {
    case COMMAND_OPEN:
        return "OPEN";
    
    case COMMAND_RELEASE:
        return "RELEASE";

    case COMMAND_GRAB:
        return "GRAB";
    
    case COMMAND_STOP:
        return "STOP";
    
    case COMMAND_NONE:
    default:
        return  "NONE";
    }
}