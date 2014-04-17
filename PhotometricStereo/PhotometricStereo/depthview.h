#pragma once

#include "PixelInfoSetView.h"
#include <QMimeData>
#include <QDrag>

class DepthView : public PixelInfoSetView
{
    Q_OBJECT

public:

    DepthView(PixelInfoSetViewDialog *parent);

    ~DepthView() { Clear(); }

    void ResetModelViewMatrix() override  
    {
        ModelViewMatrix_.setToIdentity();
        scale(1.0, -1.0, 1.0);
        translate(0.0, Region_.height(), 0.0);
        scaleBasedOnCenter(0.618);
    }
    
protected:
    void keyReleaseEvent(QKeyEvent *e) override
    {
        __super::keyReleaseEvent(e);
        if (e->isAccepted())
            return;
        if (e->key() == Qt::Key_Space)
        {
            RenderMode_ = RenderMode((RenderMode_ + 1) % ModeCount);
            e->accept();
            Update();
        }
        else if (e->key() == Qt::Key_R)
        {
            ZScaleFactor_ = 1.0;
            ResetModelViewMatrix();
            e->accept();
            Update();
        }
    }

    void keyPressEvent(QKeyEvent * e) override
    {
        const double factor = 1.05;
        __super::keyPressEvent(e);
        if (e->isAccepted())
            return;
        if (e->key() == Qt::Key_Up)
        {
            ZScaleFactor_ *= factor;
            e->accept();
            Update();
        }
        else if (e->key() == Qt::Key_Down)
        {
            ZScaleFactor_ /= factor;
            e->accept();
            Update();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override
    {
        QGLWidget::mouseMoveEvent(event);
        QPoint p = LastPoint_;
        LastPoint_ = event->pos();
        if (event->buttons() == Qt::LeftButton)
        {
            translateByScreenCoord((LastPoint_.x() - p.x()), -(LastPoint_.y() - p.y()));
            Update();
        }
        else if (event->buttons() == Qt::RightButton)
        {
            rotateBasedOnCenter((LastPoint_.x() - p.x()) / 2.5, (LastPoint_.y() - p.y()) / 2.5);
            Update();
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        __super::mousePressEvent(event);
        LastPoint_ = event->pos();
    }

    
    void wheelEvent(QWheelEvent *event) override
    {
        QGLWidget::wheelEvent(event);
        QPoint p = event->angleDelta();
        if (p.y() > 0)
            scaleBasedOnCenter(1 + p.y() / 100.0);
        else 
            scaleBasedOnCenter(1 / (1 + ::std::abs(p.y()) / 100.0));
        Update();

    }
    void render() override
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);

        if (Pixels_->size() == 0)
            return;
       
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        (this->*(RenderingMethods_[RenderMode_]))(); 

    }

    virtual void initializeGL() override
    {
        __super::initializeGL();

    }

    virtual void resizeGL(int w, int h) override
    {
        __super::resizeGL(w, h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (static_cast<double>(w) / h >= static_cast<double>(Region_.width()) / Region_.height())
        {
            ViewCube_[0] = (Region_.width() - Region_.height() * static_cast<double>(w) / h) / 2.0;
            ViewCube_[1] = (Region_.width() + Region_.height() * static_cast<double>(w) / h) / 2.0;
            ViewCube_[2] = 0; 
            ViewCube_[3] = Region_.height();  
            ViewCube_[4] = -100.0 * Region_.width(); 
            ViewCube_[5] = 100.0 * Region_.width();
        }
        else
        {
            ViewCube_[0] = 0;
            ViewCube_[1] = Region_.width();
            ViewCube_[2] = (Region_.height() - Region_.width() * static_cast<double>(h) / w) / 2.0;
            ViewCube_[3] = (Region_.height() + Region_.width() * static_cast<double>(h) / w) / 2.0;
            ViewCube_[4] = -100.0 * Region_.height();
            ViewCube_[5] = 100.0 * Region_.height();
        }

        glOrtho(ViewCube_[0], ViewCube_[1], ViewCube_[2], ViewCube_[3], ViewCube_[4], ViewCube_[5]);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

    }

protected:
    // connections
    virtual void ApplySetting(PixelInfoSetView *view) override;

    virtual char *GetMimeType() const override
    {
        return "application/{C5BD01BB-AC45-4ED3-B045-3709AF80AA54}";
    }

private:
    // transformation
    void translate(double x, double y, double z)
    {
        QMatrix4x4 tmp;
        tmp.translate(x, y, z);
        ModelViewMatrix_ = tmp * ModelViewMatrix_;
    }

    void rotate(double x, double y)
    {
        QMatrix4x4 tmp;
        tmp.rotate(x, 0, 1, 0);
        tmp.rotate(y, 1, 0, 0);
        ModelViewMatrix_ = tmp * ModelViewMatrix_;
    }

    void scale(double factor)
    {
        scale(factor, factor, factor);
    }

    void scale(double x, double y, double z)
    {
        QMatrix4x4 tmp;
        tmp.scale(x, y, z);
        ModelViewMatrix_ = tmp * ModelViewMatrix_;
    }

    void scaleBasedOnCenter(double x, double y, double z)
    {
        QVector4D current = ModelViewMatrix_ * QVector4D(Region_.width() / 2.0, Region_.height() / 2.0, 0.0, 1.0);
        translate(-current.x() / current.w(), -current.y() / current.w(), -current.z() / current.w());
        scale(x, y, z);
        translate(current.x() / current.w(), current.y() / current.w(), current.z() / current.w());
    }

    void scaleBasedOnCenter(double factor)
    {
        scaleBasedOnCenter(factor, factor, factor);
    }
    void rotateBasedOnCenter(double x, double y)
    {
        QVector4D current = ModelViewMatrix_ * QVector4D(Region_.width() / 2.0, Region_.height() / 2.0, 0.0, 1.0);
        translate(-current.x() / current.w(), -current.y() / current.w(), -current.z() / current.w());
        rotate(x, y);
        translate(current.x() / current.w(), current.y() / current.w(), current.z() / current.w());
    }

    void translateByScreenCoord(double dx, double dy)
    {
        int w = width();
        int h = height();
        double rx = dx / static_cast<double>(w);
        double ry = dy / static_cast<double>(h);
        double xx = rx * (ViewCube_[1] - ViewCube_[0]);
        double yy = ry * (ViewCube_[3] - ViewCube_[2]);
        translate(xx, yy, 0);
    }

    // rendering
    void MeshModeRendering() 
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);

        RenderingWithMesh();

    }

    void RenderingWithMesh() 
    {
        glLoadMatrixf(ModelViewMatrix_.constData());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3d(1.0, 1.0, 1.0);

        int h = Region_.height();
        int w = Region_.width();

        for (int i = 0; i < h - 1; i++)
        {
            for (int j = 0; j < w - 1; j++)
            {
                PixelInfo &ij1 = Pixels_[i * w + j];
                PixelInfo &ij2 = Pixels_[(i + 1) * w + j];
                PixelInfo &ij3 = Pixels_[(i + 1) * w + j + 1];
                PixelInfo &ij4 = Pixels_[i * w + j + 1];

                if (SkipDarkPixel_ && (ij1.DarkPixel() && ij2.DarkPixel() && ij3.DarkPixel() && ij4.DarkPixel()))
                    continue;
                glColor3d(0.2, 0.8, 0.2);
                glBegin(GL_QUADS);
                glNormal3d(ij1.Normal.X, ij1.Normal.Y, ij1.Normal.Z);
                glVertex3d(ij1.Position.X, 
                    ij1.Position.Y, 
                    ij1.Position.Z * ZScaleFactor_);
                glNormal3d(ij2.Normal.X, ij2.Normal.Y, ij2.Normal.Z);
                glVertex3d(ij2.Position.X, 
                    ij2.Position.Y, 
                    ij2.Position.Z * ZScaleFactor_);

                glNormal3d(ij3.Normal.X, ij3.Normal.Y, ij3.Normal.Z);
                glVertex3d(ij3.Position.X, 
                    ij3.Position.Y, 
                    ij3.Position.Z * ZScaleFactor_);

                glNormal3d(ij4.Normal.X, ij4.Normal.Y, ij4.Normal.Z);
                glVertex3d(ij4.Position.X, 
                    ij4.Position.Y, 
                    ij4.Position.Z * ZScaleFactor_);
                glEnd();

                glColor3d(1.0, 1.0, 1.0);
                glBegin(GL_LINE_LOOP);
                glNormal3d(ij1.Normal.X, ij1.Normal.Y, ij1.Normal.Z);
                glVertex3d(ij1.Position.X, 
                    ij1.Position.Y, 
                    ij1.Position.Z * ZScaleFactor_);
                glNormal3d(ij2.Normal.X, ij2.Normal.Y, ij2.Normal.Z);
                glVertex3d(ij2.Position.X, 
                    ij2.Position.Y, 
                    ij2.Position.Z * ZScaleFactor_);

                glNormal3d(ij3.Normal.X, ij3.Normal.Y, ij3.Normal.Z);
                glVertex3d(ij3.Position.X, 
                    ij3.Position.Y, 
                    ij3.Position.Z * ZScaleFactor_);

                glNormal3d(ij4.Normal.X, ij4.Normal.Y, ij4.Normal.Z);
                glVertex3d(ij4.Position.X, 
                    ij4.Position.Y, 
                    ij4.Position.Z * ZScaleFactor_);
                glEnd();
            }
        }
    }

    void RelightningModeRendering() 
    {
        PrepareLightning();
        RenderingWithNormal();
    }

    void PrepareLightning() 
    {
        GLfloat specular [] = {1.0, 1.0, 1.0, 1.0};
        GLfloat shininess [] = {50.0};
        GLfloat light_position [] = {1.0, 1.0, 1.0, 0.0};
        GLfloat white_light [] = {1.0, 1.0, 1.0, 1.0};
        GLfloat lmodel_ambient[] = {0.1, 0.1, 0.1, 1.0};
        glShadeModel(GL_SMOOTH);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
        glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glEnable(GL_NORMALIZE);
    }

    void RenderingWithNormal() 
    {
        glLoadMatrixf(ModelViewMatrix_.constData());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3d(1.0, 1.0, 1.0);

        int h = Region_.height();
        int w = Region_.width();

        for (int i = 0; i < h - 1; i++)
        {
            for (int j = 0; j < w - 1; j++)
            {
                PixelInfo &ij1 = Pixels_[i * w + j];
                PixelInfo &ij2 = Pixels_[(i + 1) * w + j];
                PixelInfo &ij3 = Pixels_[(i + 1) * w + j + 1];
                PixelInfo &ij4 = Pixels_[i * w + j + 1];

                if (SkipDarkPixel_ && (ij1.DarkPixel() && ij2.DarkPixel() && ij3.DarkPixel() && ij4.DarkPixel()))
                    continue;
                glBegin(GL_QUADS);
                glNormal3d(ij1.Normal.X, ij1.Normal.Y, ij1.Normal.Z);
                glVertex3d(ij1.Position.X, 
                    ij1.Position.Y, 
                    ij1.Position.Z * ZScaleFactor_);
                glNormal3d(ij2.Normal.X, ij2.Normal.Y, ij2.Normal.Z);
                glVertex3d(ij2.Position.X, 
                    ij2.Position.Y, 
                    ij2.Position.Z * ZScaleFactor_);

                glNormal3d(ij3.Normal.X, ij3.Normal.Y, ij3.Normal.Z);
                glVertex3d(ij3.Position.X, 
                    ij3.Position.Y, 
                    ij3.Position.Z * ZScaleFactor_);

                glNormal3d(ij4.Normal.X, ij4.Normal.Y, ij4.Normal.Z);
                glVertex3d(ij4.Position.X, 
                    ij4.Position.Y, 
                    ij4.Position.Z * ZScaleFactor_);
                glEnd();
            }
        }
    }

    void ActualLightningModeRendering() 
    {
        PrepareLightning();
        RenderingWithActualNormal();
    }

    void RenderingWithActualNormal() 
    {
        glLoadMatrixf(ModelViewMatrix_.constData());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3d(1.0, 1.0, 1.0);

        int h = Region_.height();
        int w = Region_.width();

        for (int i = 0; i < h - 1; i++)
        {
            for (int j = 0; j < w - 1; j++)
            {
                PixelInfo &ij1 = Pixels_[i * w + j];
                PixelInfo &ij2 = Pixels_[(i + 1) * w + j];
                PixelInfo &ij3 = Pixels_[(i + 1) * w + j + 1];
                PixelInfo &ij4 = Pixels_[i * w + j + 1];

                if (SkipDarkPixel_ && (ij1.DarkPixel() && ij2.DarkPixel() && ij3.DarkPixel() && ij4.DarkPixel()))
                    continue;
                glBegin(GL_QUADS);
                glNormal3d(ZScaleFactor_ * ij1.ActualNormal.X, ZScaleFactor_ * ij1.ActualNormal.Y, ij1.ActualNormal.Z);
                glVertex3d(ij1.Position.X, 
                    ij1.Position.Y, 
                    ij1.Position.Z * ZScaleFactor_);
                glNormal3d(ZScaleFactor_ * ij2.ActualNormal.X, ZScaleFactor_ * ij2.ActualNormal.Y, ij2.ActualNormal.Z);
                glVertex3d(ij2.Position.X, 
                    ij2.Position.Y, 
                    ij2.Position.Z * ZScaleFactor_);

                glNormal3d(ZScaleFactor_ * ij3.ActualNormal.X, ZScaleFactor_ * ij3.ActualNormal.Y, ij3.ActualNormal.Z);
                glVertex3d(ij3.Position.X, 
                    ij3.Position.Y, 
                    ij3.Position.Z * ZScaleFactor_);

                glNormal3d(ZScaleFactor_ * ij4.ActualNormal.X, ZScaleFactor_ * ij4.ActualNormal.Y, ij4.ActualNormal.Z);
                glVertex3d(ij4.Position.X, 
                    ij4.Position.Y, 
                    ij4.Position.Z * ZScaleFactor_);
                glEnd();
            }
        }
    }

    QMatrix4x4 ModelViewMatrix_;
    double ViewCube_[6];
    QPoint LastPoint_;

    typedef void (DepthView::*RenderingMethod)();
    enum RenderMode { MeshMode, LightningMode, ActualLightningMode,     /* count */ ModeCount};
    RenderingMethod RenderingMethods_[ModeCount];
    RenderMode RenderMode_;
    double ZScaleFactor_;
};

