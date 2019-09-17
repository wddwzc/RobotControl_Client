#ifndef DIALPLATE_H
#define DIALPLATE_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QPalette>
#include <QLabel>
#include <QGridLayout>

class SpeedPlate : public QWidget
{
    Q_OBJECT
public:
    SpeedPlate(QWidget * parent = 0);
    ~SpeedPlate();

public:
    void UpdatePose(float yaw, float pitch, float roll);

public slots:
    void UpdateSpeed(int linear, int angular);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    void PaintArc(int radius, int startAngle, int angleLength, int arcHeight, QColor color);

private:
    QPainter *paint;
    QLabel *label_yaw;
    QLabel *label_pitch;
    QLabel *label_roll;
    QLabel *label_yawval;
    QLabel *label_pitchval;
    QLabel *label_rollval;
    QGridLayout *layout_main;

    int veer;
    int speed;
    int width;
    int length;
};

#endif // DIALPLATE_H
