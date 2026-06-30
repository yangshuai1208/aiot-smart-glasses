#include "hand_servo.h"
#include "pca9685.h"

#define HAND_SERVO_CHANNEL	0

#define HAND_OPEN_ANGEL			30
#define HAND_GRAB_ANGEL			130
#define HAND_RELEASE_ANGEL	60
#define HAND_STOP_ANGEL			90

HAL_StatusTypeDef hand_servo_init(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef ret;
	
	ret=pca9685_init(hi2c);
	if(ret!=HAL_OK)
	{
		return ret;
	}
	return pca9685_set_servo_angle(HAND_SERVO_CHANNEL,HAND_STOP_ANGEL);
}
HAL_StatusTypeDef hand_servo_apply_action(hand_action_t action)
{
	uint16_t angle;
	
	switch(action){
		case HAND_ACTION_OPEN:
			angle=HAND_OPEN_ANGEL;
			break;
		case HAND_ACTION_GRAB:
			angle=HAND_GRAB_ANGEL;
			break;
		case HAND_ACTION_RELEASE:
			angle=HAND_RELEASE_ANGEL;
			break;
		case HAND_ACTION_STOP:
			angle=HAND_STOP_ANGEL;
			break;
		
		case HAND_ACTION_NONE:
		default:
			angle=HAND_STOP_ANGEL;
			break;
	}
	return pca9685_set_servo_angle(HAND_SERVO_CHANNEL,angle);
}
