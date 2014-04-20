#include "stdafx.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QLineEdit>
#include "relightningview.h"
#include "depthview.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), Model(nullptr), RelightningThread_(nullptr), ProgressDialog_(nullptr), ProgressTimer_(nullptr), MaxPositionUsed_(0)
{
    ui.setupUi(this);
    ui.imageView->UI = this;
    ui.imageView_ShadowRemoved->UI = this;

    LoadModel("");

    connect(ui.pushButtonHoughCircle, &QPushButton::clicked, this, &MainWindow::On_houghCircleButton_clicked);
    connect(ui.pushButtonShowObject, &QPushButton::clicked, this, &MainWindow::On_pushButtonShowObject_clicked);
    connect(ui.pushButtonRelightning, &QPushButton::clicked, this, &MainWindow::On_relightningButton_clicked);
    connect(ui.pushButtonSmoothNormal, &QPushButton::clicked, this, &MainWindow::On_pushButtonSmoothNormal_clicked);
    connect(ui.pushBUttonSmoothBorderNormal, &QPushButton::clicked, this, &MainWindow::On_pushButtonSmoothBorderNormal_clicked);
    connect(ui.pushButtonSwitch, &QPushButton::clicked, this, &MainWindow::On_switchButton_clicked);
    connect(ui.pushButtonSaveImageSet, &QPushButton::clicked, this, &MainWindow::On_pushButtonSaveImageSet_clicked);
    connect(ui.pushButtonGo, &QPushButton::clicked, this, &MainWindow::On_actionImage_Set_triggered);
    connect(ui.pushButtonLoadDepthView, &QPushButton::clicked, this, &MainWindow::On_pushButtonLoadDepthView_clicked);
    connect(ui.pushButtonPlotCircle, &QPushButton::clicked, this, &MainWindow::On_pushButtonPlotCircle);
    connect(ui.pushButtonPS, &QPushButton::clicked, this, &MainWindow::On_pushButtonPS__clicked);
    connect(ui.pushButtonShadowDifference, &QPushButton::clicked, this, &MainWindow::On_pushButtonShadowDifference_clicked);
    connect(ui.pushButtonNext, &QPushButton::clicked, this, &MainWindow::On_nextButton_clicked);
    connect(ui.pushButtonPrevious, &QPushButton::clicked, this, &MainWindow::On_previousButton_clicked);
    connect(ui.pushButtonCheckMatch, &QPushButton::clicked, this, &MainWindow::On_checkMatchButton_clicked);
    connect(ui.pushButtonDetectShadow, &QPushButton::clicked, this, &MainWindow::On_pushButtonDetectShadow_clicked);
    connect(ui.pushButtonPlotCircle, &QPushButton::clicked, this, &MainWindow::On_checkable_button_clicked);
    connect(ui.pushButtonShadowDifference, &QPushButton::clicked, this, &MainWindow::On_checkable_button_clicked);
    connect(ui.pushButtonCheckMatch, &QPushButton::clicked, this, &MainWindow::On_checkable_button_clicked);
    connect(ui.pushButtonPlotTargetObject, &QPushButton::clicked, this, &MainWindow::On_pushButtonPlotTargetCircle__clicked);
    connect(ui.pushButtonGroundTruth, &QPushButton::clicked, this, &MainWindow::On_pushButtonGroundTruth__clicked);
    connect(ui.pushButtonReconstructionError, &QPushButton::clicked, this, &MainWindow::On_pushButtonReconstructionDifference__clicked);
    
    ActionShowModelConfiguration_ = new QAction(this);
    ActionShowModelConfiguration_->setShortcut( Qt::Key_P);
    this->addAction(ActionShowModelConfiguration_);
    connect(ActionShowModelConfiguration_, &QAction::triggered, this, &MainWindow::On_actionShowModelConfiguration_triggered);

    LabelModelConfiguration_ = new QLabel(this);
    LabelModelConfiguration_->setWindowFlags(LabelModelConfiguration_->windowFlags() | Qt::Window | Qt::FramelessWindowHint);
    LabelModelConfiguration_->setWindowModality(Qt::NonModal);

    LabelModelConfigurationAnimation_ = new QPropertyAnimation(LabelModelConfiguration_, "windowOpacity", LabelModelConfiguration_);
    LabelModelConfigurationAnimation_->setDuration(500);
    LabelModelConfigurationAnimationGroup_ = new QSequentialAnimationGroup(LabelModelConfiguration_);
    LabelModelConfigurationAnimationPause_ = LabelModelConfigurationAnimationGroup_->addPause(2000);
    LabelModelConfigurationAnimationGroup_->addAnimation(LabelModelConfigurationAnimation_);
    connect(LabelModelConfigurationAnimationGroup_, &QAbstractAnimation::finished, [=]() { LabelModelConfiguration_->hide(); } );
}
MainWindow::~MainWindow()
{
    delete Model;
    //delete LabelModelConfiguration_;
}
void MainWindow::On_switchButton_clicked()
{
    if (Model != nullptr)
    {
        if (ui.imageView_ShadowRemoved->CurrentGroup == ShadowCube)
            ui.imageView_ShadowRemoved->CurrentGroup = DifferenceCube;
        else if (ui.imageView_ShadowRemoved->CurrentGroup == DifferenceCube)
            ui.imageView_ShadowRemoved->CurrentGroup = ShadowCube;
        auto shadow_set = Model->ImageData.ShadowCube;
        if (shadow_set != nullptr && shadow_set->C > 0)
            ui.imageView_ShadowRemoved->SetCurrentIndex(ui.imageView->CurrentIndex);
        else
            ui.imageView_ShadowRemoved->SetCurrentIndex(-1);
    }

}
void MainWindow::On_houghCircleButton_clicked()
{
    if (Model != nullptr)
    {
        if (Model->HoughCircle())
        {
            ui.imageView->PlotCircleCenter.setX(Model->Configuration.ObjectLoadingCircle.X);
            ui.imageView->PlotCircleCenter.setY(Model->Configuration.ObjectLoadingCircle.Y);
            ui.imageView->PlotCircleRadius.setX(Model->Configuration.ObjectLoadingCircle.X);
            ui.imageView->PlotCircleRadius.setY(Model->Configuration.ObjectLoadingCircle.Y + Model->Configuration.ObjectLoadingCircle.Z);
        }
    }
    ui.imageView->update();
}


void MainWindow::On_pushButtonShowObject_clicked()
{
    if (Model != nullptr)
    {
        if (Model->DetectObject())
        {
            ui.imageView->RelightingStart.setX(Model->Configuration.ObjectLoadingRegion.left());
            ui.imageView->RelightingStart.setY(Model->Configuration.ObjectLoadingRegion.top());
            ui.imageView->RelightingEnd.setX(Model->Configuration.ObjectLoadingRegion.right());
            ui.imageView->RelightingEnd.setY(Model->Configuration.ObjectLoadingRegion.bottom());
        }
    }
    ui.imageView->update();
}

void MainWindow::On_previousButton_clicked()
{
    if (Model != nullptr)
    {
        auto set = Model->ImageData.GetImageDataCube(ui.imageView->CurrentGroup);
        auto shadow_set = Model->ImageData.GetImageDataCube(ui.imageView_ShadowRemoved->CurrentGroup);

        if (ui.imageView->CurrentIndex >= 0)
        {
            ui.imageView->SetCurrentIndex((ui.imageView->CurrentIndex + set->C - 1) % set->C);
            if (shadow_set != nullptr && shadow_set->C > 0)
                ui.imageView_ShadowRemoved->SetCurrentIndex(ui.imageView->CurrentIndex);
            UpdateLabelIndex();
        }
    }

}
void MainWindow::On_nextButton_clicked()
{
    if (Model != nullptr)
    {
        auto set = Model->ImageData.GetImageDataCube(ui.imageView->CurrentGroup);
        auto shadow_set = Model->ImageData.GetImageDataCube(ui.imageView_ShadowRemoved->CurrentGroup);

        if (ui.imageView->CurrentIndex >= 0)
        {
            ui.imageView->SetCurrentIndex((ui.imageView->CurrentIndex + 1) % set->C);
            if (shadow_set != nullptr && shadow_set->C > 0)
                ui.imageView_ShadowRemoved->SetCurrentIndex(ui.imageView->CurrentIndex);
            UpdateLabelIndex();

        }
    }
}
void MainWindow::On_checkMatchButton_clicked()
{
    if (ui.pushButtonCheckMatch->isChecked())
    {
        ui.imageView->SetCurrentState(ImageView::CheckNormalState::Instance());
    }
    else
    {
        ui.imageView->SetCurrentState(ImageView::NoneState::Instance());
    }
    ui.imageView->update();
}
void MainWindow::On_actionImage_Set_triggered()
{
    QString dir = ui.lineEditFolder->text();
    if (dir.isEmpty())
    {
        dir = QFileDialog::getExistingDirectory(this, "Choose Image Set...", "F:\\Research\\Project\\Pictures", QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    }

    if (!dir.isEmpty())
    {
        ui.lineEditFolder->setText(dir);
        LoadModel(dir);
        update();
    }

}
void MainWindow::On_relightningButton_clicked(bool checked)
{
    if (checked)
    {
        ui.imageView->SetCurrentState(ImageView::RelightningState::Instance());
        ui.EnableAllWidgetBut(false, 3, ui.pushButtonRelightning, ui.lineEditShadowThreshold, ui.comboBoxPixelDistance);
    }
    else
    {
        ui.imageView->SetCurrentState(ImageView::NoneState::Instance());
        ui.imageView->update();
        if (Model != nullptr)
        {

            Model->Configuration.ObjectLoadingRegion = ui.imageView->GetRelightingRect();
            Model->Configuration.ShadowThreshold = ui.lineEditShadowThreshold->text().toInt();
            Model->Configuration.NormalSmoothing = 0;
            Model->Configuration.BorderNormalSmoothing = 0;
            Model->Configuration.PixelDistance = ModelConfiguration::PixelDistanceMethod(ui.comboBoxPixelDistance->currentIndex());
            Model->Configuration.ShadowUse = ModelConfiguration::ShadowUseMethod(ui.comboBoxShadowUse->currentIndex());
            Model->Configuration.ObjectLoadingCircle = ui.imageView->GetPlotCircle();
            Model->Configuration.UsingCuda = ui.checkBoxCuda->isChecked();
            // progress dialog
            // timer
            delete ProgressTimer_;
            ProgressTimer_ = new QTimer(this);
            ProgressTimer_->setInterval(100);
            connect(ProgressTimer_, &QTimer::timeout, this, &MainWindow::On_relightningThreadTimer_ticked);
            ProgressTimer_->start();

            delete ProgressDialog_;
            ProgressDialog_ = new QProgressDialog(QString("Estimating pixel normals, please wait..."), "Cancel", 0, 10000, this);
            ProgressDialog_->setWindowTitle("Photometric Stereo");
            ProgressDialog_->resize(600, ProgressDialog_->sizeHint().height());
            connect(ProgressDialog_, &QProgressDialog::canceled, this, &MainWindow::On_relightningThread_Canceled);
            ProgressDialog_->setWindowModality(Qt::NonModal);
            ProgressDialog_->setMinimumDuration(0);
            ProgressDialog_->show();

            // Thread
            delete RelightningThread_;
            RelightningThread_ = new RelightningThread(this, Model);
            connect(RelightningThread_, &QThread::finished, this, &MainWindow::On_relightningThread_finished);
            RelightningThread_->start();
            ui.EnableAllWidgetBut(false, 0);
        }
    }
    
}  

void MainWindow::On_pushButtonPlotCircle(bool checked)
{
    if (checked)
    {
        ui.imageView->SetCurrentState(ImageView::PlotCircleState::Instance());
    }
    else
    {
        ui.imageView->SetCurrentState(ImageView::NoneState::Instance());
        Model->Configuration.ObjectLoadingCircle = ui.imageView->GetPlotCircle();
        ui.imageView->update();
    }
}
void MainWindow::On_pushButtonPS__clicked(bool checked)
{
    if (Model != nullptr)
    {
        Model->LoadObjectDepth();
        ShowDepthView(true);
    }
    ui.imageView->update();
}
void MainWindow::On_pushButtonShadowDifference_clicked(bool checked)
{
    if (checked)
        ui.imageView_ShadowRemoved->SetCurrentState(ImageView::CheckShadowState::Instance());
    else
        ui.imageView_ShadowRemoved->SetCurrentState(ImageView::NoneState::Instance());
}
void MainWindow::On_pushButtonPlotTargetCircle__clicked(bool checked)
{
    if (checked)
    {
        switch (ui.comboBoxTargetObject->currentIndex())
        {
        case 0: //sphere
            ui.imageView->SetCurrentState(ImageView::PlotTargetCircleState::Instance());
            break;
        case 1: // cylinder
            ui.imageView->SetCurrentState(ImageView::PlotTargetCylinderState::Instance());
            break;
        case 2:// cone
            ui.imageView->SetCurrentState(ImageView::PlotTargetConeState::Instance());
            break;
        default:
            break;
        }
    }
    else
    {
        ui.imageView->SetCurrentState(ImageView::NoneState::Instance());
        if (Model != nullptr)
        { 
            if (ui.comboBoxTargetObject->currentIndex() == 0) // sphere
            {
                Model->LoadTargetBallPixels(ui.imageView->GetPlotTargetCircle());
            }
            else if (ui.comboBoxTargetObject->currentIndex() == 1) // cylinder
            {
                QRect region = ui.imageView->GetPlotTargetCylinder();
                Model->LoadTargetCylinderPixels(region.x(), region.y(), region.width(), region.height());
            }
            else if (ui.comboBoxTargetObject->currentIndex() == 2) //cone
            {
                QRect region = ui.imageView->GetPlotTargetCone();
                Model->LoadTargetConePixels(region.bottom(), region.top(), region.left(), region.right());
            }

           
        }
        
    }
}

void MainWindow::On_pushButtonGroundTruth__clicked()
{
    if (Model == nullptr)
        return;
    PixelInfoSet pixels = Model->GetGroundTruth();
    ShowRelightingView(pixels, Model->Configuration.ObjectLoadingRegion, "Ground Truth");
    ShowDepthView(pixels, Model->Configuration.ObjectLoadingRegion, "Ground Truth");
}


void MainWindow::On_pushButtonReconstructionDifference__clicked()
{
    if (Model == nullptr)
        return;
    double error = Model->ComputeAverageError();
    QMessageBox::information(this, "Error", QString("The average error angle is %0 degree.").arg(QString::number(error * 180.0 / PI, 'g', 6)));

    PixelInfoSet pixels = Model->GetGroundTruth();
    
    auto result = Model->GetReconstructDifference();
    QLabel *label = new QLabel(this);
    label->setPixmap(QPixmap::fromImage(result.first));
    label->setWindowFlags(Qt::Dialog);
    label->move(200, 200);
    label->show();
    ShowDepthView(result.second, Model->Configuration.ObjectLoadingRegion, "Difference");
    ui.imageView->update();
    

}


void MainWindow::On_pushButtonLoadDepthView_clicked()
{
    QStringList file_name = QFileDialog::getOpenFileNames(this, tr("Load View..."), ".", tr("View dump file (*.vdf)"), nullptr, QFileDialog::DontUseNativeDialog);
    Q_FOREACH (QString s, file_name)
    {
        ShowDepthView(FromQStringToStdString(s));
        ShowRelightingView(FromQStringToStdString(s));
    }
}

void MainWindow::On_checkable_button_clicked(bool checked)
{
    if (checked)
        ui.EnableAllWidgetBut(qobject_cast<QWidget *>(sender()), false);
    else
        ui.EnableAllWidgetBut(nullptr, true);
}
void MainWindow::On_pushButtonSmoothNormal_clicked()
{
    if (Model != nullptr)
    {
        int count = ui.lineEditSmoothNormal->text().toInt();
        Model->Configuration.NormalSmoothing += count;
        Model->SmoothObjectNormalField(count, false);
        ShowRelightingView();
        ui.lineEditSmoothNormal->setText(QString::number(1));
    }
}

void MainWindow::On_pushButtonSmoothBorderNormal_clicked()
{
    if (Model != nullptr)
    {
        int count = ui.lineEditSmoothNormal->text().toInt();
        Model->Configuration.BorderNormalSmoothing += count;
        Model->SmoothObjectNormalField(count, true);
        ShowRelightingView();
        ui.lineEditSmoothNormal->setText(QString::number(1));
    }
}




void MainWindow::On_relightningThread_finished()
{
    if (ProgressTimer_ != nullptr)
        ProgressTimer_->stop();
    if (ProgressDialog_ != nullptr)
        ProgressDialog_->hide();
    if (Model != nullptr && Model->Progress.IsAlive)
    {
        ShowRelightingView();
    }
    ui.EnableAllWidgetBut(true, 0);
}

void MainWindow::On_relightningThreadTimer_ticked()
{
    if (ProgressDialog_ != nullptr && Model != nullptr)
    {
        if (Model->Progress.Min != Model->Progress.Max)
            ProgressDialog_->setValue(ProgressDialog_->minimum() +
                (ProgressDialog_->maximum() - ProgressDialog_->minimum()) * 
                static_cast<double>(Model->Progress.Current - Model->Progress.Min) / 
                (Model->Progress.Max - Model->Progress.Min));
        else
        {
            ProgressDialog_->setRange(0, 0);
            ProgressDialog_->setMinimumDuration(0);
            ProgressDialog_->show();
            ProgressTimer_->stop();
        }
    }

}

void MainWindow::On_relightningThread_Canceled()
{
    if (Model != nullptr)
    {
        Model->Progress.IsAlive = false;
        if (ProgressTimer_ != nullptr)
            ProgressTimer_->stop();
        if (ProgressDialog_ != nullptr)
            ProgressDialog_->hide();
            
    }
    ui.EnableAllWidgetBut(true, 0);
}

void MainWindow::UpdateLabelIndex()
{
    ui.labelIndex->setText(QString("%1").arg(QString::number(ui.imageView->CurrentIndex)));
}

void MainWindow::On_pushButtonDetectShadow_clicked()
{
    if (Model != nullptr)
    {
        Model->Configuration.ShadowDetectionCircle = ui.imageView->GetPlotCircle();
        Model->Configuration.ShadowDetection = ModelConfiguration::ShadowDetectionMethod(ui.comboBoxShadowDetection->currentIndex());
        Model->CreateShadowRemovedImages();

        auto shadow_set = Model->ImageData.GetImageDataCube(ui.imageView_ShadowRemoved->CurrentGroup);

        if (ui.imageView->CurrentIndex >= 0)
        {
            if (shadow_set != nullptr && shadow_set->C > 0)
                ui.imageView_ShadowRemoved->SetCurrentIndex(ui.imageView->CurrentIndex);
        }
        ui.imageView->update();
        ui.imageView_ShadowRemoved->update();
    }
}

void MainWindow::ResetUI()
{
    ui.EnableAllWidgetBut(true, 0);
    ui.checkBoxCuda->setChecked(false);
    ui.pushButtonCheckMatch->setChecked(false);
    ui.pushButtonShadowDifference->setChecked(false);
    ui.pushButtonRelightning->setChecked(false);
    ui.pushButtonPlotCircle->setChecked(false);
    ui.lineEditShadowThreshold->setText(QString::number(20));
    ui.lineEditSmoothNormal->setText(QString::number(1));
    ui.comboBoxShadowDetection->setCurrentIndex(0);
    ui.comboBoxPixelDistance->setCurrentIndex(0);
}

void MainWindow::ShowRelightingView()
{
    ShowRelightingView(Model->ImageData.ObjectPixels, Model->Configuration.ObjectLoadingRegion, Model->Configuration.Description);
}

void MainWindow::ShowRelightingView(const string &file_name)
{
    PixelInfoSetViewDialog *view = new PixelInfoSetViewDialog(this, new RelightningView(nullptr));
    view->SetData(file_name);
    view->show();
    view->Update();
    ui.viewHistory->AddViewHistory(view);
}

void MainWindow::ShowRelightingView(PixelInfoSet pixels, QRect region, const string &info)
{
    PixelInfoSetViewDialog *rv = new PixelInfoSetViewDialog(this, new RelightningView(nullptr));
    rv->SetData(pixels, region, info);
    rv->show();
    rv->Update();
    ui.viewHistory->AddViewHistory(rv);
}

void MainWindow::ShowDepthView(bool copy /*= false*/)
{
    ShowDepthView(Model->ImageData.ObjectPixels, Model->Configuration.ObjectLoadingRegion, Model->Configuration.Description, copy);
}

void MainWindow::ShowDepthView(const string &file_name)
{
    PixelInfoSetViewDialog *view = new PixelInfoSetViewDialog(this, new DepthView(nullptr));
    view->SetData(file_name);
    view->show();
    view->Update();

    ui.viewHistory->AddViewHistory(view);
}

void MainWindow::ShowDepthView( PixelInfoSet pixels, QRect region, const string &info, bool copy /*= false*/ )
{
    PixelInfoSetViewDialog *view = new PixelInfoSetViewDialog(this, new DepthView(nullptr));
    view->SetData(pixels, region, info, copy);
    view->show();
    view->Update();
    ui.viewHistory->AddViewHistory(view);
}

bool MainWindow::LoadModel(const QString &dir)
{
    ClearModel();
    ResetUI();
    if (dir.isEmpty())
        return false;
    if (!QDir(dir).exists())
        return false;

    Model = PSModel::CreateModel(FromQStringToStdString(dir));
    if (Model == nullptr)
        return false;
    ui.imageView->Model = Model;
    ui.imageView->CurrentGroup = NormalCube;
    ui.imageView->CurrentIndex = 0;

    ui.imageView_ShadowRemoved->Model = Model;
    ui.imageView_ShadowRemoved->CurrentGroup = ShadowCube;
    ui.imageView_ShadowRemoved->CurrentIndex = -1;

    ui.imageView->update();
    ui.imageView_ShadowRemoved->update();

    UpdateUI();

    return true;
}

void MainWindow::ClearVectorView()
{
    ui.vectorView->Clear();
}

void MainWindow::ClearModel()
{
    delete Model;
    ui.imageView->Model = nullptr;
    ui.imageView->CurrentGroup = NormalCube;
    ui.imageView->CurrentIndex = -1;

    ui.imageView_ShadowRemoved->Model = nullptr;
    ui.imageView_ShadowRemoved->CurrentGroup = ShadowCube;
    ui.imageView_ShadowRemoved->CurrentIndex = -1;

    ui.imageView->update();
    ui.imageView_ShadowRemoved->update();

    ui.vectorView->Clear();
    ui.ShadowLabel_->clear();
    ui.LightningVectorLabel_->clear();
}

void MainWindow::AddVectorToVectorView(PixelInfo vector, int index /*= -1*/)
{
    if (vector.IsInvalid)
        return;
    if (index == -1)
        ui.vectorView->Add(vector);
    else
        ui.vectorView->SetVector(vector, 2);
}


void MainWindow::On_actionShowModelConfiguration_triggered()
{
    if (Model == nullptr)
        return;
    LabelModelConfiguration_->setText(FromStdStringToQString(Model->Configuration.Description));
    if (!LabelModelConfiguration_->isVisible())
    {
        QRect r = this->geometry();
        QPoint p = r.center();
        QSize s = LabelModelConfiguration_->sizeHint();
        LabelModelConfiguration_->setGeometry(p.x() - s.width() / 2, p.y() - s.height() / 2, s.width(), s.height());
        LabelModelConfiguration_->show();
    }
    LabelModelConfiguration_->setWindowOpacity(1.0);
    LabelModelConfigurationAnimation_->setStartValue(1.0);
    LabelModelConfigurationAnimation_->setEndValue(0.0);
    LabelModelConfigurationAnimationGroup_->start();

}

void MainWindow::UpdateUI()
{
    UpdateLabelIndex();
    ui.imageView->setFixedSize(Model->GetSize());
    ui.imageView_ShadowRemoved->setFixedSize(Model->GetSize());
}

int MainWindow::AllocLocationIndex()
{
    int index = 0;
    if (!EmptySlots_.empty())
    {
        index = *EmptySlots_.begin();
        EmptySlots_.erase(EmptySlots_.begin());
    }
    else
    {
        index = MaxPositionUsed_;
        ++MaxPositionUsed_;
    }
    return index;
}

void MainWindow::LocateWidget(PixelInfoSetViewDialog *w)
{
    static QPoint Base(150, 150);
    static QSize Increment(20, 10);
    int index = AllocLocationIndex();
    QPoint location = Base + QPoint(Increment.width() * index, Increment.height() * index);
    LocationIndexMap_.insert(make_pair(w, index));
    w->setGeometry(QRect(location, w->size()));
}

void MainWindow::FreeLocation(PixelInfoSetViewDialog *w)
{
    auto i = LocationIndexMap_.find(w);
    if (i == LocationIndexMap_.end())
        return;
    int index = i->second;
    if (index == MaxPositionUsed_ - 1)
    {
        --MaxPositionUsed_;
        while (!EmptySlots_.empty() && *(--EmptySlots_.end()) == MaxPositionUsed_ - 1) 
        {
            EmptySlots_.erase(--EmptySlots_.end());
            MaxPositionUsed_--;
        } 
    }
    else
    {
        EmptySlots_.insert(index);
    }
    LocationIndexMap_.erase(i);
}

void MainWindow::On_widget_showing()
{
    LocateWidget(static_cast<PixelInfoSetViewDialog *>(sender()));
}

void MainWindow::On_widget_hiding()
{
    FreeLocation(static_cast<PixelInfoSetViewDialog *>(sender()));
}

void MainWindow::On_pushButtonSaveImageSet_clicked()
{
    if (Model != nullptr)
    {
        auto group_index = ui.imageView_ShadowRemoved->CurrentGroup;
        int index = ui.imageView_ShadowRemoved->CurrentIndex;
        if (index >= 0)
        {
            QString dir = QFileDialog::getExistingDirectory(this, "Image Set Folder", "", QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
            if (dir.isEmpty())
                return;
            Model->ImageData.GetImageDataCube(group_index)->SaveToFolder(dir);
        }
    }
}




