#include "videorecorder.h"

extern "C" {
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
}

VideoRecorder::VideoRecorder(char * filename, int width, int height, int fps, int bps) :
    QImage(width, height, QImage::Format_RGB32)
{
    AVOutputFormat *fmt;

    av_register_all();
    m_formatContext = avformat_alloc_context();
    fmt=av_guess_format(NULL,filename,NULL);
    fmt->video_codec=CODEC_ID_H264;
    m_formatContext->oformat=fmt;

    m_codec = avcodec_find_encoder(CODEC_ID_H264);
    m_videoStream=avformat_new_stream(m_formatContext, m_codec);
    m_codecContext=m_videoStream->codec;
    m_codecContext->bit_rate = bps;
    m_codecContext->width = width;
    m_codecContext->height = height;
    m_codecContext->time_base= (AVRational){1,fps};
    m_codecContext->gop_size = fps;
    m_codecContext->max_b_frames=1;
    m_codecContext->pix_fmt = PIX_FMT_YUV420P;
    if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        m_codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    avcodec_open2(m_codecContext, m_codec, NULL);

    m_outFrame = avcodec_alloc_frame();
    m_outFrame->format = m_codecContext->pix_fmt;
    m_outFrame->width  = m_codecContext->width;
    m_outFrame->height = m_codecContext->height;
    av_image_alloc(m_outFrame->data, m_outFrame->linesize, m_codecContext->width, m_codecContext->height, m_codecContext->pix_fmt, 32);

    m_inFrame = avcodec_alloc_frame();

    avio_open(&m_formatContext->pb, filename, AVIO_FLAG_WRITE);
    avformat_write_header(m_formatContext, NULL);

    //av_dump_format(m_formatContext, 0, "", 1);

    m_swsContext = sws_getContext(width, height, PIX_FMT_RGB32, width, height, PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);

    frameNumber=0;
}

VideoRecorder::~VideoRecorder(){
    for(int i=0; i<m_codecContext->max_b_frames; i++)
        frameReady();
    av_write_trailer(m_formatContext);
    av_free(m_inFrame);
    av_free(m_outFrame);

    avio_close(m_formatContext->pb);
    av_free(m_formatContext);
}

void VideoRecorder::frameReady(){
    int gotPkt;
    av_init_packet(&m_packet);
    m_packet.data=NULL;
    m_packet.size = 0;
    const unsigned char* data[]={constBits()};
    int lines[]={(int)(m_codecContext->width*sizeof(unsigned int))};
    sws_scale(m_swsContext, data, lines, 0, m_codecContext->height, m_outFrame->data, m_outFrame->linesize);
    m_outFrame->pts=av_rescale_q(frameNumber++, m_codecContext->time_base, m_videoStream->time_base);
    avcodec_encode_video2(m_codecContext, &m_packet, m_outFrame, &gotPkt);
    if(gotPkt){
        av_write_frame(m_formatContext, &m_packet);
        av_free_packet(&m_packet);
    }
}
