/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionFront;
    QAction *actionBack;
    QAction *actionTop;
    QAction *actionBottom;
    QAction *actionLeft;
    QAction *actionRight;
    QAction *actionIsometric;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionZoom_All;
    QAction *actionZoom_Window;
    QAction *actionShade;
    QAction *actionMesh;
    QAction *actionNode;
    QAction *actionSave;
    QAction *actionSelectNode;
    QAction *actionSelectFace;
    QAction *actionShifttoOrigin;
    QAction *actionProfile;
    QAction *actionFaceNormal;
    QAction *actionNodeNormal;
    QAction *actionSelectEdge;
    QAction *actionGenerate;
    QAction *actionTest_1;
    QAction *actionSelectFix;
    QAction *actionSelectHandle;
    QAction *actionSaveSelection;
    QAction *actionReadSelection;
    QAction *actionSelectChamber;
    QAction *actionExport_to_Abaqus_model;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QToolBar *navigationToolBar;
    QStatusBar *statusBar;
    QToolBar *selectionToolBar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton_readSourceLayer;
    QFrame *line;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_Xmove;
    QDoubleSpinBox *doubleSpinBox_Xmove;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_Ymove;
    QDoubleSpinBox *doubleSpinBox_Ymove;
    QLabel *label_Zmove;
    QDoubleSpinBox *doubleSpinBox_Zmove;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_PosNorFile;
    QDoubleSpinBox *doubleSpinBox_Xrot;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinBox_Yrot;
    QLabel *label_2;
    QDoubleSpinBox *doubleSpinBox_Zrot;
    QFrame *line_4;
    QPushButton *pushButton_model_positionUpdate;
    QLabel *label_6;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_7;
    QSpinBox *spinBox_ShowLayerIndex;
    QCheckBox *checkBox_EachLayerSwitch;
    QPushButton *pushButton_ShowAllLayers;
    QCheckBox *checkBox_showCNC;
    QFrame *line_3;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_8;
    QDoubleSpinBox *doubleSpinBox_toolPathWidth;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBox_toolPathDistance;
    QPushButton *pushButton_spiralTp_generation;
    QPushButton *pushButton_contourTp_generation;
    QPushButton *pushButton_zigzagTp_generation;
    QFrame *line_2;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_10;
    QLineEdit *lineEdit_SorceDataDir;
    QPushButton *pushButton_readGcodeSourceData;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_11;
    QSpinBox *spinBox_GcodeGeneFromIndex;
    QLabel *label_12;
    QSpinBox *spinBox_GcodeGeneToIndex;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_14;
    QDoubleSpinBox *doubleSpinBox_toolLength;
    QPushButton *pushButton_calDWH;
    QPushButton *pushButton_calSingularOpt;
    QPushButton *pushButton_Gcode_writting;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *pushButton_GcodeSimulation;
    QProgressBar *progressBar_GcodeSimulation;
    QCheckBox *checkBox_stopSimulation;
    QTreeView *treeView;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *pushButton_clearAll;
    QSpacerItem *horizontalSpacer;
    QCheckBox *boxDeselect;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuSelect;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1328, 991);
        MainWindow->setMinimumSize(QSize(0, 0));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        font.setKerning(true);
        MainWindow->setFont(font);
        MainWindow->setMouseTracking(true);
        MainWindow->setFocusPolicy(Qt::StrongFocus);
        MainWindow->setAcceptDrops(true);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/Open Folder.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon);
        actionFront = new QAction(MainWindow);
        actionFront->setObjectName(QString::fromUtf8("actionFront"));
        actionFront->setCheckable(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/resource/Front View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFront->setIcon(icon1);
        actionBack = new QAction(MainWindow);
        actionBack->setObjectName(QString::fromUtf8("actionBack"));
        actionBack->setCheckable(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/resource/Back View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionBack->setIcon(icon2);
        actionTop = new QAction(MainWindow);
        actionTop->setObjectName(QString::fromUtf8("actionTop"));
        actionTop->setCheckable(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/resource/Top View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionTop->setIcon(icon3);
        actionBottom = new QAction(MainWindow);
        actionBottom->setObjectName(QString::fromUtf8("actionBottom"));
        actionBottom->setCheckable(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/resource/Bottom View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionBottom->setIcon(icon4);
        actionLeft = new QAction(MainWindow);
        actionLeft->setObjectName(QString::fromUtf8("actionLeft"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/resource/Left View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLeft->setIcon(icon5);
        actionRight = new QAction(MainWindow);
        actionRight->setObjectName(QString::fromUtf8("actionRight"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/resource/Right View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRight->setIcon(icon6);
        actionIsometric = new QAction(MainWindow);
        actionIsometric->setObjectName(QString::fromUtf8("actionIsometric"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/resource/Isometric View.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionIsometric->setIcon(icon7);
        actionZoom_In = new QAction(MainWindow);
        actionZoom_In->setObjectName(QString::fromUtf8("actionZoom_In"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/resource/Zoom In.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_In->setIcon(icon8);
        actionZoom_Out = new QAction(MainWindow);
        actionZoom_Out->setObjectName(QString::fromUtf8("actionZoom_Out"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/resource/Zoom Out.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Out->setIcon(icon9);
        actionZoom_All = new QAction(MainWindow);
        actionZoom_All->setObjectName(QString::fromUtf8("actionZoom_All"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/resource/Zoom All.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_All->setIcon(icon10);
        actionZoom_Window = new QAction(MainWindow);
        actionZoom_Window->setObjectName(QString::fromUtf8("actionZoom_Window"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/resource/Zoom Window.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Window->setIcon(icon11);
        actionShade = new QAction(MainWindow);
        actionShade->setObjectName(QString::fromUtf8("actionShade"));
        actionShade->setCheckable(true);
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/resource/Shade.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionShade->setIcon(icon12);
        actionMesh = new QAction(MainWindow);
        actionMesh->setObjectName(QString::fromUtf8("actionMesh"));
        actionMesh->setCheckable(true);
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/resource/Mesh.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMesh->setIcon(icon13);
        actionNode = new QAction(MainWindow);
        actionNode->setObjectName(QString::fromUtf8("actionNode"));
        actionNode->setCheckable(true);
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/resource/Node.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNode->setIcon(icon14);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/resource/Save as.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon15);
        actionSelectNode = new QAction(MainWindow);
        actionSelectNode->setObjectName(QString::fromUtf8("actionSelectNode"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/resource/selectNode.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectNode->setIcon(icon16);
        actionSelectFace = new QAction(MainWindow);
        actionSelectFace->setObjectName(QString::fromUtf8("actionSelectFace"));
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/resource/selectFace.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectFace->setIcon(icon17);
        actionShifttoOrigin = new QAction(MainWindow);
        actionShifttoOrigin->setObjectName(QString::fromUtf8("actionShifttoOrigin"));
        actionProfile = new QAction(MainWindow);
        actionProfile->setObjectName(QString::fromUtf8("actionProfile"));
        actionProfile->setCheckable(true);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/resource/Profile.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionProfile->setIcon(icon18);
        actionFaceNormal = new QAction(MainWindow);
        actionFaceNormal->setObjectName(QString::fromUtf8("actionFaceNormal"));
        actionFaceNormal->setCheckable(true);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/resource/FaceNormal.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFaceNormal->setIcon(icon19);
        actionNodeNormal = new QAction(MainWindow);
        actionNodeNormal->setObjectName(QString::fromUtf8("actionNodeNormal"));
        actionNodeNormal->setCheckable(true);
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/resource/NodeNormal.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNodeNormal->setIcon(icon20);
        actionSelectEdge = new QAction(MainWindow);
        actionSelectEdge->setObjectName(QString::fromUtf8("actionSelectEdge"));
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/resource/selectEdge.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectEdge->setIcon(icon21);
        actionGenerate = new QAction(MainWindow);
        actionGenerate->setObjectName(QString::fromUtf8("actionGenerate"));
        actionTest_1 = new QAction(MainWindow);
        actionTest_1->setObjectName(QString::fromUtf8("actionTest_1"));
        actionSelectFix = new QAction(MainWindow);
        actionSelectFix->setObjectName(QString::fromUtf8("actionSelectFix"));
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/resource/selectFix.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectFix->setIcon(icon22);
        actionSelectHandle = new QAction(MainWindow);
        actionSelectHandle->setObjectName(QString::fromUtf8("actionSelectHandle"));
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/resource/selectHandle.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectHandle->setIcon(icon23);
        actionSaveSelection = new QAction(MainWindow);
        actionSaveSelection->setObjectName(QString::fromUtf8("actionSaveSelection"));
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/resource/SaveSelection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSaveSelection->setIcon(icon24);
        actionReadSelection = new QAction(MainWindow);
        actionReadSelection->setObjectName(QString::fromUtf8("actionReadSelection"));
        QIcon icon25;
        icon25.addFile(QString::fromUtf8(":/resource/InputSelection.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionReadSelection->setIcon(icon25);
        actionSelectChamber = new QAction(MainWindow);
        actionSelectChamber->setObjectName(QString::fromUtf8("actionSelectChamber"));
        actionExport_to_Abaqus_model = new QAction(MainWindow);
        actionExport_to_Abaqus_model->setObjectName(QString::fromUtf8("actionExport_to_Abaqus_model"));
        actionExport_to_Abaqus_model->setCheckable(false);
        QIcon icon26;
        icon26.addFile(QString::fromUtf8(":/resource/abaqus logo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport_to_Abaqus_model->setIcon(icon26);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setMouseTracking(true);
        centralWidget->setAcceptDrops(true);
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        MainWindow->setCentralWidget(centralWidget);
        navigationToolBar = new QToolBar(MainWindow);
        navigationToolBar->setObjectName(QString::fromUtf8("navigationToolBar"));
        navigationToolBar->setMovable(false);
        navigationToolBar->setIconSize(QSize(25, 25));
        navigationToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, navigationToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        selectionToolBar = new QToolBar(MainWindow);
        selectionToolBar->setObjectName(QString::fromUtf8("selectionToolBar"));
        selectionToolBar->setMovable(false);
        selectionToolBar->setIconSize(QSize(25, 25));
        selectionToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, selectionToolBar);
        dockWidget = new QDockWidget(MainWindow);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
        dockWidget->setSizePolicy(sizePolicy);
        dockWidget->setMinimumSize(QSize(300, 900));
        dockWidget->setMaximumSize(QSize(300, 524287));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        dockWidgetContents->setLayoutDirection(Qt::LeftToRight);
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pushButton_readSourceLayer = new QPushButton(dockWidgetContents);
        pushButton_readSourceLayer->setObjectName(QString::fromUtf8("pushButton_readSourceLayer"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("3ds"));
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setItalic(false);
        font1.setUnderline(false);
        font1.setWeight(75);
        pushButton_readSourceLayer->setFont(font1);
        pushButton_readSourceLayer->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));

        verticalLayout->addWidget(pushButton_readSourceLayer);

        line = new QFrame(dockWidgetContents);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setSpacing(6);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("3ds"));
        font2.setPointSize(10);
        font2.setBold(true);
        font2.setWeight(75);
        font2.setKerning(true);
        label->setFont(font2);
        label->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_12->addWidget(label);


        verticalLayout->addLayout(horizontalLayout_12);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_Xmove = new QLabel(dockWidgetContents);
        label_Xmove->setObjectName(QString::fromUtf8("label_Xmove"));
        label_Xmove->setMaximumSize(QSize(16777215, 16777215));
        QFont font3;
        font3.setPointSize(8);
        font3.setBold(false);
        font3.setWeight(50);
        font3.setKerning(true);
        label_Xmove->setFont(font3);

        horizontalLayout_11->addWidget(label_Xmove);

        doubleSpinBox_Xmove = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Xmove->setObjectName(QString::fromUtf8("doubleSpinBox_Xmove"));
        doubleSpinBox_Xmove->setFont(font3);
        doubleSpinBox_Xmove->setMinimum(-150.000000000000000);
        doubleSpinBox_Xmove->setMaximum(150.000000000000000);
        doubleSpinBox_Xmove->setValue(0.000000000000000);

        horizontalLayout_11->addWidget(doubleSpinBox_Xmove);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_4);

        label_Ymove = new QLabel(dockWidgetContents);
        label_Ymove->setObjectName(QString::fromUtf8("label_Ymove"));
        label_Ymove->setMaximumSize(QSize(16777215, 16777215));
        label_Ymove->setFont(font3);

        horizontalLayout_11->addWidget(label_Ymove);

        doubleSpinBox_Ymove = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Ymove->setObjectName(QString::fromUtf8("doubleSpinBox_Ymove"));
        doubleSpinBox_Ymove->setFont(font3);
        doubleSpinBox_Ymove->setMinimum(-150.000000000000000);
        doubleSpinBox_Ymove->setMaximum(150.000000000000000);
        doubleSpinBox_Ymove->setValue(0.000000000000000);

        horizontalLayout_11->addWidget(doubleSpinBox_Ymove);

        label_Zmove = new QLabel(dockWidgetContents);
        label_Zmove->setObjectName(QString::fromUtf8("label_Zmove"));
        label_Zmove->setMaximumSize(QSize(16777215, 16777215));
        label_Zmove->setFont(font3);

        horizontalLayout_11->addWidget(label_Zmove);

        doubleSpinBox_Zmove = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Zmove->setObjectName(QString::fromUtf8("doubleSpinBox_Zmove"));
        doubleSpinBox_Zmove->setFont(font3);
        doubleSpinBox_Zmove->setMaximum(150.000000000000000);
        doubleSpinBox_Zmove->setValue(0.000000000000000);

        horizontalLayout_11->addWidget(doubleSpinBox_Zmove);


        verticalLayout->addLayout(horizontalLayout_11);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setSpacing(6);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_PosNorFile = new QLabel(dockWidgetContents);
        label_PosNorFile->setObjectName(QString::fromUtf8("label_PosNorFile"));
        label_PosNorFile->setFont(font3);

        horizontalLayout_16->addWidget(label_PosNorFile);

        doubleSpinBox_Xrot = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Xrot->setObjectName(QString::fromUtf8("doubleSpinBox_Xrot"));
        doubleSpinBox_Xrot->setFont(font3);
        doubleSpinBox_Xrot->setMinimum(-180.000000000000000);
        doubleSpinBox_Xrot->setMaximum(180.000000000000000);

        horizontalLayout_16->addWidget(doubleSpinBox_Xrot);

        label_3 = new QLabel(dockWidgetContents);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font3);

        horizontalLayout_16->addWidget(label_3);

        doubleSpinBox_Yrot = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Yrot->setObjectName(QString::fromUtf8("doubleSpinBox_Yrot"));
        doubleSpinBox_Yrot->setFont(font3);
        doubleSpinBox_Yrot->setMinimum(-180.000000000000000);
        doubleSpinBox_Yrot->setMaximum(180.000000000000000);

        horizontalLayout_16->addWidget(doubleSpinBox_Yrot);

        label_2 = new QLabel(dockWidgetContents);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font3);

        horizontalLayout_16->addWidget(label_2);

        doubleSpinBox_Zrot = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_Zrot->setObjectName(QString::fromUtf8("doubleSpinBox_Zrot"));
        doubleSpinBox_Zrot->setFont(font3);
        doubleSpinBox_Zrot->setMinimum(-180.000000000000000);
        doubleSpinBox_Zrot->setMaximum(180.000000000000000);

        horizontalLayout_16->addWidget(doubleSpinBox_Zrot);


        verticalLayout->addLayout(horizontalLayout_16);

        line_4 = new QFrame(dockWidgetContents);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_4);

        pushButton_model_positionUpdate = new QPushButton(dockWidgetContents);
        pushButton_model_positionUpdate->setObjectName(QString::fromUtf8("pushButton_model_positionUpdate"));
        QFont font4;
        font4.setFamily(QString::fromUtf8("3ds"));
        font4.setPointSize(10);
        font4.setBold(true);
        font4.setWeight(75);
        pushButton_model_positionUpdate->setFont(font4);
        pushButton_model_positionUpdate->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));

        verticalLayout->addWidget(pushButton_model_positionUpdate);

        label_6 = new QLabel(dockWidgetContents);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setFont(font2);

        verticalLayout->addWidget(label_6);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_7 = new QLabel(dockWidgetContents);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setFont(font3);

        horizontalLayout_2->addWidget(label_7);

        spinBox_ShowLayerIndex = new QSpinBox(dockWidgetContents);
        spinBox_ShowLayerIndex->setObjectName(QString::fromUtf8("spinBox_ShowLayerIndex"));
        spinBox_ShowLayerIndex->setFont(font3);
        spinBox_ShowLayerIndex->setMaximum(999);

        horizontalLayout_2->addWidget(spinBox_ShowLayerIndex);

        checkBox_EachLayerSwitch = new QCheckBox(dockWidgetContents);
        checkBox_EachLayerSwitch->setObjectName(QString::fromUtf8("checkBox_EachLayerSwitch"));
        checkBox_EachLayerSwitch->setFont(font3);

        horizontalLayout_2->addWidget(checkBox_EachLayerSwitch);

        pushButton_ShowAllLayers = new QPushButton(dockWidgetContents);
        pushButton_ShowAllLayers->setObjectName(QString::fromUtf8("pushButton_ShowAllLayers"));
        pushButton_ShowAllLayers->setFont(font3);

        horizontalLayout_2->addWidget(pushButton_ShowAllLayers);

        checkBox_showCNC = new QCheckBox(dockWidgetContents);
        checkBox_showCNC->setObjectName(QString::fromUtf8("checkBox_showCNC"));
        checkBox_showCNC->setEnabled(false);
        QFont font5;
        font5.setPointSize(8);
        font5.setBold(false);
        font5.setWeight(50);
        checkBox_showCNC->setFont(font5);

        horizontalLayout_2->addWidget(checkBox_showCNC);


        verticalLayout->addLayout(horizontalLayout_2);

        line_3 = new QFrame(dockWidgetContents);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_3);

        label_4 = new QLabel(dockWidgetContents);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font2);
        label_4->setStyleSheet(QString::fromUtf8(""));

        verticalLayout->addWidget(label_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_8 = new QLabel(dockWidgetContents);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setFont(font3);

        horizontalLayout_3->addWidget(label_8);

        doubleSpinBox_toolPathWidth = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_toolPathWidth->setObjectName(QString::fromUtf8("doubleSpinBox_toolPathWidth"));
        doubleSpinBox_toolPathWidth->setFont(font3);
        doubleSpinBox_toolPathWidth->setMinimum(0.200000000000000);
        doubleSpinBox_toolPathWidth->setSingleStep(0.100000000000000);
        doubleSpinBox_toolPathWidth->setValue(0.600000000000000);

        horizontalLayout_3->addWidget(doubleSpinBox_toolPathWidth);

        label_9 = new QLabel(dockWidgetContents);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setFont(font3);

        horizontalLayout_3->addWidget(label_9);

        doubleSpinBox_toolPathDistance = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_toolPathDistance->setObjectName(QString::fromUtf8("doubleSpinBox_toolPathDistance"));
        doubleSpinBox_toolPathDistance->setFont(font3);
        doubleSpinBox_toolPathDistance->setMinimum(0.500000000000000);
        doubleSpinBox_toolPathDistance->setSingleStep(0.100000000000000);
        doubleSpinBox_toolPathDistance->setValue(0.500000000000000);

        horizontalLayout_3->addWidget(doubleSpinBox_toolPathDistance);


        verticalLayout->addLayout(horizontalLayout_3);

        pushButton_spiralTp_generation = new QPushButton(dockWidgetContents);
        pushButton_spiralTp_generation->setObjectName(QString::fromUtf8("pushButton_spiralTp_generation"));
        pushButton_spiralTp_generation->setFont(font4);
        pushButton_spiralTp_generation->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));

        verticalLayout->addWidget(pushButton_spiralTp_generation);

        pushButton_contourTp_generation = new QPushButton(dockWidgetContents);
        pushButton_contourTp_generation->setObjectName(QString::fromUtf8("pushButton_contourTp_generation"));
        pushButton_contourTp_generation->setFont(font4);
        pushButton_contourTp_generation->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));

        verticalLayout->addWidget(pushButton_contourTp_generation);

        pushButton_zigzagTp_generation = new QPushButton(dockWidgetContents);
        pushButton_zigzagTp_generation->setObjectName(QString::fromUtf8("pushButton_zigzagTp_generation"));
        pushButton_zigzagTp_generation->setFont(font4);
        pushButton_zigzagTp_generation->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));

        verticalLayout->addWidget(pushButton_zigzagTp_generation);

        line_2 = new QFrame(dockWidgetContents);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        label_5 = new QLabel(dockWidgetContents);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font2);
        label_5->setStyleSheet(QString::fromUtf8(""));

        verticalLayout->addWidget(label_5);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_10 = new QLabel(dockWidgetContents);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font3);

        horizontalLayout_4->addWidget(label_10);

        lineEdit_SorceDataDir = new QLineEdit(dockWidgetContents);
        lineEdit_SorceDataDir->setObjectName(QString::fromUtf8("lineEdit_SorceDataDir"));
        QFont font6;
        font6.setPointSize(8);
        font6.setBold(true);
        font6.setWeight(75);
        font6.setKerning(true);
        lineEdit_SorceDataDir->setFont(font6);

        horizontalLayout_4->addWidget(lineEdit_SorceDataDir);

        pushButton_readGcodeSourceData = new QPushButton(dockWidgetContents);
        pushButton_readGcodeSourceData->setObjectName(QString::fromUtf8("pushButton_readGcodeSourceData"));
        pushButton_readGcodeSourceData->setFont(font4);
        pushButton_readGcodeSourceData->setStyleSheet(QString::fromUtf8("color: rgb(128,0,128);"));

        horizontalLayout_4->addWidget(pushButton_readGcodeSourceData);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_11 = new QLabel(dockWidgetContents);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setFont(font5);

        horizontalLayout_7->addWidget(label_11);

        spinBox_GcodeGeneFromIndex = new QSpinBox(dockWidgetContents);
        spinBox_GcodeGeneFromIndex->setObjectName(QString::fromUtf8("spinBox_GcodeGeneFromIndex"));
        spinBox_GcodeGeneFromIndex->setFont(font5);

        horizontalLayout_7->addWidget(spinBox_GcodeGeneFromIndex);

        label_12 = new QLabel(dockWidgetContents);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setFont(font5);

        horizontalLayout_7->addWidget(label_12);

        spinBox_GcodeGeneToIndex = new QSpinBox(dockWidgetContents);
        spinBox_GcodeGeneToIndex->setObjectName(QString::fromUtf8("spinBox_GcodeGeneToIndex"));
        spinBox_GcodeGeneToIndex->setFont(font5);

        horizontalLayout_7->addWidget(spinBox_GcodeGeneToIndex);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_2);

        label_14 = new QLabel(dockWidgetContents);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setFont(font5);

        horizontalLayout_7->addWidget(label_14);

        doubleSpinBox_toolLength = new QDoubleSpinBox(dockWidgetContents);
        doubleSpinBox_toolLength->setObjectName(QString::fromUtf8("doubleSpinBox_toolLength"));
        doubleSpinBox_toolLength->setFont(font5);
        doubleSpinBox_toolLength->setValue(68.000000000000000);

        horizontalLayout_7->addWidget(doubleSpinBox_toolLength);


        verticalLayout->addLayout(horizontalLayout_7);

        pushButton_calDWH = new QPushButton(dockWidgetContents);
        pushButton_calDWH->setObjectName(QString::fromUtf8("pushButton_calDWH"));
        pushButton_calDWH->setFont(font4);
        pushButton_calDWH->setStyleSheet(QString::fromUtf8("color: rgb(128,0,128);"));

        verticalLayout->addWidget(pushButton_calDWH);

        pushButton_calSingularOpt = new QPushButton(dockWidgetContents);
        pushButton_calSingularOpt->setObjectName(QString::fromUtf8("pushButton_calSingularOpt"));
        pushButton_calSingularOpt->setFont(font4);
        pushButton_calSingularOpt->setStyleSheet(QString::fromUtf8("color: rgb(128,0,128);"));

        verticalLayout->addWidget(pushButton_calSingularOpt);

        pushButton_Gcode_writting = new QPushButton(dockWidgetContents);
        pushButton_Gcode_writting->setObjectName(QString::fromUtf8("pushButton_Gcode_writting"));
        pushButton_Gcode_writting->setFont(font4);
        pushButton_Gcode_writting->setStyleSheet(QString::fromUtf8("color: rgb(128,0,128);"));

        verticalLayout->addWidget(pushButton_Gcode_writting);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        pushButton_GcodeSimulation = new QPushButton(dockWidgetContents);
        pushButton_GcodeSimulation->setObjectName(QString::fromUtf8("pushButton_GcodeSimulation"));
        pushButton_GcodeSimulation->setFont(font1);
        pushButton_GcodeSimulation->setStyleSheet(QString::fromUtf8("color: rgb(128,0,128);"));

        horizontalLayout_5->addWidget(pushButton_GcodeSimulation);

        progressBar_GcodeSimulation = new QProgressBar(dockWidgetContents);
        progressBar_GcodeSimulation->setObjectName(QString::fromUtf8("progressBar_GcodeSimulation"));
        QFont font7;
        font7.setPointSize(1);
        font7.setBold(false);
        font7.setWeight(50);
        font7.setKerning(true);
        progressBar_GcodeSimulation->setFont(font7);
        progressBar_GcodeSimulation->setValue(0);

        horizontalLayout_5->addWidget(progressBar_GcodeSimulation);

        checkBox_stopSimulation = new QCheckBox(dockWidgetContents);
        checkBox_stopSimulation->setObjectName(QString::fromUtf8("checkBox_stopSimulation"));
        checkBox_stopSimulation->setFont(font3);

        horizontalLayout_5->addWidget(checkBox_stopSimulation);


        verticalLayout->addLayout(horizontalLayout_5);

        treeView = new QTreeView(dockWidgetContents);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        treeView->setEnabled(true);
        treeView->setProperty("showDropIndicator", QVariant(true));
        treeView->setIndentation(5);
        treeView->header()->setVisible(false);

        verticalLayout->addWidget(treeView);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        pushButton_clearAll = new QPushButton(dockWidgetContents);
        pushButton_clearAll->setObjectName(QString::fromUtf8("pushButton_clearAll"));

        horizontalLayout_6->addWidget(pushButton_clearAll);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        boxDeselect = new QCheckBox(dockWidgetContents);
        boxDeselect->setObjectName(QString::fromUtf8("boxDeselect"));
        boxDeselect->setFont(font3);

        horizontalLayout_6->addWidget(boxDeselect);


        verticalLayout->addLayout(horizontalLayout_6);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1328, 26));
        menuBar->setLayoutDirection(Qt::LeftToRight);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuSelect = new QMenu(menuBar);
        menuSelect->setObjectName(QString::fromUtf8("menuSelect"));
        MainWindow->setMenuBar(menuBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setMovable(false);
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        navigationToolBar->addAction(actionFront);
        navigationToolBar->addAction(actionBack);
        navigationToolBar->addAction(actionTop);
        navigationToolBar->addAction(actionBottom);
        navigationToolBar->addAction(actionLeft);
        navigationToolBar->addAction(actionRight);
        navigationToolBar->addAction(actionIsometric);
        navigationToolBar->addSeparator();
        navigationToolBar->addAction(actionZoom_In);
        navigationToolBar->addAction(actionZoom_Out);
        navigationToolBar->addAction(actionZoom_All);
        navigationToolBar->addAction(actionZoom_Window);
        navigationToolBar->addSeparator();
        navigationToolBar->addAction(actionShade);
        navigationToolBar->addAction(actionMesh);
        navigationToolBar->addAction(actionNode);
        navigationToolBar->addAction(actionProfile);
        navigationToolBar->addAction(actionFaceNormal);
        navigationToolBar->addAction(actionNodeNormal);
        selectionToolBar->addAction(actionSaveSelection);
        selectionToolBar->addAction(actionReadSelection);
        selectionToolBar->addSeparator();
        selectionToolBar->addAction(actionSelectNode);
        selectionToolBar->addAction(actionSelectEdge);
        selectionToolBar->addAction(actionSelectFace);
        selectionToolBar->addAction(actionSelectFix);
        selectionToolBar->addAction(actionSelectHandle);
        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuSelect->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSaveSelection);
        menuFile->addAction(actionReadSelection);
        menuView->addAction(actionFront);
        menuView->addAction(actionBack);
        menuView->addAction(actionTop);
        menuView->addAction(actionBottom);
        menuView->addAction(actionLeft);
        menuView->addAction(actionRight);
        menuView->addAction(actionIsometric);
        menuView->addSeparator();
        menuView->addAction(actionZoom_In);
        menuView->addAction(actionZoom_Out);
        menuView->addAction(actionZoom_All);
        menuView->addAction(actionZoom_Window);
        menuView->addSeparator();
        menuView->addAction(actionShade);
        menuView->addAction(actionMesh);
        menuView->addAction(actionNode);
        menuView->addAction(actionProfile);
        menuView->addSeparator();
        menuView->addAction(actionShifttoOrigin);
        menuSelect->addAction(actionSelectNode);
        menuSelect->addAction(actionSelectEdge);
        menuSelect->addAction(actionSelectFace);
        menuSelect->addSeparator();
        menuSelect->addAction(actionSelectFix);
        menuSelect->addAction(actionSelectHandle);
        menuSelect->addSeparator();
        toolBar->addAction(actionOpen);
        toolBar->addAction(actionSave);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionOpen->setText(QApplication::translate("MainWindow", "Open", nullptr));
        actionFront->setText(QApplication::translate("MainWindow", "Front", nullptr));
        actionBack->setText(QApplication::translate("MainWindow", "Back", nullptr));
        actionTop->setText(QApplication::translate("MainWindow", "Top", nullptr));
        actionBottom->setText(QApplication::translate("MainWindow", "Bottom", nullptr));
        actionLeft->setText(QApplication::translate("MainWindow", "Left", nullptr));
        actionRight->setText(QApplication::translate("MainWindow", "Right", nullptr));
        actionIsometric->setText(QApplication::translate("MainWindow", "Isometric", nullptr));
        actionZoom_In->setText(QApplication::translate("MainWindow", "Zoom In", nullptr));
        actionZoom_Out->setText(QApplication::translate("MainWindow", "Zoom Out", nullptr));
        actionZoom_All->setText(QApplication::translate("MainWindow", "Zoom All", nullptr));
        actionZoom_Window->setText(QApplication::translate("MainWindow", "Zoom Window", nullptr));
        actionShade->setText(QApplication::translate("MainWindow", "Shade", nullptr));
        actionMesh->setText(QApplication::translate("MainWindow", "Mesh", nullptr));
        actionNode->setText(QApplication::translate("MainWindow", "Node", nullptr));
        actionSave->setText(QApplication::translate("MainWindow", "Save", nullptr));
        actionSelectNode->setText(QApplication::translate("MainWindow", "Node", nullptr));
        actionSelectFace->setText(QApplication::translate("MainWindow", "Face", nullptr));
        actionShifttoOrigin->setText(QApplication::translate("MainWindow", "Shift to Origin", nullptr));
        actionProfile->setText(QApplication::translate("MainWindow", "Profile", nullptr));
        actionFaceNormal->setText(QApplication::translate("MainWindow", "FaceNormal", nullptr));
        actionNodeNormal->setText(QApplication::translate("MainWindow", "NodeNormal", nullptr));
        actionSelectEdge->setText(QApplication::translate("MainWindow", "Edge", nullptr));
        actionGenerate->setText(QApplication::translate("MainWindow", "Generate", nullptr));
        actionTest_1->setText(QApplication::translate("MainWindow", "Test_1", nullptr));
        actionSelectFix->setText(QApplication::translate("MainWindow", "Fix", nullptr));
        actionSelectHandle->setText(QApplication::translate("MainWindow", "Handle & Rigid", nullptr));
        actionSaveSelection->setText(QApplication::translate("MainWindow", "Save selection", nullptr));
        actionReadSelection->setText(QApplication::translate("MainWindow", "Read selection", nullptr));
        actionSelectChamber->setText(QApplication::translate("MainWindow", "Select Chamber (SORO)", nullptr));
        actionExport_to_Abaqus_model->setText(QApplication::translate("MainWindow", "Export to Abaqus model", nullptr));
        navigationToolBar->setWindowTitle(QApplication::translate("MainWindow", "navigationToolBar", nullptr));
        selectionToolBar->setWindowTitle(QApplication::translate("MainWindow", "selectionToolBar", nullptr));
        pushButton_readSourceLayer->setText(QApplication::translate("MainWindow", "1.Read Layers", nullptr));
        label->setText(QApplication::translate("MainWindow", "Coord3D Modification --> Z up", nullptr));
        label_Xmove->setText(QApplication::translate("MainWindow", "Xm", nullptr));
        label_Ymove->setText(QApplication::translate("MainWindow", "Ym", nullptr));
        label_Zmove->setText(QApplication::translate("MainWindow", "Zm", nullptr));
        label_PosNorFile->setText(QApplication::translate("MainWindow", "Xr", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Yr", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "Zr", nullptr));
        pushButton_model_positionUpdate->setText(QApplication::translate("MainWindow", "2.Update Pos and Ort", nullptr));
        label_6->setText(QApplication::translate("MainWindow", "Display", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "show", nullptr));
        checkBox_EachLayerSwitch->setText(QApplication::translate("MainWindow", "each", nullptr));
        pushButton_ShowAllLayers->setText(QApplication::translate("MainWindow", "All", nullptr));
        checkBox_showCNC->setText(QApplication::translate("MainWindow", "cnc", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "Toolpath Generation", nullptr));
        label_8->setText(QApplication::translate("MainWindow", "width", nullptr));
        label_9->setText(QApplication::translate("MainWindow", "distance", nullptr));
        pushButton_spiralTp_generation->setText(QApplication::translate("MainWindow", "3.1 Spiral Tp Generation", nullptr));
        pushButton_contourTp_generation->setText(QApplication::translate("MainWindow", "3.2 Contour Tp Generation", nullptr));
        pushButton_zigzagTp_generation->setText(QApplication::translate("MainWindow", "3.3 Zigzag Tp Generation", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "CNC Motion Planning", nullptr));
        label_10->setText(QApplication::translate("MainWindow", "Model", nullptr));
        lineEdit_SorceDataDir->setText(QApplication::translate("MainWindow", "Dome", nullptr));
        pushButton_readGcodeSourceData->setText(QApplication::translate("MainWindow", "1.Read data", nullptr));
        label_11->setText(QApplication::translate("MainWindow", "From", nullptr));
        label_12->setText(QApplication::translate("MainWindow", "To", nullptr));
        label_14->setText(QApplication::translate("MainWindow", "toolLns", nullptr));
        pushButton_calDWH->setText(QApplication::translate("MainWindow", "2.Calculate Extrusion Volume", nullptr));
        pushButton_calSingularOpt->setText(QApplication::translate("MainWindow", "3.Singularity Optimization", nullptr));
        pushButton_Gcode_writting->setText(QApplication::translate("MainWindow", "4.G Code Writing", nullptr));
        pushButton_GcodeSimulation->setText(QApplication::translate("MainWindow", "5.Simulation", nullptr));
        checkBox_stopSimulation->setText(QApplication::translate("MainWindow", "stop", nullptr));
        pushButton_clearAll->setText(QApplication::translate("MainWindow", "Clear All", nullptr));
        boxDeselect->setText(QApplication::translate("MainWindow", "deselect", nullptr));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", nullptr));
        menuView->setTitle(QApplication::translate("MainWindow", "View", nullptr));
        menuSelect->setTitle(QApplication::translate("MainWindow", "Select", nullptr));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
