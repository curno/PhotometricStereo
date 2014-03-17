#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCombobox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QWidget>
#include "imageview.h"
#include "vectorview.h"
#include "ModelConfiguration.h"
#include "ViewHistoryWidget.h"

class Ui_MainWindowClass
{
public:
    ImageView *imageView;
    ImageView *imageView_ShadowRemoved;
    VectorView *vectorView;
    QLabel *LightningVectorLabel_;
    QLabel *LinearCombinationParameterLabel_;
    QLabel *ShadowLabel_;
    QStatusBar *statusBar;
    QLineEdit *lineEditFolder;
    QLineEdit *lineEditSmoothNormal;
    QLineEdit *lineEditShadowThreshold;
    QPushButton *pushButtonGo;
    QPushButton *pushButtonLoadDepthView;
    QPushButton *pushButtonHoughCircle;
    QPushButton *pushButtonPlotCircle;
    QComboBox *comboBoxShadowDetection; 
    QPushButton *pushButtonDetectShadow;
    QComboBox *comboBoxShadowUse;
    QComboBox *comboBoxPixelDistance;
    QPushButton *pushButtonRelightning;
    QPushButton *pushButtonSmoothNormal;
    QPushButton *pushButtonPS;
    QPushButton *pushButtonCheckMatch;
    QPushButton *pushButtonShadowDifference;
    QComboBox *comboBoxTargetObject;
    QPushButton *pushButtonPlotTargetObject;
    QPushButton *pushButtonSwitch;
    QPushButton *pushButtonSaveImageSet;
    QPushButton *pushButtonPrevious;
    QPushButton *pushButtonNext;
    QCheckBox *checkBoxCuda;
    QLabel *labelIndex;
    ViewHistoryWidget *viewHistory;
    vector<QWidget *> LogicWidgets_;


    void setupUi(QMainWindow *MainWindowClass)
    {
        static const int GroupSpacing = 10;
        if (MainWindowClass->objectName().isEmpty())
        MainWindowClass->resize(1411, 828);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        MainWindowClass->setFont(font);
        auto *centralWidget = new QWidget(MainWindowClass);
        QHBoxLayout *horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        auto *widget_2 = new QWidget(centralWidget);
        QHBoxLayout * horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        auto *widget = new QWidget(widget_2);
        widget->setMinimumSize(QSize(0, 400));
        QVBoxLayout *verticalLayout_3 = new QVBoxLayout(widget);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        QScrollArea *scrollArea = new QScrollArea(widget);
        scrollArea->setMinimumSize(QSize(0, 0));
        scrollArea->setMaximumSize(QSize(16777215, 16777215));
        scrollArea->setWidgetResizable(true);
        QWidget *scrollAreaWidgetContents = new QWidget();
        scrollArea->setObjectName("mainScene");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 1224, 551));
        QHBoxLayout *horizontalLayout_4 = new QHBoxLayout(scrollAreaWidgetContents);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        imageView = new ImageView(scrollAreaWidgetContents);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHeightForWidth(imageView->sizePolicy().hasHeightForWidth());
        imageView->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(imageView);

        imageView_ShadowRemoved = new ImageView(scrollAreaWidgetContents);

        horizontalLayout_4->addWidget(imageView_ShadowRemoved);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_3->addWidget(scrollArea);

        auto *widget3 = new QWidget(widget);
        widget3->setMinimumSize(QSize(0, 200));
        widget3->setMaximumSize(QSize(16777215, 200));
        auto *horizontalLayout_5 = new QHBoxLayout(widget3);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        vectorView = new VectorView(widget3);

        horizontalLayout_5->addWidget(vectorView);

        auto *verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        LightningVectorLabel_ = new QLabel(widget3);
        LightningVectorLabel_->setMinimumSize(QSize(0, 0));
        LightningVectorLabel_->setMaximumSize(QSize(16777215, 16777215));

        verticalLayout_2->addWidget(LightningVectorLabel_);

        LinearCombinationParameterLabel_ = new QLabel(widget3);

        verticalLayout_2->addWidget(LinearCombinationParameterLabel_);

        ShadowLabel_ = new QLabel(widget3);

        verticalLayout_2->addWidget(ShadowLabel_);
        horizontalLayout_5->addLayout(verticalLayout_2);
        verticalLayout_3->addWidget(widget3);
        horizontalLayout_2->addWidget(widget);
        horizontalLayout_3->addWidget(widget_2);

        MainWindowClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindowClass);
        MainWindowClass->setStatusBar(statusBar);
        
        #pragma region Dock Widget Tool Box
        auto *dockWidget_5 = new QDockWidget(MainWindowClass);
        dockWidget_5->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        auto *dockWidgetContents_5 = new QWidget();
        auto *verticalLayout = new QVBoxLayout(dockWidgetContents_5);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        auto *widget_3 = new QWidget(dockWidgetContents_5);
        auto * horizontalLayout_6 = new QHBoxLayout(widget_3);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        lineEditFolder = new QLineEdit(widget_3);

        horizontalLayout_6->addWidget(lineEditFolder);
        pushButtonGo = new QPushButton(widget_3);
        horizontalLayout_6->addWidget(pushButtonGo);

        horizontalLayout_6->setStretch(0, 1);
        horizontalLayout_6->setStretch(1, 0);

        verticalLayout->addWidget(widget_3);
        verticalLayout->addSpacing(GroupSpacing);

        

        pushButtonLoadDepthView = new QPushButton(widget_3);
        verticalLayout->addWidget(pushButtonLoadDepthView);
        verticalLayout->addSpacing(GroupSpacing);

        pushButtonHoughCircle = new QPushButton(dockWidgetContents_5);
        verticalLayout->addWidget(pushButtonHoughCircle);

        pushButtonPlotCircle = new QPushButton(dockWidgetContents_5);
        pushButtonPlotCircle->setCheckable(true);
        verticalLayout->addWidget(pushButtonPlotCircle);

        verticalLayout->addSpacing(GroupSpacing);

        comboBoxShadowDetection = new QComboBox(dockWidgetContents_5);
        for (int i = 0; i < static_cast<int>(ModelConfiguration::ShadowDetectionMethod::ShadowDetectionMethod_Count); ++i)
        {
            comboBoxShadowDetection->addItem(FromStdStringToQString(
                ModelConfiguration::ShadowDetetionMethodStr(
                ModelConfiguration::ShadowDetectionMethod(i))));
        }
        verticalLayout->addWidget(comboBoxShadowDetection);

        pushButtonDetectShadow = new QPushButton(dockWidgetContents_5);
        verticalLayout->addWidget(pushButtonDetectShadow);

        verticalLayout->addSpacing(GroupSpacing);

        QWidget *widget_tool_bar = new QWidget;
        QHBoxLayout *horizontalLayout_tool_bar = new QHBoxLayout(widget_tool_bar);
        horizontalLayout_tool_bar->setContentsMargins(0, 0, 0, 0);

        checkBoxCuda = new QCheckBox;
        checkBoxCuda->setCursor(Qt::PointingHandCursor);
        horizontalLayout_tool_bar->addWidget(checkBoxCuda);

        horizontalLayout_tool_bar->addStretch();
        verticalLayout->addWidget(widget_tool_bar);

        verticalLayout->addSpacing(GroupSpacing);

        comboBoxShadowUse = new QComboBox(dockWidgetContents_5);
        for (int i = 0; i < static_cast<int>(ModelConfiguration::ShadowUseMethod::ShadowUse_Count); ++i)
        {
            comboBoxShadowUse->addItem(FromStdStringToQString(
                ModelConfiguration::ShadowUseStr(
                ModelConfiguration::ShadowUseMethod(i))));
        }
        verticalLayout->addWidget(comboBoxShadowUse);

        comboBoxPixelDistance = new QComboBox(dockWidgetContents_5);
        for (int i = 0; i < static_cast<int>(ModelConfiguration::PixelDistanceMethod::PixelDistanceMethod_Count); ++i)
        {
            comboBoxPixelDistance->addItem(FromStdStringToQString(
                ModelConfiguration::PixelDistanceMethodStr(
                ModelConfiguration::PixelDistanceMethod(i))));
        }
        verticalLayout->addWidget(comboBoxPixelDistance);

        auto *widget_relightning = new QWidget;
        auto *horizontalLayout_ps = new QHBoxLayout(widget_relightning);
        horizontalLayout_ps->setContentsMargins(0, 0, 0, 0);
        lineEditShadowThreshold = new QLineEdit(dockWidgetContents_5);
        lineEditShadowThreshold->setValidator(new QIntValidator(0, 255, lineEditShadowThreshold));
        lineEditShadowThreshold->setText(QString::number(20));
        lineEditShadowThreshold->setFixedWidth(40);
        lineEditShadowThreshold->setFixedHeight(26);
        pushButtonRelightning = new QPushButton(dockWidgetContents_5);
        pushButtonRelightning->setCheckable(true);
        horizontalLayout_ps->addWidget(lineEditShadowThreshold, 0);
        horizontalLayout_ps->addWidget(pushButtonRelightning, 1);
        widget_relightning->setLayout(horizontalLayout_ps);
        verticalLayout->addWidget(widget_relightning);

        auto *widget_smooth_normal = new QWidget;
        auto *horizontalLaout_SmoothNormal = new QHBoxLayout(widget_smooth_normal);
        horizontalLaout_SmoothNormal->setContentsMargins(0, 0, 0, 0);
        lineEditSmoothNormal = new QLineEdit(dockWidgetContents_5);
        lineEditSmoothNormal->setValidator(new QIntValidator(1, 10, lineEditSmoothNormal));
        lineEditSmoothNormal->setText(QString::number(1));
        lineEditSmoothNormal->setFixedWidth(40);
        lineEditSmoothNormal->setFixedHeight(26);
        pushButtonSmoothNormal = new QPushButton(dockWidgetContents_5);
        horizontalLaout_SmoothNormal->addWidget(lineEditSmoothNormal, 0);
        horizontalLaout_SmoothNormal->addWidget(pushButtonSmoothNormal, 1);
        widget_smooth_normal->setLayout(horizontalLaout_SmoothNormal);
        verticalLayout->addWidget(widget_smooth_normal);

        pushButtonPS = new QPushButton(dockWidgetContents_5);
        verticalLayout->addWidget(pushButtonPS);

        auto *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        pushButtonCheckMatch = new QPushButton(dockWidgetContents_5);
        pushButtonCheckMatch->setCheckable(true);

        verticalLayout->addWidget(pushButtonCheckMatch);

        pushButtonShadowDifference = new QPushButton(dockWidgetContents_5);
        pushButtonShadowDifference->setCheckable(true);

        verticalLayout->addWidget(pushButtonShadowDifference);

        comboBoxTargetObject = new QComboBox;
        comboBoxTargetObject->addItem("Sphere");
        comboBoxTargetObject->addItem("Cylinder");
        comboBoxTargetObject->addItem("Cone");
        verticalLayout->addWidget(comboBoxTargetObject);

        pushButtonPlotTargetObject = new QPushButton(dockWidgetContents_5);
        pushButtonPlotTargetObject->setCheckable(true);

        verticalLayout->addWidget(pushButtonPlotTargetObject);

        pushButtonSwitch = new QPushButton(dockWidgetContents_5);

        verticalLayout->addWidget(pushButtonSwitch);

        pushButtonSaveImageSet = new QPushButton(dockWidgetContents_5);
        verticalLayout->addWidget(pushButtonSaveImageSet);

        auto *widget2 = new QWidget(dockWidgetContents_5);
        auto *horizontalLayout = new QHBoxLayout(widget2);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        pushButtonPrevious = new QPushButton(widget2);
        pushButtonPrevious->setMaximumSize(QSize(25, 16777215));

        horizontalLayout->addWidget(pushButtonPrevious);

        labelIndex = new QLabel;
        labelIndex->setObjectName("labelIndex");
        labelIndex->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        horizontalLayout->addWidget(labelIndex);
        pushButtonNext = new QPushButton(widget2);
        pushButtonNext->setMaximumSize(QSize(25, 16777215));

        horizontalLayout->addWidget(pushButtonNext);

        verticalLayout->addWidget(widget2);

        dockWidget_5->setWidget(dockWidgetContents_5);
        MainWindowClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_5);
        dockWidget_5->setWindowTitle("Tool Box");
        dockWidget_5->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        #pragma endregion 

        #pragma region Dock Widget View History
        auto *dockWidget_6 = new QDockWidget(MainWindowClass);
        dockWidget_6->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        viewHistory = new ViewHistoryWidget(MainWindowClass);
        dockWidget_6->setWidget(viewHistory);
        dockWidget_6->setWindowTitle("View History");
        dockWidget_6->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        MainWindowClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_6);
        #pragma endregion 

        MainWindowClass->tabifyDockWidget(dockWidget_6, dockWidget_5);
        MainWindowClass->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
        MainWindowClass->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

        MainWindowClass->setStatusBar(nullptr);
        retranslateUi(MainWindowClass);

        LogicWidgets_.push_back(pushButtonGo);
        LogicWidgets_.push_back(pushButtonHoughCircle);
        LogicWidgets_.push_back(pushButtonPlotCircle);
        LogicWidgets_.push_back(pushButtonDetectShadow);
        LogicWidgets_.push_back(comboBoxShadowDetection);
        LogicWidgets_.push_back(comboBoxPixelDistance);
        LogicWidgets_.push_back(pushButtonRelightning);
        LogicWidgets_.push_back(pushButtonSmoothNormal);
        LogicWidgets_.push_back(pushButtonPS);
        LogicWidgets_.push_back(pushButtonCheckMatch);
        LogicWidgets_.push_back(pushButtonShadowDifference);
        LogicWidgets_.push_back(comboBoxTargetObject);
        LogicWidgets_.push_back(pushButtonPlotTargetObject);
        LogicWidgets_.push_back(lineEditFolder);
        LogicWidgets_.push_back(lineEditSmoothNormal);
        LogicWidgets_.push_back(lineEditShadowThreshold);
        LogicWidgets_.push_back(comboBoxShadowUse);

    } 

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "Photometric Stereo", 0));
        LightningVectorLabel_->setText(QString());
        LinearCombinationParameterLabel_->setText(QString());
        ShadowLabel_->setText(QString());
        pushButtonGo->setText(QApplication::translate("MainWindowClass", "Go", 0));
        checkBoxCuda->setText(QApplication::translate("MainWindowClass", "CUDA", 0));
        pushButtonLoadDepthView->setText(QApplication::translate("MainWindowClass", "Load .dvf", 0));
        pushButtonHoughCircle->setText(QApplication::translate("MainWindowClass", "Hough Circle", 0));
        pushButtonPlotCircle->setText(QApplication::translate("MainWindowClass", "Plot Circle", 0));
        pushButtonDetectShadow->setText(QApplication::translate("MainWindowClass", "Detect Shadow", 0));
        pushButtonRelightning->setText(QApplication::translate("MainWindowClass", "Relighting", 0));
        pushButtonSmoothNormal->setText(QApplication::translate("MainWindowClass", "Smooth Normal", 0));
        pushButtonPS->setText(QApplication::translate("MainWindowClass", "Reconstruct", 0));
        pushButtonCheckMatch->setText(QApplication::translate("MainWindowClass", "Match", 0));
        pushButtonShadowDifference->setText(QApplication::translate("MainWindowClass", "Shadow Difference", 0));
        pushButtonPlotTargetObject->setText(QApplication::translate("MainWindowClass", "Plot Target Circle", 0));
        pushButtonSwitch->setText(QApplication::translate("MainWindowClass", "Switch Image Sets", 0));
        pushButtonSaveImageSet->setText(QApplication::translate("MainWindowClass", "Save Image Sets", 0));
        pushButtonPrevious->setText(QApplication::translate("MainWindowClass", "<<", 0));
        pushButtonNext->setText(QApplication::translate("MainWindowClass", ">>", 0));
    }

    void EnableAllWidgetBut(QWidget *w, bool enable)
    {
        for (auto i = LogicWidgets_.begin(); i != LogicWidgets_.end(); ++i)
        {
            if (*i == w)
                continue;
            (*i)->setEnabled(enable);
        }
    }
    void EnableAllWidgetBut(bool enable, int count, ...)
    {
        for (auto i = LogicWidgets_.begin(); i != LogicWidgets_.end(); ++i)
        {
            va_list vl = 0;
            va_start(vl, count);
            bool skip = false;
            for(int ii = 0; ii < count; ++ii)
            {
                QWidget *n = va_arg(vl, QWidget *);
                if (*i == n)
                {
                    skip = true;
                    break;
                }
            }

            va_end(vl);
            if (skip)
                continue;
            (*i)->setEnabled(enable);
        }
        
    }
};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} 
