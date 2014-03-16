#pragma once

#include "PixelInfo.h"
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class VectorView : public QWidget
{
    Q_OBJECT

public:
    VectorView(QWidget *parent)
        : QWidget(parent) { setMouseTracking(true); UpdateDisplayData(); }
    ~VectorView() {}

    vector<PixelInfo> Vectors;

    virtual void paintEvent(QPaintEvent *e) override
    {
        QWidget::paintEvent(e);

        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);

        for (int i = 0; i < TmpPoints_.size(); i++)
        {
            painter.setBrush(Qt::NoBrush);
            if (i == FocusedI_)
                continue;
            painter.setPen(QPen(Qt::red));
            for (int j = 1; j < TmpPoints_[i].size(); j++)
                painter.drawLine(TmpPoints_[i][j - 1], TmpPoints_[i][j]);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(Qt::green));
            for (int j = 0; j < TmpPoints_[i].size(); j++)
                painter.drawEllipse(TmpPoints_[i][j], PointSize, PointSize);
        }

        if (FocusedI_ >= 0 || FocusedJ_ >= 0)
        {
            painter.setPen(QPen(Qt::white, 2, Qt::SolidLine));
            painter.setBrush(QBrush(Qt::red));
            for (int i = 1; i < TmpPoints_[FocusedI_].size(); ++i)
                painter.drawLine(TmpPoints_[FocusedI_][i - 1], TmpPoints_[FocusedI_][i]);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(Qt::green));
            for (int j = 0; j < TmpPoints_[FocusedI_].size(); j++)
                painter.drawEllipse(TmpPoints_[FocusedI_][j], PointSize * 2, PointSize * 2);
            QPoint center = TmpPoints_[FocusedI_][FocusedJ_];
            QRect rect(center.x() - PointSize * 4, center.y() - PointSize * 4, PointSize * 8, PointSize * 8);
            painter.drawEllipse(rect);
            painter.setPen(QPen(Qt::red));
            painter.drawText(rect, Qt::AlignCenter, QString::number(FocusedI_));
        }
    }
    void SetVector(PixelInfo vector, int index)
    {
        if (index >= Vectors.size())
            Add(vector);
        else
        {
            Vectors[index] = vector;
            UpdateDisplayData();
        }
        update();
    }

    void Clear()
    {
        Vectors.clear();
        update();
    }

    void Add(PixelInfo info)
    {
        Vectors.push_back(info);
        UpdateDisplayData();
        update();
    }
protected:
    void mouseMoveEvent(QMouseEvent *evt) override
    {
        __super::mouseMoveEvent(evt);
        FocusedI_ = FocusedJ_ = -1;
        for (auto i = TmpPoints_.begin(); i != TmpPoints_.end(); ++i)
        {
            for (auto j = i->begin(); j != i->end(); ++j)
            {
                QPoint &p = *j;
                if (abs(p.x() - evt->pos().x()) <= PointSize && 
                    abs(p.y() - evt->pos().y()) <= PointSize)
                {
                    FocusedI_ = distance(TmpPoints_.begin(), i);
                    FocusedJ_ = distance(i->begin(), j);
                    update();
                    return;
                }
            }
        }
        update();
    }

private:
    static const int Margin = 10;
    static const int PointSize = 2;

    NormalType Max_, Min_;
    vector<vector<QPoint>> TmpPoints_;
    int FocusedI_, FocusedJ_;

    void UpdateDisplayData()
    {
        Max_ = INT_MIN;
        Min_ = INT_MAX;
        TmpPoints_.clear();
        FocusedI_ = FocusedJ_ = -1;
        int basey = this->height() - Margin;
        int h = this->height() - 2 * Margin;
        int basex = Margin;
        int w = this->width() - 2 * Margin;

        for (auto i = Vectors.begin(); i != Vectors.end(); ++i)
        {
            PixelInfo &p = *i;
            NormalType *v = p.GetData<NormalType>(NormalCube);
            int dimension = p.Dimension;
            for (int j = 0; j < dimension; ++j)
            {
                Max_ = max(Max_, v[j]);
                Min_ = min(Min_, v[j]);
            }
        }

        double range = Max_ - Min_;
        for (auto i = Vectors.begin(); i != Vectors.end(); ++i)
        {
            PixelInfo &p = *i;
            NormalType *v = p.GetData<NormalType>(NormalCube);
            int dimension = p.Dimension;
            TmpPoints_.push_back(vector<QPoint>());
            int x_interval = static_cast<int>(static_cast<double>(w) / (dimension - 1));
            for (int j = 0; j < dimension; ++j)
            {
                TmpPoints_[TmpPoints_.size() - 1].push_back(QPoint(j * x_interval, basey - h * (v[j] - Min_) / range));
            }
        }
        FocusedJ_ = FocusedI_ = -1;
    }
};

