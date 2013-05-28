#include "videorecorderdemo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoRecorderDemo w;
    w.show();
    
    return a.exec();
}
