#include "stdafx.h"

#include "RelightningView.h"
#include "PixelInfoSetViewDialog.h"

RelightningView::RelightningView(PixelInfoSetViewDialog *parent) : PixelInfoSetView(parent), RenderMode_(RelightningMode)
{
    setMouseTracking(true);
}

void RelightningView::ApplySetting(PixelInfoSetView *view)
{
    __super::ApplySetting(view);
    RelightningView *that = dynamic_cast<RelightningView *>(view);
    if (that == nullptr)
        return;
    that->RenderMode_ = RenderMode_;
    copy(LightPosition_, LightPosition_ + sizeof(LightPosition_) / sizeof(LightPosition_[0]), that->LightPosition_);
    if (that->Parent_ != nullptr && Parent_ != nullptr)
        if (that->Parent_->size() != Parent_->size())
            that->Parent_->resize(Parent_->size());
    that->update();
}
