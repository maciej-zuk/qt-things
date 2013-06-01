#ifndef VIDEOPLAYERDEMO_H
#define VIDEOPLAYERDEMO_H

#include <QMainWindow>
#include <../videoplayer.h>

namespace Ui {
class VideoPlayerDemo;
}

class VideoPlayerDemo : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit VideoPlayerDemo(QWidget *parent = 0);
    ~VideoPlayerDemo();
    
private:
    Ui::VideoPlayerDemo *ui;

private slots:
    void openVideo();
};

#endif // VIDEOPLAYERDEMO_H
