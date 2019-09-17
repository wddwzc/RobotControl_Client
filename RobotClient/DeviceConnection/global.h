#ifndef GLOBAL_H
#define GLOBAL_H

enum CmdType{
    //停止并关闭套接字
    CMD_ROBOT_OFF = 0,
    //暂停
    CMD_ROBOT_PAUSE,
    //空挡，类似于暂停状态，但是仅仅是车辆停在原处
    CMD_ROBOT_STAY,

    //一般速度前进后退，左右直转
    CMD_ROBOT_UP,
    CMD_ROBOT_BACK,
    CMD_ROBOT_LEFT,
    CMD_ROBOT_RIGHT,

    //加速状态前进后退，左右直转
    CMD_ROBOT_ACC_UP,
    CMD_ROBOT_ACC_BACK,
    CMD_ROBOT_ACC_LEFT,
    CMD_ROBOT_ACC_RIGHT,

    //发生错误
    CMD_ROBOT_ERROR
};



#endif // GLOBAL_H
