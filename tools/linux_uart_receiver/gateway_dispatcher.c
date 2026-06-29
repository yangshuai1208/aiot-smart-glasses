#include "gateway_dispatcher.h"

#include <stdio.h>
#include <string.h>

gateway_cmd_t gateway_cmd_from_string(const char*cmd)
{
    if(cmd==NULL){
        return GW_CMD_NONE;
    }
    if(strcmp(cmd,"OPEN")==0){
        return GW_CMD_OPEN;
    }
    if(strcmp(cmd,"GRAB")==0){
        return GW_CMD_GRAB; 
    }
    if(strcmp(cmd,"RELEASE")==0){
        return GW_CMD_RELEASE;
    }
    if(strcmp(cmd,"STOP")==0){
        return GW_CMD_STOP;
    }
    return GW_CMD_NONE;
}
const char*gateway_cmd_to_string(gateway_cmd_t cmd)
{
    switch (cmd)
    {
    case GW_CMD_OPEN:
        return "OPEN";
    case GW_CMD_GRAB:
        return "GARB";
    case GW_CMD_RELEASE:
        return "RELEASE";
    case GW_CMD_STOP:
        return "STOP";
    case GW_CMD_NONE:
    default:    
        return "NONE";
    }
}
const char*gateway_cmd_to_stm32_protocol(gateway_cmd_t cmd)
{
    switch ( cmd)
    {
    case GW_CMD_OPEN:
        return "HAND_OPEN";
    case GW_CMD_GRAB:
        return "HAND_GRAB";
    case GW_CMD_RELEASE:
        return "HAND_RELEASE";
    case GW_CMD_STOP:
        return "HAND_STOP";
    case GW_CMD_NONE:
    default:
        return "HAND_NONE";
    }
}
void gateway_dispatcher_handle_cmd(gateway_cmd_t cmd)
{
    printf(" GATEWAY CMD =%s\n  ",gateway_cmd_to_string(cmd));
    printf( "STM32 CMD =%s\n",gateway_cmd_to_stm32_protocol(cmd));

    switch ( cmd)
    {
    case GW_CMD_OPEN:
        printf("ACTION  =dispatch open nana command\n");
        break;
    case GW_CMD_GRAB:
        printf("ACTION =dispatch grab command\n");
        break;
    case GW_CMD_RELEASE:
        printf("ACTION =dispatch release command\n");
        break;
    case GW_CMD_STOP:
        printf("ACTION =dispatch stop command\n");
        break;
    case GW_CMD_NONE:
    default:
        printf("ACTION =no vaild command\n");
        break;
    }
}