#ifndef GESTURE_H
#define GESTURE_H

#include "mpu6050.h"

typedef enum
{
    GESTURE_NONE=0,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_NOD,
    GESTURE_SHAKE,
    GESTURE_UNKNOWN
}gesture_type_t;

gesture_type_t gesture_detect(const mpu6050_raw_data_t *data);
const char*gesture_to_string(gesture_type_t gesture);

#endif