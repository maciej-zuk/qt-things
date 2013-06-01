#include "videoplayerdemo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayerDemo w;
    w.show();
    
    return a.exec();
}
