#include "correct_pose.h"
#include "DataStructure/vpose.h"
#include <iostream>
#include <QFileDialog>
#include <fstream>
#include <QString>
#include <cmath>
#include <Eigen>
#include <iomanip>
#define M_PI  3.14159265358979323846

int index_close = 0;
int index_open = 0;
int PI = 180;
Pose_error err;
double _x_sum,_y_sum,_z_sum;
vector<double> bilvs;
Correct_Pose::Correct_Pose()
{
    is_point = false;
    is_feature_point = false;

}
Correct_Pose::~Correct_Pose(){

    if(!nodes.empty())
        nodes.clear();
    if(!poses.empty())
        poses.clear();
    if(!poses_distance.empty())
        poses_distance.clear();
}
/*接收校正内容类型指令*/
void Correct_Pose::recv_correct_view(int cmd){
    switch(cmd){
    case 00:{
        is_point = true;
        is_feature_point = false;
    }break;
    case 01:{
        is_point = false;
        is_feature_point = true;
        std::cout<<"is_feature_point = true"<<std::endl;
    }
    default:{
    } break;
    }

}
//打开节点文件
void Correct_Pose::Open_NodeFile(){
    QString fileName = QFileDialog::getOpenFileName(0,tr("Open The Node File"),"G:/QT/point",tr("文本文件(*txt)"));
    int count = 0;
    if(!fileName.isEmpty()){
        QFile file(fileName);
        std::cout<<fileName.toStdString()<<std::endl;
        file.open(QIODevice::ReadOnly);
        NodeType test_node;
        while(file.read((char *)&test_node,sizeof(NodeType)))
        {
            nodes.push_back(test_node);
            count ++;
        }
        file.close();
    }
    std::cout<<"the num of node is"<<nodes.size()<<std::endl;
}
//打开路径点文件
void Correct_Pose::Open_PoseFile(){
    //QString fileName = QFileDialog::getOpenFileName(NULL,tr("打开文件"),"G:/QT/point",tr("文本文件(*txt)"));
    int count = 0;
    VPoseStamped test_pose;
    std::ifstream infile("G:/QT/point/poses_huahuan1.txt",std::ios::in |std::ios::binary);
    if(!infile.is_open()){
        std::cout<<"open poses_huahuan1 failed"<<std::endl;
        return;
    }
    while(infile.read((char *)&test_pose,sizeof(VPoseStamped))){
        poses.push_back(test_pose);
        poses_distance.push_back(test_pose);
        count++;
    }
    infile.close();
    std::ofstream ofile("G:/QT/point/Data_test/poses_huahuan1lujing_test.txt",std::ios::out|std::ios::app);
    if(!ofile.is_open()){
        std::cout<<"open lujing failed"<<std::endl;
        return;
    }
    int _test = 0;
    while(_test<poses.size()){
        ofile<<setprecision(10)<<poses[_test].timestamp<<std::endl;
        _test++;
     //   std::cout<<"_test   "<<_test<<std::endl;
    }
    ofile.close();
  //  std::cout<<"the num of pose is"<<poses.size()<<std::endl;
}
//寻找闭环节点
void Correct_Pose::closed_loop_pose(){
    std::cout<<"close_loop"<<std::endl;
    //test
    Open_NodeFile();
    Open_PoseFile();
    ////////////
  //  return;
    for(int i=1;i<nodes.size();i++){
        if(nodes[0].ID == nodes[i].ID){
            index_open = nodes[0].vec_ID;
            index_close = nodes[i].vec_ID;
            break;
        }
    }
    std::cout<<"index_open"<<index_open<<std::endl;
    std::cout<<"index_close"<<index_close<<std::endl;
    otherThings(index_open-1,index_close-1);
}
//角度校正
void Correct_Pose::otherThings(const int index_open,const int index_close){
    //节点ID
    std::cout<<"<===>" << poses[index_open].pose_id << "," << poses[index_close].pose_id <<std::endl;
    double delta_x_sum = poses[index_close].x - poses[index_open].x;
    double delta_y_sum = poses[index_close].y - poses[index_open].y;
    double delta_z_sum = poses[index_close].z - poses[index_open].z;
    std::cout<<"x:"<<delta_x_sum<<",y:"<<delta_y_sum<<",z:"<<delta_z_sum<<std::endl;
    for(int i = index_open+1;i<index_close+1;i++){
        if(poses[i].yaw > M_PI)
            poses[i].yaw -=(2 * M_PI);
        if(poses[i].yaw < (-1 * M_PI))
            poses[i].yaw +=(2 * M_PI);
        /**/
        if(poses[i].pitch > M_PI)
            poses[i].pitch -=(2 * M_PI);
        if(poses[i].pitch < (-1 * M_PI))
            poses[i].pitch +=(2 * M_PI);
        //
        if(poses[i].roll > M_PI)
            poses[i].roll -=(2 * M_PI);
        if(poses[i].roll < (-1 * M_PI))
            poses[i].roll +=(2 * M_PI);
    }
    //偏航角总差值
    std::cout<<poses[index_open].yaw<<","<<poses[index_close].yaw<<std::endl;
    std::cout<<poses[index_open].pitch<<","<<poses[index_close].pitch<<std::endl;
    std::cout<<poses[index_open].roll<<","<<poses[index_close].roll<<std::endl;
    double delta_yaw_all = poses[index_open].yaw - poses[index_close].yaw;      //偏航角总差值
    double delta_pitch_all = poses[index_open].pitch - poses[index_close].pitch;      //俯仰角总差值
    double delta_roll_all = poses[index_open].roll - poses[index_close].roll;      //翻滚角总差值
    std::cout << "yaw error is " << delta_yaw_all << std::endl;
    std::cout << "pitch error is " << delta_pitch_all << std::endl;
    std::cout << "roll error is " << delta_roll_all << std::endl;
    if(delta_x_sum == 0 && delta_y_sum == 0 && delta_z_sum == 0 && delta_yaw_all == 0
            && delta_pitch_all == 0 && delta_roll_all == 0){
        std::cout<<"no error"<<std::endl;
        return;
    }
    std::cout<<"range correct"<<std::endl;
    double distance_all = 0.0;     //距离总和
    //得到移动的角度差的总和
    for (int i = index_open + 1; i < index_close + 1; i++){
     //   yaw_all += fabs(poses[i].yaw - poses[i - 1].yaw);
        distance_all += sqrt((poses[i].x-poses[i-1].x)*(poses[i].x-poses[i-1].x) + (poses[i].y-poses[i-1].y)*(poses[i].y-poses[i-1].y)
                + (poses[i].z-poses[i-1].z)*(poses[i].z-poses[i-1].z));

    }
  //  std::cout<<"the sum of yaw"<<yaw_all<<std::endl;
    std::cout<<"the sum of range"<<distance_all<<std::endl;
    //校正
    double test_detal_distance;
    double detal_distance_all = 0.0;
    for(int i = index_open + 1; i< index_close + 1; i++){
        //将偏航误差分配到每个点云
        //误差 = 总误差 * 点云偏转角度/所有点云总偏转角度
     //   test_detal_yaw = fabs(poses[i].yaw - poses[i - 1].yaw);
        test_detal_distance = sqrt((poses_distance[i].x-poses_distance[i-1].x)*(poses_distance[i].x-poses_distance[i-1].x)
                + (poses_distance[i].y-poses_distance[i-1].y)*(poses_distance[i].y-poses_distance[i-1].y)
                + (poses_distance[i].z-poses_distance[i-1].z)*(poses_distance[i].z-poses_distance[i-1].z));


        detal_distance_all += test_detal_distance;

        double current_detal_yaw = delta_yaw_all * test_detal_distance/distance_all;     //偏航误差
        double current_detal_pitch = delta_pitch_all * test_detal_distance/distance_all;     //偏航误差
        double current_detal_roll = delta_roll_all * test_detal_distance/distance_all;     //偏航误差
    //    std::cout<<i<<":"<<current_detal_yaw<<","<<current_detal_pitch<<","<<current_detal_roll<<std::endl;
        err.yaw = current_detal_yaw;
        err.pitch = current_detal_pitch;
        err.roll = current_detal_roll;
        Current_detal.push_back(err);
        double post_x, post_y,post_z;//post_x和post_y为校正后的点的坐标
        //三维角度校正
        test_3d(poses[i].x,poses[i].y,poses[i].z,post_x,post_y,post_z,
                poses[i].i,poses[i].j,poses[i].k,poses[i].w,
                current_detal_roll,current_detal_yaw,current_detal_pitch);
        //角度校正时产生的位移校正
        poses[i].x = post_x;
        poses[i].y = post_y;
        poses[i].z = post_z;
      //  std::cout<<"the yaw"<<poses[i].yaw<<std::endl;
        //角度校正
        poses[i].yaw += delta_yaw_all*detal_distance_all/distance_all;
        poses[i].pitch += delta_pitch_all*detal_distance_all/distance_all;
        poses[i].roll += delta_roll_all*detal_distance_all/distance_all;
       // std::cout<<"after adjust the yaw"<<poses[i].yaw<<std::endl;

        oula_Quaternion((poses[i].roll)/2,(poses[i].yaw)/2,(poses[i].pitch)/2,poses[i].i,poses[i].j,poses[i].k,poses[i].w);

    }

    std::cout<<"after adjust yaw error "<<poses[index_open].yaw<<","<<poses[index_close].yaw<<std::endl;
    std::cout<<"the detal_distance_all"<<detal_distance_all<<std::endl;

    Range_Correction(index_open,index_close);
    Write_PoseFile();
    /*判断特征点云校正还是先验地图校正*/
    if(is_point)
        Point_Correction(index_open,index_close);
    if(is_feature_point){
        Feature_Correction(index_open,index_close);
        std::cout<<"Feature_Correction"<<std::endl;
    }
    std::cout<<"correct over"<<std::endl;

}
 //距离校正
void Correct_Pose::Range_Correction(const int index_open,const int index_close){
    std::cout<<"range_correction"<<std::endl;
    double delta_x_sum = poses[index_open].x - poses[index_close].x;				//x总误差
    double delta_y_sum = poses[index_open].y - poses[index_close].y;				//y总误差
    double delta_z_sum = poses[index_open].z - poses[index_close].z;				//z总误差
    _x_sum = delta_x_sum;
    _y_sum = delta_y_sum;
    _z_sum = delta_z_sum;
    double l_all = 0.0;
  //  vector<double> bilvs;
    bilvs.clear();
    double current_l = 0.0;
    for (int i = index_open + 1; i < index_close + 1; i++)
    {
        double xxx = poses[i].x - poses[i - 1].x;
        double yyy = poses[i].y - poses[i - 1].y;
        double zzz = poses[i].z - poses[i - 1].z;
        l_all += sqrt(xxx*xxx + yyy*yyy + zzz*zzz);
    }

    for (int i = index_open + 1; i < index_close + 1; i++)
    {
        double x_x = poses[i].x - poses[i - 1].x;
        double y_y = poses[i].y - poses[i - 1].y;
        double z_z = poses[i].z - poses[i - 1].z;
        current_l += sqrt(x_x * x_x + y_y * y_y + z_z*z_z);
        bilvs.push_back(current_l/ l_all);
    }

    for (int i = index_open + 1; i < index_close + 1; i++)
    {
        poses[i].x += delta_x_sum * bilvs[i - (index_open + 1)];
        poses[i].y += delta_y_sum * bilvs[i - (index_open + 1)];
        poses[i].z += delta_z_sum * bilvs[i - (index_open + 1)];
    }
    std::cout<<"after adjust_x"<<poses[index_open].x<<","<<poses[index_close].x<<std::endl;
    std::cout<<"after adjust_y"<<poses[index_open].y<<","<<poses[index_close].y<<std::endl;
    std::cout<<"after adjust_z"<<poses[index_open].z<<","<<poses[index_close].z<<std::endl;

}
//校正后路径点重新写入文件
void Correct_Pose::Write_PoseFile(){
    ofstream outfile("G:/QT/point/Data_test/points_daheilou_c.txt", std::ios::out | std::ios::app | std::ios::binary);
    std::cout<<"G:/QT/point/points_daheilou_c.txt"<<std::endl;
    if(!outfile.is_open()){
        std::cout<<"open filed"<<std::endl;
        return;
    }
    for(int i = 0;i<index_close;i++)
    {
        outfile.write((char*)&poses[i],sizeof(VPoseStamped));

    }
    outfile.close();
    /*******************/
    ofstream outfile2("G:/QT/point/Data_test/points_daheilou_c_node.txt", std::ios::out | std::ios::app | std::ios::binary);
    std::cout<<"G:/QT/point/points_daheilou_c_node.txt"<<std::endl;
    if(!outfile2.is_open()){
        std::cout<<"open filed"<<std::endl;
        return;
    }
    int node_id;
    NodeType node_test;
    for(int i = 0;i<nodes.size();i++)
    {
        node_id = nodes[i].vec_ID;
        node_test.ID = nodes[i].ID;
        node_test.vec_ID = node_id;

        node_test.pitch = poses[node_id].pitch;
        node_test.yaw = poses[node_id].yaw;
        node_test.roll = poses[node_id].roll;
        node_test.x = poses[node_id].x;
        node_test.y = poses[node_id].y;
        node_test.z = poses[node_id].z;
        node_test.i = poses[node_id].i;
        node_test.j = poses[node_id].j;
        node_test.k = poses[node_id].k;
        node_test.w = poses[node_id].w;

        outfile2.write((char*)&node_test,sizeof(NodeType));

    }
    outfile2.close();

}
//重新给欧拉角赋值
void Correct_Pose::oula_Quaternion(const double roll, const double yaw, const double pitch,
                                   float &posti, float &postj, float &postk, float &postw){
    //std::cout<<posti<<";"<<postj<<";"<<postk<<";"<<postw<<std::endl;
    postw = cos(roll)*cos(pitch)*cos(yaw) + sin(roll)*sin(pitch)*sin(yaw);
    posti = sin(roll)*cos(pitch)*cos(yaw) - cos(roll)*sin(pitch)*sin(yaw);
    postj = cos(roll)*sin(pitch)*cos(yaw) + sin(roll)*cos(pitch)*sin(yaw);
    postk = cos(roll)*cos(pitch)*sin(yaw) - sin(roll)*sin(pitch)*cos(yaw);

}

//将四元数转换为旋转矩阵
void Correct_Pose::transe(double q0, double q1, double q2, double q3,Eigen::Matrix3d &m){

    m(0,0) = 1-2*q2*q2 - 2*q3*q3;
    m(0,1) = 2*q1*q2 + 2*q0*q3;
    m(0,2) = 2*q1*q3 - 2*q0*q2;
    //
    m(1,0) = 2*q1*q2 - 2*q0*q3;
    m(1,1) = 1-2*q1*q1 - 2*q3*q3;
    m(1,2) = 2*q2*q3 + 2*q0*q1;
    //
    m(2,0) = 2*q1*q3 + 2*q0*q2;
    m(2,1) = 2*q2*q3 - 2*q0*q1;
    m(2,2) = 1 - 2*q1*q1 - 2*q2*q2;

}
//特征点云校正
void Correct_Pose::Feature_Correction(const int index_open, const int index_close){
    CPackage pack;
    VPointFeature point_surf;
    Pose_error err_poses;
    double point_time;
    int point_num;
    float error_i,error_j,error_k,error_w;
    Eigen::Matrix3d Matrix_err_point(1,3);      //原xyz
    Eigen::Matrix3d Matrix_point(1,3);          //修改后的xyz
    Eigen::Matrix3d error_m(3,3);               //误差矩阵
    Eigen::Matrix3d error_inv(3,3);             //误差矩阵的逆
    std::cout<<"feature point correct"<<std::endl;
    std::ifstream infile("G:/QT/point/Data_test/daheilou_c_corner_feature_with_time.xyzi",std::ios::in |std::ios::binary);
    if(!infile.is_open()){
        std::cout<<"open daheilou_c failed"<<std::endl;
        return;
    }
    std::cout<<"_x_sum"<<_x_sum<<"_y_sum"<<_y_sum<<"_z_sum"<<_z_sum<<std::endl;
    /***********************************/
    //特征点云校正
    /***********************************/
    while(infile.read((char*)&pack,sizeof(CPackage))){
        point_num = pack.msgLen;
        point_time = pack.time_cloud;
        if(!point_feature.empty())
            point_feature.clear();
        while(point_num){
            infile.read((char*)&point_surf,sizeof(VPointFeature));         //读取点云数据
            point_feature.push_back(point_surf);
            point_num--;
        }
        for(int i = index_open+1;i<index_close+1;i++){
            if(fabs(point_time - poses[i].timestamp)<0.05f){
            //    std::cout<<"find the same time"<<std::endl;
                Write_feature_point("G:/QT/point/Data_test/daheilou_c_corner_error.xyzi",point_feature);
                //寻找该帧点云对应的路径点的误差
                //姿态校正
                err_poses.yaw = Current_detal[i-index_open-1].yaw;
                err_poses.pitch = Current_detal[i-index_open-1].pitch;
                err_poses.roll = Current_detal[i-index_open-1].roll;
                //将角度误差转换为四元数
                oula_Quaternion(err_poses.roll/2.0,err_poses.yaw/2.0,err_poses.pitch/2.0,error_i,error_j,error_k,error_w);
                //误差旋转矩阵
                transe(error_w,error_i,error_j,error_k,error_m);
                error_inv = error_m.inverse();//求矩阵的逆
                for(int jjj = 0;jjj<point_feature.size();jjj++){

                    Matrix_err_point(0,0)=point_feature[jjj].x;
                    Matrix_err_point(0,1)=point_feature[jjj].y;
                    Matrix_err_point(0,2)=point_feature[jjj].z;
                  //  std::cout<<error_inv<<std::endl;
                    Matrix_point = error_inv * Matrix_err_point;
                    point_feature[jjj].x = Matrix_point(0,0);
                    point_feature[jjj].y = Matrix_point(0,1);
                    point_feature[jjj].z = Matrix_point(0,2);
                }
                //校正点云距离误差
                for(int iii = 0; iii <point_feature.size();iii++){
                    point_feature[iii].x += _x_sum * bilvs[i - (index_open + 1)];
                    point_feature[iii].y += _y_sum * bilvs[i - (index_open + 1)];
                    point_feature[iii].z += _z_sum * bilvs[i - (index_open + 1)];
                }
                Write_feature_point("G:/QT/point/Data_test/daheilou_c_corner_corr.xyzi",point_feature);
                break;
            }
        }
    }
    infile.close();

}
//校正点云误差
void Correct_Pose::Point_Correction(const int index_open, const int index_close){
    CPackage pack;
    streampos sp;
    VPointI point;
    VPointFeature point_surf;
    Pose_error err_poses;
    double point_time;
    int point_num;
    float error_i,error_j,error_k,error_w;
    Eigen::Matrix3d Matrix_err_point(1,3);      //原xyz
    Eigen::Matrix3d Matrix_point(1,3);          //修改后的xyz
    Eigen::Matrix3d error_m(3,3);               //误差矩阵
    Eigen::Matrix3d error_inv(3,3);             //误差矩阵的逆
    std::cout<<"point correct"<<std::endl;
    std::ifstream infile("G:/QT/point/points_huahuan1.xyzi",std::ios::in |std::ios::binary);
    if(!infile.is_open()){
        std::cout<<"open huahuan failed"<<std::endl;
        return;
    }
    std::cout<<"_x_sum"<<_x_sum<<"_y_sum"<<_y_sum<<"_z_sum"<<_z_sum<<std::endl;
    /***********************************/
    //xyzi点云校正
    /***********************************/
   //读取Cpackage获取时间戳
    int test_num =0;
    while(infile.read((char*)&pack,sizeof(CPackage))){
        point_num = pack.msgLen;
        point_time = pack.time_cloud;
        point_time_test.push_back(point_time);
        test_num++;
        if(!point_corr.empty())
            point_corr.clear();
        while(point_num){
            infile.read((char*)&point,sizeof(VPointI));         //读取点云数据
            point_corr.push_back(point);
            point_num--;
        }
        //角度误差校正

        for(int i = index_open+1;i<index_close+1;i++){
            if(fabs(point_time - poses[i].timestamp)<0.05f){
                //std::cout<<poses[i].timestamp<<std::endl;                   //判断时间戳是否相等
                sp = infile.tellg();
                Write_point_corr("G:/QT/point/Data_test/daheilou_c-point_err_3.xyzi",point_corr);      //将
                err_poses.yaw = Current_detal[i-index_open-1].yaw;
                err_poses.pitch = Current_detal[i-index_open-1].pitch;
                err_poses.roll = Current_detal[i-index_open-1].roll;
               // std::cout<<i<<":"<<err_poses.yaw<<","<<err_poses.pitch<<","<<err_poses.roll<<std::endl;
                oula_Quaternion(err_poses.roll/2.0,err_poses.yaw/2.0,err_poses.pitch/2.0,error_i,error_j,error_k,error_w);
                transe(error_w,error_i,error_j,error_k,error_m);
                error_inv = error_m.inverse();//求矩阵的逆
                //对该帧点云校正
                for(int jjj = 0;jjj<point_corr.size();jjj++){

                    Matrix_err_point(0,0)=point_corr[jjj].x;
                    Matrix_err_point(0,1)=point_corr[jjj].y;
                    Matrix_err_point(0,2)=point_corr[jjj].z;
                  //  std::cout<<error_inv<<std::endl;

                    Matrix_point = error_inv * Matrix_err_point;
                    point_corr[jjj].x = Matrix_point(0,0);
                    point_corr[jjj].y = Matrix_point(0,1);
                    point_corr[jjj].z = Matrix_point(0,2);

                }
                //校正点云距离误差
                for(int iii = 0; iii <point_corr.size();iii++){
                    point_corr[iii].x += _x_sum * bilvs[i - (index_open + 1)];
                    point_corr[iii].y += _y_sum * bilvs[i - (index_open + 1)];
                    point_corr[iii].z += _z_sum * bilvs[i - (index_open + 1)];
                }

          //      std::cout<<i<<std::endl;
                Write_point_corr("G:/QT/point/Data_test/daheilou_c_corr_3.xyzi",point_corr);
                break;
            }
            else{
               // std::cout<<"no same time poses"<<std::endl;
            }
        }
    }
    infile.close();
    std::cout<<"test_num"<<test_num<<std::endl;
    std::cout<<"point_time_test"<<point_time_test.size()<<std::endl;
    std::ofstream ofile("G:/QT/point/Data_test/dianyun_test.txt",std::ios::out|std::ios::app);
    if(!ofile.is_open()){
        std::cout<<"open lujing failed"<<std::endl;
        return;
    }
    int _test_point = 0;
    while(_test_point<point_time_test.size()){
        ofile<<setprecision(10)<<point_time_test[_test_point]<<std::endl;
        _test_point++;
       // std::cout<<"_test   "<<_test<<std::endl;
    }
    ofile.close();
    std::cout<<"write dianyun_test success"<<std::endl;


}
//三维角度校正参数：原始坐标，修正后坐标，原始四元数，角度误差
void Correct_Pose::test_3d(const double prex, const double prey, const double prez,
                           double &postx, double &posty, double &postz,
                           const float i, const float j,const float k, const float w,
                           const double roll, const double yaw, const double pitch){
    //得到误差角的四元数
    float error_i,error_j,error_k,error_w;
    Eigen::Matrix3d error_m(3,3);
    Eigen::Matrix3d error_inv(3,3);
    //将误差的欧拉角转换为四元数
    /*
    error_w = cos(roll/2)*cos(pitch/2)*cos(yaw/2) + sin(roll/2)*sin(pitch/2)*sin(yaw/2);
    error_i = sin(roll/2)*cos(pitch/2)*cos(yaw/2) - cos(roll/2)*sin(pitch/2)*sin(yaw/2);
    error_j = cos(roll/2)*sin(pitch/2)*cos(yaw/2) + sin(roll/2)*cos(pitch/2)*sin(yaw/2);
    error_k = cos(roll/2)*cos(pitch/2)*sin(yaw/2) - sin(roll/2)*sin(pitch/2)*cos(yaw/2);
    */
    oula_Quaternion(roll/2.0,yaw/2.0,pitch/2.0,error_i,error_j,error_k,error_w);
    //误差四元数转换为旋转矩阵
    transe(error_w,error_i,error_j,error_k,error_m);
    //求矩阵的逆
    error_inv = error_m.inverse();//求矩阵的逆
   // std::cout<<error_inv<<std::endl;
    //原四元数矩阵
    Eigen::Matrix3d Matrix_err(3,3);
    transe(w,i,j,k,Matrix_err);             //四元数转换为矩阵
    Eigen::Matrix3d Matrix_err_xyz(1,3);
    Matrix_err_xyz(0,0)=prex;
    Matrix_err_xyz(0,1)=prey;
    Matrix_err_xyz(0,2)=prez;

    Eigen::Matrix3d Matrix_xyz(1,3);
    //修正误差后的坐标
    Matrix_xyz = error_inv * Matrix_err_xyz;
    postx = Matrix_xyz(0,0);
    posty = Matrix_xyz(0,1);
    postz = Matrix_xyz(0,2);
}
void Correct_Pose::Write_point_corr(const string path, std::vector<VPointI> &point){
    ofstream outfile(path, std::ios::out | std::ios::app | std::ios::binary);
    if(!outfile.is_open()){
        std::cout<<"open filed"<<std::endl;
        return;
    }
    for(int i=0;i<point.size();i++){
        outfile.write((char*)&point[i],sizeof(VPointI));
    }
    outfile.close();
}
void Correct_Pose::Write_feature_point(const string path, std::vector<VPointFeature> &point){
    ofstream outfile(path, std::ios::out | std::ios::app | std::ios::binary);
    if(!outfile.is_open()){
        std::cout<<"open filed"<<std::endl;
        return;
    }
    for(int i=0;i<point.size();i++){
        outfile.write((char*)&point[i],sizeof(VPointFeature));
    }
    outfile.close();
}
