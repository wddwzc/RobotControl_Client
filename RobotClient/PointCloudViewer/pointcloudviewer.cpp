#include "PointCloudViewer/pointcloudview.h"

PointCloudViewer::PointCloudViewer(QWidget *parent)
    : QWidget(parent)
{
    InitDisplayModule();
    InitToolBar();
    InitCheckBox();
    InitSettings();
    LayoutSet();
    StyleConfiguration();
    is_picking = false;
    p_global = PointCloudData::GetInstance();
}

PointCloudViewer::~PointCloudViewer()
{

}

void PointCloudViewer::InitToolBar() {
    button_openfile = new QToolButton(this);
    button_close = new QToolButton(this);
    button_picking = new QToolButton(this);
    button_load = new QToolButton(this);
    button_save = new QToolButton(this);
    button_undo = new QToolButton(this);
    button_upload = new QToolButton(this);
    button_clear = new QToolButton(this);
    button_display = new QToolButton(this);
    button_loadpath = new QToolButton(this);
    button_configure = new QToolButton(this);
    button_close->setEnabled(false);
    //button_picking->setEnabled(false);
    button_openfile->setText(tr("Open"));
    button_close->setText(tr("Close"));
    button_picking->setText(tr("Picking"));
    button_load->setText(tr("Load"));
    button_save->setText(tr("Save"));
    button_undo->setText(tr("Undo"));
    button_upload->setText(tr("Upload"));
    button_clear->setText(tr("Clear"));
    button_display->setText(tr("Display"));
    button_loadpath->setText(tr("LoadPath"));
    button_configure->setText(tr("Configure"));
    button_load->hide();
    button_save->hide();
    button_undo->hide();
    button_upload->hide();
    button_clear->hide();
    connect(button_openfile, &QToolButton::clicked, this, &PointCloudViewer::OpenFile);
    connect(button_close, &QToolButton::clicked, this, &PointCloudViewer::CloseFile);
    connect(button_picking, &QToolButton::clicked, this, &PointCloudViewer::PickButtonClicked);
    connect(button_load, &QToolButton::clicked, opengl_window_offline, &OpenglDisplay::LoadPath);
    connect(button_save, &QToolButton::clicked, opengl_window_offline, &OpenglDisplay::SavePointsPicked);
    connect(button_undo, &QToolButton::clicked, opengl_window_offline, &OpenglDisplay::RepealPointsPicked);
    connect(button_upload, &QToolButton::clicked, this, &PointCloudViewer::UploadPointsPicked);
    connect(button_clear, &QToolButton::clicked, opengl_window_offline, &OpenglDisplay::ClearPath);
    connect(button_loadpath, &QToolButton::clicked, opengl_window_offline, &OpenglDisplay::LoadGraphPath);
    connect(button_display, &QToolButton::clicked, this, &PointCloudViewer::DisplayClicked);
}

void PointCloudViewer::InitCheckBox() {
    check_prior = new QCheckBox(tr("Prior"), this);
    check_grid = new QCheckBox(tr("Grid"), this);
    check_origin = new QCheckBox(tr("Origin"), this);
    check_picked = new QCheckBox(tr("Picked"), this);
    check_path = new QCheckBox(tr("Path"), this);
    check_all = new QCheckBox(tr("All"), this);
    check_prior->setChecked(true);
    check_grid->setChecked(true);
    check_origin->setChecked(true);
    check_picked->setChecked(true);
    check_path->setChecked(true);
    check_all->setChecked(true);
    check_prior->hide();
    check_grid->hide();
    check_origin->hide();
    check_picked->hide();
    check_path->hide();
    check_all->hide();
    connect(check_prior, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
    connect(check_grid, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
    connect(check_origin, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
    connect(check_picked, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
    connect(check_path, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
    connect(check_all, &QCheckBox::clicked, this, &PointCloudViewer::CheckBoxChanged);
}

void PointCloudViewer::InitDisplayModule() {
    opengl_window_offline = new OpenglDisplay(false, this);
    connect(this, &PointCloudViewer::DisplayChange, opengl_window_offline, &OpenglDisplay::DisplayFlagChange);

    flag_display.cloud =false;
    flag_display.grid = true;
    flag_display.origin = true;
    flag_display.path = true;
    flag_display.picked = true;
    flag_display.pose = false;
    flag_display.prior = true;
    flag_display.trail = false;

    flag_display.mode = true;
    flag_display.style = true;
}

void PointCloudViewer::InitSettings() {
    combox_display_mode = new QComboBox(this);
    combox_display_mode->setFixedWidth(90);
    combox_display_mode->addItem(tr("Elevation"));
    combox_display_mode->addItem(tr("Grayscale"));
    combox_display_mode->hide();
    connect(combox_display_mode, &QComboBox::currentTextChanged, this, &PointCloudViewer::ChangeDisplayMode);
}

void PointCloudViewer::LayoutSet() {
    layout_toolbar = new QVBoxLayout;
    layout_toolbar->addWidget(button_openfile);
    layout_toolbar->addWidget(button_close);
    layout_toolbar->addWidget(button_picking);
    layout_toolbar->addWidget(button_load, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_save, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_undo, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_upload, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_clear, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_loadpath);
    layout_toolbar->addWidget(button_display);
    layout_toolbar->addWidget(check_prior, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(check_grid, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(check_origin, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(check_picked, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(check_path, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(check_all, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(combox_display_mode, 0, Qt::AlignCenter);
    layout_toolbar->addWidget(button_configure);
    layout_toolbar->addStretch();
    //layout_toolbar->setMargin(0);
    layout_main = new QHBoxLayout(this);
    layout_main->addWidget(opengl_window_offline);
    layout_main->addLayout(layout_toolbar);
    //layout_main->setMargin(0);
}

void PointCloudViewer::StyleConfiguration() {
    const char button_stylesheet[] = "QToolButton{ background: #CCCCCC; width: 100px; padding: 5px; border-radius: 10px; }"
                                     "QToolButton:hover{ background: #FFFFFF; }"
                                     "QToolButton:pressed{ background: #999999; }";
    const char button1_stylesheet[] = "QToolButton{ background: #CCCCCC; width: 60px; padding: 5px; border-radius: 10px; }"
                                      "QToolButton:hover{ background: #FFFFFF; }"
                                      "QToolButton:pressed{ background: #999999; }";
    const char checkbox_stylesheet[] = "QCheckBox{ color: white; width: 50px; }";
    //const char combox_stylesheet[] = "QComboBox{ padding: 5px; border-radius: 10px; }";
    button_openfile->setStyleSheet(button_stylesheet);
    button_close->setStyleSheet(button_stylesheet);
    button_picking->setStyleSheet(button_stylesheet);
    button_loadpath->setStyleSheet(button_stylesheet);
    button_configure->setStyleSheet(button_stylesheet);
    button_load->setStyleSheet(button1_stylesheet);
    button_save->setStyleSheet(button1_stylesheet);
    button_undo->setStyleSheet(button1_stylesheet);
    button_upload->setStyleSheet(button1_stylesheet);
    button_clear->setStyleSheet(button1_stylesheet);
    button_display->setStyleSheet(button_stylesheet);
    check_prior->setStyleSheet(checkbox_stylesheet);
    check_grid->setStyleSheet(checkbox_stylesheet);
    check_origin->setStyleSheet(checkbox_stylesheet);
    check_picked->setStyleSheet(checkbox_stylesheet);
    check_path->setStyleSheet(checkbox_stylesheet);
    check_all->setStyleSheet(checkbox_stylesheet);
    //combox_display_mode->setStyleSheet(combox_stylesheet);
}

void PointCloudViewer::StyleChange() {
    if (!is_picking) {
        button_picking->setStyleSheet("QToolButton{ background: #CCCCCC; width: 100px; padding: 5px; border-radius: 10px; }"
                                      "QToolButton:hover{ background: #FFFFFF; }"
                                      "QToolButton:pressed{ background: #999999; }");
    }
    else {
        button_picking->setStyleSheet("QToolButton{ background: #5050FF; width: 100px; padding: 5px; border-radius: 10px; }"
                                      "QToolButton:hover{ background: #6060FF; }"
                                      "QToolButton:pressed{ background: #4040FF; }");
    }
}

void PointCloudViewer::OpenFile() {
    if (opengl_window_offline->OpenFile()) {
        button_close->setEnabled(true);
        button_picking->setEnabled(true);
    }
}

void PointCloudViewer::CloseFile() {
    button_close->setEnabled(false);
    button_picking->setEnabled(false);
    opengl_window_offline->CloseFile();
    is_picking = false;
    StyleChange();
}

void PointCloudViewer::PickButtonClicked() {
    if (!is_picking) {
        is_picking = true;
        button_load->show();
        button_save->show();
        button_undo->show();
        button_upload->show();
        button_clear->show();
        StyleChange();
    }
    else {
        is_picking = false;
        button_load->hide();
        button_save->hide();
        button_undo->hide();
        button_upload->hide();
        button_clear->hide();
        StyleChange();
    }
    opengl_window_offline->PointPicking();
}

void PointCloudViewer::UploadPointsPicked() {
    if (QMessageBox::Yes == QMessageBox::question(this, tr("Question?"), tr("Do you affirm to upload?"), QMessageBox::Yes, QMessageBox::No)) {
        qDebug() << "Uploading";
        PackageHeader cmd;
        cmd.msg_type = CMD_TYPE_DATA;
        cmd.msg_data = CMD_DATA_PATHNODE;
        int nodenum = (int)p_global->PathNode.size();
        char *sbuffer = new char[sizeof(PackageHeader) + nodenum * sizeof(glm::vec3)];
        int len = 0;
        char *ch = sbuffer + sizeof(PackageHeader);
        glm::vec3 temp_node;
        for (int i = 0; i < nodenum; i++) {
            temp_node = p_global->PathNode.at(i);
            memcpy(ch, (char*)&temp_node, sizeof(temp_node));
            ch += sizeof(temp_node);
            len += sizeof(temp_node);
        }
        cmd.msg_len = len;
        memcpy(sbuffer, (char*)&cmd, sizeof(PackageHeader));
        len += sizeof(PackageHeader);
        emit DataTransfer(sbuffer, len);
    }
}

void PointCloudViewer::CheckBoxChanged() {
    QCheckBox *cbx_statechanged = qobject_cast<QCheckBox *>(sender());
    if (cbx_statechanged == check_all) {
        if (check_all->isChecked()) {
            check_prior->setChecked(true);
            check_grid->setChecked(true);
            check_origin->setChecked(true);
            check_picked->setChecked(true);
            check_path->setChecked(true);
        }
        else {
            check_prior->setChecked(false);
            check_grid->setChecked(false);
            check_origin->setChecked(false);
            check_picked->setChecked(false);
            check_path->setChecked(false);
        }
    }
    else {
        if (check_prior->isChecked() && check_grid->isChecked() && check_origin->isChecked()
                && check_picked->isChecked() && check_path->isChecked()) {
            check_all->setChecked(true);
        }
        if (!check_prior->isChecked() || !check_grid->isChecked() || !check_origin->isChecked()
                || !check_picked->isChecked() && check_path->isChecked()) {
            check_all->setChecked(false);
        }
    }
    if (check_prior->isChecked())   flag_display.prior = true;
    else                            flag_display.prior = false;
    if (check_grid->isChecked())    flag_display.grid = true;
    else                            flag_display.grid = false;
    if (check_origin->isChecked())  flag_display.origin = true;
    else                            flag_display.origin = false;
    if (check_picked->isChecked())  flag_display.picked = true;
    else                            flag_display.picked = false;
    if (check_path->isChecked())    flag_display.path = true;
    else                            flag_display.path = false;
    emit DisplayChange(flag_display);
}

void PointCloudViewer::DisplayClicked() {
    if (check_all->isVisible()) {
        check_prior->hide();
        check_grid->hide();
        check_origin->hide();
        check_picked->hide();
        check_path->hide();
        check_all->hide();
        combox_display_mode->hide();
    }
    else {
        check_prior->show();
        check_grid->show();
        check_origin->show();
        check_picked->show();
        check_path->show();
        check_all->show();
        combox_display_mode->show();
    }
}

void PointCloudViewer::ChangeDisplayMode(const QString &mode) {
    if (mode.startsWith("Elevation")) {
        flag_display.style = true;
        qDebug() << mode;
    }
    else if (mode.startsWith("Grayscale")) {
        flag_display.style = false;
        qDebug() << mode;
    }
    emit DisplayChange(flag_display);
}
