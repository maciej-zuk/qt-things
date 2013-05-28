#ifndef VIDEORECORDERDEMO_H
#define VIDEORECORDERDEMO_H

#include <QMainWindow>


namespace Ui {
class VideoRecorderDemo;
}

class VideoRecorderDemo : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit VideoRecorderDemo(QWidget *parent = 0);
    ~VideoRecorderDemo();
    
private slots:
    void on_btnRender_clicked();

private:
    Ui::VideoRecorderDemo *ui;
};

#endif // VIDEORECORDERDEMO_H
