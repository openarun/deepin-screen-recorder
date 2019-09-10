#include "camerawidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>

CameraWidget::CameraWidget(DWidget *parent) : DWidget(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    m_cameraUI = new DLabel(this);

    QHBoxLayout *t_hlayout = new QHBoxLayout(this);
    t_hlayout->addWidget(m_cameraUI);
    this->setLayout(t_hlayout);
    initCamera();
}

CameraWidget::~CameraWidget()
{
    camera->stop();
    timer_image_capture->stop();
    delete camera;
    delete viewfinder;
    delete imageCapture;
    delete timer_image_capture;
    delete m_cameraUI;
}


void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    this->recordX = x;
    this->recordY = y;
    this->recordWidth = width;
    this->recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();
    move(pos.x(), pos.y());
}

int CameraWidget::getRecordX()
{
    return recordX;
}
int CameraWidget::getRecordY()
{
    return recordY;
}
int CameraWidget::getRecordWidth()
{
    return recordWidth;
}
int CameraWidget::getRecordHeight()
{
    return recordHeight;
}
void CameraWidget::initCamera()
{
    camera = new QCamera(this);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imageCapture = new QCameraImageCapture(camera);

    timer_image_capture = new QTimer(this);
    connect(timer_image_capture, &QTimer::timeout, this, &CameraWidget::captureImage);
}

void CameraWidget::cameraStart()
{
    timer_image_capture->start(50);
    camera->start();

    if (imageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToBuffer)) {
        imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
        qDebug() << imageCapture->supportedBufferFormats();
        imageCapture->setBufferFormat(QVideoFrame::PixelFormat::Format_Jpeg);
        qDebug() << imageCapture->supportedResolutions(imageCapture->encodingSettings());
        QImageEncoderSettings iamge_setting;
        iamge_setting.setResolution(640, 360);
        imageCapture->setEncodingSettings(iamge_setting);
        connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWidget::processCapturedImage);
    }

    else {
        return;
    }
}

void CameraWidget::cameraStop()
{
    timer_image_capture->stop();
    camera->stop();
}

void CameraWidget::captureImage()
{
    imageCapture->capture();
}

void CameraWidget::processCapturedImage(int request_id, const QImage &img)
{
    QImage t_image = img.scaled(this->width(), this->height(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    m_cameraUI->setPixmap(QPixmap::fromImage(t_image));
}
void CameraWidget::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qDebug() << "CameraWidget enterEvent";
}

void CameraWidget::paintEvent(QPaintEvent *e)
{
//    QPainter painter(this);
//    painter.setPen(QColor(255, 255, 255, 255));
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.drawRect(rect());

    DWidget::paintEvent(e);
}
void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton) {
        qDebug() << "CameraWidget mousePressEvent";
        m_move = true;
        //记录鼠标的世界坐标.
        m_startPoint = event->globalPos();
        //记录窗体的世界坐标.
        m_windowTopLeftPoint = this->frameGeometry().topLeft();
        m_windowTopRightPoint = this->frameGeometry().topRight();
        m_windowBottomLeftPoint = this->frameGeometry().bottomLeft();
    }
}

void CameraWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
//        //移动中的鼠标位置相对于初始位置的相对位置.
//        QPoint relativePos = event->globalPos() - m_startPoint;
//        QRect recordRect = QRect(recordX, recordY, recordWidth, recordHeight);
//        QPoint removePos = m_windowTopLeftPoint + relativePos;
//        QPoint removeTopRightPos = m_windowTopRightPoint + relativePos;
//        QPoint removeBottomLeftPos = m_windowBottomLeftPoint + relativePos;
//        //然后移动窗体即可.
//        if (recordRect.contains(removePos) &&
//                recordRect.contains(removeTopRightPos) &&
//                recordRect.contains(removeBottomLeftPos) ) {
//            this->move(removePos);
//        }

//        else {

//        }
        QSize size = this->size();
        QSize parentSize(recordWidth, recordHeight);
        QPoint curPos = QPoint(event->globalX() - size.width() / 2,
                               event->globalY() - size.height() / 2);
        if (curPos.x() < recordX) //left
            curPos.setX(recordX);

        if (curPos.y() < recordY) //top
            curPos.setY(recordY);

        if ( (curPos.x() + size.width()) > (recordX + parentSize.width())) //right
            curPos.setX(recordX + parentSize.width() - size.width());

        if ( (curPos.y() + size.height()) > (recordY + parentSize.height())) //bottom
            curPos.setY(recordY + parentSize.height() - size.height());

//        QPoint resultPos = QPoint(curPos.x() - size.width() / 2,
//                                  curPos.y() - size.height() / 2);
        this->move(curPos);
    }
    DWidget::mouseMoveEvent(event);
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //改变移动状态.
        m_move = false;
    }
}
