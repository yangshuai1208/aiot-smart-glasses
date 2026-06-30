#include "hand_protocol.h"

#include <stdio.h>

static void test_one_cmd(const char *cmd)
{
    hand_action_t action = hand_protocol_parse(cmd);

    printf("CMD    : %s\n", cmd);
    printf("ACTION : %s\n", hand_action_to_string(action));
    hand_action_execute(action);
    printf("--------------------------------\n");
}

int main(void)
{
    test_one_cmd("HAND_OPEN");
    test_one_cmd("HAND_GRAB");
    test_one_cmd("HAND_RELEASE");
    test_one_cmd("HAND_STOP");
    test_one_cmd("UNKNOWN_CMD");

    return 0;
}
