#include "videorecorderdemo.h"
#include "ui_videorecorderdemo.h"
#include "../videorecorder.h"

#include <QFileDialog>
#include <QPainter>

VideoRecorderDemo::VideoRecorderDemo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoRecorderDemo)
{
    ui->setupUi(this);
    ui->pbProgress->setVisible(false);
}

VideoRecorderDemo::~VideoRecorderDemo()
{
    delete ui;
}

void VideoRecorderDemo::on_btnRender_clicked()
{
    QString path=QFileDialog::getSaveFileName(this, tr("Select output file"), QString("."), tr("AVI (*.avi);;MPG (*.mpg);;MKV (*.mkv);;Other (*)"));
    if(!path.isEmpty()){
        ui->pbProgress->setVisible(true);
        VideoRecorder vr(path.toAscii().data(), 640, 480, 25);
        float vx=3;
        float vy=4;
        float x=100;
        float y=100;
        QPainter painter;
        QLinearGradient gradient(0, 240, 640, 240);
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(0.5, Qt::green);
        gradient.setColorAt(1, Qt::blue);
        for(int i=0; i<25*60; i++){
            painter.begin(&vr);
            x+=vx;
            y+=vy;
            if(x>(640-100) || x<0) {vx=-vx; x+=vx;}
            if(y>(480-100) || y<0) {vy=-vy; y+=vy;}
            painter.setBrush(gradient);
            painter.drawRect(vr.rect());
            painter.setBrush(Qt::white);
            painter.drawEllipse(x, y, 100, 100);
            painter.end();
            vr.frameReady();
            ui->pbProgress->setValue(100.*i/(25*60));
            update();
        }
        ui->pbProgress->setVisible(false);
    }

}
