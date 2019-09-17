#ifndef CORRECTNODE_H
#define CORRECTNODE_H

class NodeType
{
public:
    float ID;           //节点ID
    float vec_ID;       //节点在数组中的ID
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

    double timestamp;
};

class Pose_error
{
public:
    double yaw;
    double pitch;
    double roll;
};

#endif // CORRECTNODE_H
