#pragma once

#include <QGLWidget>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include <PSModel.h>
#include "glut.h"
#include "PixelInfoSet.h"
#include <QMimeData>
#include <QDrag>

class PixelInfoSetViewDialog;
class PixelInfoSetView : public QGLWidget
{
    friend class PixelInfoSetViewDialog;
    Q_OBJECT
signals:
    void paintUpdated();
public:
    PixelInfoSetView(PixelInfoSetViewDialog *parent);
    ~PixelInfoSetView();
    void SetData(PixelInfoSet pixels, const QRect &rect, const string &description, bool copy = false)
    {
        Clear();

        Region_ = rect;
        if (!copy)
            Pixels_ = pixels;
        else
        {
            Pixels_ = PixelInfoSet();
            Pixels_->assign(pixels->begin(), pixels->end());
        }
        Description_ = description;
        ResetModelViewMatrix();
        Update();
    }

    void SaveImage(const QString &file_or_dir_name, bool only_self = true)
    {
        if (only_self)
        {
            this->grabFrameBuffer().save(file_or_dir_name);
            ofstream of(FromQStringToStdString(file_or_dir_name) + ".html");
            of << Description_ << endl;
            of.close();
        }
        else
        {
            QDir dir(file_or_dir_name);

            if (ConnectedViews_ == nullptr)
                SaveImage(GetNewFileName("", ".png", dir), true);
            else
            {
                int index = 0;
                for each (PixelInfoSetView *view in *ConnectedViews_)
                    view->SaveImage(GetNewFileName("", ".png", dir), true);
            }
        }
    }


    string GetDescription() const { return Description_; }

    void SetData(const string &file_name)
    {
        Clear();
        loadFromFile(file_name);
        ResetModelViewMatrix();
        Update();
    }

    virtual void ResetModelViewMatrix() { }
    virtual void Clear() { }

    void DumpToFile(const string &file_or_dir_name, bool only_self = true)
    {
        if (only_self)
            DumpToFileInternal(file_or_dir_name);
        else
        {
            QDir dir(FromStdStringToQString(file_or_dir_name));

            if (ConnectedViews_ == nullptr)
                DumpToFile(FromQStringToStdString(GetNewFileName("", ".vdf", dir)), true);
            else
            {
                int index = 0;
                for each (PixelInfoSetView *view in *ConnectedViews_)
                    view->DumpToFile(FromQStringToStdString(GetNewFileName("", ".vdf", dir)), true);
            }
        }

    }


    QRect GetRegion() const { return Region_; }

    void Update()
    {
        update();
        ApplySettingToConnectedViews();
    }
protected:
    void DumpToFileInternal(const string & file_name);
    void keyReleaseEvent(QKeyEvent *e) override;

    // draging
    void mousePressEvent(QMouseEvent *event) override
    {
        __super::mousePressEvent(event);
        if (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier)
        {
            // data
            QByteArray data;
            QMimeData *mimeData = new QMimeData;
            mimeData->setData(GetMimeType(), data);

            // drag
            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            QPixmap pixel(":/setting.png");
            pixel = pixel.scaledToWidth(28);
            drag->setPixmap(pixel);
            drag->setHotSpot(QPoint(30, 30));

            drag->exec(Qt::CopyAction);
        }
    }

    void dragEnterEvent(QDragEnterEvent *evt) override
    {
        __super::dragEnterEvent(evt);
        if (evt->mimeData()->hasFormat(GetMimeType()))
        {
            if (evt->source() == this)
            {
                evt->ignore();
                return;
            }
            evt->acceptProposedAction();
        }
    }

    void dropEvent(QDropEvent *evt) override
    {
        __super::dropEvent(evt);

        QByteArray data = evt->mimeData()->data(GetMimeType());
        auto *source = static_cast<PixelInfoSetView *>(evt->source());
        if (IsConnected(source))
        {
            if (QMessageBox::question(this, "Disconnect...", "Disconnect this view from the source view?") ==
                QMessageBox::Yes)
                source->Disconnect(this); // not this->Disconnect(source);
        }
        else
        {
            Connect(source);
            source->ApplySettingToConnectedViews();
        }
    }

    virtual void paintGL() override
    {
        __super::paintGL();
        render();
    }

    virtual void resizeEvent(QResizeEvent*evt) override
    {
        __super::resizeEvent(evt);
        ApplySettingToConnectedViews();
    }
    virtual void render() = 0; 
private:
    void loadFromFile(const string &file_name)
    {
        ifstream ifile(file_name, ios::in | ios::binary);
        int x, y, w, h;
        Read(ifile, x);
        Read(ifile, y);
        Read(ifile, w);
        Read(ifile, h);
        Region_ = QRect(QPoint(x, y), QSize(w, h));
        Read(ifile, this->Pixels_);
        Read(ifile, this->Description_);
    }

protected:
    // Persistent Data
    PixelInfoSet Pixels_;
    QRect Region_;
    string Description_;

    bool SkipDarkPixel_;

    void paintEvent(QPaintEvent* evt) override
    {
        __super::paintEvent(evt);
        emit paintUpdated();
    }

    PixelInfoSetViewDialog *Parent_;

// connections
protected:
    shared_ptr<vector<PixelInfoSetView *>> ConnectedViews_;
private:
    void Connect(PixelInfoSetView *view)
    {
        if (view == this)
            return;
        if (typeid(*view) != typeid(*this))
            return;
        if (view->ConnectedViews_ == nullptr && ConnectedViews_ == nullptr)
        {
            view->ConnectedViews_ = ConnectedViews_ = make_shared<vector<PixelInfoSetView *>>();
            ConnectedViews_->push_back(this);
            ConnectedViews_->push_back(view);
        }
        else if (view->ConnectedViews_ == nullptr)
        {
            ConnectedViews_->push_back(view);
            view->ConnectedViews_ = ConnectedViews_;
        }
        else if (ConnectedViews_ == nullptr)
        {
            view->ConnectedViews_->push_back(this);
            ConnectedViews_ = view->ConnectedViews_;
        }
        else
        {
            view->ConnectedViews_->insert(view->ConnectedViews_->end(), ConnectedViews_->begin(), ConnectedViews_->end());
            auto this_array = ConnectedViews_;
            for (auto i = this_array->begin(); i != this_array->end(); ++i)
                (*i)->ConnectedViews_ = view->ConnectedViews_;
        }
    }

    void Disconnect(PixelInfoSetView *view)
    {
        if (view == this)
            return;
        // Because this functions will be called during the destruction of the PixelInfoSetView where the typeid(*view) will certainly be typeid(PixelInfoSetView), 
        // the if expression will be false, which is incorrect.
        // Hence remove the if expression.
        /*if (typeid(*view) != typeid(*this))
            return;*/
        if (ConnectedViews_ == nullptr)
            return;
        auto index = ::std::find(ConnectedViews_->begin(), ConnectedViews_->end(), view);
        if (index == ConnectedViews_->end())
            return;
        if (ConnectedViews_->size() == 2)
            ConnectedViews_ = view->ConnectedViews_ = nullptr;
        else
        {
            view->ConnectedViews_ = nullptr;
            ConnectedViews_->erase(index);
        }
    }

    bool IsConnected(PixelInfoSetView *view)
    {
        if (view == this)
            return false;
        if (ConnectedViews_ == nullptr)
            return false;
        return ::std::find(ConnectedViews_->begin(), ConnectedViews_->end(), view) != ConnectedViews_->end();
    }
    void ApplySettingToConnectedViews()
    {
        if (ConnectedViews_ != nullptr)
        {
            for (auto i = ConnectedViews_->begin(); i != ConnectedViews_->end(); ++i)
                if (*i != this)
                    ApplySetting(*i);
        }
    }

protected:
    virtual void ApplySetting(PixelInfoSetView *view) { view->SkipDarkPixel_ = SkipDarkPixel_; }

    virtual char *GetMimeType() const { return nullptr; }

    QString GetNewFileName(const QString &prefix, const QString &extention, const QDir &dir)
    {
        int number = 0;
        while (true)
        {
            QString name = dir.filePath(prefix + QString::number(number) + extention);
            if (!QFileInfo(name).exists())
                return name;
            ++number;
        }
    }
};

