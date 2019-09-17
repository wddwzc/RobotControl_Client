#ifndef OPENGLDISPAY_H
#define OPENGLDISPAY_H

//选择地图编辑变换模式 二选一
#define ViewMatrixMode      //摄像机系统旋转平移
//#define ModelMatrixMode     //模型矩阵旋转平移

#include "DataStructure/pointcloud.h"
#include "DataStructure/displayflag.h"
#include "DeviceControl/dialplate.h"
#include "DataStructure/nodeformat.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <time.h>

#include <QString>
#include <QDebug>
#include <QMouseEvent>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QStyle>
#include <QFileInfo>

//OpenGL Mathematics 用来进行数学变换
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class OpenglDisplay : public QOpenGLWidget,
                      protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    OpenglDisplay(QWidget * parent = 0);
    OpenglDisplay(bool mode, QWidget * parent = 0);
    ~OpenglDisplay();

public:
    bool OpenFile();                           //打开点云文件
    void CloseFile();                          //关闭文件，清除显示

public slots:
    void DisplayFlagChange(DisplayFlag flag);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private slots:
    void GridSizeChange();
    void ShowStatus();
    void RecoverConversion();

private:
    void InitToolModule();
    void InitParameters();
    void printContextInformation();            //输出OpenGL信息
    void CreateVertexShader(GLuint index);     //创建顶点着色器
    void CreateFragmentShader(GLuint index);   //创建片段着色器
    void CreateShaderProgram(GLuint index);    //链接着色器对象
    void SetCloudVertex(GLboolean mode);       //设置点云顶点属性（VAO/VBO）
    void SetPickedVertex(GLboolean mode);      //设置拾取点顶点属性（VAO/VBO）
    void SetCoordsVertex(GLboolean mode);      //设置坐标顶点属性（VAO/VBO）
    void SetGridVertex(GLboolean mode);        //设置网格顶点属性（VAO/VBO）
    void SetOriginVertex(GLboolean mode);
    void SetPoseVertex(GLboolean mode);
    void SetPathVertex(GLboolean mode);
    void SetPriorVertex(GLboolean mode);
    void LoadPoints(const QString&);           //加载点云
    void CreatePoseMark(GLuint num, GLfloat x, GLfloat y, GLfloat z, GLfloat *vertices, GLuint *vertices_index);
    void CreatePathMark(GLuint num, glm::vec3 point_now, glm::vec3 point_last, GLfloat *vertices, GLuint *vertices_index);

private:
    //**********************************************************控制标志位
    bool is_online;
    DisplayFlag flag_display;
    //**********************************************************功能栏及状态监控 & 窗口布局
    QToolButton *btn_enlarge;
    QToolButton *btn_shrink;
    QToolButton *btn_increase;
    QToolButton *btn_reduce;
    QToolButton *btn_recover;

    SpeedPlate *speedplate;
    QToolButton *btn_showstatus;

    QGridLayout *layout_main;
    QHBoxLayout *layout_tool;
    QHBoxLayout *layout_status;
    //**********************************************************数据缓冲
    PointCloudData *p_global;
    GLuint capacity_que;

    GLfloat *vertices_cloud;
    GLuint PointNum;

    GLfloat *vertices_prior;
    std::vector<glm::vec4> PointPriorQue;
    GLuint PriorNum;

    VPoseStamped robot_pose;
    int PathNum;

    std::vector<glm::vec3> selected_point;
    std::vector<glm::vec3> filt_point;
    GLfloat *vertices_picked;
    GLuint *vertices_picked_index;
    GLuint PickedNum;

    std::vector<PathNode> pre_pathpoint;
    std::vector<PathNode> path_point;
    //**********************************************************窗口参数
    QCursor cursor;
    //点云尺寸大小，单位为像素
    GLfloat PointSize;
    //点云的高度值的上下界
    GLfloat m_maxZ;
    GLfloat m_minZ;
    //地面网格
    int m_rangeLength;    //地面网格范围的边长
    int m_cellLength;     //地面网格单元的边长
    //**********************************************************鼠标操作的一些设置
    //相机位置及朝向，用来构造ViewMatrix，进行“世界空间”到“观察空间”的转换
    glm::vec3 cameraPos;    //相机位置
    glm::vec3 worldCentrol; //世界坐标原点，相机始终朝向这个方向
    glm::vec3 cameraFront;  //摄像机的朝向
    glm::vec3 cameraUp;     //相机的顶部始终朝向y轴

    //构建ModelMatrix，进行“局部空间”到“世界空间”的转换
    glm::vec3 transVec;     //局部坐标在世界坐标中的平移量

    GLfloat yaw;        //偏航角
    GLfloat pitch;      //俯仰角
    GLfloat lastX;      //光标上次x值
    GLfloat lastY;      //光标上次y值
    GLfloat radius;      //距离
    //**********************************************************着色器、着色器程序、VAO（顶点数组对象）、VBO（顶点缓冲对象）
    GLuint *IDVertexShader;
    GLuint *IDFragmentShader;
    GLuint *IDShaderProgram;
    GLuint *IDVAO;
    GLuint *IDVBO;
    GLuint *IDEBO;

//********************************************************************
//            离线部分
//********************************************************************
public:
    void PointPicking();

public slots:
    void SavePointsPicked();
    void RepealPointsPicked();
    void LoadPath();
    void ClearPath();
    void LoadGraphPath();

private:
    void SetPickedPointQueue();
    //点云拾取
    void RayCastingClickedPoint(int &mouseX,
                                int &mouseY,
                                int &viewportWidth,
                                int &viewportHeight,
                                glm::mat4 &projectionMat,
                                glm::mat4 &viewMat,
                                glm::vec3 &directVec);
    void GetAddedPoint(int &mouseX,
                       int &mouseY,
                       int &viewportWidth,
                       int &viewportHeight,
                       int &AdPointX,
                       int &AdPointY);
    void GetPickingRadius(glm::vec3 &cameraPos,
                          glm::vec3 &directVec,
                          glm::vec3 &directVecAd,
                          float &pickingRadius);
    bool IsPointInCircle(glm::mat4 &modelMat,
                         glm::vec3 &cameraPos,
                         glm::vec3 &directVec,
                         float &pickingRadius,
                         float &currentRadius,
                         float &x,
                         float &y,
                         float &z);

private:
    //**********************************************************拾取操作相关变量
    glm::vec3 selectedPos;         //光标选中的点
    int mouseX;                    //拾取时光标的像素坐标x值
    int mouseY;                    //拾取时光标的像素坐标y值
    int lastXY;                    //记录上一次光标坐标之和，用于点云拾取
    int viewportWidth;             //OpenGL视口尺寸
    int viewportHeight;
    bool IsPickingState;           //拾取状态标志位
    bool ClickedFlag;              //光标点击标志
    bool is_rotate;                //标记是否在进行旋转操作
    int radiusPixels;              //点云拾取半径

//********************************************************************
//            在线部分
//********************************************************************
public slots:
    void OnlineUpdate();
    void UpdateStatus(int linear, int angular);
};

#endif // OPENGLDISPAY_H
