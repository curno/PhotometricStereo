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

    private:

    };

    class PlotTargetCircleState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(QPen(QBrush(Qt::yellow), 2.5));
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


    class PlotTargetCylinderState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(QPen(QBrush(Qt::yellow), 2.5));
            
            painter.drawRect(owner->PlotTargetCylinder);

            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            painter.drawEllipse(QPoint(owner->Model->Configuration.ObjectLoadingCircle.X, owner->Model->Configuration.ObjectLoadingCircle.Y), (int)owner->Model->Configuration.ObjectLoadingCircle.Z, owner->Model->Configuration.ObjectLoadingCircle.Z);
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetCylinder.setTopLeft(e->pos());
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetCylinder.setBottomRight(e->pos());
            }
            owner->update();
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetCylinder.setTopLeft(e->pos());
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetCylinder.setBottomRight(e->pos());
            }
            owner->update();
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static PlotTargetCylinderState Instance_;
            return &Instance_;
        }
    };

    class PlotTargetConeState : public State
    {
    public:
        virtual void paintEvent(ImageView *owner) override
        {
            QPainter painter(owner);
            painter.setPen(QPen(QBrush(Qt::yellow), 2.5));

            painter.drawLine(owner->PlotTargetConeP0, owner->PlotTargetConeP1);
            painter.drawLine(owner->PlotTargetConeP1, QPoint(2 * owner->PlotTargetConeP0.x() - owner->PlotTargetConeP1.x(), owner->PlotTargetConeP1.y()));
            painter.drawLine(owner->PlotTargetConeP0, QPoint(2 * owner->PlotTargetConeP0.x() - owner->PlotTargetConeP1.x(), owner->PlotTargetConeP1.y()));

            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            painter.drawEllipse(QPoint(owner->Model->Configuration.ObjectLoadingCircle.X, owner->Model->Configuration.ObjectLoadingCircle.Y), (int)owner->Model->Configuration.ObjectLoadingCircle.Z, owner->Model->Configuration.ObjectLoadingCircle.Z);
        }
        virtual void mousePressEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetConeP0 = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetConeP1 = e->pos();
            }
            owner->update();
        }
        virtual void mouseMoveEvent(ImageView *owner, QMouseEvent *e) override
        {
            if (e->buttons() == Qt::LeftButton)
            {
                owner->PlotTargetConeP0 = e->pos();
            }
            else if (e->buttons() == Qt::RightButton)
            {
                owner->PlotTargetConeP1 = e->pos();
            }
            owner->update();
        }
        virtual void mouseReleaseEvent(ImageView *owner, QMouseEvent *event) override
        {

        }

        static State *Instance()
        {
            static PlotTargetConeState Instance_;
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
public:
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

    QRect PlotTargetCylinder;

    QPoint PlotTargetConeP0;
    QPoint PlotTargetConeP1;

    QPoint LocationToCheckShadow;

    // show only object loading region.
    QRect ClipRect;
    bool Clip;
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

    QRect GetPlotTargetCylinder() const
    {
        return PlotTargetCylinder;
    }

    QRect GetPlotTargetCone() const 
    {
        return QRect(PlotTargetConeP1.x(), PlotTargetConeP0.y(), 2 * (PlotTargetConeP0.x() - PlotTargetConeP1.x()), PlotTargetConeP1.y() - PlotTargetConeP0.y());
    }

    QRect GetRelightingRect() const
    {
        int x = RelightingStart.x() < RelightingEnd.x() ? RelightingStart.x() : RelightingEnd.x();
        int y = RelightingStart.y() < RelightingEnd.y() ? RelightingStart.y() : RelightingEnd.y();
        int w = qAbs(RelightingEnd.x() - RelightingStart.x());
        int h = qAbs(RelightingEnd.y() - RelightingStart.y());
        return QRect(x, y, w, h);
    }

    void ResetUIData()
    {
        LastLocationToCheck = QPoint();
        LocationResponding = QPoint();
        LastLocationToCheck = QPoint();
        RelightingStart = QPoint();
        RelightingEnd = QPoint();
        PlotCircleCenter = QPoint();
        PlotCircleRadius = QPoint();
        PlotTargetCircleCenter = QPoint();
        PlotTargetCircleRadius = QPoint();
        PlotTargetCylinder = QRect();
        PlotTargetConeP0 = QPoint();
        PlotTargetConeP1 = QPoint();
        LocationToCheckShadow = QPoint();

        ClipRect = QRect();
        Clip = false;
    }

    QPoint UnClipPoint(const QPoint &point)
    {
        if (!Clip)
            return point;
        else
            return QPoint(point.x() + ClipRect.x(), point.y() + ClipRect.y());
    }

    bool PointInImage(const QPoint &point)
    {
        QPoint p = UnClipPoint(point);
        return QRect(0, 0, Model->ImageData.NormalCube->M, Model->ImageData.NormalCube->N).contains(point);
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
        {
            if (!Clip)
                painter.drawImage(0, 0, *image);
            else
                painter.drawImage(QPoint(0, 0), *image, ClipRect);
        }

        if (image != nullptr)
        {
            painter.setPen(QPen(QBrush(Qt::red), 2.5));
            auto c = GetPlotCircle();
            painter.drawEllipse(QPoint(c.X, c.Y), 
                (int)c.Z, (int)c.Z);

            painter.setPen(Qt::green);
            int x = RelightingStart.x() < RelightingEnd.x() ? RelightingStart.x() : RelightingEnd.x();
            int y = RelightingStart.y() < RelightingEnd.y() ? RelightingStart.y() : RelightingEnd.y();
            int w = qAbs(RelightingEnd.x() - RelightingStart.x());
            int h = qAbs(RelightingEnd.y() - RelightingStart.y());
            painter.drawRect(QRect(x, y, w, h));
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
    static const int PointSize = 3;
};
