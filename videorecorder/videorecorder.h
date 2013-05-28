#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <QImage>

extern "C" {
typedef unsigned long long int UINT64_C;
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoRecorder : public QImage
{
public:
    VideoRecorder(char *filename, int width, int height, int fps=25, int bps=1024*1024);
    ~VideoRecorder();

    void frameReady();

private:
    AVFormatContext * m_formatContext;
    AVCodec * m_codec;
    AVCodecContext * m_codecContext;
    AVFrame * m_outFrame;
    AVFrame * m_inFrame;
    AVPacket m_packet;
    AVStream * m_videoStream;
    SwsContext *m_swsContext;

    int frameNumber;
};

#endif // VIDEORECORDER_H
