#ifndef VPOSE_H
#define VPOSE_H

#include <iostream>

class VPose
{
public:
    float x;
    float y;
    float z;

    float i;
    float j;
    float k;
    float w;

    float yaw;
    float pitch;
    float roll;

};

class VPoseStamped :public VPose
{
public:
    double timestamp;
    int pose_id;
};

#endif // VPOSE_H
