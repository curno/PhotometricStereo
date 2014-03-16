#include "stdafx.h"

#include "mainwindow.h"
#include "PixelInfoSetViewDialog.h"

PixelInfoSetViewDialog::PixelInfoSetViewDialog(MainWindow *parent, PixelInfoSetView *view ) : QDialog(parent), View(view)
{
    view->setParent(this);
    view->Parent_ = this;
    view->show();
    Info = new QLabel(this);
    Info->setStyleSheet("QLabel{color:#ffffff; background-color:#2d2d2f; text-align:left;}");
    this->setModal(false);
    Info->hide();
    if (dynamic_cast<DepthView *>(view) != nullptr)
        setWindowTitle("Photometric Stereo - Depth View");
    else if (dynamic_cast<RelightningView *>(view) != nullptr)
        setWindowTitle("Photometric Stereo - Relightning View");
    if (parent != nullptr)
    {
        connect(this, &PixelInfoSetViewDialog::hiding, parent, &MainWindow::On_widget_hiding);
        connect(this, &PixelInfoSetViewDialog::showing, parent, &MainWindow::On_widget_showing);
    }
}

void PixelInfoSetViewDialog::UpdateLayout()
{
    View->setGeometry(0, 0, width(), height());
    if (!Info->text().isEmpty())
        Info->show();
    Info->setGeometry(0, height() - Info->sizeHint().height(), Info->sizeHint().width(), Info->sizeHint().height());
}
