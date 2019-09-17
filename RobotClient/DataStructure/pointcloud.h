#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <QMutex>
#include <QString>
#include "DataStructure/vpoint.h"
#include "DataStructure/vqueue.h"
#include "DataStructure/vpose.h"

#include <glm/gtc/type_ptr.hpp>

class PointCloudData
{
public:
    PointCloudData();
    ~PointCloudData();

public:
    std::string onlineFilePath;     //在线时，实时存储点云的文件夹路径
    VQue<VPointI> point_que;        //在线的点云（循环队列）
    QMutex que_lock;                //在线点云队列的互斥锁
    std::vector<VPointIL> PointCloud;//离线的点云（vector数组）
    VPoseStamped RobotPose;
    std::vector<glm::vec3> PathNode;
    //显示时的一些参数
    int displayFlag;                //1-高程 2-强度  3-固定颜色
    bool isShowGridOn;              //是否显示地面网格
    bool isShowPCOn;
    bool isShowPoseOn;
    int modedisplay;                //显示模式，1为offline，2为mapping，3为locate
    QString CurrentPath;//当前目录
    //QString LastPath;
    QString FilePath;
    //重置一些全局变量
    void reset();

private:
    static PointCloudData *pGD;

public:
    // 获得全局数据类的单例指针
    static PointCloudData* GetInstance();
};

#endif // POINTCLOUD_H
