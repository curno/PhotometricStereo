#include "stdafx.h"
#include "imageview.h"
#include "mainwindow.h"
#include "relightningview.h"
#include "depthview.h"

ImageView::ImageView(QWidget *parent)
    : QWidget(parent), Model(nullptr), UI(nullptr)
{
    CurrentStates.push_back(NoneState::Instance());
}

ImageView::~ImageView()
{

}

void ImageView::CheckNormalState::paintEvent(ImageView *owner)
{
    QPainter painter(owner);
    painter.setBrush(QBrush(Qt::green));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(owner->LocationToCheck, PointSize, PointSize);
    painter.drawEllipse(owner->LocationResponding, PointSize, PointSize);

    ImageView::RelightningState::Instance()->paintEvent(owner);
}

void ImageView::CheckNormalState::mousePressEvent(ImageView *owner, QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        ShowMatchedPixel(owner, e);
    }
    else if (e->buttons() & Qt::RightButton)
    {
        auto m2 = owner->Model->ImageData.CreatePixelInfo(e->pos().x(), e->pos().y());
        auto m1 = owner->Model->ImageData.CreatePixelInfo(owner->LocationToCheck.x(), owner->LocationToCheck.y());
        owner->UI->AddVectorToVectorView(m2, 2);
        owner->UI->ui.LightningVectorLabel_->setText(FromStdStringToQString((m1.LightningVectorStr() + "\n" + m2.LightningVectorStr())));
    }
    owner->update();
}

void ImageView::CheckNormalState::mouseMoveEvent(ImageView *owner, QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        ShowMatchedPixel(owner, e);
    }
    owner->update();
}

void ImageView::CheckNormalState::ShowMatchedPixel(ImageView * owner, QMouseEvent * e)
{
    owner->LocationToCheck = e->pos();
    owner->LocationResponding = owner->Model->Configuration.ShadowDetection != ModelConfiguration::ShadowDetectionMethod::Ignore_Method ? 
        owner->Model->GetNearestPixelPositionConsideringShadow(owner->LocationToCheck.x(), owner->LocationToCheck.y()) 
        : owner->Model->GetNearestPixelPosition(owner->LocationToCheck.x(), owner->LocationToCheck.y());
    owner->UI->ClearVectorView();
    auto m1 = owner->Model->ImageData.CreatePixelInfo(owner->LocationToCheck.x(), owner->LocationToCheck.y());
    auto m2 = owner->Model->ImageData.CreatePixelInfo(owner->LocationResponding.x(), owner->LocationResponding.y());
    owner->UI->AddVectorToVectorView(m1, 0);
    owner->UI->AddVectorToVectorView(m2, 1);
    owner->UI->ui.LightningVectorLabel_->setText(FromStdStringToQString((m1.LightningVectorStr() + "\n" + m2.LightningVectorStr())));
}

void ImageView::CheckShadowState::mousePressEvent( ImageView *owner, QMouseEvent *e )
{
    owner->LocationToCheckShadow = e->pos();
    double shadow = owner->Model->ImageData.DifferenceCube->GetData(e->y(), e->x())[owner->CurrentIndex];
    double origin = owner->Model->ImageData.NormalCube->GetData(e->y(), e->x())[owner->CurrentIndex];
    owner->UI->ui.ShadowLabel_->setText(QString("Origin:%0 Shadow Removed: %1").arg(QString::number(origin, 'f', 1)).arg(QString::number(shadow, 'f', 1)));
    owner->update();
}

void ImageView::CheckShadowState::mouseMoveEvent( ImageView *owner, QMouseEvent *e )
{
    if (e->buttons() == Qt::LeftButton)
    {
        owner->LocationToCheckShadow = e->pos();
        double shadow = owner->Model->ImageData.DifferenceCube->GetData(e->y(), e->x())[owner->CurrentIndex];
        double origin = owner->Model->ImageData.NormalCube->GetData(e->y(), e->x())[owner->CurrentIndex];
        owner->UI->ui.ShadowLabel_->setText(QString("Origin:%0 Shadow Removed: %1").arg(QString::number(origin, 'f', 1)).arg(QString::number(shadow, 'f', 1)));
        owner->update();
    }
}
