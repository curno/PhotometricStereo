#include "stdafx.h"

#include "depthview.h"
#include "PixelInfoSetViewDialog.h"

DepthView::DepthView(PixelInfoSetViewDialog *parent) : PixelInfoSetView(parent), ZScaleFactor_(1.0), RenderMode_(MeshMode)
{
    RenderingMethods_[MeshMode] = &DepthView::MeshModeRendering;
    RenderingMethods_[LightningMode] = &DepthView::RelightningModeRendering;
    RenderingMethods_[ActualLightningMode] = &DepthView::ActualLightningModeRendering;
    setAcceptDrops(true);
}

void DepthView::ApplySetting(PixelInfoSetView *view)
{
    __super::ApplySetting(view);
    DepthView *that = dynamic_cast<DepthView *>(view);
    if (that == nullptr)
        return;
    that->ModelViewMatrix_ = ModelViewMatrix_;
    copy(ViewCube_, ViewCube_ + sizeof(ViewCube_) / sizeof(ViewCube_[0]), that->ViewCube_);
    that->ZScaleFactor_ = ZScaleFactor_;
    that->RenderMode_ = RenderMode_;
    if (that->Parent_ != nullptr && Parent_ != nullptr)
        if (that->Parent_->size() != Parent_->size())
            that->Parent_->resize(Parent_->size());
    that->update();
}
