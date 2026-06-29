#ifndef GATEWAY_DISPATCHER_H
#define GATEWAY_DISPATCHER_H

typedef enum{
    GW_CMD_NONE=0,
    GW_CMD_OPEN,
    GW_CMD_GRAB,
    GW_CMD_RELEASE,
    GW_CMD_STOP
}gateway_cmd_t;

gateway_cmd_t gateway_cmd_from_string(const char*cmd);
const char*gateway_cmd_to_string(gateway_cmd_t cmd);
const char*gateway_cmd_to_stm32_protocol(gateway_cmd_t cmd);
void gateway_dispatcher_handle_cmd(gateway_cmd_t cmd);

#endif