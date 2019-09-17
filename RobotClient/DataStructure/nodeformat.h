#ifndef NODEFORMAT_H
#define NODEFORMAT_H

typedef struct {
    int ID;
    int type;
    double x, y, z;
    double yaw, pitch, roll;
    double gpsx, gpsy, gpsz, gpsyaw;
    double odomx, odomy, odomz, odomyaw;
    int gps_status, gps_num;
    int latFlag, lngFlag;
    double latValue, lngValue, altValue;
    double n1, n2, n3;
}PathNode;

typedef struct {
   int nodeid1;
   int nodeid2;
   float dis;
   float odomdis;
   int flag;
}PathEdge;

#endif // NODEFORMAT_H
