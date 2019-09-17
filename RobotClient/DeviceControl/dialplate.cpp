#include "dialplate.h"

SpeedPlate::SpeedPlate(QWidget *parent)
        : QWidget(parent)
{
    this->setFixedSize(300, 140);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, QColor(20, 20, 20, 80));
    this->setAutoFillBackground(true);
    this->setPalette(palette);
    veer = 0;
    speed = 0;
    width = 140;
    length = 140;
    label_yaw = new QLabel(tr("yaw:"), this);
    label_pitch = new QLabel(tr("pitch:"),this);
    label_roll = new QLabel(tr("roll:"),this);
    label_yawval = new QLabel(tr("--"), this);
    label_pitchval = new QLabel(tr("--"), this);
    label_rollval = new QLabel(tr("--"), this);
    const char label_stylesheet[] = "QLabel{ color: white; }";
    label_yaw->setStyleSheet(label_stylesheet);
    label_pitch->setStyleSheet(label_stylesheet);
    label_roll->setStyleSheet(label_stylesheet);
    label_yawval->setStyleSheet(label_stylesheet);
    label_pitchval->setStyleSheet(label_stylesheet);
    label_rollval->setStyleSheet(label_stylesheet);
    layout_main = new QGridLayout(this);
    layout_main->setColumnStretch(0, 1);
    layout_main->addWidget(label_yaw, 0, 1, 1, 1);
    layout_main->addWidget(label_pitch, 1, 1, 1, 1);
    layout_main->addWidget(label_roll, 2, 1, 1, 1);
    layout_main->addWidget(label_yawval, 0, 2, 1, 1);
    layout_main->addWidget(label_pitchval, 1, 2, 1, 1);
    layout_main->addWidget(label_rollval, 2, 2, 1, 1);
}

SpeedPlate::~SpeedPlate() {

}

void SpeedPlate::UpdatePose(float yaw, float pitch, float roll) {
    //label_yawval->setText(QString("%1").arg(yaw));
    //label_pitchval->setText(QString("%1").arg(pitch));
    //label_rollval->setText(QString("%1").arg(roll));
    label_yawval->setText(QString::number(yaw, 'f', 4));
    label_pitchval->setText(QString::number(pitch, 'f', 4));
    label_rollval->setText(QString::number(roll, 'f', 4));
}

void SpeedPlate::UpdateSpeed(int linear, int angular) {
    speed = qAbs(linear);
    veer = qAbs(angular);
    int direct = 0;
    if (speed > 0) {
        direct = linear / speed;
        veer = (9 - direct * 5 - direct * angular) % 20;
    }
    else if (veer > 0) {
        speed = qAbs(angular);
        direct = angular / qAbs(angular);
        veer = 4 - direct * 5;
    }
    if (veer < 0)   veer += 20;
    //qDebug() << direct;
    update();
}

void SpeedPlate::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QColor color_background(170, 170, 170);
    QColor color_active(80, 80, 255);
    QColor color_speed[4];
    QColor color_stop = color_background;
    paint = new QPainter(this);
    paint->begin(this);
    paint->translate(width >> 1, length >> 1);
    /*
        * Parameter 1: radius
        * Parameter 2: start angle
        * Parameter 3: angleLength
        * Parameter 4: arcHeight
        * Parameter 5: color
    */
    for (int i = 0; i < 20; i++) {
        if (veer == i && speed > 0) {
            //qDebug() << "active" << speed << "-" << veer;
            for (int j = 0; j < 4; j++) {
                if (j < speed) {
                    color_speed[j] = color_active;
                }
                else {
                    color_speed[j] = color_background;
                }
            }
            PaintArc(65, 18 * i + 11, 14, 8, color_speed[3]);
            PaintArc(54, 18 * i + 11, 14, 8, color_speed[2]);
            PaintArc(43, 18 * i + 11, 14, 10, color_speed[1]);
            PaintArc(31, 18 * i + 11, 14, 10, color_speed[0]);
        }
        else {
            PaintArc(65, 18 * i + 11, 14, 8, color_background);
            PaintArc(54, 18 * i + 11, 14, 8, color_background);
            PaintArc(43, 18 * i + 11, 14, 10, color_background);
            PaintArc(31, 18 * i + 11, 14, 10, color_background);
        }

    }
    if (speed == 0 && veer == 0) {
        color_stop = color_active;
    }
    paint->setBrush(QBrush(color_stop, Qt::SolidPattern));
    paint->drawEllipse(-10, -10, 20, 20);
    paint->end();
}

void SpeedPlate::PaintArc(int radius, int startAngle, int angleLength, int arcHeight, QColor color) {
    paint->setBrush(QBrush(color, Qt::SolidPattern));
    paint->setPen(QPen(color, 1, Qt::SolidLine));
    // << 1（左移1位）相当于radius*2 即：150*2=300
    //QRectF(-150, -150, 300, 300)
    QRectF rect(-radius, -radius, radius << 1, radius << 1);
    QPainterPath path;
    path.arcTo(rect, startAngle, angleLength);
    // QRectF(-120, -120, 240, 240)
    QPainterPath subPath;
    subPath.addEllipse(rect.adjusted(arcHeight, arcHeight, -arcHeight, -arcHeight));
    // path为扇形 subPath为椭圆
    path -= subPath;
    paint->drawPath(path);
}
