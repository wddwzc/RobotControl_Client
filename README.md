# RobotControl_Client

## 概述

用于远程操控机器人、编辑点云数据的Windows应用。基于Qt5.7构建，OpenGL API由glm提供，矩阵计算由Eigen支持，均为头文件库，已置于工程目录下。

## 功能介绍

客户端包含两部分功能：

- 远程控制机器人移动，实时回传传感器数据
- 离线编辑大范围点云地图，选取路径点

### 在线监控&控制

用于远程监视与控制移动机器人，在线查看点云数据、传感器信息、机器人位姿

![image](https://github.com/wddwzc/RobotControl_Client/tree/master/img/online.png)

### 离线编辑点云

激光构图后，查看建图效果，提供旋转、平移、放缩等基本功能

![image](https://github.com/wddwzc/RobotControl_Client/tree/master/img/offline_display.png)

两种高程显示模式

![image](https://github.com/wddwzc/RobotControl_Client/tree/master/img/elevation_gray.png)
![image](https://github.com/wddwzc/RobotControl_Client/tree/master/img/elevation_rgb.png)

在点云图上手动选取路径点，为自主导航提供大致的可通行参考路径

![image](https://github.com/wddwzc/RobotControl_Client/tree/master/img/offline.png)

## 其他

**静态编译发行版**RobotClient.exe，可在任意windows环境运行