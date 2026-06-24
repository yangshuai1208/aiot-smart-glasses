#include "gesture.h"

#define ACC_TILT_THRESHOLD        8000
#define GYRO_NOD_THRESHOLD        12000
#define GYRO_SHAKE_THRESHOLD      12000

gesture_type_t gesture_detect(const mpu6050_raw_data_t *data)
{
    if(data==NULL){
        return GESTURE_UNKNOWN;
    }
    if(data->acc_x>ACC_TILT_THRESHOLD){
        return GESTURE_RIGHT;
    }
    if(data->acc_x<-ACC_TILT_THRESHOLD){
        return GESTURE_LEFT;
    }
    if((data->gyro_x>GYRO_NOD_THRESHOLD)||
    (data->gyro_x<-GYRO_NOD_THRESHOLD)){
        return GESTURE_NOD;
    }
    if((data->gyro_z>GYRO_SHAKE_THRESHOLD)||
    (data->gyro_z<-GYRO_SHAKE_THRESHOLD)){
        return GESTURE_SHAKE;
    }
    return GESTURE_NONE;
}
const char*gesture_to_string(gesture_type_t gesture)
{
    switch(gesture){
        case GESTURE_LEFT:
          return "LEFT";
        case GESTURE_RIGHT:
          return "RIGHT";
        case GESTURE_NOD:
          return "NOD";
        case GESTURE_SHAKE:
          return "SHAKE";
        case GESTURE_NONE:
          return "NONE";
        default:
          return "UNKNOWN";
    }
}