#include "stdafx.h"

#include "RelightningThread.h"
#include "PSModel.h"

void RelightningThread::run()
{
    Owner_->LoadObjectPixelNormals();
}
