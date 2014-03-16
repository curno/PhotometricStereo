#pragma once

#include "RelightningView.h"
#include "DepthView.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>

class MainWindow;
class PixelInfoSetViewDialog : public QDialog
{
    Q_OBJECT
public:
    PixelInfoSetViewDialog(MainWindow *parent, PixelInfoSetView *view);
    PixelInfoSetView *View;
    QLabel *Info;

    void Update()
    {
        Info->setText(FromStdStringToQString(View->GetDescription()));
        UpdateLayout();
        update();
    }

    void DestroySelf()
    {
        this->hide();
        delete this;
    }
    
    void SetData(PixelInfoSet pixels, const QRect &rect, const string &description, bool copy = false)
    {
        View->SetData(pixels, rect, description, copy);
        InitLayout();
    }

    void SetData(const string &file_name)
    {
        View->SetData(file_name);
        InitLayout();
    }

    void ToggleConfigurationVisibility()
    {
        if (Info->isVisible())
            Info->hide();
        else
            Info->show();
    }
signals:
    void showing();
    void hiding();
protected:
    void resizeEvent(QResizeEvent *evt) override
    {
        __super::resizeEvent(evt);
        UpdateLayout();
    }

    void InitLayout() 
    {
        static const unsigned int Scale = 3;
        QRect rect = View->GetRegion();
        this->setGeometry(this->x(), this->y(), rect.width() * Scale, rect.height() * Scale + 150);
    }

    void UpdateLayout();

    virtual void showEvent(QShowEvent *evt) override
    {
        __super::showEvent(evt);
        emit showing();
    }

    virtual void hideEvent(QHideEvent *evt) override
    {
        __super::hideEvent(evt);
        emit hiding();
    }
    
    
};