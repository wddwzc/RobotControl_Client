#include "pointcloud.h"
PointCloudData* PointCloudData::pGD = 0;

PointCloudData::PointCloudData() {
    //isOnlineOct = true;
    displayFlag = 1;
    isShowPCOn = true;
    isShowPoseOn = true;
}

PointCloudData::~PointCloudData() {

}

PointCloudData* PointCloudData::GetInstance() {
    if(0 == pGD)
        pGD = new PointCloudData;
    return pGD;
}

void PointCloudData::reset() {
    point_que.clear();
    PointCloud.clear();
    //p_octomap.reset();
}
