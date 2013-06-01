#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QGLWidget>

extern "C" {
#define UINT64_C uint64_t
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <QMap>

class VideoPlayer : public QGLWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

    inline int getFramesCount(){
        return m_framesCount;
    }

    inline int getVideoWidth(){
        if(m_codecContext)
            return m_codecContext->width;
        return 0;
    }

    inline int getVideoHeight(){
        if(m_codecContext)
            return m_codecContext->height;
        return 0;
    }


protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

signals:
    void currentFrameChange(int frame);
    void framesCountChanged(int newFramesCount);
    void pointClicked(float x, float y);

public slots:
    void seekNextFrame(bool doUpdate=true);
    void seekPreviousFrame();
    void seekRewind(int frames=10);
    void seekFastForward(int frames=10);
    void seekFirstFrame();
    void seekLastFrame();
    void seek(int nframe);
    bool changeVideoFilePath(const QString &filePath);

private:

    void limitOffsets();
    void uploadFrame();
    void cleanUp();

    QPointF mapWSpaceToVSpace(const QPoint &wSpacePoint);
    QPoint mapVSpaceToWSpace(const QPointF &vSpacePoint);

    bool m_isLoaded;

    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    AVCodec *m_codec;
    AVFrame *m_oryginalFrame;
    AVFrame *m_uploadFrame;
    AVPacket m_packet;

    int m_videoStreamIndex;
    int m_isFrameFinished;
    int m_framesCount;
    int m_currentFrame;

    QMap<int, int> m_keyFrameIndex;
    QString m_fileName;

    //gl stuff
    GLuint m_videoTexture;
    GLuint m_textureWidth;
    GLuint m_textureHeight;

    //size aspects for video and widget
    float m_aspectVideo;
    float m_aspectWidget;
    //half sizes of video rectangle
    float rx;
    float ry;
    //half sizes of widget rectangle
    float cx;
    float cy;
    //zoom
    float zoom;
    //offsets
    float offX;
    float offY;


    QPoint m_lastMousePosition;
    SwsContext *m_swsContext;
};

#endif // VIDEOPLAYER_H
