#ifndef HAND_SERVO_H
#define HAND_SERVO_H

#include "main.h"
#include "hand_protocol.h"

HAL_StatusTypeDef hand_servo_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef hand_servo_apply_action(hand_action_t action);
HAL_StatusTypeDef hand_servo_set_all_stop(void);

#endif
