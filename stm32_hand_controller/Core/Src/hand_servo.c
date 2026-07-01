#include "hand_servo.h"
#include "pca9685.h"

#define HAND_SERVO_NUM	5


#define THUMB_SERVO_CHANNEL	0
#define INDEX_SERVO_CHANNEL	1
#define MIDDLE_SERVO_CHANNEL	2
#define RING_SERVO_CHANNEL	3
#define LITTLE_SERVO_CHANNEL	4

static const uint8_t hand_servo_channels[HAND_SERVO_NUM]=
{
	
	THUMB_SERVO_CHANNEL,	
	INDEX_SERVO_CHANNEL,	
	MIDDLE_SERVO_CHANNEL,	
	RING_SERVO_CHANNEL,	
	LITTLE_SERVO_CHANNEL
};
static const uint16_t hand_open_angles[HAND_SERVO_NUM]=
{
	30,30,30,30,30
};
static const uint16_t hand_grab_angles[HAND_SERVO_NUM]=
{
	130,130,130,130,130
};
static const uint16_t hand_release_angles[HAND_SERVO_NUM]=
{
	60,60,60,60,60
};

static const uint16_t hand_stop_angles[HAND_SERVO_NUM]=
{
	90,90,90,90,90
};
static HAL_StatusTypeDef hand_servo_apply_angles(const uint16_t angles[])
{
	HAL_StatusTypeDef ret;
	
	for(uint8_t i=0;i<HAND_SERVO_NUM;i++)
	{
		ret=pca9685_set_servo_angle(hand_servo_channels[i],angles[i]);
		if(ret!=HAL_OK)
		{
			return ret;
		}
		
		
		HAL_Delay(50);
	}
	return HAL_OK;
}
HAL_StatusTypeDef hand_servo_init(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef ret;
	
	ret=pca9685_init(hi2c);
	if(ret!=HAL_OK)
	{
		return ret;
	}
	return hand_servo_set_all_stop();
}
HAL_StatusTypeDef hand_servo_set_all_stop(void)
{
	return hand_servo_apply_angles(hand_stop_angles);
}

HAL_StatusTypeDef hand_servo_apply_action(hand_action_t action)
{
	
	switch(action){
		case HAND_ACTION_OPEN:
			return hand_servo_apply_angles(hand_open_angles);
		case HAND_ACTION_GRAB:
				return hand_servo_apply_angles(hand_grab_angles);
		
		case HAND_ACTION_RELEASE:
					return hand_servo_apply_angles(hand_release_angles);
		case HAND_ACTION_STOP:
				return hand_servo_apply_angles(hand_stop_angles);
		
		
		case HAND_ACTION_NONE:
		default:
				return hand_servo_apply_angles(hand_stop_angles);
	}
}
