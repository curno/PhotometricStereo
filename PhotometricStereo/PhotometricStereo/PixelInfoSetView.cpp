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
        QString filter = "View dump files (*.vdf)";
        QString file_name = QFileDialog::getSaveFileName(this, tr("Dump to file..."), ".", filter, &filter, QFileDialog::DontUseNativeDialog);

        if (!file_name.toLower().endsWith(".vdf"))
            file_name += ".vdf";
        DumpToFile(FromQStringToStdString(file_name));
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
