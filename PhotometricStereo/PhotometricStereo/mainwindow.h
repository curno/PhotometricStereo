#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressDialog>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QTimer>
#include "imageview.h"
#include "PSModel.h"
#include "vectorview.h"
#include "MainWindowUI.h"
#include "VectorView.h"
#include "PixelInfoSetViewDialog.h"
#include "RelightningThread.h"
#include "ViewHistoryItemWidget.h"
#include "ViewHistoryWidget.h"

class ImageView;
class MainWindow : public QMainWindow
{
    friend class ImageView;
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void On_houghCircleButton_clicked();
    void On_previousButton_clicked();
    void On_nextButton_clicked();
    void UpdateLabelIndex();
    void On_checkMatchButton_clicked();
    void On_relightningButton_clicked(bool checked);
    void On_pushButtonSmoothNormal_clicked();
    void On_pushButtonSmoothBorderNormal_clicked();
    void On_switchButton_clicked();
    void On_pushButtonSaveImageSet_clicked();
    void On_actionImage_Set_triggered();
    void On_pushButtonPlotCircle(bool checked);
    void On_pushButtonPS__clicked(bool checked);
    void On_pushButtonDetectShadow_clicked();
    void On_pushButtonShadowDifference_clicked(bool checked);
    void On_pushButtonPlotTargetCircle__clicked(bool checked);
    void On_pushButtonReconstructionDifference__clicked();
    void On_pushButtonGroundTruth__clicked();
    void On_pushButtonLoadDepthView_clicked();
    void On_checkable_button_clicked(bool checked);
    void On_relightningThread_finished();
    void On_relightningThreadTimer_ticked();
    void On_relightningThread_Canceled();
    void On_actionShowModelConfiguration_triggered();
private:
    Ui::MainWindowClass ui;

    PSModel *Model;

    RelightningThread *RelightningThread_;
    QProgressDialog *ProgressDialog_;
    QTimer *ProgressTimer_;
    QAction *ActionShowModelConfiguration_;
    QLabel *LabelModelConfiguration_;
    QPropertyAnimation *LabelModelConfigurationAnimation_;
    QSequentialAnimationGroup *LabelModelConfigurationAnimationGroup_;
    QPauseAnimation *LabelModelConfigurationAnimationPause_;
private:
    void ResetUI();
    void UpdateUI();
    void ShowRelightingView();
    void ShowRelightingView(const string &file_name);
    void ShowRelightingView(PixelInfoSet pixels, QRect region, const string &info);
    void ShowDepthView(bool copy = false);
    void ShowDepthView(PixelInfoSet pixels, QRect region, const string &info, bool copy = false);
    void ShowDepthView(const string &file_name);
public:
    bool LoadModel(const QString &dir);
private:
    void ClearModel();
    void ClearVectorView();
    void AddVectorToVectorView(PixelInfo vector, int index = -1);

private: // popup views' location management.
    void LocateWidget(PixelInfoSetViewDialog *w);
    int AllocLocationIndex();
    void FreeLocation(PixelInfoSetViewDialog *w);
    int MaxPositionUsed_;
    set<int> EmptySlots_;
    map<PixelInfoSetViewDialog *, int> LocationIndexMap_;
public slots:
    void On_widget_showing();
    void On_widget_hiding();
    
};
