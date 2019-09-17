#include "opengldispay.h"
#include "shader.h"

//#define DEBUG
//#define VertexNum 100000
#define Xcovert(X, Y, W) (X*cos(W)-Y*sin(W))
#define Ycovert(X, Y, W) (X*sin(W)+Y*cos(W))

OpenglDisplay::OpenglDisplay(QWidget *parent)
    : QOpenGLWidget(parent) {
    //设置OpenGL的版本信息
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4,3);
    setFormat(format);
    InitParameters();
    vertices_cloud = NULL;
    p_global = PointCloudData::GetInstance();
    capacity_que = 0;
    InitToolModule();
}

//*****************************************************
//      参数1-类型 true 在线 false 离线
//      参数2-父窗口指针
//*****************************************************
OpenglDisplay::OpenglDisplay(bool mode, QWidget *parent)
    : QOpenGLWidget(parent), is_online(mode) {
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4,3);
    setFormat(format);
    InitParameters();
    InitToolModule();
}

OpenglDisplay::~OpenglDisplay() {
    delete []IDVertexShader;
    delete []IDFragmentShader;
    delete []IDShaderProgram;
    delete []IDVAO;
    delete []IDVBO;
    delete []IDEBO;
}

//****************** 打开点云文件槽函数 ******************
//
//*****************************************************
bool OpenglDisplay::OpenFile()
{
    if(PointPriorQue.capacity() != 0) {
        CloseFile();
    }
    //QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), "F:", tr("文本文件(*txt)"));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "F:");
    if(fileName == "")
        return false;
    LoadPoints(fileName);
    return true;
}

//****************** 关闭点云文件槽函数 ******************
//
//*****************************************************
void OpenglDisplay::CloseFile()
{
    PointPriorQue.clear();
    delete[]vertices_prior;
    std::vector<glm::vec4>().swap(PointPriorQue);
    qDebug() << PointPriorQue.size() << PointPriorQue.capacity();
    PriorNum = 0;
    update();
}

//*************** 接收显示标志位（槽函数） ***************
//      参数1-显示标志位集合
//*****************************************************
void OpenglDisplay::DisplayFlagChange(DisplayFlag flag) {
    if (flag_display.style != flag.style) {
        flag_display.style = flag.style;
        SetPriorVertex(MODE_SET_ATTRIB);
    }
    flag_display = flag;
    update();
}

//******************* OpenGL Events *******************
//      initializeGL 初始化
//      resizeGL 尺寸变化
//      paintGL 绘制
//*****************************************************
void OpenglDisplay::initializeGL() {
    // Initialize OpenGL Backend
    initializeOpenGLFunctions();
    printContextInformation();
    // Set global information
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //black
    //glClearColor(0.3f, 0.3f, 0.3f, 1.0f); //gray
    glClearColor(0.23f, 0.23f, 0.23f, 1.0f);
    //**********************************************************顶点着色器创建
    CreateVertexShader(MODE_POINTCLOUD);    //第一个顶点着色器（点云）
    CreateVertexShader(MODE_POINTPICKED);   //第二个顶点着色器（拾取点)
    CreateVertexShader(MODE_COORDS);        //第三个顶点着色器（坐标)
    CreateVertexShader(MODE_GRID);          //第四个顶点着色器（网格）
    CreateVertexShader(MODE_ORIGIN);        //第五个顶点着色器（原点）
    CreateVertexShader(MODE_POSE);          //第六个顶点着色器（位置）
    CreateVertexShader(MODE_PATH);          //第七个顶点着色器（路径）
    CreateVertexShader(MODE_PRIOR);         //第八个顶点着色器（先验地图）
    //**********************************************************片段着色器创建
    CreateFragmentShader(MODE_POINTCLOUD);  //第一个片段着色器（点云）
    CreateFragmentShader(MODE_POINTPICKED); //第二个片段着色器（拾取点）
    CreateFragmentShader(MODE_COORDS);      //第三个片段着色器（坐标）
    CreateFragmentShader(MODE_GRID);        //第四个片段着色器（网格）
    CreateFragmentShader(MODE_ORIGIN);
    CreateFragmentShader(MODE_POSE);
    CreateFragmentShader(MODE_PATH);
    CreateFragmentShader(MODE_PRIOR);
    //**********************************************************链接着色器
    CreateShaderProgram(MODE_POINTCLOUD);   //第一个着色器程序（点云)
    CreateShaderProgram(MODE_POINTPICKED);  //第二个着色器程序（拾取点）
    CreateShaderProgram(MODE_COORDS);       //第三个着色器程序（坐标）
    CreateShaderProgram(MODE_GRID);         //第四个着色器程序（网格）
    CreateShaderProgram(MODE_ORIGIN);
    CreateShaderProgram(MODE_POSE);
    CreateShaderProgram(MODE_PATH);
    CreateShaderProgram(MODE_PRIOR);
    //**********************************************************VAO\VBO\顶点属性指针
    glGenVertexArrays(NumVAO, IDVAO);   //创建VAO
    glGenBuffers(NumVBO, IDVBO);        //创建VBO
    glGenBuffers(NumEBO, IDEBO);        //创建EBO
    SetCloudVertex(MODE_SET_ATTRIB);    //点云
    SetPickedVertex(MODE_SET_ATTRIB);   //拾取点
    SetCoordsVertex(MODE_SET_ATTRIB);   //坐标
    SetGridVertex(MODE_SET_ATTRIB);     //网格
    SetOriginVertex(MODE_SET_ATTRIB);   //原点
    SetPoseVertex(MODE_SET_ATTRIB);     //位置
    SetPathVertex(MODE_SET_ATTRIB);     //路径
    SetPriorVertex(MODE_SET_ATTRIB);    //先验地图
    //固定属性区域
    glLineWidth(10.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  //线框模式开启
    glEnable(GL_DEPTH_TEST);                    //开启深度测试
}

void OpenglDisplay::resizeGL(int width, int height)
{
    viewportHeight = height;
    viewportWidth = width;
}

void OpenglDisplay::paintGL()
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(PointSize);               //调整点的大小
    SetCoordsVertex(MODE_PAINT);
    if (flag_display.cloud) SetCloudVertex(MODE_PAINT);
    if (flag_display.picked) {
        if (PickedNum != 0) {
            SetPickedVertex(MODE_PAINT);
        }
    }
    if (flag_display.grid)  SetGridVertex(MODE_PAINT);
    if (flag_display.origin)SetOriginVertex(MODE_PAINT);
    if (flag_display.pose)  SetPoseVertex(MODE_PAINT);
    if (flag_display.path)  SetPathVertex(MODE_PAINT);
    if (flag_display.prior) SetPriorVertex(MODE_PAINT);
    glUseProgram(0);
    glFlush();
}

//******************** Mouse Events *******************
//
//*****************************************************
void OpenglDisplay::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "pressed.....!!!!";
    if(event->button() == Qt::LeftButton) {
        //设置光标形状
        cursor.setShape(Qt::PointingHandCursor);
        setCursor(cursor);
        if(IsPickingState) {
            mouseX = event->x();
            mouseY = event->y();
        }
    }
    else if(event->button() == Qt::RightButton) {
        //设置光标形状
        cursor.setShape(Qt::SizeAllCursor);
        setCursor(cursor);
    }
    //记录点击光标时的位置
    lastX = event->x();
    lastY = event->y();
    update();
}

void OpenglDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "Release.....!!!!";

    //设置光标形状
    cursor.setShape(Qt::ArrowCursor);
    setCursor(cursor);
    if(event->button() == Qt::LeftButton) {
        //获取在拾取模式下的光标在屏幕上的像素坐标
        if(IsPickingState && !is_rotate) {
            mouseX = event->x();
            mouseY = event->y();
            ClickedFlag = true;
            SetPickedPointQueue();
        }
    }
    else if(event->button() == Qt::RightButton) {

    }
    //qDebug() << lastX << "," << lastY << ";";
    //qDebug() << viewportWidth << "," << viewportHeight << ";";
    is_rotate = false;
    update();
}

void OpenglDisplay::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "Moving.....!!!!";
    //鼠标左键用来实现对物体的旋转功能
    if(event->buttons() == Qt::LeftButton)
    {
        is_rotate = true;
        //设置光标形状
        cursor.setShape(Qt::ClosedHandCursor);
        setCursor(cursor);
        //计算yaw,pitch值的改变量
        GLfloat xoffset = event->x() - lastX;
        GLfloat yoffset = event->y() - lastY;
        lastX = event->x();
        lastY = event->y();

        GLfloat sensitivity = 0.1f;     //旋转时的灵敏度
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw   += xoffset;
        pitch += yoffset;
#ifdef DEBUG
        qDebug() << "yaw:[" << yaw << "] pitch:[" << pitch << "]";
#endif//DEBUG
#ifdef ViewMatrixMode
        //可以用来设置俯仰角的上下界
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        glm::vec3 up;
        up.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        up.y = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        up.z = - sin(glm::radians(pitch));
        cameraUp = glm::normalize(up);
        glm::vec3 front;
        front.x = - sin(glm::radians(pitch)) * sin(glm::radians(yaw));
        front.y = - sin(glm::radians(pitch)) * cos(glm::radians(yaw));
        front.z = - cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
        cameraPos = worldCentrol - radius * cameraFront;
#endif
    }
    //鼠标右键用来实现对移动物体（即局部坐标在世界坐标中的移动）
    else if(event->buttons() == Qt::RightButton)
    {
        //计算x,y方向的偏移量
        GLfloat xoffset = event->x() - lastX;
        GLfloat yoffset = event->y() - lastY;
        lastX = event->x();
        lastY = event->y();

        GLfloat sensitivity = 0.05f;    //移动时的灵敏度
        xoffset *= sensitivity;
        yoffset *= sensitivity;
#ifdef DEBUG
        qDebug() << xoffset << "-" << yoffset;
#endif//DEBUG
        if (!flag_display.trail || !flag_display.pose) {
#ifdef ModelMatrixMode
            transVec += glm::vec3(xoffset, -yoffset, 0.0f);
#endif
#ifdef ViewMatrixMode
            glm::vec3 vec_offset = glm::vec3(cos(glm::radians(yaw)) * (-xoffset) + sin(glm::radians(yaw)) * yoffset,
                                            - sin(glm::radians(yaw)) * (-xoffset) + cos(glm::radians(yaw)) * yoffset,
                                            0.0f);
            cameraPos += vec_offset;
            worldCentrol += vec_offset;
#endif
        }
    }
    update();
}

//滚轮实现对物体的放大缩小，摄像机距离远近（放大缩小）
void OpenglDisplay::wheelEvent(QWheelEvent *event)
{
    GLfloat sensitivity = 0.0005f;
#ifdef ModelMatrixMode
    cameraPos *= (1.0f - event->delta() * sensitivity);
#endif
#ifdef ViewMatrixMode
    radius *= (1.0f - event->delta() * sensitivity);
    cameraPos = worldCentrol - radius * cameraFront;
#endif
    update();
}

//********************* 网格尺寸变化 ********************
//
//*****************************************************
void OpenglDisplay::GridSizeChange() {
    QToolButton *btn_tool = qobject_cast<QToolButton *>(sender());
    if (btn_tool == btn_enlarge) {
        if (m_rangeLength < 2000) {
            m_rangeLength += 100;
        }
    }
    if (btn_tool == btn_shrink) {
        if (m_rangeLength > 100) {
            m_rangeLength -= 100;
        }
    }
    if (btn_tool == btn_increase) {
        if (m_cellLength < 50) {
            if (m_cellLength == 25) m_cellLength = 50;
            else if (m_cellLength == 20) m_cellLength = 25;
            else if (m_cellLength == 10) m_cellLength = 20;
            else if (m_cellLength == 5) m_cellLength = 10;
        }
    }
    if (btn_tool == btn_reduce) {
        if (m_cellLength > 5) {
            if (m_cellLength == 50) m_cellLength = 25;
            else if (m_cellLength == 25) m_cellLength = 20;
            else if (m_cellLength == 20) m_cellLength = 10;
            else if (m_cellLength == 10) m_cellLength = 5;
        }
    }
    SetGridVertex(MODE_SET_ATTRIB);
    update();
}

//********************* 网格尺寸变化 ********************
//
//*****************************************************
void OpenglDisplay::ShowStatus() {
    if (speedplate->isVisible()) speedplate->hide();
    else speedplate->show();
}

//********************* 恢复初始位置 ********************
//
//*****************************************************
void OpenglDisplay::RecoverConversion() {
    qDebug() << "recover";
    cameraPos    = glm::vec3(0.0f, 0.0f, 9.0f);
    worldCentrol = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraUp     = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraFront  = glm::vec3(0.0f, 0.0f, -1.0f);
    transVec     = glm::vec3(0.0f, 0.0f, 0.0f);
    yaw     = 0.0f;
    pitch   = 0.0f;
    radius  = 9.0f;
    update();
}

//********************* 初始化工具栏 ********************
//
//*****************************************************
void OpenglDisplay::InitToolModule() {
    btn_enlarge = new QToolButton(this);
    btn_shrink = new QToolButton(this);
    btn_increase = new QToolButton(this);
    btn_reduce = new QToolButton(this);
    btn_recover = new QToolButton(this);
    btn_enlarge->setText(tr("+"));
    btn_shrink->setText(tr("-"));
    btn_increase->setText(tr("<"));
    btn_reduce->setText(tr(">"));
    btn_recover->setText(tr("R"));
    connect(btn_enlarge, &QToolButton::clicked, this, &OpenglDisplay::GridSizeChange);
    connect(btn_shrink, &QToolButton::clicked, this, &OpenglDisplay::GridSizeChange);
    connect(btn_increase, &QToolButton::clicked, this, &OpenglDisplay::GridSizeChange);
    connect(btn_reduce, &QToolButton::clicked, this, &OpenglDisplay::GridSizeChange);
    connect(btn_recover, &QToolButton::clicked, this, &OpenglDisplay::RecoverConversion);
    const char button_stylesheet[] = "QToolButton{ background: #222222;  padding: 5px; border-radius: 10px; color: white; }"
                                     "QToolButton:hover{ background: #444444; }"
                                     "QToolButton:pressed{ background: #151515; }";
    btn_enlarge->setStyleSheet(button_stylesheet);
    btn_shrink->setStyleSheet(button_stylesheet);
    btn_increase->setStyleSheet(button_stylesheet);
    btn_reduce->setStyleSheet(button_stylesheet);
    btn_recover->setStyleSheet(button_stylesheet);

    speedplate = new SpeedPlate(this);
    btn_showstatus = new QToolButton(this);
    btn_showstatus->setText(tr("OB"));
    connect(btn_showstatus, &QToolButton::clicked, this, &OpenglDisplay::ShowStatus);
    btn_showstatus->setStyleSheet(button_stylesheet);

    layout_tool = new QHBoxLayout;
    layout_tool->addStretch();
    layout_tool->addWidget(btn_enlarge);
    layout_tool->addWidget(btn_shrink);
    layout_tool->addWidget(btn_increase);
    layout_tool->addWidget(btn_reduce);
    layout_tool->addWidget(btn_recover);
    layout_tool->setMargin(0);
    layout_status = new QHBoxLayout;
    layout_status->addStretch();
    layout_status->addWidget(speedplate);
    layout_status->addWidget(btn_showstatus, 0, Qt::AlignBottom);
    layout_status->setMargin(0);
    layout_main = new QGridLayout(this);
    layout_main->addLayout(layout_tool, 0, 0, 1, 1);
    layout_main->setRowStretch(1, 1);
    layout_main->addLayout(layout_status, 2, 0, 1, 1);
    layout_main->setMargin(5);

    if (!is_online) {
        speedplate->hide();
        btn_showstatus->hide();
    }
}

//********************** 初始化参数 *********************
//
//*****************************************************
void OpenglDisplay::InitParameters() {
    //**********************************************************窗口相关参数
    //设置OpenGL视口尺寸
    viewportWidth = this->width();
    viewportHeight = this->height();
    //设置光标形状
    cursor.setShape(Qt::ArrowCursor);
    setCursor(cursor);
    //设置点云尺寸
    PointSize = 1.0f;
    //设置点云高度值的上下界
    m_maxZ = 10.0;
    m_minZ = -6.0;
    //地面网格参数设置
    m_rangeLength = 300;
    m_cellLength = 10;
    //**********************************************************显示标志位初始设置
    is_rotate = false;
    if (is_online) {
        flag_display.cloud = false;
        flag_display.grid = true;
        flag_display.origin = true;
        flag_display.pose = false;
        flag_display.prior = true;
        flag_display.trail = false;

        flag_display.path = false;
        flag_display.picked = false;

        flag_display.mode = true;
        flag_display.style = false;
    }
    else {
        flag_display.grid = true;
        flag_display.origin = true;
        flag_display.path = true;
        flag_display.picked = true;
        flag_display.prior = true;
        flag_display.trail = false;

        flag_display.cloud = false;
        flag_display.pose = false;

        flag_display.mode = true;
        flag_display.style = true;
    }
    //**********************************************************数据缓冲初始化
    p_global = PointCloudData::GetInstance();
    //点的数量的4倍（三维点）
    vertices_cloud = NULL;
    vertices_picked = NULL;
    vertices_picked_index = NULL;
    vertices_prior = NULL;
    PointNum = 0;
    PriorNum = 0;
    PickedNum = 0;
    PathNum = 0;

    capacity_que = 0;
    //**********************************************************鼠标操作的一些设置
    //相机位置及朝向，用来构造ViewMatrix，进行“世界空间”到“观察空间”的转换
    cameraPos    = glm::vec3(0.0f, 0.0f, 9.0f);   //相机位置
    worldCentrol = glm::vec3(0.0f, 0.0f, 0.0f);   //世界坐标原点，相机始终朝向这个方向
    cameraUp     = glm::vec3(0.0f, 1.0f, 0.0f);   //相机的顶部始终朝向y轴
    cameraFront  = glm::normalize(worldCentrol - cameraPos);
    //构建ModelMatrix，进行“局部空间”到“世界空间”的转换
    transVec     = glm::vec3(0.0f, 0.0f, 0.0f);   //局部坐标在世界坐标中的平移量
    yaw   = 0.0f;   //偏航角
    pitch = 0.0f;   //俯仰角
    lastX = 0;      //光标上次x值
    lastY = 0;      //光标上次y值
    radius = glm::length(cameraPos);
    //**********************************************************拾取操作相关变量
    selectedPos  = glm::vec3(0.0f, 0.0f, 0.0f);   //光标选中的点
    mouseX = -1;                    //拾取时光标的像素坐标x值
    mouseY = -1;                    //拾取时光标的像素坐标y值
    lastXY = mouseX + mouseY;       //记录上一次光标坐标之和，用于点云拾取
    IsPickingState = false;         //拾取状态标志位
    ClickedFlag = false;            //光标点击标志
    radiusPixels = 5;               //点云拾取半径
    //**********************************************************着色器ID索引
    IDVertexShader = new GLuint[NumVertexShader];
    IDFragmentShader = new GLuint [NumFragmentShader];
    IDShaderProgram = new GLuint [NumShaderProgram];
    IDVAO = new GLuint [NumVAO];
    IDVBO = new GLuint [NumVBO];
    IDEBO = new GLuint [NumEBO];
}

//****************** 输出OpenGL版本信息 *****************
//
//*****************************************************
void OpenglDisplay::printContextInformation()
{
    QString glType;
    QString glVersion;
    QString glProfile;
    // Get Version Information
    glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    // Get Profile Information
    #define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
    switch (format().profile())
    {
        CASE(NoProfile);
        CASE(CoreProfile);
        CASE(CompatibilityProfile);
        }
    #undef CASE
    // qPrintable() will print our QString w/o quotes around it.
    qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}

//******************** 创建顶点着色器 *******************
//      参数1-着色器索引号
//*****************************************************
void OpenglDisplay::CreateVertexShader(GLuint index) {
    IDVertexShader[index] = glCreateShader(GL_VERTEX_SHADER);
    switch (index)
    {
        case 0: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource, nullptr); break;
        case 1: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource1, nullptr); break;
        case 2: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource2, nullptr); break;
        case 3: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource3, nullptr); break;
        case 4: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource4, nullptr); break;
        case 5: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource5, nullptr); break;
        case 6: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource6, nullptr); break;
        case 7: glShaderSource(IDVertexShader[index], 1, &vertexShaderSource7, nullptr); break;
        default: break;
    }
    glCompileShader(IDVertexShader[index]); //编译着色器
    //检查编译是否出错
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(IDVertexShader[index], GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(IDVertexShader[index], 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

//******************** 创建片断着色器 *******************
//      参数1-着色器索引号
//*****************************************************
void OpenglDisplay::CreateFragmentShader(GLuint index) {
    IDFragmentShader[index] = glCreateShader(GL_FRAGMENT_SHADER);
    switch (index)
    {
        case 0: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource, nullptr); break;
        case 1: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource1, nullptr); break;
        case 2: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource2, nullptr); break;
        case 3: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource3, nullptr); break;
        case 4: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource4, nullptr); break;
        case 5: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource5, nullptr); break;
        case 6: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource6, nullptr); break;
        case 7: glShaderSource(IDFragmentShader[index], 1, &fragmentShaderSource7, nullptr); break;
        default: break;
    }
    glCompileShader(IDFragmentShader[index]);   //编译着色器
    //检查编译是否出错
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(IDFragmentShader[index], GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(IDFragmentShader[index], 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

//******************** 链接着色器对象 *******************
//      参数1-着色器索引号
//*****************************************************
void OpenglDisplay::CreateShaderProgram(GLuint index) {
    IDShaderProgram[index] = glCreateProgram();
    glAttachShader(IDShaderProgram[index], IDVertexShader[index]);
    glAttachShader(IDShaderProgram[index], IDFragmentShader[index]);
    glLinkProgram(IDShaderProgram[index]);
    //检查链接错误
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(IDShaderProgram[index], GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(IDShaderProgram[index], 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    //删除着色器对象
    glDeleteShader(IDVertexShader[index]);
    glDeleteShader(IDFragmentShader[index]);
}

//************** 设置点云顶点属性（VAO/VBO） *************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetCloudVertex(GLboolean mode) {
    if (mode) {
        glBindVertexArray(IDVAO[MODE_POINTCLOUD]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_POINTCLOUD]);
        glBufferData(GL_ARRAY_BUFFER, PointNum * sizeof(GLfloat), vertices_cloud, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);       //结束记录状态信息
    }
    else {
        //绘制点云
        glUseProgram(IDShaderProgram[0]);
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 model;
        GLint modelLoc = glGetUniformLocation(IDShaderProgram[0], "model");
        GLint viewLoc = glGetUniformLocation(IDShaderProgram[0], "view");
        GLint projLoc = glGetUniformLocation(IDShaderProgram[0], "projection");
        view = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        model = glm::translate(model, transVec);
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(IDVAO[0]);
        glPointSize(0.1f);
        glDrawArrays(GL_POINTS, 0, (GLuint)(PointNum / 6));
        glPointSize(PointSize);
        glBindVertexArray(0);
    }
}

//************* 设置拾取点顶点属性（VAO/VBO） ************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetPickedVertex(GLboolean mode) {
    if (mode) {
        PickedNum = (GLuint)(selected_point.size() * 3);
        if (PickedNum == 0) {
            vertices_picked = NULL;
            vertices_picked_index = NULL;
        }
        else {
            vertices_picked = new GLfloat[PickedNum * 105 * 2 / 3];
            vertices_picked_index = new GLuint[PickedNum * 120 / 3];
        }
        qDebug() << "Picked point queue:   Queue size: " << PickedNum / 3;
        GLfloat *temp_picked = vertices_picked;
        GLuint *temp_index = vertices_picked_index;
        for (int i = 0; i < selected_point.size(); i++) {
            CreatePoseMark(i, selected_point.at(i).x, selected_point.at(i).y, selected_point.at(i).z, temp_picked, temp_index);
            temp_picked += 210;
            temp_index += 120;
        }
        glBindVertexArray(IDVAO[MODE_POINTPICKED]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_POINTPICKED]);
        glBufferData(GL_ARRAY_BUFFER, PickedNum / 3 * 105 * 2 * sizeof(GLfloat), vertices_picked, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IDEBO[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, PickedNum / 3 * 120 * sizeof(GLuint), vertices_picked_index, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);           //结束记录状态信息
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (vertices_picked) delete []vertices_picked;
        if (vertices_picked_index) delete []vertices_picked_index;
    }
    else {
        //绘制拾取点
        glUseProgram(IDShaderProgram[1]);
        glm::mat4 modelPicking;        //模型矩阵
        glm::mat4 viewPicking;         //观察矩阵
        glm::mat4 projectionPicking;   //投影矩阵
        GLint modelPickingLoc = glGetUniformLocation(IDShaderProgram[1], "modelPicking");
        GLint viewPickingLoc = glGetUniformLocation(IDShaderProgram[1], "viewPicking");
        GLint projPickingLoc = glGetUniformLocation(IDShaderProgram[1], "projectionPicking");
        viewPicking = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionPicking = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelPicking = glm::translate(modelPicking, transVec);
        modelPicking = glm::rotate(modelPicking, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPicking = glm::rotate(modelPicking, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelPickingLoc, 1, GL_FALSE, glm::value_ptr(modelPicking));
        glUniformMatrix4fv(viewPickingLoc, 1, GL_FALSE, glm::value_ptr(viewPicking));
        glUniformMatrix4fv(projPickingLoc, 1, GL_FALSE, glm::value_ptr(projectionPicking));
        glBindVertexArray(IDVAO[1]);
        glPointSize(1.0);
        glDrawElements(GL_TRIANGLES, 120 * PickedNum / 3, GL_UNSIGNED_INT, 0);
        glPointSize(PointSize);
        glBindVertexArray(0);
    }
}

//************** 设置坐标顶点属性（VAO/VBO） *************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetCoordsVertex(GLboolean mode) {
    if (mode) {
        //最标定点
        GLfloat coorVertices[] =
        {
            0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            0.05f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,  0.05f, 0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            0.0f,  0.0f,  0.05f, 0.0f, 0.0f, 1.0f
        };

        glBindVertexArray(IDVAO[MODE_COORDS]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_COORDS]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(coorVertices), coorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);           //结束记录状态信息
    }
    else {
        //绘制坐标
        glUseProgram(IDShaderProgram[2]);
        glm::mat4 viewCoor;
        glm::mat4 projectionCoor;
        glm::mat4 modelCoor;
        GLint modelCoorLoc = glGetUniformLocation(IDShaderProgram[2], "modelCoor");
        GLint viewCoorLoc = glGetUniformLocation(IDShaderProgram[2], "viewCoor");
        GLint projCoorLoc = glGetUniformLocation(IDShaderProgram[2], "projectionCoor");
#ifdef ModelMatrixMode
        viewCoor = glm::lookAt(cameraPos, worldCentrol, cameraUp);;
        projectionCoor = glm::perspective(glm::radians(45.0f), 5.0f / 3.0f, 0.1f, 100000.0f);
        //modelCoor = glm::translate(modelCoor, glm::vec3(-0.38f, -0.28f, glm::length(cameraPos) - 0.8f));
        modelCoor = glm::translate(modelCoor, glm::vec3(-0.48f, -0.28f, glm::length(cameraPos) - 0.8f));
#endif
#ifdef ViewMatrixMode
        viewCoor = glm::lookAt(glm::vec3(0.0f, 0.0f, 9.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));;
        projectionCoor = glm::perspective(glm::radians(45.0f), 5.0f / 3.0f, 0.1f, 100000.0f);
        modelCoor = glm::translate(modelCoor, glm::vec3(-0.48f, -0.28f, 9.0f - 0.8f));
#endif
        modelCoor = glm::rotate(modelCoor, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelCoor = glm::rotate(modelCoor, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(viewCoorLoc, 1, GL_FALSE, glm::value_ptr(viewCoor));
        glUniformMatrix4fv(projCoorLoc, 1, GL_FALSE, glm::value_ptr(projectionCoor));
        glUniformMatrix4fv(modelCoorLoc, 1, GL_FALSE, glm::value_ptr(modelCoor));
        glBindVertexArray(IDVAO[2]);
        glDrawArrays(GL_LINES, 0, 2);
        glDrawArrays(GL_LINES, 2, 2);
        glDrawArrays(GL_LINES, 4, 2);
        glBindVertexArray(0);
    }
}

//************** 设置网格顶点属性（VAO/VBO） *************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetGridVertex(GLboolean mode) {
    int gridline = m_rangeLength / m_cellLength + 1;
    if (mode) {
        GLfloat *gridVertices = new GLfloat[gridline * 12];
        GLfloat temp = m_rangeLength / 2;
        for(int i = 0 ; i < gridline * 6 ; i += 6)
        {
            gridVertices[i] = m_rangeLength / 2;
            gridVertices[i+3] = -m_rangeLength / 2;
            gridVertices[i+1] = temp;
            gridVertices[i+4] = temp;
            temp -= m_cellLength;
            gridVertices[i+2] = 0;
            gridVertices[i+5] = 0;
        }
        temp = m_rangeLength / 2;
        for(int i = gridline * 6 ; i < gridline * 12 ; i += 6)
        {
            gridVertices[i] = temp;
            gridVertices[i+3] = temp;
            temp -= m_cellLength;
            gridVertices[i+1] = m_rangeLength / 2;
            gridVertices[i+4] = -m_rangeLength / 2;
            gridVertices[i+2] = 0;
            gridVertices[i+5] = 0;
        }
        glBindVertexArray(IDVAO[MODE_GRID]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_GRID]);
        glBufferData(GL_ARRAY_BUFFER, gridline * 12 * 4, gridVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);           //结束记录状态信息
        delete[]gridVertices;
    }
    else {
        //绘制点云
        int i = 0;
        glUseProgram(IDShaderProgram[3]);
        glm::mat4 viewGrid;
        glm::mat4 projectionGrid;
        glm::mat4 modelGrid;
        GLint modelGridLoc = glGetUniformLocation(IDShaderProgram[3], "modelGrid");
        GLint viewGridLoc = glGetUniformLocation(IDShaderProgram[3], "viewGrid");
        GLint projGridLoc = glGetUniformLocation(IDShaderProgram[3], "projectionGrid");
        viewGrid = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionGrid = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelGrid = glm::translate(modelGrid, transVec);
        modelGrid = glm::rotate(modelGrid, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelGrid = glm::rotate(modelGrid, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelGridLoc, 1, GL_FALSE, glm::value_ptr(modelGrid));
        glUniformMatrix4fv(viewGridLoc, 1, GL_FALSE, glm::value_ptr(viewGrid));
        glUniformMatrix4fv(projGridLoc, 1, GL_FALSE, glm::value_ptr(projectionGrid));
        glBindVertexArray(IDVAO[3]);
        for (i = 0; i < gridline * 2; i++) {
            glDrawArrays(GL_LINES, i * 2, 2);
        }
        glBindVertexArray(0);
    }
}

//************ 设置坐标原点顶点属性（VAO/VBO） ***********
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetOriginVertex(GLboolean mode) {
    if (mode) {
        GLfloat vertices_origin[3] = {0.0f, 0.0f, 0.0f};
        glBindVertexArray(IDVAO[MODE_ORIGIN]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_ORIGIN]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_origin), vertices_origin, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);           //结束记录状态信息
    }
    else {
        //绘制原点
        glUseProgram(IDShaderProgram[4]);
        glm::mat4 viewOrigin;
        glm::mat4 projectionOrigin;
        glm::mat4 modelOrigin;
        GLint modelOriginLoc = glGetUniformLocation(IDShaderProgram[4], "modelOrigin");
        GLint viewOriginLoc = glGetUniformLocation(IDShaderProgram[4], "viewOrigin");
        GLint projOriginLoc = glGetUniformLocation(IDShaderProgram[4], "projectionOrigin");
        viewOrigin = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionOrigin = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelOrigin = glm::translate(modelOrigin, transVec);
        modelOrigin = glm::rotate(modelOrigin, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelOrigin = glm::rotate(modelOrigin, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelOriginLoc, 1, GL_FALSE, glm::value_ptr(modelOrigin));
        glUniformMatrix4fv(viewOriginLoc, 1, GL_FALSE, glm::value_ptr(viewOrigin));
        glUniformMatrix4fv(projOriginLoc, 1, GL_FALSE, glm::value_ptr(projectionOrigin));
        glBindVertexArray(IDVAO[4]);
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, 1);
        glPointSize(PointSize);
        glBindVertexArray(0);
    }
}

//************** 设置位置顶点属性（VAO/VBO） *************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetPoseVertex(GLboolean mode) {
    if (mode) {
        GLfloat vertices_pose[3] = {robot_pose.y, robot_pose.z, robot_pose.x};
        //位置标记数据
        float x = vertices_pose[0];
        float y = vertices_pose[1];
        float z = vertices_pose[2];
        float yaw = robot_pose.roll;
        float r = 0.1f;             //整体放缩比例
        float l = 5.0 * r;          //单位长度
        GLfloat posMark[] =
        {
            x+Xcovert(l, l, yaw),       y+Ycovert(l, l, yaw),       z,      0.5f, 0.0f, 0.5f,
            x+Xcovert(l, (-l), yaw),    y+Ycovert(l, (-l), yaw),    z,      0.5f, 0.0f, 0.5f,
            x+Xcovert((-l), l, yaw),    y+Ycovert((-l), l, yaw),    z,      0.0f, 0.0f, 1.0f,
            x+Xcovert((-l), (-l), yaw), y+Ycovert((-l), (-l), yaw), z,      0.0f, 0.0f, 1.0f,
            x+Xcovert(2*l, 0.0, yaw),   y+Ycovert(2*l, 0.0, yaw),   z,      1.0f, 0.0f, 0.0f,        //4
            x+Xcovert(l, l, yaw),       y+Ycovert(l, l, yaw),       z+2*l,  0.5f, 0.0f, 0.5f,
            x+Xcovert(l, (-l), yaw),    y+Ycovert(l, (-l), yaw),    z+2*l,  0.5f, 0.0f, 0.5f,
            x+Xcovert((-l), l, yaw),    y+Ycovert((-l), l, yaw),    z+2*l,  0.0f, 0.0f, 1.0f,
            x+Xcovert((-l), (-l), yaw), y+Ycovert((-l), (-l), yaw), z+2*l,  0.0f, 0.0f, 1.0f,
            x+Xcovert(2*l, 0.0, yaw),   y+Ycovert(2*l, 0.0, yaw),   z+2*l,  1.0f, 0.0f, 0.0f,       //9
        };
        GLuint posMarkIndex[] =
        {
            0, 1, 2,
            2, 3, 1,
            0, 1, 4,    //bottom

            5, 6, 7,
            7, 8, 6,
            5, 6, 9,    //top

            0, 4, 9,
            9, 5, 0,
            1, 4, 9,
            9, 6, 1,
            1, 3, 8,
            8, 6, 1,
            2, 3, 8,
            8, 7, 2,
            2, 0, 5,
            5, 7, 2,    //side
        };
        glBindVertexArray(IDVAO[MODE_POSE]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_POSE]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(posMark), posMark, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IDEBO[0]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(posMarkIndex), posMarkIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);       //结束记录状态信息
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else {
        //绘制先验地图
        glUseProgram(IDShaderProgram[5]);
        glm::mat4 viewPose;
        glm::mat4 projectionPose;
        glm::mat4 modelPose;
        GLint modelPoseLoc = glGetUniformLocation(IDShaderProgram[5], "modelPose");
        GLint viewPoseLoc = glGetUniformLocation(IDShaderProgram[5], "viewPose");
        GLint projPoseLoc = glGetUniformLocation(IDShaderProgram[5], "projectionPose");
        viewPose = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionPose = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelPose = glm::translate(modelPose, transVec);
        modelPose = glm::rotate(modelPose, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPose = glm::rotate(modelPose, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelPoseLoc, 1, GL_FALSE, glm::value_ptr(modelPose));
        glUniformMatrix4fv(viewPoseLoc, 1, GL_FALSE, glm::value_ptr(viewPose));
        glUniformMatrix4fv(projPoseLoc, 1, GL_FALSE, glm::value_ptr(projectionPose));
        glBindVertexArray(IDVAO[5]);
        glPointSize(5.0f);
        //glDrawArrays(GL_POINTS, 0, 1);
        glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
        glPointSize(PointSize);
        glBindVertexArray(0);
    }
}

//************** 设置路径顶点属性（VAO/VBO） *************
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetPathVertex(GLboolean mode) {
    if (mode) {
        GLfloat *vertices_path = NULL;
        GLuint *vertices_path_index = NULL;
        PathNum = (int)selected_point.size() - 1;
        if (PathNum > 0) {
            vertices_path = new GLfloat[PathNum * 36];
            vertices_path_index = new GLuint[PathNum * 72];
            GLfloat *temp_path = vertices_path;
            GLuint *temp_path_index = vertices_path_index;
            for (int i = 0; i < PathNum; i++) {
                CreatePathMark(i, selected_point.at(i + 1), selected_point.at(i), temp_path, temp_path_index);
                temp_path += 36;
                temp_path_index += 72;
            }
        }
        else if (PathNum < 0) {
            PathNum = 0;
        }
        glBindVertexArray(IDVAO[MODE_PATH]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_PATH]);
        glBufferData(GL_ARRAY_BUFFER, PathNum * 36 * sizeof(GLfloat), vertices_path, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IDEBO[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, PathNum * 72 * sizeof(GLuint), vertices_path_index, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);           //结束记录状态信息
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        if (vertices_path)  delete[]vertices_path;
        if (vertices_path_index)    delete[]vertices_path_index;
    }
    else {
        glUseProgram(IDShaderProgram[6]);
        glm::mat4 viewPath;
        glm::mat4 projectionPath;
        glm::mat4 modelPath;
        GLint modelPathLoc = glGetUniformLocation(IDShaderProgram[6], "modelPath");
        GLint viewPathLoc = glGetUniformLocation(IDShaderProgram[6], "viewPath");
        GLint projPathLoc = glGetUniformLocation(IDShaderProgram[6], "projectionPath");
        viewPath = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionPath = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelPath = glm::translate(modelPath, transVec);
        modelPath = glm::rotate(modelPath, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPath = glm::rotate(modelPath, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelPathLoc, 1, GL_FALSE, glm::value_ptr(modelPath));
        glUniformMatrix4fv(viewPathLoc, 1, GL_FALSE, glm::value_ptr(viewPath));
        glUniformMatrix4fv(projPathLoc, 1, GL_FALSE, glm::value_ptr(projectionPath));
        glBindVertexArray(IDVAO[6]);
        glPointSize(1.0);
        glDrawElements(GL_TRIANGLES, (GLuint)PathNum * 72, GL_UNSIGNED_INT, 0);
        glPointSize(PointSize);
        glBindVertexArray(0);
    }

}

//************ 设置先验地图顶点属性（VAO/VBO） ***********
//      参数1-模式 1 设置顶点属性 0 绘制顶点
//*****************************************************
void OpenglDisplay::SetPriorVertex(GLboolean mode) {
    if (mode) {
        GLuint i = 0;
        if (PriorNum == 0) {
            vertices_prior = NULL;
        }
        else {
            float yanse;
            GLint index;   //用来存储在颜色表中的下标
            GLfloat gap;
            GLint PaletteSize = sizeof(PesudoPalette) / sizeof(float) / 3;
            glm::vec4 temp;
            vertices_prior = new GLfloat[PriorNum * 6];
            GLfloat maxZ, minZ;
            if (false) {
                maxZ = m_maxZ;
                minZ = m_minZ;
            }
            else {
                maxZ = 15.0f;
                minZ = -5.0f;
            }
            for (i = 0; i < PriorNum; i++) {
                temp = PointPriorQue.at(i);
                vertices_prior[i * 6] = temp.x;
                vertices_prior[i * 6 + 1] = temp.y;
                vertices_prior[i * 6 + 2] = temp.z;
                if (flag_display.style) {
                    gap = (maxZ - minZ)/PaletteSize;
                    index = (GLint)((temp.z - minZ) / gap);
                    if(index >= PaletteSize)
                        index = PaletteSize - 1;
                    else if(index < 0)
                        index = 0;
                    vertices_prior[i * 6 + 3] = PesudoPalette[index][0];
                    vertices_prior[i * 6 + 4] = PesudoPalette[index][1];
                    vertices_prior[i * 6 + 5] = PesudoPalette[index][2];
                }
                else {
                    yanse = (temp.z - minZ) / (maxZ - minZ);
                    if (yanse>1.0) yanse = 1.0;
                    else if (yanse < 0.0) yanse = 0.0;
                    vertices_prior[i * 6 + 3] = yanse;
                    vertices_prior[i * 6 + 4] = yanse;
                    vertices_prior[i * 6 + 5] = yanse;
                }
            }
        }
        qDebug() << "Prior count: " << PriorNum;

        glBindVertexArray(IDVAO[MODE_PRIOR]);    //开始记录状态信息
        glBindBuffer(GL_ARRAY_BUFFER, IDVBO[MODE_PRIOR]);
        glBufferData(GL_ARRAY_BUFFER, PriorNum * 6 * sizeof(GLfloat), vertices_prior, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);       //结束记录状态信息
    }
    else {
        //绘制先验地图
        glUseProgram(IDShaderProgram[7]);
        glm::mat4 viewPrior;
        glm::mat4 projectionPrior;
        glm::mat4 modelPrior;
        GLint modelPriorLoc = glGetUniformLocation(IDShaderProgram[7], "modelPrior");
        GLint viewPriorLoc = glGetUniformLocation(IDShaderProgram[7], "viewPrior");
        GLint projPriorLoc = glGetUniformLocation(IDShaderProgram[7], "projectionPrior");
        viewPrior = glm::lookAt(cameraPos, worldCentrol, cameraUp);
        projectionPrior = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
#ifdef ModelMatrixMode
        modelPrior = glm::translate(modelPrior, transVec);
        modelPrior = glm::rotate(modelPrior, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPrior = glm::rotate(modelPrior, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
        glUniformMatrix4fv(modelPriorLoc, 1, GL_FALSE, glm::value_ptr(modelPrior));
        glUniformMatrix4fv(viewPriorLoc, 1, GL_FALSE, glm::value_ptr(viewPrior));
        glUniformMatrix4fv(projPriorLoc, 1, GL_FALSE, glm::value_ptr(projectionPrior));
        glBindVertexArray(IDVAO[7]);
        glPointSize(1.0f);
        glDrawArrays(GL_POINTS, 0, (GLuint)PriorNum);
        glPointSize(PointSize);
        glBindVertexArray(0);
    }
}

//********************** 加载点云 **********************
//      参数1-点云文件路径 格式为xyzi
//*****************************************************
void OpenglDisplay::LoadPoints(const QString &fileName)
{
    QFileInfo finfo(fileName);
    qDebug() << "File size: " << finfo.size() << " byte";
    int filesize = finfo.size() / 1024 / 1024;
    int filestep = filesize / 200;
    int readcount = 0;
    std::ifstream ifile;
    ifile.open(fileName.toStdString(), std::ifstream::binary);
    glm::vec4 temp;
    //qDebug() << sizeof(temp) << endl;
    while (ifile.peek() != EOF)
    {
        ifile.read((char*)&temp, sizeof(temp));
        if (readcount < filestep) {
            readcount++;
            continue;
        }
        readcount = 0;
        PointPriorQue.push_back(temp);
        if (m_maxZ < temp.z) {
            m_maxZ = temp.z;
        }
        if (m_minZ > temp.z) {
            m_minZ = temp.z;
        }
    }
    ifile.close();
    qDebug() << PointPriorQue.size() << endl;
    PriorNum = (GLuint)PointPriorQue.size();
    SetPriorVertex(MODE_SET_ATTRIB);
    qDebug() << "Already";
    update();
}

//*********** 创建对应位置处用于标记的红蓝箭头顶点 **********
//      参数1-在位置队列中的索引
//      参数234-位置坐标
//      参数5-顶点缓冲地址
//      参数6-顶点索引缓冲地址
//******************************************************
void OpenglDisplay::CreatePoseMark(GLuint num, GLfloat x, GLfloat y, GLfloat z, GLfloat *vertices, GLuint *vertices_index) {
    //位置标记数据
    float r = 0.1f;             //整体放缩比例
    float l = 1.0 * r;          //单位长度
    float offsetZ = 5 * l;      //z方向偏移量(圆柱长度)
    const float pi = 3.1415926f;
    //data 105
    //size sizeof(GLfloat) * 105 * 2
    GLfloat posMark[] =
    {
        x+l*cos(0),      y+l*sin(0),      z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6),   y+l*sin(pi/6),   z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3),   y+l*sin(pi/3),   z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/2),   y+l*sin(pi/2),   z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3*2), y+l*sin(pi/3*2), z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6*5), y+l*sin(pi/6*5), z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi),     y+l*sin(pi),     z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6*5), y-l*sin(pi/6*5), z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3*2), y-l*sin(pi/3*2), z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/2),   y-l*sin(pi/2),   z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3),   y-l*sin(pi/3),   z, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6),   y-l*sin(pi/6),   z, 0.0f, 0.0f, 1.0f,
        x,               y,               z, 0.0f, 0.0f, 1.0f,            //13

        x+l*cos(0),      y+l*sin(0),      z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6),   y+l*sin(pi/6),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3),   y+l*sin(pi/3),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/2),   y+l*sin(pi/2),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3*2), y+l*sin(pi/3*2), z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6*5), y+l*sin(pi/6*5), z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi),     y+l*sin(pi),     z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6*5), y-l*sin(pi/6*5), z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3*2), y-l*sin(pi/3*2), z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/2),   y-l*sin(pi/2),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/3),   y-l*sin(pi/3),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x+l*cos(pi/6),   y-l*sin(pi/6),   z+offsetZ, 0.0f, 0.0f, 1.0f,
        x,               y,               z+offsetZ, 1.0f, 0.0f, 0.0f,     //26

        x+3*l*cos(0),       y+3*l*sin(0),       z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/4),    y+3*l*sin(pi/4),    z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/2),    y+3*l*sin(pi/2),    z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/4*3),  y+3*l*sin(pi/4*3),  z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi),      y+3*l*sin(pi),      z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/4*3),  y-3*l*sin(pi/4*3),  z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/2),    y-3*l*sin(pi/2),    z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x+3*l*cos(pi/4),    y-3*l*sin(pi/4),    z+3*l+offsetZ, 1.0f, 0.0f, 0.0f,
        x,                  y,                  z+6*l+offsetZ, 1.0f, 0.0f, 0.0f,    //35
    };
    //point 120
    //size sizeof(GLuint) * 120
    GLuint posMarkIndex[] =
    {
        0, 1, 13, 13, 14, 1,
        1, 2, 14, 14, 15, 2,
        2, 3, 15, 15, 16, 3,
        3, 4, 16, 16, 17, 4,
        4, 5, 17, 17, 18, 5,
        5, 6, 18, 18, 19, 6,
        6, 7, 19, 19, 20, 7,
        7, 8, 20, 20, 21, 8,
        8, 9, 21, 21, 22, 9,
        9, 10, 22, 22, 23, 10,
        10, 11, 23, 23, 24, 11,
        11, 0, 24, 24, 13, 0,

        26, 27, 25, 26, 27, 34,
        27, 28, 25, 27, 28, 34,
        28, 29, 25, 28, 29, 34,
        29, 30, 25, 29, 30, 34,
        30, 31, 25, 30, 31, 34,
        31, 32, 25, 31, 32, 34,
        32, 33, 25, 32, 33, 34,
        33, 26, 25, 33, 26, 34
    };
    for (int i = 0; i < 120; i++) {
        posMarkIndex[i] += num * 35;
    }
    memcpy((char*)vertices, (char*)posMark, sizeof(posMark));
    memcpy((char*)vertices_index, (char*)posMarkIndex, sizeof(posMarkIndex));
}

//*********** 创建对应位置处用于标记的红蓝箭头顶点 **********
//      参数1-在路径点队列中的索引
//      参数2-当前路径点
//      参数3-前一路径点
//      参数4-顶点缓冲地址
//      参数5-顶点索引缓冲地址
//******************************************************
void OpenglDisplay::CreatePathMark(GLuint num, glm::vec3 point_now, glm::vec3 point_last, GLfloat *vertices, GLuint *vertices_index) {
    //位置标记数据
    float r = 0.1f;             //整体放缩比例
    float l = 1.0 * r;          //单位长度
    //data 36
    //size sizeof(GLfloat) * 36
    GLfloat pathMark[] =
    {
        point_now.x + l,    point_now.y,        point_now.z,
        point_now.x - l,    point_now.y,        point_now.z,
        point_now.x,        point_now.y + l,    point_now.z,
        point_now.x,        point_now.y - l,    point_now.z,
        point_now.x,        point_now.y,        point_now.z + l,
        point_now.x,        point_now.y,        point_now.z - l,  //6

        point_last.x + l,   point_last.y,       point_last.z,
        point_last.x - l,   point_last.y,       point_last.z,
        point_last.x,       point_last.y + l,   point_last.z,
        point_last.x,       point_last.y - l,   point_last.z,
        point_last.x,       point_last.y,       point_last.z + l,
        point_last.x,       point_last.y,       point_last.z - l  //12
    };
    //point 72
    //size sizeof(GLuint) * 72
    GLuint pathMarkIndex[] =
    {
        0, 2, 8,  8, 6, 0,
        0, 3, 9,  9, 6, 0,
        0, 4, 10,10, 6, 0,
        0, 5, 11,11, 6, 0,

        1, 2, 8,  8, 7, 1,
        1, 3, 9,  9, 7, 1,
        1, 4, 10,10, 7, 1,
        1, 5, 11,11, 7, 1,

        2, 4, 10,10, 8, 2,
        2, 5, 11,11, 8, 2,

        3, 4, 10,10, 9, 3,
        3, 5, 11,11, 9, 3
    };
    for (int i = 0; i < 72; i++) {
        pathMarkIndex[i] += (int)num * 12;
    }
    memcpy((char*)vertices, (char*)pathMark, sizeof(pathMark));
    memcpy((char*)vertices_index, (char*)pathMarkIndex, sizeof(pathMarkIndex));
}

//******************* 点云拾取模式切换 ********************
//
//******************************************************
void OpenglDisplay::PointPicking() {
    if (IsPickingState)
        IsPickingState = false;
    else
        IsPickingState = true;
}

//**************** 路径节点存储（槽函数） *****************
//
//******************************************************
void OpenglDisplay::SavePointsPicked() {
    if(selected_point.size() == 0) {
        QMessageBox::question(this, tr("tips"), tr("There is no points picked!"));
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("打开文件"), "F:", tr("文本文件(*txt)"));
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "F:");
    if(fileName == "")
        return;
    if (QMessageBox::Yes == QMessageBox::question(this, tr("Question?"), tr("Do you affirm to save?"), QMessageBox::Yes, QMessageBox::No)) {
        char str_FID[128] = {0};
        //整理时间字符串，作为FID
        time_t t;
        tm* local;
        t = time(NULL);
        local = localtime(&t);
        strftime(str_FID, 64, "%Y_%m_%d_%H_%M_%S", local);
        //std::vector<PathNode> NavnodeSet;
        std::vector<PathNode> NavnodeSet(pre_pathpoint);
        for (int i =0; i < NavnodeSet.size(); ++i) {
            NavnodeSet[i].ID = i + 1;
        }
        /*
        for(int i = 0; i < selected_point.size(); i++)
        {
            PathNode tempnode;
            tempnode.ID = i+1;
            tempnode.type = 1;
            tempnode.x = selected_point.at(i).x;
            tempnode.y = selected_point.at(i).y;
            tempnode.z = selected_point.at(i).z;
            tempnode.yaw = 0.0;
            tempnode.pitch = 0.0;
            tempnode.roll = 0.0;
            tempnode.gpsx = 0.0;
            tempnode.gpsy = 0.0;
            tempnode.gpsz = 0.0;
            tempnode.gpsyaw = 0.0;
            tempnode.odomx= 0.0;
            tempnode.odomy = 0.0;
            tempnode.odomz = 0.0;
            tempnode.odomyaw = 0.0;
            tempnode.gps_status = 0;
            tempnode.gps_num = 0;
            tempnode.latFlag= 0;
            tempnode.lngFlag = 0;
            tempnode.latValue = 0.0;
            tempnode.lngValue= 0.0;
            tempnode.altValue = 0.0;
            tempnode.n1 = 0.0;
            tempnode.n2 = 0.0;
            tempnode.n3 = 0.0;
            NavnodeSet.push_back(tempnode);
        }
        */
        std::vector<PathEdge> MapEdgeSet;
        for(int i = 1; i < NavnodeSet.size(); i++)
        {
            int index1 = i;
            int index2 = i - 1;
            PathEdge tempEdge;
            tempEdge.nodeid1 = NavnodeSet[i].ID;
            tempEdge.nodeid2 = NavnodeSet[i].ID;
            double x =  NavnodeSet[index1].x - NavnodeSet[index2].x;
            double y =  NavnodeSet[index1].y - NavnodeSet[index2].y;
            tempEdge.dis = sqrt(x*x + y*y);
            double odomx =  NavnodeSet[index1].odomx - NavnodeSet[index2].odomx;
            double odomy =  NavnodeSet[index1].odomy - NavnodeSet[index2].odomy;
            tempEdge.odomdis = sqrt(odomx*odomx + odomy*odomy);
            tempEdge.flag = 2;
            MapEdgeSet.push_back(tempEdge);
        }
        // *************** 写文件 **************
        std::ofstream out;
        out.open(fileName.toStdString());
        //文件头
        out << "FID " << str_FID << std::endl;
        out << "NN " << NavnodeSet.size() << std::endl;
        out << "EN " << MapEdgeSet.size() << std::endl;
        for(int i = 0; i < NavnodeSet.size(); i++)
        {
            int indexx = i;
            out << "N " << NavnodeSet[indexx].ID <<" " << NavnodeSet[indexx].type << " "
                << NavnodeSet[indexx].x<< " " <<NavnodeSet[indexx].y << " " << NavnodeSet[indexx].z << " "
                << NavnodeSet[indexx].yaw << " " << NavnodeSet[indexx].pitch << " " << NavnodeSet[indexx].roll << " "
                << NavnodeSet[indexx].gpsx << " " << NavnodeSet[indexx].gpsy << " " << NavnodeSet[indexx].gpsz << " "
                << NavnodeSet[indexx].gpsyaw << " " << NavnodeSet[indexx].odomx << " " << NavnodeSet[indexx].odomy << " "
                << NavnodeSet[indexx].odomz << " " << NavnodeSet[indexx].odomyaw << " " << NavnodeSet[indexx].gps_status << " "
                << NavnodeSet[indexx].gps_num << " " << NavnodeSet[indexx].latFlag << " " << NavnodeSet[indexx].lngFlag << " "
                << NavnodeSet[indexx].latValue << " " << NavnodeSet[indexx].lngValue << " " << NavnodeSet[indexx].altValue
                << std::endl;
        }
        //边
        for(int i=0; i<MapEdgeSet.size(); i++)
        {
            out << "E " << MapEdgeSet[i].nodeid1 << " " << MapEdgeSet[i].nodeid2 << " "
                << MapEdgeSet[i].dis << " " << MapEdgeSet[i].odomdis << " " << MapEdgeSet[i].flag << std::endl;
        }
        //global-path
        out << "GLOBAL-PATH";
        for(int i = 0; i < NavnodeSet.size(); i++)
        {
            int indexx = i;
            out << " " << NavnodeSet[indexx].ID;
        }
        out << std::endl;
        out.close();
    }
}

//*************** 路径节点撤销操作（槽函数） ***************
//
//******************************************************
void OpenglDisplay::RepealPointsPicked() {
    if (!selected_point.empty()) {

        pre_pathpoint.pop_back();

        selected_point.pop_back();
        p_global->PathNode.pop_back();
        SetPickedVertex(MODE_SET_ATTRIB);
        SetPathVertex(MODE_SET_ATTRIB);
        update();
    }
}

//****************** 路径读取（槽函数） ******************
//
//******************************************************
void OpenglDisplay::LoadPath() {
    if (QMessageBox::Yes == QMessageBox::question(this, tr("Question?"), tr("Do you affirm to clear path points?"), QMessageBox::Yes, QMessageBox::No)) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "F:");
        std::ifstream in;
        PathNode tempnode;
        glm::vec4 temppoint;
        char type;
        in.open(fileName.toStdString());
        if (in) {
            while (!in.eof()) {
                in >> type >> tempnode.ID >> tempnode.type
                    >> tempnode.x >>tempnode.y >> tempnode.z
                    >> tempnode.yaw >> tempnode.pitch >> tempnode.roll
                    >> tempnode.gpsx >> tempnode.gpsy >> tempnode.gpsz
                    >> tempnode.gpsyaw >> tempnode.odomx >> tempnode.odomy
                    >> tempnode.odomz >> tempnode.odomyaw >> tempnode.gps_status
                    >> tempnode.gps_num >> tempnode.latFlag >> tempnode.lngFlag
                    >> tempnode.latValue >> tempnode.lngValue >> tempnode.altValue;
                temppoint.x = tempnode.x;
                temppoint.y = tempnode.y;
                temppoint.z = tempnode.z;
                temppoint.w = 0.0;
                PointPriorQue.push_back(temppoint);
                path_point.push_back(tempnode);
            }
        }
        in.close();
        PriorNum = (GLuint)PointPriorQue.size();
        SetPriorVertex(MODE_SET_ATTRIB);
        update();
    }
}

//*************** 当前点选路径清除（槽函数） ***************
//
//******************************************************
void OpenglDisplay::ClearPath() {

    pre_pathpoint.clear();

    selected_point.clear();
    SetPickedVertex(MODE_SET_ATTRIB);
    SetPathVertex(MODE_SET_ATTRIB);
    update();
}

//*************** 加载vins路径点（槽函数） ***************
//                  临时处理方案
//******************************************************
void OpenglDisplay::LoadGraphPath() {
    LoadPath();
}

//****************** 点云拾取并筛选缓存 ******************
//
//******************************************************
void OpenglDisplay::SetPickedPointQueue() {
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;
    view = glm::lookAt(cameraPos, worldCentrol, cameraUp);
    projection = glm::perspective(glm::radians(45.0f), (float)viewportWidth / viewportHeight, 0.1f, 100000.0f);
    model = glm::translate(model, transVec);
#ifdef ModelMatrixMode
    model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
    if(IsPickingState)      //点云拾取相关
    {
        /*************************在世界坐标系中进行拾取判定*************************/
        if (ClickedFlag) {
            glm::vec3 directVec;        //摄像机到点击处的方向向量
            RayCastingClickedPoint(mouseX, mouseY, viewportWidth, viewportHeight, projection, view, directVec);
            int AdPointX, AdPointY;     //辅助点用来获得点击半径
            GetAddedPoint(mouseX, mouseY, viewportWidth, viewportHeight, AdPointX, AdPointY);
            glm::vec3 directVecAd;      //摄像机到拾取半径上一点（辅助点）的方向向量
            RayCastingClickedPoint(AdPointX, AdPointY, viewportWidth, viewportHeight, projection, view, directVecAd);
            float pickingRadius;        //拾取半径
            GetPickingRadius(cameraPos, directVec, directVecAd, pickingRadius);
            float currentRadius;                                        //当前测试点到射线的距离
            glm::vec3 selectedPoint = glm::vec3(0.0f, 0.0f, 9999.0f);   //光标选中的点的坐标
            int selectedID = -1;                                        //光标选中的点在数组中的下标
            float selectedRadius = 10000.0f;                            //光标选中的点到射线的距离

            std::vector<int> preID;
            int ID;

            for(unsigned int i = 0 ; i < PriorNum ; i++) {
                if (IsPointInCircle(model, cameraPos, directVec, pickingRadius, currentRadius, vertices_prior[i*6], vertices_prior[i*6+1], vertices_prior[i*6+2]))
                {
                    if (currentRadius < selectedRadius) {
                        selectedRadius = currentRadius;
                        selectedID = i;
                        selectedPoint = glm::vec3(vertices_prior[i*6], vertices_prior[i*6+1], vertices_prior[i*6+2]);
                        if (selectedRadius < 1.0f) {    // 原值为0.1f
                            filt_point.push_back(selectedPoint);
                            preID.push_back(i);
                            ID = i;
                        }
                    }
                }
            }
            if (filt_point.size() != 0) {
                for (unsigned int j = 0; j < filt_point.size(); j++) {
                    if (selectedPoint.z < filt_point.at(j).z) {
                        selectedPoint = filt_point.at(j);

                        ID = preID[j];

                    }
                }
            }
            filt_point.clear();

            preID.clear();

            if (selectedPoint != glm::vec3(0.0f, 0.0f, 9999.0f)) {
                selectedPos = selectedPoint;
                selected_point.push_back(selectedPoint);
                p_global->PathNode.push_back(selectedPoint);

                pre_pathpoint.push_back(path_point[ID]);

                SetPickedVertex(MODE_SET_ATTRIB);
                SetPathVertex(MODE_SET_ATTRIB);
                //qDebug() << "point: (" << selectedPos.x << ", " << selectedPos.y << ", " << selectedPos.z << ")";
            }
            else {
                qDebug() << "point invalid!" ;
            }
            if(lastXY != mouseX + mouseY) {
                ClickedFlag = false;
                lastXY = mouseX + mouseY;
            }
        }
        update();
    }
}

//*************** 求取投射射线的单位方向向量 ***************
//      mouseX mouseY-鼠标事件的二维坐标
//      viewportWidth viewportHeight-视口尺寸
//      projectionMat-当前投影矩阵
//      viewMat-当前观察矩阵
//      directVec-沿相机射线方向的方向向量
//******************************************************
void OpenglDisplay::RayCastingClickedPoint(int &mouseX,
                                    int &mouseY,
                                    int &viewportWidth,
                                    int &viewportHeight,
                                    glm::mat4 &projectionMat,
                                    glm::mat4 &viewMat,
                                    glm::vec3 &directVec)
{
    float x = (2.0f * mouseX) / viewportWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / viewportHeight;
    float z = 1.0f;
    glm::vec3 rayNDC = glm::vec3(x, y, z);      //将屏幕像素坐标转换成标准设备坐标（NDC）
    glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, -1.0f, 1.0f);     //将NDC转换成裁剪坐标
    glm::vec4 rayCamera = glm::inverse(projectionMat) * rayClip;        //将裁减坐标转换成摄像机坐标
    rayCamera = glm::vec4(rayCamera.x, rayCamera.y, -1.0f, 0.0f);
    glm::vec4 rayWorld = glm::inverse(viewMat) * rayCamera;            //将摄像机坐标转换成世界坐标
    directVec = glm::normalize(glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z)); //射线的单位方向向量
}

//************ 生成一个辅助点用于帮助确定拾取半径 ************
//      mouseX mouseY-鼠标事件的二维坐标
//      viewportWidth viewportHeight-视口尺寸
//      AdPointX AdPointY-辅助点的二维坐标
//******************************************************
void OpenglDisplay::GetAddedPoint(int &mouseX,
                           int &mouseY,
                           int &viewportWidth,
                           int &viewportHeight,
                           int &AdPointX,
                           int &AdPointY)
{
    Q_UNUSED(viewportWidth);
    AdPointX = mouseX;
    if((mouseY + radiusPixels) > viewportHeight) {
        AdPointY = mouseY - radiusPixels;
    }
    else {
        AdPointY = mouseY + radiusPixels;
    }
}

//******************** 得到拾取的半径 ********************
//      cameraPos-相机的空间位置
//      directVec-沿相机朝向的方向向量
//      directVecAd-由辅助点确定的方向向量
//      pickingRadius-点云拾取半径
//******************************************************
void OpenglDisplay::GetPickingRadius(glm::vec3 &cameraPos,
                                     glm::vec3 &directVec,
                                     glm::vec3 &directVecAd,
                                     float &pickingRadius)
{
    float ratio1, ratio2;
    ratio1 = glm::length(cameraPos) / glm::dot(directVec, glm::normalize(cameraPos));
    ratio2 = glm::length(cameraPos) / glm::dot(directVecAd, glm::normalize(cameraPos));
    pickingRadius = glm::length(glm::cross(ratio1*directVec, ratio2*directVecAd)) / ratio1;
    pickingRadius = fabs(pickingRadius);
}

//*************** 判断当前点是否在拾取半径内 ***************
//      modelMat-当前模型矩阵
//      cameraPos-相机的空间位置
//      directVec-沿相机朝向的方向向量
//      pickingRadius-点云拾取半径
//      currentRadius-当前输入点云到方向向量的距离
//      x y z-当前输入点云坐标
//******************************************************
bool OpenglDisplay::IsPointInCircle(glm::mat4 &modelMat,
                             glm::vec3 &cameraPos,
                             glm::vec3 &directVec,
                             float &pickingRadius,
                             float &currentRadius,
                             float &x,
                             float &y,
                             float &z)
{
    glm::vec4 localCoor = modelMat * glm::vec4(x, y, z, 1.0f);
    glm::vec3 testVec = glm::vec3(localCoor.x, localCoor.y, localCoor.z);
    float ratio = glm::dot(directVec, testVec - cameraPos);
    glm::vec3 crossVec = glm::cross(ratio * directVec, testVec - cameraPos);
    currentRadius = glm::length(crossVec) / fabs(ratio);
    if(currentRadius < pickingRadius) {
        return true;
    }
    else {
        return false;
    }
}

//*************** 实时点云在线刷新（槽函数） ***************
//
//******************************************************
void OpenglDisplay::OnlineUpdate() {
    //判断是否需要重新申请内存
    if (flag_display.cloud) {
        GLuint tmp_capacity = p_global->point_que.get_capacity();
        if(capacity_que != tmp_capacity)
        {
            if(vertices_cloud) delete[]vertices_cloud;
            vertices_cloud = new GLfloat[tmp_capacity * 6];
            capacity_que = tmp_capacity;
        }
        //队列中的数据，整理到内存vertices_que中
        GLuint que_num = p_global->point_que.size();
        for(GLuint i=0; i < que_num ; i++) {
            //坐标值
            /*
            vertices_cloud[i*6]   = p_global->point_que[i].x;
            vertices_cloud[i*6 + 1] = p_global->point_que[i].y;
            vertices_cloud[i*6 + 2] = p_global->point_que[i].z;
            */
            vertices_cloud[i*6]   = p_global->point_que[i].y;
            vertices_cloud[i*6 + 1] = p_global->point_que[i].z;
            vertices_cloud[i*6 + 2] = p_global->point_que[i].x;
            vertices_cloud[i*6 + 3] = 0.0f;
            vertices_cloud[i*6 + 4] = 1.0f;
            vertices_cloud[i*6 + 5] = 0.0f;
        }
        qDebug() << "online painting";
        PointNum = que_num * 6;
        SetCloudVertex(MODE_SET_ATTRIB);
    }
    if (flag_display.pose) {
        robot_pose = p_global->RobotPose;
        SetPoseVertex(MODE_SET_ATTRIB);
        speedplate->UpdatePose(robot_pose.roll, robot_pose.yaw, robot_pose.pitch);
        if (flag_display.trail) {
#ifdef ModelMatrixMode
            transVec = glm::vec3(-robot_pose.y, -robot_pose.z, 0.0f);
#endif
#ifdef ViewMatrixMode
            worldCentrol = glm::vec3(-robot_pose.y, -robot_pose.z, 0.0f);
            cameraPos = worldCentrol - radius * cameraFront;
#endif
        }
    }
    //p_global->reset();
    //SetCloudVertex(MODE_PAINT);
    update();
}

//**************** 在线更新状态栏（槽函数） ****************
//
//******************************************************
void OpenglDisplay::UpdateStatus(int linear, int angular) {
    speedplate->UpdateSpeed(linear, angular);
}
