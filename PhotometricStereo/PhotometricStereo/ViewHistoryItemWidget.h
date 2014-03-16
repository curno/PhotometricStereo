#pragma once

#include "stdafx.h"
#include <QtWidgets/QLabel>
#include <QtGui/QPainter>
#include <QtOpenGL/QGLWidget>
#include "PixelInfoSetViewDialog.h"

class ViewHistoryWidget;

class ViewHistoryItemWidget : public QWidget
{
    Q_OBJECT
    bool MouseHover_;
    ViewHistoryWidget *ViewHistory_;
public:
    ViewHistoryItemWidget(PixelInfoSetViewDialog *d, ViewHistoryWidget *parent);
    PixelInfoSetViewDialog *View;
    
protected:
    void paintEvent(QPaintEvent *evt) override
    {
        static const int Margin = 4;
        __super::paintEvent(evt);
        double w = View->width();
        double h = View->height();
        int target_w, target_h;
        if (this->width() / static_cast<double>(this->height()) > w / h)
        {
            target_h = this->height() - 2 * Margin;
            target_w = static_cast<int>(target_h * w / h);
        }
        else
        {
            target_w = this->width() - 2 * Margin;
            target_h = static_cast<int>(target_w * h / w);
        }

        QImage bmp = View->View->grabFrameBuffer(/*&bmp*/);
        QPainter painter(this);
        QRect target = QRect((this->width() - target_w) / 2, (this->height() - target_h) / 2, target_w, target_h);
        painter.drawImage(target, bmp);

        // Info.
        if (!MouseHover_)
            painter.fillRect(target, QColor(255, 255, 255, 32));

        if (hasFocus())
        {
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.drawRect(rect());
        }
        
    }

    void enterEvent(QEvent *evt) override
    {
        __super::enterEvent(evt);
        MouseHover_ = true;
        update();
    }

    void leaveEvent(QEvent *evt) override
    {
        __super::leaveEvent(evt);
        MouseHover_ = false;
        update();
    }
    void mouseDoubleClickEvent(QMouseEvent *evt) override
    {
        __super::mouseDoubleClickEvent(evt);
        ShowView();

    }

    void ShowView();

    void keyReleaseEvent(QKeyEvent *evt) override;
    
};