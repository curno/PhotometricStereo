#pragma once

#include "PSModel.h"
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class MainWindow;

class ImageView : public QWidget
{
    Q_OBJECT
public:
    class State abstract 
    {
    protected:
        State() { }
        State(const State &);
    public:
        virtual void paintEvent(ImageView *owner) = 0;
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *event) = 0;
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *event) = 0;
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) = 0;
    };

    class NoneState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static NoneState Instance_;
            return &Instance_;
        }
    };
    class CheckNormalState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override;
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override;
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override;

        void ShowMatchedPixel( ImageView * owner, QMouseEvent * e );

        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static CheckNormalState Instance_;
            return &Instance_;
        }
    };

    class PlotCircleState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(QPen(QBrush(Qt::green), 2.5));
            if (!owner->PlotCircleCenter.isNull() && !owner->PlotCircleRadius.isNull())
            {
                CircleType c = owner->GetPlotCircle();
                painter.drawEllipse(owner->PlotCircleCenter, (int)c.Z, (int)c.Z);
                painter.drawPoint(owner->PlotCircleCenter);
                painter.drawPoint(owner->PlotCircleRadius);
            }
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotCircleCenter = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotCircleRadius = e->pos();
            }
            owner->update();
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotCircleCenter = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotCircleRadius = e->pos();
            }
            owner->update();
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static PlotCircleState Instance_;
            return &Instance_;
        }
    };

    class RelightningState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(Qt::green);
            int x = owner->RelightingStart.x() < owner->RelightingEnd.x() ? owner->RelightingStart.x() : owner->RelightingEnd.x();
            int y = owner->RelightingStart.y() < owner->RelightingEnd.y() ? owner->RelightingStart.y() : owner->RelightingEnd.y();
            int w = qAbs(owner->RelightingEnd.x() - owner->RelightingStart.x());
            int h = qAbs(owner->RelightingEnd.y() - owner->RelightingStart.y());
            painter.drawRect(QRect(x, y, w, h));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(Qt::green));
            painter.drawRect(QRect(owner->RelightingStart.x() - Radius, owner->RelightingStart.y() - Radius, 2 * Radius, 2 * Radius));
            painter.drawRect(QRect(owner->RelightingEnd.x() - Radius, owner->RelightingEnd.y() - Radius, 2 * Radius, 2 * Radius));
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (NearPoint(owner->RelightingStart, e->pos()))
            {
                owner->EndFlag = ImageView::StartEnd;
                owner->RelightingStart = e->pos();
            }
            else if (NearPoint(owner->RelightingEnd, e->pos()))
            {
                owner->EndFlag = ImageView::EndEnd;
                owner->RelightingEnd = e->pos();
            }
            else
            {
                owner->EndFlag = ImageView::NoEnd;
                owner->RelightingStart = e->pos();
                owner->RelightingEnd = e->pos();
            }
            owner->update();
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() & Qt::LeftButton)
            {
                switch (owner->EndFlag)
                {
                case ImageView::StartEnd:
                    owner->RelightingStart = e->pos();
                    break;
                case ImageView::NoEnd:
                case ImageView::EndEnd: // fall through
                    owner->RelightingEnd = e->pos();
                default:
                    break;
                }
            }
            owner->update();
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static RelightningState Instance_;
            return &Instance_;
        }

    private:
        bool NearPoint(const QPoint &center, const QPoint &p)
        {
            return qAbs(center.x() - p.x()) <= Radius && qAbs(center.y() - p.y()) <= Radius;
        }
        static const int Radius = 3;

    };

    class CheckShadowState : public State
    { 
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setBrush(QBrush(Qt::green));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(owner->LocationToCheckShadow, PointSize, PointSize);

            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPoint(owner->Model->Configuration.ObjectLoadingCircle.X, owner->Model->Configuration.ObjectLoadingCircle.Y), 
                (int)owner->Model->Configuration.ObjectLoadingCircle.Z, (int)owner->Model->Configuration.ObjectLoadingCircle.Z);
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override;
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override;
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static CheckShadowState Instance_;
            return &Instance_;
        }
    };

    class PlotTargetCircleState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(QPen(QBrush(Qt::darkYellow), 2.5));
            if (!owner->PlotTargetCircleCenter.isNull() && !owner->PlotTargetCircleRadius.isNull())
            {
                CircleType c = owner->GetPlotTargetCircle();
                painter.drawEllipse(owner->PlotTargetCircleCenter, (int)c.Z, (int)c.Z);
                painter.drawPoint(owner->PlotTargetCircleRadius);
                painter.drawPoint(owner->PlotTargetCircleCenter);
            }

            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            painter.drawEllipse(QPoint(owner->Model->Configuration.ObjectLoadingCircle.X, owner->Model->Configuration.ObjectLoadingCircle.Y), (int)owner->Model->Configuration.ObjectLoadingCircle.Z, owner->Model->Configuration.ObjectLoadingCircle.Z);
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetCircleCenter = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetCircleRadius = e->pos();
            }
            owner->update();
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetCircleCenter = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetCircleRadius = e->pos();
            }
            owner->update();
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static PlotTargetCircleState Instance_;
            return &Instance_;
        }
    };
    friend class State;
    friend class NoneState;
    friend class CheckNormalState;
    friend class CheckShadowState;
    friend class RelightningState;
    friend class PlotCircleState;
    friend class PlotTargetCircleState;
public:
    void SetCurrentState(State *state)
    {
        CurrentStates.clear();
        CurrentStates.push_back(state);
        update();
    }
    void SetCurrentIndex(int index)
    {
        CurrentIndex = index;
        update();
    }
    ImageView(QWidget *parent);
    ~ImageView();
    int CurrentIndex;
    CubeType CurrentGroup;
    MainWindow *UI;
    PSModel *Model;
private:
    vector<State *> CurrentStates;
    QPoint LocationToCheck;
    QPoint LocationResponding;
    QPoint LastLocationToCheck;

    QPoint RelightingStart;
    QPoint RelightingEnd;
    enum {NoEnd, StartEnd, EndEnd} EndFlag; // 0 for no end, 1 for start, 2 for end

    QPoint PlotCircleCenter;
    QPoint PlotCircleRadius;

    QPoint PlotTargetCircleCenter;
    QPoint PlotTargetCircleRadius;

    QPoint LocationToCheckShadow;
public:
    CircleType GetPlotCircle() const
    {
        return CircleType(PlotCircleCenter.x(), PlotCircleCenter.y(), std::sqrt((PlotCircleCenter.x() - PlotCircleRadius.x()) * (PlotCircleCenter.x() - PlotCircleRadius.x()) + 
            (PlotCircleCenter.y() - PlotCircleRadius.y()) * (PlotCircleCenter.y() - PlotCircleRadius.y())));
    }

    CircleType GetPlotTargetCircle() const
    {
        return CircleType(PlotTargetCircleCenter.x(), PlotTargetCircleCenter.y(), std::sqrt((PlotTargetCircleCenter.x() - PlotTargetCircleRadius.x()) * (PlotTargetCircleCenter.x() - PlotTargetCircleRadius.x()) + 
            (PlotTargetCircleCenter.y() - PlotTargetCircleRadius.y()) * (PlotTargetCircleCenter.y() - PlotTargetCircleRadius.y())));
    }
    QRect GetRelightingRect() const
    {
        int x = RelightingStart.x() < RelightingEnd.x() ? RelightingStart.x() : RelightingEnd.x();
        int y = RelightingStart.y() < RelightingEnd.y() ? RelightingStart.y() : RelightingEnd.y();
        int w = qAbs(RelightingEnd.x() - RelightingStart.x());
        int h = qAbs(RelightingEnd.y() - RelightingStart.y());
        return QRect(x, y, w, h);
    }
protected:
    virtual void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);

        if (Model == nullptr)
            return;
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing);
        QImage *image = Model->GetImage(CurrentGroup, CurrentIndex);
        if (image != nullptr)
            painter.drawImage(0, 0, *image);

        if (image != nullptr)
        {
            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            auto c = GetPlotCircle();
            painter.drawEllipse(QPoint(c.X, c.Y), 
                (int)c.Z, (int)c.Z);
        }

        Q_FOREACH (State *state, CurrentStates)
            state->paintEvent(this);

    }

    virtual void mousePressEvent(QMouseEvent *e) override
    {
        if (!Model)
            return;
        Q_FOREACH (State *state, CurrentStates)
            state->mousePressEvent(this, e);
    }

    virtual void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!Model)
            return;
        Q_FOREACH (State *state, CurrentStates)
            state->mouseMoveEvent(this, e);
    }

    virtual void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (!Model)
            return;
        Q_FOREACH (State *state, CurrentStates)
            state->mouseReleaseEvent(this, e);
    }
private:
    static const int PointSize = 2;
};