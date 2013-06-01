#include "videoplayer.h"
#include <GL/glu.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>

//#define UNSAFE_TRYRELOAD

VideoPlayer::VideoPlayer(QWidget *parent) :
    QGLWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_isLoaded=false;
    m_formatContext=NULL;
    m_codecContext=NULL;
    m_codec=NULL;
    m_oryginalFrame=NULL;
    m_uploadFrame=NULL;
    m_aspectWidget=1;
    m_aspectVideo=1;
    m_videoTexture=0;
    cleanUp();
}



VideoPlayer::~VideoPlayer(){
    cleanUp();
}


void VideoPlayer::initializeGL(){
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glColor4f(0, 0, 0, 0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_MODELVIEW);
}



void VideoPlayer::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    if(m_videoTexture){
        glBindTexture(GL_TEXTURE_2D, m_videoTexture);
        glColor4f(1,1,1,1);
    }
    else{
        glBindTexture(GL_TEXTURE_2D, 0);
        glColor4f(1,0,1,1);
    }

    glTranslatef(cx, cy, 0);
    glScalef(zoom, zoom, 1);
    glTranslatef(offX, -offY, 0);


    glBegin(GL_QUADS);
    glTexCoord2f(0,1);
    glVertex3f(-rx,-ry,0);
    glTexCoord2f(1,1);
    glVertex3f(rx,-ry,0);
    glTexCoord2f(1,0);
    glVertex3f(rx,ry,0);
    glTexCoord2f(0,0);
    glVertex3f(-rx,ry,0);
    glEnd();

}



void VideoPlayer::resizeGL(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    m_aspectWidget=float(w)/h;
    cx=w/2.0;
    cy=h/2.0;
    if(m_aspectWidget>m_aspectVideo){
        ry=cy;
        rx=cy*m_aspectVideo;
    }
    else{
        rx=cx;
        ry=cx/m_aspectVideo;
    }
    offX=0;
    offY=0;
}



void VideoPlayer::mouseMoveEvent(QMouseEvent *e){
    if(!m_isLoaded){
        return;
    }
    QPoint moveDelta=e->pos()-m_lastMousePosition;
    if(e->buttons()&Qt::RightButton){
        offX+=moveDelta.x()/zoom;
        offY+=moveDelta.y()/zoom;
        limitOffsets();
        updateGL();
    }
    m_lastMousePosition=e->pos();
}



void VideoPlayer::wheelEvent(QWheelEvent *e){
    if(!m_isLoaded){
        return;
    }
    if(e->delta()>0)
        zoom*=1.1;
    else
        zoom*=0.9;
    if(zoom<1){
        zoom=1;
    }
    else if(zoom>20){
        zoom=20;
    }
    limitOffsets();
    updateGL();
}



void VideoPlayer::mousePressEvent(QMouseEvent *e){
    if(!m_isLoaded){
        return;
    }
    if(e->buttons()&Qt::LeftButton){
        QPointF vPoint = mapWSpaceToVSpace(e->pos());
        if(vPoint.x()<0 || vPoint.y()<0 || vPoint.x()>m_codecContext->width || vPoint.y()>m_codecContext->height){
            return;
        }
        emit pointClicked(vPoint.x(), vPoint.y());
    }
}



void VideoPlayer::keyPressEvent(QKeyEvent *e){
    switch(e->key()){
    case Qt::Key_Left:
        seekPreviousFrame();
        break;
    case Qt::Key_Right:
        seekNextFrame();
        break;
    case Qt::Key_Home:
        seekFirstFrame();
        break;
    case Qt::Key_End:
        seekLastFrame();
        break;
    case Qt::Key_PageDown:
        seekRewind();
        break;
    case Qt::Key_PageUp:
        seekFastForward();
        break;
    }
}



void VideoPlayer::seekNextFrame(bool doUpdate){
    if(!m_isLoaded){
        return;
    }
    if(m_currentFrame-1>m_framesCount){
        return;
    }
    while (av_read_frame(m_formatContext, &m_packet) == 0) {
        if (m_packet.stream_index == m_videoStreamIndex) {
            avcodec_decode_video2(m_codecContext, m_oryginalFrame, &m_isFrameFinished, &m_packet);
            if (m_isFrameFinished) {
                av_free_packet(&m_packet);
                break;
            }
        }
        av_free_packet(&m_packet);
    }
    m_currentFrame++;
    if(doUpdate){
        uploadFrame();
        updateGL();
        emit currentFrameChange(m_currentFrame-2);
    }
}



void VideoPlayer::seekPreviousFrame(){
    if(m_currentFrame<=2){
        return;
    }
    seek(m_currentFrame-3);
}



void VideoPlayer::seekRewind(int frames){
    if(m_currentFrame-2-frames<0){
        seekFirstFrame();
    }
    else{
        seek(m_currentFrame-frames-2);
    }
}



void VideoPlayer::seekFastForward(int frames){
    if(m_currentFrame-2+frames>=m_framesCount){
        seekLastFrame();
    }
    else{
        seek(m_currentFrame+frames-2);
    }
}



void VideoPlayer::seekFirstFrame(){
    seek(0);
}



void VideoPlayer::seekLastFrame(){
    seek(m_framesCount);
}



void VideoPlayer::seek(int nframe){
    if(!m_isLoaded){
        return;
    }
    nframe+=2;
    if(nframe==m_currentFrame)
        return;
    if (nframe < 0 || nframe > m_framesCount){
        return;
    }
    int sframe = nframe;
    if (m_keyFrameIndex.find(sframe + 1) == m_keyFrameIndex.end()){
        sframe--;
    }
    if (sframe < 0){
        sframe = 0;
    }
    while (sframe > 0 && m_keyFrameIndex.find(sframe) == m_keyFrameIndex.end()) {
        sframe--;
    }
    m_currentFrame = sframe;
    int bpos = m_keyFrameIndex[sframe];
    av_seek_frame(m_formatContext, m_videoStreamIndex, bpos, 0);
    avcodec_flush_buffers(m_codecContext);
    while (m_currentFrame < nframe) {
        int e;
        if ((e = av_read_frame(m_formatContext, &m_packet)) < 0) {
            return;
        }
        if (m_packet.stream_index == m_videoStreamIndex) {
            avcodec_decode_video2(m_codecContext, m_oryginalFrame, &m_isFrameFinished, &m_packet);
            if (m_isFrameFinished) {
                m_currentFrame++;
            }
        }
        av_free_packet(&m_packet);
    }
    uploadFrame();
    updateGL();
    emit currentFrameChange(m_currentFrame-2);
}


bool VideoPlayer::changeVideoFilePath(const QString &filePath){
    cleanUp();

    av_register_all();
    m_fileName=filePath;
    if(avformat_open_input(&m_formatContext, filePath.toAscii().data(), NULL, NULL)!=0){
        QMessageBox::critical(0, tr("File could not be opened."), tr("File does not exist or type of this file is invalid."));
        return true;
    }
    if(avformat_find_stream_info(m_formatContext, NULL)<0){
        QMessageBox::critical(0, tr("Unsupported video codec."), tr("File could not be opened because it is encodec with unsupported codec."));
        return true;
    }
    m_videoStreamIndex = -1;
    for (uint i = 0; i < m_formatContext->nb_streams; i++){
        if (m_formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }
    if(m_videoStreamIndex<0){
        QMessageBox::critical(0, tr("No video stream"), tr("This file does not contain any video stream."));
        return true;
    }
    m_codecContext = m_formatContext->streams[m_videoStreamIndex]->codec;
    m_codec = avcodec_find_decoder(m_codecContext->codec_id);
    if(m_codec == NULL){
        QMessageBox::critical(0, tr("Unsupported video codec."), tr("File could not be opened because it is encodec with unsupported codec."));
        return true;
    }
    if(avcodec_open2(m_codecContext, m_codec, NULL)<0){
        return true;
    }

    m_aspectVideo=float(m_codecContext->width)/m_codecContext->height;

    if(QGLFormat::openGLVersionFlags()&QGLFormat::OpenGL_Version_2_0){
        m_textureWidth=m_codecContext->width;
        m_textureHeight=m_codecContext->height;
    }
    else{
        int hiDim;
        if(m_codecContext->height>m_codecContext->width){
            hiDim=m_codecContext->height;
        }
        else{
            hiDim=m_codecContext->width;
        }
        m_textureWidth = m_textureHeight = pow(2, ceil(log2(hiDim)));
    }
    m_oryginalFrame = avcodec_alloc_frame();
    m_uploadFrame = avcodec_alloc_frame();


    av_image_alloc(m_uploadFrame->data, m_uploadFrame->linesize, m_textureWidth, m_textureHeight, PIX_FMT_RGB24, 1);

    int frames = 0;
    int keys = 0;
    int p=0;
    av_seek_frame(m_formatContext, m_videoStreamIndex, 0, 0);
    int zeroPos=-1;
    while (1) {
        if (av_read_frame(m_formatContext, &m_packet) < 0){
            break;
        }
        if (m_packet.stream_index == m_videoStreamIndex) {
            frames++;
            if ((m_packet.flags & AV_PKT_FLAG_KEY) && !(m_packet.flags & AV_PKT_FLAG_CORRUPT)) {
                int k = frames;
                m_keyFrameIndex[k] = p;
                keys++;
            }
            p = m_packet.dts;
            if(zeroPos<0){
                zeroPos=p;
            }
        }
        av_free_packet(&m_packet);
    }
    m_keyFrameIndex[0] = zeroPos;
    av_seek_frame(m_formatContext, m_videoStreamIndex, 0, 0);
    m_framesCount=frames-1;
    m_swsContext = sws_getContext(m_codecContext->width, m_codecContext->height,
                                  m_codecContext->pix_fmt, m_textureWidth, m_textureHeight,
                                  PIX_FMT_RGB24, SWS_POINT, NULL, NULL, NULL);
    m_isLoaded=true;
    seek(0);
    if(m_aspectWidget>m_aspectVideo){
        ry=cy;
        rx=cy*m_aspectVideo;
    }
    else{
        rx=cx;
        ry=cx/m_aspectVideo;
    }
    glGenTextures(1, &m_videoTexture);
    glBindTexture(GL_TEXTURE_2D, m_videoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_textureWidth, m_textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    uploadFrame();
    emit framesCountChanged(m_framesCount);
    glColor4f(1,1,1,1);
    return false;
}


void VideoPlayer::limitOffsets(){
    if((rx+offX)*zoom+cx<width() && (-rx+offX)*zoom+cx>=0){
        //we have both borders visible so we just center the picture
        offX=0;
    }
    else{
        //we have one border or both borders invisible so we correct them
        if((rx+offX)*zoom+cx<width()){
            offX=(width()-cx)/zoom-rx;
        }
        if((-rx+offX)*zoom+cx>=0){
            offX=rx-cx/zoom;
        }
    }

    if(((ry+offY)*zoom+cy<height()) && ((-ry+offY)*zoom+cy>=0)){
        offY=0;
    }
    else{
        if((ry+offY)*zoom+cy<height()){
            offY=(height()-cy)/zoom-ry;
        }
        if((-ry+offY)*zoom+cy>=0){
            offY=ry-cy/zoom;
        }
    }
}



void VideoPlayer::uploadFrame(){
    sws_scale(m_swsContext, m_oryginalFrame->data, m_oryginalFrame->linesize, 0, m_codecContext->height, m_uploadFrame->data, m_uploadFrame->linesize);
    glBindTexture(GL_TEXTURE_2D, m_videoTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_textureWidth, m_textureHeight, GL_RGB, GL_UNSIGNED_BYTE, m_uploadFrame->data[0]);
}



void VideoPlayer::cleanUp(){
    if(m_isLoaded){
        glDeleteTextures(1, &m_videoTexture);
        if(m_oryginalFrame)
            av_free(m_oryginalFrame);
        if(m_uploadFrame){
            av_freep(&m_uploadFrame->data[0]);
            av_free(m_uploadFrame);
        }
        if(m_codecContext)
            avcodec_close(m_codecContext);
        if(m_formatContext)
            avformat_close_input(&m_formatContext);
        if(m_swsContext)
            sws_freeContext(m_swsContext);
        m_keyFrameIndex.clear();
    }
    m_oryginalFrame=NULL;
    m_uploadFrame=NULL;
    m_codecContext=NULL;
    m_formatContext=NULL;
    m_swsContext=NULL;
    zoom=1;
    offX=0;
    offY=0;
    m_currentFrame=-1;
}



QPointF VideoPlayer::mapWSpaceToVSpace(const QPoint &wSpacePoint){
    float x, y;
    float vcx, vcy;
    vcx=m_codecContext->width/2.0;
    vcy=m_codecContext->height/2.0;
    x=wSpacePoint.x();
    y=(height()-wSpacePoint.y());
    x=((x-cx)/zoom-offX)/cx;
    y=((y-cy)/zoom+offY)/cy;
    //x,y now normalised in (-1, 1) in widget space
    x=x*vcx*cx/rx+vcx;
    y=y*vcy*cy/ry+vcy;
    return QPointF(x, y);
}



QPoint VideoPlayer::mapVSpaceToWSpace(const QPointF &vSpacePoint){
    float x, y;
    x=vSpacePoint.x();
    y=vSpacePoint.y();
    float vcx, vcy;
    vcx=m_codecContext->width/2.0;
    vcy=m_codecContext->height/2.0;
    x=(x-vcx)*rx/(vcx*cx);
    y=(y-vcy)*ry/(vcy*cy);
    x=(cx*((x*zoom) + 1)) + (offX*zoom);
    y=(cy*((y*zoom) + 1)) + (offY*zoom);
    return QPoint(round(x),round(height()-y));
}
