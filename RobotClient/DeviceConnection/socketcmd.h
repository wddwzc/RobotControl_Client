#ifndef SOCKETCMD_H
#define SOCKETCMD_H

enum CmdType
{
    CMD_TYPE_DATA = 0,           // request data
    CMD_DATA_GETCLOUD,
    CMD_DATA_GETPOSE,
    CMD_DATA_ENDCLOUD,
    CMD_DATA_ENDPOSE,
    CMD_DATA_GETALL,
    CMD_DATA_ENDALL,
    CMD_DATA_PATHNODE,

    CMD_TYPE_MOVEMENT,
    CMD_MOVE_LEFTFORWARD,       // forward to left
    CMD_MOVE_FORWARD,               // go straight
    CMD_MOVE_RIGHTFORWARD,      // forward to right
    CMD_MOVE_LEFT,                          // turn left
    CMD_MOVE_STOP,                      // stop
    CMD_MOVE_RIGHT,                     // turn right
    CMD_MOVE_LEFTBACK,              // back to left
    CMD_MOVE_BACK,                      // back off
    CMD_MOVE_RIGHTBACK,              // back to right

    CMD_TYPE_INSTRUCTION,
    CMD_INSTRUCTION_INTERRUPT,
    CMD_INSTRUCTION_MANUAL,         // manual mode
    CMD_INSTRUCTION_NAVIGATION,     // navigation mode
    CMD_INSTRUCTION_MAPPING,        // mapping mode

    CMD_NONE,
    CMD_OFF,
    CMD_MOVE_KEEP
};

#endif // SOCKETCMD_H
