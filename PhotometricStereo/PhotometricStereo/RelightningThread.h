#pragma once

#include "stdafx.h"

class PSModel;
class RelightningThread : public QThread
{
    Q_OBJECT
    PSModel *Owner_;
protected:
    void run() override;
public:
    RelightningThread(QObject *parent, PSModel *owner) : QThread(parent), Owner_(owner) { }
    int ShadowThreshold;
};