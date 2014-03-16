#pragma once

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include "ViewHistoryItemWidget.h"

class ViewHistoryWidget : public QScrollArea
{
    QWidget *Content_;
    QVBoxLayout *Layout_;
    QList<ViewHistoryItemWidget *> Items_;
public:
    ViewHistoryWidget(QWidget *parent) : QScrollArea(parent) 
    {
        Content_ = new QWidget;
        Layout_ = new QVBoxLayout;
        
        Content_->setLayout(Layout_);
        Layout_->setContentsMargins(Margin, Margin, Margin, Margin);
        this->setWidget(Content_);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        UpdateLayout();
    }

    void AddViewHistory(PixelInfoSetViewDialog *d)
    {
        ViewHistoryItemWidget *w = new ViewHistoryItemWidget(d, this);
        w->setFixedSize(Width, Height);
        Layout_->addWidget(w);
        Items_.append(w);
        UpdateLayout();
    }

    void RemoveViewHistory(ViewHistoryItemWidget *w)
    {
        if (qApp->focusWidget() == w)
        {
            if (!this->focusNextChild())
                this->focusPreviousChild();
        }
        Layout_->removeWidget(w);
        Items_.removeAt(Items_.indexOf(w));
        delete w;
        UpdateLayout();
    }

protected:
    void resizeEvent(QResizeEvent *evt) override
    {
        __super::resizeEvent(evt);
        UpdateLayout();
    }

    virtual void keyPressEvent(QKeyEvent *evt) override
    {
        __super::keyPressEvent(evt);
        QWidget *w = focusWidget();
        if (evt->key() == Qt::Key_Up)
        {
            int index_to_focus;
            if (w == nullptr)
                index_to_focus = Items_.size() - 1;
            else if (!Items_.empty())
            {
                int index = Items_.indexOf(static_cast<ViewHistoryItemWidget *>(w));
                index_to_focus = (index + Items_.size() - 1) % Items_.size(); 
            }
            if (index_to_focus >= 0)
                Items_[index_to_focus]->setFocus();
        }
        else if (evt->key() == Qt::Key_Down)
        {
            int index_to_focus = -1;
            if (w == nullptr)
            {
                if (!Items_.empty())
                    index_to_focus = 0;
            }
            else if (!Items_.empty())
            {
                int index = Items_.indexOf(static_cast<ViewHistoryItemWidget *>(w));
                index_to_focus = (index + 1) % Items_.size(); 
            }
            if (index_to_focus >= 0)
                Items_[index_to_focus]->setFocus();
        }
    }
private:
    void UpdateLayout()
    {
        QSize size(Width + 2 * Margin, (Height + Margin) * Items_.size());
        Content_->resize(size);
    }

protected:
    static const int Width = 100;
    static const int Height = 120;
    static const int Margin = 10;
};