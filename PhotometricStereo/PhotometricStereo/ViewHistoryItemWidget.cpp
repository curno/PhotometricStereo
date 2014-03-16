#include "stdafx.h"

#include "ViewHistoryItemWidget.h"
#include "ViewHistoryWidget.h"

ViewHistoryItemWidget::ViewHistoryItemWidget(PixelInfoSetViewDialog *d, ViewHistoryWidget *parent) : QWidget(parent), View(d), MouseHover_(false)
{
    setFocusPolicy(Qt::StrongFocus);
    ViewHistory_ = parent;
    connect(d->View, &PixelInfoSetView::paintUpdated, this, (void (QWidget::*)())(&ViewHistoryItemWidget::update));
    this->setToolTip(FromStdStringToQString(d->View->GetDescription()));
}

void ViewHistoryItemWidget::keyReleaseEvent(QKeyEvent *evt)
{
    __super::keyReleaseEvent(evt);
    if (evt->key() == Qt::Key_Delete)
    {
        View->DestroySelf();
        ViewHistory_->RemoveViewHistory(this);
    }
    else if (evt->key() == Qt::Key_Enter)
    {
        ShowView();
    }
}

void ViewHistoryItemWidget::ShowView()
{
    View->show();
    View->raise();
}
