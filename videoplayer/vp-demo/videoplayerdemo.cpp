#include "videoplayerdemo.h"
#include "ui_videoplayerdemo.h"
#include <QFileDialog>

VideoPlayerDemo::VideoPlayerDemo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoPlayerDemo)
{
    ui->setupUi(this);
    connect(ui->sbFrame, SIGNAL(valueChanged(int)), ui->player, SLOT(seek(int)));
    connect(ui->player, SIGNAL(currentFrameChange(int)), ui->sbFrame, SLOT(setValue(int)));
    connect(ui->btnOpen, SIGNAL(clicked()),  this, SLOT(openVideo()));
}

VideoPlayerDemo::~VideoPlayerDemo()
{
    delete ui;
}

void VideoPlayerDemo::openVideo(){
    QString path=QFileDialog::getOpenFileName(this, tr("Select video file to open"));
    if(!path.isEmpty()){
        ui->player->changeVideoFilePath(path);
        ui->sbFrame->setMaximum(ui->player->getFramesCount());
    }
}
