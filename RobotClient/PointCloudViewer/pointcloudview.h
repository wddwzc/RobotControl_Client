#ifndef POINTCLOUDVIEW_H
#define POINTCLOUDVIEW_H

#include "DataStructure/packageheader.h"
#include "DataStructure/pointcloud.h"
#include "opengldispay.h"

#include <QWidget>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class PointCloudViewer : public QWidget
{
    Q_OBJECT

public:
    PointCloudViewer(QWidget * parent = 0);
    ~PointCloudViewer();

public:
    OpenglDisplay *opengl_window_offline;

signals:
    void DisplayChange(DisplayFlag);
    void DataTransfer(char *data, int num);

private slots:
    void PickButtonClicked();
    void UploadPointsPicked();
    void CheckBoxChanged();
    void DisplayClicked();
    void ChangeDisplayMode(const QString &mode);

private:
    void InitToolBar();
    void InitCheckBox();
    void InitDisplayModule();
    void InitSettings();
    void LayoutSet();
    void StyleConfiguration();
    void StyleChange();
    void OpenFile();
    void CloseFile();

private:
    QToolButton *button_openfile;
    QToolButton *button_close;
    QToolButton *button_picking;
    QToolButton *button_load;
    QToolButton *button_save;
    QToolButton *button_undo;
    QToolButton *button_upload;
    QToolButton *button_clear;
    QToolButton *button_display;
    QCheckBox *check_prior;
    QCheckBox *check_grid;
    QCheckBox *check_origin;
    QCheckBox *check_picked;
    QCheckBox *check_path;
    QCheckBox *check_all;
    QComboBox *combox_display_mode;
    QToolButton *button_loadpath;
    QToolButton *button_configure;

    QHBoxLayout *layout_main;
    QVBoxLayout *layout_toolbar;

    PointCloudData *p_global;
    DisplayFlag flag_display;
    bool is_picking;
};

#endif // POINTCLOUDVIEW_H
