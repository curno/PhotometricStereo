#include "stdafx.h"
 
#include "PixelInfoSetView.h"
#include "PixelInfoSetViewDialog.h"

PixelInfoSetView::PixelInfoSetView(PixelInfoSetViewDialog *parent) : QGLWidget(parent), SkipDarkPixel_(false), Parent_(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
}

PixelInfoSetView::~PixelInfoSetView()
{
    if (ConnectedViews_ != nullptr)
    {
        for (auto i = ConnectedViews_->begin(); i != ConnectedViews_->end(); ++i)
        {
            if (*i != this)
            {
                (*i)->Disconnect(this);
                break;
            }
        }
    }
}

void PixelInfoSetView::keyReleaseEvent( QKeyEvent *e )
{
    __super::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Shift)
    {
        SkipDarkPixel_ = !SkipDarkPixel_;
        e->accept();
        Update();
    }
    else if (e->key() == Qt::Key_S && e->modifiers() & Qt::ControlModifier)
    {
        QString folder_name = QFileDialog::getExistingDirectory(this, tr("Dump to file..."), ".", QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);

        if (!folder_name.isEmpty())
        {
            DumpToFile(FromQStringToStdString(folder_name), false);
        }
        e->accept();
    }
    else if (e->key() == Qt::Key_I && e->modifiers() & Qt::ControlModifier)
    {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Save Images..."), ".", QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);

        if (!dir.isEmpty())
        {
            this->Update();
            SaveImage(dir, false);
        }
        e->accept();
    }
    else if (e->key() == Qt::Key_C)
    {
        if (Parent_ != nullptr)
            Parent_->ToggleConfigurationVisibility();
        Update();
        e->accept();
    }
}

void PixelInfoSetView::DumpToFileInternal(const string & file_name)
{
    ofstream of(file_name, ios::out | ios::binary);
    Write(of, Region_.left());
    Write(of, Region_.top());
    Write(of, Region_.width());
    Write(of, Region_.height());
    Write(of, Pixels_);
    Write(of, Description_);
}
