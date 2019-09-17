#ifndef CORRECT_POSE_H
#define CORRECT_POSE_H

/*
#include <QWidget>
#include <vector>
#include "DataStructure/correctnode.h"
#include <Eigen>

class Correct_Pose: public QObject
{
   Q_OBJECT

public:
    Correct_Pose();
    ~Correct_Pose();

public:
    void otherThings(const int index_open,const int index_close);
    //三维路径点校正
    void test_3d(const double prex, const double prey,const double prez,
                 double &postx, double &posty,double &postz,
                  const float posti, const float postj, const float postk,const  float postw,
                  const double roll,const double yaw,const double pitch);
    void transe(double q0,double q1,double q2,double q3,Eigen::Matrix3d &m);    //四元数转换为选择平移矩阵

public slots:
    void closed_loop_pose();                    //寻找闭环节点
    void recv_correct_view(int cmd);            //接收校正内容类型指令

private:
    void Open_NodeFile();                                                           //打开节点文件
    void Open_PoseFile();                                                           //打开路径点文件
    void oula_Quaternion(const double roll,const double yaw,const double pitch,
                         float &posti,float &postj,float &postk,float &postw);      //欧拉角转换为四元数
    void Write_PoseFile();                                                          //写文件
    void Range_Correction(const int index_open,const int index_close);              //距离校正
    //将校正后的点云写入文件
    void Write_point_corr(const string path, std::vector<VPointI>&point);
    void Write_feature_point(const string path, std::vector<VPointFeature>&point);  //特征点云写入文件
    void Point_Correction(const int index_open,const int index_close);              //点云校正
    void Feature_Correction(const int index_open,const int index_close);            //特征点云校正

public:
    bool is_point;
    bool is_feature_point;

private:
    vector<NodeType>nodes ;
    vector<VPoseStamped>poses;                  //读取路径点数组
    vector<VPoseStamped>poses_distance;         //存放原路径点数据
    vector<VPointI>point_corr;                  //点云校正数组
    vector<VPointFeature>point_feature;
    vector<Pose_error>Current_detal;            //存放每个路径点处误差
    vector<double>point_time_test;
};
*/

#endif // CORRECT_POSE_H
