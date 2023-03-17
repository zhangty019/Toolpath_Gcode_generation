#include "stdafx.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QtDebug>
#include <QDesktopWidget>
#include <QCoreApplication>
#include <QMimeData>
#include <QTreeView>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QScreen>
#include <QStyleFactory>
#include <fstream>

#include "../GLKLib/GLKCameraTool.h"
#include "../GLKLib/InteractiveTool.h"
#include "../GLKLib/GLKMatrixLib.h"
#include "../GLKLib/GLKGeometry.h"
#include "../QMeshLib/QMeshPatch.h"
#include "../QMeshLib/QMeshTetra.h"
#include "../QMeshLib/QMeshFace.h"
#include "../QMeshLib/QMeshEdge.h"
#include "../QMeshLib/QMeshNode.h"

#include "fileIO.h"
#include "spiralToolpath.h"
#include "heatmethodfield.h"
#include "toolpathGeneration.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	QApplication::setStyle(QStyleFactory::create("Fusion"));

    signalMapper = new QSignalMapper(this);
    addToolBar(ui->toolBar);
    addToolBar(ui->navigationToolBar);
    addToolBar(ui->selectionToolBar);

    createTreeView();
    createActions();

    pGLK = new GLKLib();
    ui->horizontalLayout->addWidget(pGLK);
    ui->horizontalLayout->setMargin(0);
    pGLK->setFocus();

    pGLK->clear_tools();
    pGLK->set_tool(new GLKCameraTool(pGLK,ORBITPAN));
	
	//connect timer with timer function
	connect(&Gcode_timer, SIGNAL(timeout()), this, SLOT(doTimerGcodeMoving()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions()
{
    // file IO
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));
	connect(ui->actionSaveSelection, SIGNAL(triggered(bool)), this, SLOT(saveSelection()));
	connect(ui->actionReadSelection, SIGNAL(triggered(bool)), this, SLOT(readSelection()));

    // navigation
    connect(ui->actionFront, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionBack, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionTop, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionBottom, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionLeft, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionRight, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionIsometric, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_In, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_Out, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_All, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionZoom_Window, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping (ui->actionFront, 0);
    signalMapper->setMapping (ui->actionBack, 1);
    signalMapper->setMapping (ui->actionTop, 2);
    signalMapper->setMapping (ui->actionBottom, 3);
    signalMapper->setMapping (ui->actionLeft, 4);
    signalMapper->setMapping (ui->actionRight, 5);
    signalMapper->setMapping (ui->actionIsometric, 6);
    signalMapper->setMapping (ui->actionZoom_In, 7);
    signalMapper->setMapping (ui->actionZoom_Out, 8);
    signalMapper->setMapping (ui->actionZoom_All, 9);
    signalMapper->setMapping (ui->actionZoom_Window, 10);

    // view
    connect(ui->actionShade, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionMesh, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionNode, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionProfile, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionFaceNormal, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionNodeNormal, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    signalMapper->setMapping (ui->actionShade, 20);
    signalMapper->setMapping (ui->actionMesh, 21);
    signalMapper->setMapping (ui->actionNode, 22);
    signalMapper->setMapping (ui->actionProfile, 23);
    signalMapper->setMapping (ui->actionFaceNormal, 24);
    signalMapper->setMapping (ui->actionNodeNormal, 25);
    ui->actionShade->setChecked(true);

    connect(ui->actionShifttoOrigin, SIGNAL(triggered(bool)), this, SLOT(shiftToOrigin()));

    // select
    connect(ui->actionSelectNode, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionSelectEdge, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
    connect(ui->actionSelectFace, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
	connect(ui->actionSelectFix, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
	connect(ui->actionSelectHandle, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));

	signalMapper->setMapping (ui->actionSelectNode, 30);
    signalMapper->setMapping (ui->actionSelectEdge, 31);
    signalMapper->setMapping (ui->actionSelectFace, 32);
	signalMapper->setMapping(ui->actionSelectFix, 33);
	signalMapper->setMapping(ui->actionSelectHandle, 34);


    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(signalNavigation(int)));

	//Button for display
    connect(ui->pushButton_ShowAllLayers, SIGNAL(released()), this, SLOT(all_Display()));
    connect(ui->spinBox_ShowLayerIndex, SIGNAL(valueChanged(int)), this, SLOT(update_Layer_Display()));
    //Button for toolpath generation
    connect(ui->pushButton_readSourceLayer, SIGNAL(released()), this, SLOT(readSourceLayer()));
    connect(ui->pushButton_model_positionUpdate, SIGNAL(released()), this, SLOT(update_model_postion_orientation()));
    connect(ui->pushButton_spiralTp_generation, SIGNAL(released()), this, SLOT(spiralToolpath_generation()));
    connect(ui->pushButton_contourTp_generation, SIGNAL(released()), this, SLOT(contourToolPath_Generation()));
    connect(ui->pushButton_zigzagTp_generation, SIGNAL(released()), this, SLOT(zigzigToolPath_Generation()));
    //Button for Gcode generation and simulation
    connect(ui->pushButton_readGcodeSourceData, SIGNAL(released()), this, SLOT(readGcodeSourceData()));
    connect(ui->pushButton_calDWH, SIGNAL(released()), this, SLOT(runDHWcalculation()));
    connect(ui->pushButton_calSingularOpt, SIGNAL(released()), this, SLOT(runSingularityOpt()));
    connect(ui->pushButton_Gcode_writting, SIGNAL(released()), this, SLOT(runWriteGcode()));
    connect(ui->pushButton_GcodeSimulation, SIGNAL(released()), this, SLOT(runGcodeSimulation()));

}

void MainWindow::open()
{
    QString filenameStr = QFileDialog::getOpenFileName(this, tr("Open File,"), "..", tr(""));
    QFileInfo fileInfo(filenameStr);
    QString fileSuffix = fileInfo.suffix();
    QByteArray filenameArray = filenameStr.toLatin1();
    char *filename = filenameArray.data();

    // set polygen name
    std::string strFilename(filename);
    std::size_t foundStart = strFilename.find_last_of("/");
    std::size_t foundEnd = strFilename.find_last_of(".");
    std::string modelName;
    modelName = strFilename.substr(0,foundEnd);
    modelName = modelName.substr(foundStart+1);
    
    if (QString::compare(fileSuffix,"obj") == 0){
        PolygenMesh *polygenMesh = new PolygenMesh(UNDEFINED);
        polygenMesh->ImportOBJFile(filename,modelName);
        polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
        pGLK->AddDisplayObj(polygenMesh,true);
        polygenMeshList.AddTail(polygenMesh);
    }

	else if (QString::compare(fileSuffix, "tet") == 0) {
		PolygenMesh *polygenMesh = new PolygenMesh(TET);
		std::cout << filename << std::endl;
		std::cout << modelName << std::endl;
		polygenMesh->ImportTETFile(filename, modelName);
		polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
		pGLK->AddDisplayObj(polygenMesh, true);
		polygenMeshList.AddTail(polygenMesh);
	}

    updateTree();

    shiftToOrigin();
    pGLK->refresh(true);
}

void MainWindow::save()
{
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	if (!polygenMesh)
		return;
	QString filenameStr = QFileDialog::getSaveFileName(this, tr("OBJ File Export,"), "..", tr("OBJ(*.obj)"));
	QFileInfo fileInfo(filenameStr);
	QString fileSuffix = fileInfo.suffix();

	if (QString::compare(fileSuffix, "obj") == 0) {
		QFile exportFile(filenameStr);
		if (exportFile.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&exportFile);
			for (GLKPOSITION posMesh = polygenMesh->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
				QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posMesh);
				for (GLKPOSITION posNode = patch->GetNodeList().GetHeadPosition(); posNode != nullptr;) {
					QMeshNode *node = (QMeshNode*)patch->GetNodeList().GetNext(posNode);
					double xx, yy, zz;
					node->GetCoord3D(xx, yy, zz);
					float r, g, b;
					node->GetColor(r, g, b);
					out << "v " << xx << " " << yy << " " << zz << " " << node->value1 << endl;
				}
				for (GLKPOSITION posFace = patch->GetFaceList().GetHeadPosition(); posFace != nullptr;) {
					QMeshFace *face = (QMeshFace*)patch->GetFaceList().GetNext(posFace);
					out << "f " << face->GetNodeRecordPtr(0)->GetIndexNo() << " " << face->GetNodeRecordPtr(1)->GetIndexNo() << " " << face->GetNodeRecordPtr(2)->GetIndexNo() << endl;
				}
			}
		}
		exportFile.close();
	}
}

void MainWindow::saveSelection()
{
	//printf("%s exported\n", Model->ModelName);

	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	std::string filename = polygenMesh->getModelName();
	const char * c = filename.c_str();
	char *cstr = new char[filename.length() + 1];
	strcpy(cstr, filename.c_str());

	const char * split = ".";
	char* p = strtok(cstr, split);

	char output_filename[256];
	strcpy(output_filename, "..\\selection_file\\");
	strcat(output_filename, cstr);
	char filetype[64];
	strcpy(filetype, ".txt");
	strcat(output_filename, filetype);

	ofstream nodeSelection(output_filename);
	if (!nodeSelection)
		cerr << "Sorry!We were unable to build the file NodeSelect!\n";
	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *CheckNode = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		nodeSelection << CheckNode->GetIndexNo() << ":";
		//for the selection of fixing part
		if (CheckNode->isFixed == true) nodeSelection << "1:";
		else nodeSelection << "0:";
		//for the selection of hard part
		if (CheckNode->isHandle == true) nodeSelection << "1:" << endl;
		else nodeSelection << "0:" << endl;
	}

	nodeSelection.close();
	printf("Finish output selection \n");
}

void MainWindow::readSelection()
{
	PolygenMesh *polygenMesh = getSelectedPolygenMesh();
	if (!polygenMesh)
		polygenMesh = (PolygenMesh*)polygenMeshList.GetHead();
	QMeshPatch *patch = (QMeshPatch*)polygenMesh->GetMeshList().GetHead();

	std::string filename = polygenMesh->getModelName();
	const char * c = filename.c_str();

	char *cstr = new char[filename.length() + 1];
	strcpy(cstr, filename.c_str());

	const char * split = ".";
	char* p = strtok(cstr, split);

	char input_filename[256];
	strcpy(input_filename, "..\\selection_file\\");
	strcat(input_filename, cstr);
	char filetype[64];
	strcpy(filetype, ".txt");
	strcat(input_filename, filetype);

	ifstream nodeSelect(input_filename);
	if (!nodeSelect)
		cerr << "Sorry!We were unable to open the file!\n";
	vector<int> NodeIndex(patch->GetNodeNumber()), checkNodeFixed(patch->GetNodeNumber()), checkNodeHandle(patch->GetNodeNumber());
	//string line;
	int LineIndex1 = 0;
	string sss;
	while (getline(nodeSelect, sss)){
		const char * c = sss.c_str();
		sscanf(c, "%d:%d:%d", &NodeIndex[LineIndex1], &checkNodeFixed[LineIndex1], &checkNodeHandle[LineIndex1]);
		LineIndex1++;
	}

	nodeSelect.close();
	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode *CheckNode = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		if (checkNodeFixed[CheckNode->GetIndexNo() - 1] == 1) CheckNode->isFixed = true;
		if (checkNodeHandle[CheckNode->GetIndexNo() - 1] == 1) CheckNode->isHandle = true;
	}

	for (GLKPOSITION Pos = patch->GetFaceList().GetHeadPosition(); Pos != NULL;)
	{
		QMeshFace* Face = (QMeshFace*)patch->GetFaceList().GetNext(Pos);
		if (Face->GetNodeRecordPtr(0)->isHandle == true &&
			Face->GetNodeRecordPtr(1)->isHandle == true &&
			Face->GetNodeRecordPtr(2)->isHandle == true)
			Face->isHandleDraw = true;
		else Face->isHandleDraw = false;

		if (Face->GetNodeRecordPtr(0)->isFixed == true &&
			Face->GetNodeRecordPtr(1)->isFixed == true &&
			Face->GetNodeRecordPtr(2)->isFixed == true)
			Face->isFixedDraw = true;
		else Face->isFixedDraw = false;
	}
	printf("Finish input selection \n");
	pGLK->refresh(true);

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	//QMouseEvent *e = (QMouseEvent*)event;
	//QPoint pos = e->pos();
	//cout << "Mouse position updated" << endl;
	//double wx, wy, wz;
	//pGLK->screen_to_wcl(100.0, 100.0, wx, wy, wz);
	//ui->CorrdinateMouse->setText(QString("X = %1").arg(wx));

	//QString text;
	//text = QString("%1 X %2").arg(event->pos().x()).arg(event->pos().y());
	///** Update the info text */
	//ui->statusBar->showMessage(text);
}

void MainWindow::signalNavigation(int flag)
{
    if (flag <= 10)
        pGLK->setNavigation(flag);
    if (flag >=20 && flag <=25){
        pGLK->setViewModel(flag-20);
        switch (flag) {
        case 20:
            ui->actionShade->setChecked(pGLK->getViewModel(0));
            break;
        case 21:
            ui->actionMesh->setChecked(pGLK->getViewModel(1));
            break;
        case 22:
            ui->actionNode->setChecked(pGLK->getViewModel(2));
            break;
        case 23:
            ui->actionProfile->setChecked(pGLK->getViewModel(3));
            break;
        case 24:
            ui->actionFaceNormal->setChecked(pGLK->getViewModel(4));
            break;
        case 25:
            ui->actionNodeNormal->setChecked(pGLK->getViewModel(5));
            break;
        }
    }
    if (flag==30 || flag==31 || flag==32 || flag == 33 || flag == 34){
        InteractiveTool *tool;
        switch (flag) {
        case 30:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), NODE, ui->boxDeselect->isChecked());
            break;
        case 31:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), EDGE, ui->boxDeselect->isChecked());
            break;
        case 32:
            tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), FACE, ui->boxDeselect->isChecked());
            break;
		case 33:
			tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), FIX, ui->boxDeselect->isChecked());
			break;
		case 34:
			tool = new InteractiveTool(pGLK, &polygenMeshList, (GLKMouseTool*)pGLK->GetCurrentTool(), NHANDLE, ui->boxDeselect->isChecked());
			break;
        }
        pGLK->set_tool(tool);
    }
}

void MainWindow::shiftToOrigin()
{
    
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString filenameStr;
    foreach (const QUrl &url, event->mimeData()->urls())
        filenameStr = url.toLocalFile();
    QByteArray filenameArray = filenameStr.toLatin1();
    char *filename = filenameArray.data();

    PolygenMesh *polygenMesh = new PolygenMesh(UNDEFINED);

    // set polygen name
    std::string strFilename(filename);
    std::size_t foundStart = strFilename.find_last_of("/");
    std::size_t foundEnd = strFilename.find_last_of(".");
    std::string modelName;
    modelName = strFilename.substr(0,foundEnd);
    modelName = modelName.substr(foundStart+1);
    int i = 0;
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygen = (PolygenMesh*)polygenMeshList.GetNext(pos);
        std::string name = (polygen->getModelName()).substr(0,(polygen->getModelName()).find(' '));
        if (name == modelName)
            i++;
    }
    if (i > 0)
        modelName += " "+std::to_string(i);

	QFileInfo fileInfo(filenameStr);
	QString fileSuffix = fileInfo.suffix();
	if (QString::compare(fileSuffix, "obj") == 0) {
		polygenMesh->ImportOBJFile(filename, modelName);
	}
	else if (QString::compare(fileSuffix, "tet") == 0) {
		polygenMesh->ImportTETFile(filename, modelName);
        polygenMesh->meshType = TET;
	}
	polygenMesh->m_bVertexNormalShading = false;	
    polygenMesh->BuildGLList(polygenMesh->m_bVertexNormalShading);
    pGLK->AddDisplayObj(polygenMesh,true);
    polygenMeshList.AddTail(polygenMesh);
    
    updateTree();
}

void MainWindow::createTreeView()
{
    treeModel = new QStandardItemModel();
    ui->treeView->setModel(treeModel);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->expandAll();
}

void MainWindow::updateTree()
{
    treeModel->clear();
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
        QStandardItem *modelListItem = new QStandardItem(modelName);
        modelListItem->setCheckable(true);
        modelListItem->setCheckState(Qt::Checked);
        treeModel->appendRow(modelListItem);
    }
	pGLK->refresh(true);
}

PolygenMesh *MainWindow::getSelectedPolygenMesh()
{
    if (!treeModel->hasChildren())
        return nullptr;
    QModelIndex index = ui->treeView->currentIndex();
    QString selectedModelName = index.data(Qt::DisplayRole).toString();
    for (GLKPOSITION pos=polygenMeshList.GetHeadPosition(); pos!=nullptr;){
        PolygenMesh *polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        QString modelName = QString::fromStdString(polygenMesh->getModelName());
        if (QString::compare(selectedModelName,modelName) == 0)
            return polygenMesh;
    }
    return nullptr;
}

void MainWindow::on_pushButton_clearAll_clicked()
{
    Gcode_timer.stop();

    for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
        PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        pGLK->DelDisplayObj(polygenMesh);
    }
    polygenMeshList.RemoveAll();
    pGLK->ClearDisplayObjList();

    pGLK->refresh();
    updateTree();
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    ui->treeView->currentIndex();
    QStandardItem *modelListItem = treeModel->itemFromIndex(index);
    ui->treeView->setCurrentIndex(index);
    PolygenMesh *polygenMesh = getSelectedPolygenMesh();
    if (modelListItem->checkState() == Qt::Checked)
        polygenMesh->bShow = true;
    else
        polygenMesh->bShow = false;
    pGLK->refresh(true);
}

PolygenMesh* MainWindow::_detectPolygenMesh(mesh_type type) {

    PolygenMesh* detectedMesh = NULL;
    for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
        PolygenMesh* thispolygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        if (thispolygenMesh->meshType == type) {
            detectedMesh = thispolygenMesh; break;
        }
    }
    return detectedMesh;
}

PolygenMesh* MainWindow::_buildPolygenMesh(mesh_type type, std::string name) {

    PolygenMesh* newMesh = new PolygenMesh(type);
    newMesh->setModelName(name);
    newMesh->BuildGLList(newMesh->m_bVertexNormalShading);
    pGLK->AddDisplayObj(newMesh, true);
    polygenMeshList.AddTail(newMesh);
    updateTree();
    return newMesh;

}

void MainWindow::update_Layer_Display() {

    bool single = ui->checkBox_EachLayerSwitch->isChecked();
    int currentLayerIndex = ui->spinBox_ShowLayerIndex->value();

    for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
        PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);
        if (polygenMesh->meshType != CURVED_LAYER
            && polygenMesh->meshType != TOOL_PATH) continue;

        for (GLKPOSITION posMesh = polygenMesh->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
            QMeshPatch* Patch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posMesh);

            Patch->drawThisPatch = false;

            if (single) {
                if (Patch->GetIndexNo() == currentLayerIndex)
                    Patch->drawThisPatch = true;
            }
            else {
                if (Patch->GetIndexNo() <= currentLayerIndex)
                    Patch->drawThisPatch = true;
            }
        }
    }


    pGLK->refresh(true);
}

void MainWindow::all_Display() {
    for (GLKPOSITION pos = polygenMeshList.GetHeadPosition(); pos != nullptr;) {
        PolygenMesh* polygenMesh = (PolygenMesh*)polygenMeshList.GetNext(pos);

        if (polygenMesh->meshType != CURVED_LAYER
            && polygenMesh->meshType != TOOL_PATH) continue;

        for (GLKPOSITION posMesh = polygenMesh->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
            QMeshPatch* Patch = (QMeshPatch*)polygenMesh->GetMeshList().GetNext(posMesh);
            Patch->drawThisPatch = true;
        }
    }
    pGLK->refresh(true);
}

void MainWindow::readSourceLayer() {

    this->on_pushButton_clearAll_clicked();

    PolygenMesh* isoLayerSet = this->_detectPolygenMesh(CURVED_LAYER);
    if (isoLayerSet == NULL) {
        isoLayerSet = this->_buildPolygenMesh(CURVED_LAYER, "IsoLayer");
    }
    else {
        isoLayerSet->ClearAll();
        std::cout << "There is already existing a isoLayerSet PolygenMesh, it will be reconstructed!" << std::endl;
    }

    fileIO* IO_operator = new fileIO();
    std::string path = "../DataSet/source_layer/output";
    IO_operator->input_remeshed_Layer(isoLayerSet, path);
    delete IO_operator;

    //set the move and rot value for modle
    ui->doubleSpinBox_Xmove->setValue(0.0);
    ui->doubleSpinBox_Ymove->setValue(0.0);
    ui->doubleSpinBox_Zmove->setValue(0.0);
    ui->doubleSpinBox_Xrot->setValue(0.0);
    ui->doubleSpinBox_Yrot->setValue(0.0);
    ui->doubleSpinBox_Zrot->setValue(0.0);

    ui->spinBox_ShowLayerIndex->setMaximum(isoLayerSet->GetMeshList().GetCount() - 1);

    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
}

void MainWindow::update_model_postion_orientation() {

    PolygenMesh* layerSet = this->_detectPolygenMesh(CURVED_LAYER);
    if (layerSet == nullptr) { std::cerr << "No tet mesh is detected!" << std::endl; return; }

    double xMove = ui->doubleSpinBox_Xmove->value();
    double yMove = ui->doubleSpinBox_Ymove->value();
    double zMove = ui->doubleSpinBox_Zmove->value();
    double xRot = ui->doubleSpinBox_Xrot->value();
    double yRot = ui->doubleSpinBox_Yrot->value();
    double zRot = ui->doubleSpinBox_Zrot->value();

    for (GLKPOSITION posMesh = layerSet->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
        QMeshPatch* each_patch = (QMeshPatch*)layerSet->GetMeshList().GetNext(posMesh);

        this->_rot_And_Move_Model(each_patch, xRot, yRot, zRot, xMove, yMove, zMove, false);
    }

    ui->pushButton_spiralTp_generation->setEnabled(true);
    ui->pushButton_contourTp_generation->setEnabled(true);
    ui->pushButton_zigzagTp_generation->setEnabled(true);

    fileIO* IO_operator = new fileIO();
    std::string path = "../DataSet/CURVED_LAYER/";
    IO_operator->outputIsoSurface(layerSet, path);
    delete IO_operator;

    std::cout << "\nFinish update_model_postion_orientation.\n" << std::endl;

    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
}

//Euler rotation operation
void MainWindow::_rot_And_Move_Model(QMeshPatch* m_tetModel, double xRot, double yRot, double zRot,
    double xMove, double yMove, double zMove, bool isUpdate_lastCoord3D) {

    //rotate model
    double pitch = DEGREE_TO_ROTATE(xRot);
    double yaw = DEGREE_TO_ROTATE(yRot);
    double roll = DEGREE_TO_ROTATE(zRot);

    Eigen::AngleAxisd rollAngle(roll, Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd yawAngle(yaw, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd pitchAngle(pitch, Eigen::Vector3d::UnitX());

    Eigen::Quaternion<double> q = pitchAngle * yawAngle * rollAngle;

    Eigen::Matrix3d rotationMatrix = q.matrix();

    for (GLKPOSITION posMesh = m_tetModel->GetNodeList().GetHeadPosition(); posMesh != nullptr;) {
        QMeshNode* node = (QMeshNode*)m_tetModel->GetNodeList().GetNext(posMesh);

        Eigen::Vector3d pp; node->GetCoord3D_last(pp(0), pp(1), pp(2));
        Eigen::Vector3d rotatedpp = rotationMatrix * pp;

        node->SetCoord3D(rotatedpp[0], rotatedpp[1], rotatedpp[2]);

        //std::cout << "pp \t" << pp.transpose() << "\n";
        //std::cout << "rotatedpp \t" << rotatedpp.transpose() << "\n\n";
        //std::cout << "pp -  rotatedpp \t" << pp.transpose() - rotatedpp.transpose() << "\n";
    }

    //std::cout << "\n-----------\nRotationMatrix:\n" << rotationMatrix << std::endl;
    //std::cout << "\nxRot: " << xRot << "\tyRot: " << yRot << "\tzRot: " << zRot;
    //std::cout << "\nFinish rotate model." << std::endl;

    //move model
    double pp[3];
    for (GLKPOSITION Pos = m_tetModel->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)m_tetModel->GetNodeList().GetNext(Pos);
        Node->GetCoord3D(pp[0], pp[1], pp[2]);

        pp[0] += xMove; pp[1] += yMove; pp[2] += zMove;
        Node->SetCoord3D(pp[0], pp[1], pp[2]);
        if (isUpdate_lastCoord3D)   Node->SetCoord3D_last(pp[0], pp[1], pp[2]);
    }
    //std::cout << "\nxMove: " << xMove << "\tyMove: " << yMove << "\tzMove: " << zMove;
    //std::cout << "\nFinish move model.\n" << std::endl;
}

void MainWindow::spiralToolpath_generation() {

    PolygenMesh* Model = this->_detectPolygenMesh(CURVED_LAYER);

    if (Model == NULL) {
        std::cout << "NO curved layers in the system!\nplease check ..." << std::endl;
        return;
    }

    for (GLKPOSITION posMesh = Model->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
        QMeshPatch* Patch = (QMeshPatch*)Model->GetMeshList().GetNext(posMesh);

        double pp[3];

        // record the position of the modified model into Coord3D_last
        for (GLKPOSITION Pos = Patch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)Patch->GetNodeList().GetNext(Pos);
            
            Node->GetCoord3D(pp[0], pp[1], pp[2]);
            Node->SetCoord3D_last(pp[0], pp[1], pp[2]);
        }

        std::cout << "Heat field Compute Running ..." << std::endl;
        std::printf("------------------------------------------------\n");
        long time = clock();
        /* ---- Generate boundary heat field ---- */
        heatMethodField* heatField_layer = new heatMethodField(Patch);
        heatField_layer->meshRefinement();
        heatField_layer->compBoundaryHeatKernel();
        delete heatField_layer;
        /* ---- END ---- */

        std::printf("--> Solve takes %ld ms.\n", clock() - time);
        std::printf("------------------------------------------------\n");

        for (GLKPOSITION Pos = Patch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)Patch->GetNodeList().GetNext(Pos);

            if (!isfinite(Node->boundaryValue))
                std::cout << " error: Node index: " << Node->GetIndexNo() << " Dist = "
                << Node->boundaryValue << std::endl;
        }


        std::cout << "Spiral ToolPath Compute Running ..." << std::endl;
        std::printf("------------------------------------------------\n");
        long time1 = clock();


        std::string spiralPath_Name = "SpiralPath " + to_string(Patch->GetIndexNo());
        PolygenMesh* spiralTPath = this->_buildPolygenMesh(SPIRAL_PATH, spiralPath_Name);


        spiralToolpath* spiralPathComp_layer = new spiralToolpath(Patch, spiralTPath, 
            ui->doubleSpinBox_toolPathWidth->value(), ui->doubleSpinBox_toolPathDistance->value());
        spiralPathComp_layer->generateSpiralToolPath();

        fileIO* IO_operator = new fileIO();
        std::string dir = "../DataSet/TOOL_PATH";
        IO_operator->spiralToolpath_Output(spiralTPath, dir, Patch->GetIndexNo(), true);
        delete IO_operator;

        std::printf("--> Solve takes %ld ms.\n", clock() - time1);
        std::printf("------------------------------------------------\n");
        std::cout << "--------------------- End-----------------------" << std::endl;

        /* ---- END ---- */
    }

    updateTree();
    pGLK->Zoom_All_in_View();
    pGLK->refresh(true);
}

void MainWindow::contourToolPath_Generation() {

    PolygenMesh* isoLayerSet = this->_detectPolygenMesh(CURVED_LAYER);
    if (isoLayerSet == NULL) {
        std::cout << "Error: no layerSet, please check!" << std::endl;
        return;
    }

    PolygenMesh* toolpathSet = this->_buildPolygenMesh(TOOL_PATH, "toolPath");
    toolpathGeneration* ToolPathComp_layer = new toolpathGeneration(isoLayerSet, toolpathSet,
        ui->doubleSpinBox_toolPathWidth->value(), ui->doubleSpinBox_toolPathDistance->value());

    ToolPathComp_layer->generate_all_toolPath();
    delete ToolPathComp_layer;

    fileIO* IO_operator = new fileIO();
    std::string dir = "../DataSet/TOOL_PATH/";
    IO_operator->normalToolpath_Output(toolpathSet, dir);
    delete IO_operator;

    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
    std::cout << "Finish generating contour toolpath.\n" << std::endl;
}

void MainWindow::zigzigToolPath_Generation() {

    PolygenMesh* isoLayerSet = this->_detectPolygenMesh(CURVED_LAYER);
    if (isoLayerSet == NULL) {
        std::cout << "Error: no layerSet, please check!" << std::endl;
        return;
    }

    PolygenMesh* toolpathSet = this->_buildPolygenMesh(TOOL_PATH, "toolPath");
    toolpathGeneration* ToolPathComp_layer = new toolpathGeneration(isoLayerSet, toolpathSet,
        ui->doubleSpinBox_toolPathWidth->value(), ui->doubleSpinBox_toolPathDistance->value());

    ToolPathComp_layer->generate_all_hybrid_toolPath();
    delete ToolPathComp_layer;

    fileIO* IO_operator = new fileIO();
    std::string dir = "../DataSet/TOOL_PATH/";
    IO_operator->normalToolpath_Output(toolpathSet, dir);
    delete IO_operator;

    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
    std::cout << "Finish generating toolpath in the materialSpace.\n" << std::endl;
}

//CNC motion planning and simulation

void MainWindow::readGcodeSourceData() {

    this->on_pushButton_clearAll_clicked();

    PolygenMesh* isoLayerSet = this->_detectPolygenMesh(CURVED_LAYER);
    if (isoLayerSet == NULL) {
        isoLayerSet = this->_buildPolygenMesh(CURVED_LAYER, "IsoLayer");
    }
    else {
        isoLayerSet->ClearAll();
        std::cout << "There is already existing a isoLayerSet PolygenMesh, it will be reconstructed!" << std::endl;
    }

    PolygenMesh* toolpathSet = this->_detectPolygenMesh(TOOL_PATH);
    if (toolpathSet == NULL) {
        toolpathSet = this->_buildPolygenMesh(TOOL_PATH, "Toolpath");
    }
    else {
        toolpathSet->ClearAll();
        std::cout << "There is already existing a toolpathSet PolygenMesh, it will be reconstructed!" << std::endl;
    }

    PolygenMesh* cncSet = this->_detectPolygenMesh(CNC_PRT);
    if (cncSet == NULL) {
        cncSet = this->_buildPolygenMesh(CNC_PRT, "CNC_PRT");
    }
    else {
        cncSet->ClearAll();
        std::cout << "There is already existing a CNC part PolygenMesh, it will be reconstructed!" << std::endl;
    }

    fileIO* IO_operator = new fileIO();
    std::string modelName = ui->lineEdit_SorceDataDir->text().toStdString();
    std::string FileDir = "../DataSet/FABRICATION/";
    int layerNum = IO_operator->read_layer_toolpath_cnc_files(
        isoLayerSet, toolpathSet, cncSet, FileDir, modelName);
    delete IO_operator;

    GcodeGene = new GcodeGeneration();
    GcodeGene->initial(isoLayerSet, toolpathSet, cncSet);
    cncSet->bShow = false;

    ui->spinBox_ShowLayerIndex->setMaximum(layerNum - 1);
    ui->spinBox_GcodeGeneFromIndex->setMaximum(layerNum - 1);
    ui->spinBox_GcodeGeneToIndex->setMaximum(layerNum - 1);
    ui->spinBox_GcodeGeneToIndex->setValue(layerNum - 1);

    updateTree();
    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
    std::cout << "Finish inputing GcodeSource data.\n" << std::endl;
}

void MainWindow::runDHWcalculation() {

    GcodeGene->updateParameter(
        ui->spinBox_GcodeGeneFromIndex->value(), ui->spinBox_GcodeGeneToIndex->value(), 
        6.5, ui->doubleSpinBox_toolPathWidth->value(), ui->doubleSpinBox_toolLength->value());

    GcodeGene->calDHW();
    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
    std::cout << "Finish calculating extrusion volume.\n" << std::endl;
}

void MainWindow::runSingularityOpt() {

    GcodeGene->singularityOpt();
    pGLK->refresh(true);
    pGLK->Zoom_All_in_View();
    std::cout << "Finish singularity optimization.\n" << std::endl;
}

void MainWindow::runWriteGcode() {

    GcodeGene->feedrateOpt();

    string targetFileName = (ui->lineEdit_SorceDataDir->text()).toStdString() + "_Gcode.txt";
    GcodeGene->writeGcode(targetFileName);
    pGLK->refresh(true);
}

void MainWindow::runGcodeSimulation() {

    PolygenMesh* Slices = _detectPolygenMesh(CURVED_LAYER);
    if (Slices != NULL) {
        Slices->bShow = false;
    }
    else {
        std::cout << "There is no needed mesh, return." << std::endl;
        return;
    }

    PolygenMesh* cnc_Prt = _detectPolygenMesh(CNC_PRT);
    if (cnc_Prt != NULL) {
        cnc_Prt->bShow = true;
    }
    else {
        std::cout << "There is no needed mesh, return." << std::endl;
        return;
    }

    string FileName = (ui->lineEdit_SorceDataDir->text()).toStdString() + "_Gcode.txt";
    simuLayerInd = ui->spinBox_GcodeGeneFromIndex->value() - 1;//  will meet "G1 F1500" at the first several lines, so -1
    ui->checkBox_EachLayerSwitch->setCheckState(Qt::Checked);
    GcodeGene->readGcodeFile(Gcode_Table, FileName);
    ui->progressBar_GcodeSimulation->setRange(0, Gcode_Table.rows() - 1);
    ui->checkBox_showCNC->setEnabled(true);
    gcodetimerItertime = 0;
    Gcode_timer.start(20);

    std::cout << "------------------------------------------- G code Simulation Running ..." << std::endl;
}

void MainWindow::doTimerGcodeMoving() {

    int simuLayerInd_1st = ui->spinBox_GcodeGeneFromIndex->value();
    //int simuLayerInd_1st = 0;
    bool singleLayer = ui->checkBox_EachLayerSwitch->isChecked();
    ui->spinBox_ShowLayerIndex->setValue(simuLayerInd);
    double machine_X = Gcode_Table(gcodetimerItertime, 0);
    double machine_Y = Gcode_Table(gcodetimerItertime, 1);
    double machine_Z = Gcode_Table(gcodetimerItertime, 2);
    double machine_B = Gcode_Table(gcodetimerItertime, 3);
    double machine_C = Gcode_Table(gcodetimerItertime, 4);
    double newLayerFlag = Gcode_Table(gcodetimerItertime, 5);

    PolygenMesh* waypoints = _detectPolygenMesh(TOOL_PATH);
    PolygenMesh* cnc = _detectPolygenMesh(CNC_PRT);
    if (cnc == NULL || waypoints == NULL) {
        cerr << "There is no CNC model OR waypoints" << endl; return;
    }

    if (newLayerFlag != 0.0) {
        simuLayerInd++;
        std::cout << "Simulating the printing of Layer " << simuLayerInd << std::endl;
    }

    // Model waypoint show
    for (GLKPOSITION waypointsPos = waypoints->GetMeshList().GetHeadPosition(); waypointsPos;) {
        QMeshPatch* WayPointsPatch = (QMeshPatch*)waypoints->GetMeshList().GetNext(waypointsPos);
        // show one layer
        if (singleLayer == true) {
            if (WayPointsPatch->GetIndexNo() != simuLayerInd) {
                WayPointsPatch->drawThisPatch = false;	continue;
            }
            else {
                WayPointsPatch->drawThisPatch = true;
            }
        }
        // show before layers
        else {
            if (WayPointsPatch->GetIndexNo() <= simuLayerInd && WayPointsPatch->GetIndexNo() >= simuLayerInd_1st) {
                WayPointsPatch->drawThisPatch = true;
            }
            else {
                WayPointsPatch->drawThisPatch = false; 	continue;
            }
        }
    }

    //waypoint move
    for (GLKPOSITION waypointsPos = waypoints->GetMeshList().GetHeadPosition(); waypointsPos;) {
        QMeshPatch* WayPointsPatch = (QMeshPatch*)waypoints->GetMeshList().GetNext(waypointsPos);

        if (WayPointsPatch->GetIndexNo() > simuLayerInd) continue;

        for (GLKPOSITION Pos = WayPointsPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* node = (QMeshNode*)WayPointsPatch->GetNodeList().GetNext(Pos);

            Eigen::Vector3d waypoints_Base_Position = node->m_printPos;
            //node->GetCoord3D_last(waypoints_Base_Position[0], waypoints_Base_Position[1], waypoints_Base_Position[2]);
            Eigen::Vector3d waypoints_New_Position = _calPartGuesture(true, waypoints_Base_Position, 0.0, 0.0, 0.0, 0.0, machine_C);
            node->SetCoord3D(waypoints_New_Position[0], waypoints_New_Position[1], waypoints_New_Position[2]);

            Eigen::Matrix3d Rot_C; double rad_C = DEGREE_TO_ROTATE(machine_C);
            Rot_C << cos(rad_C), -sin(rad_C), 0, sin(rad_C), cos(rad_C), 0, 0, 0, 1;
            Eigen::Vector3d waypoints_New_Normal = Rot_C * node->m_printNor;
            node->SetNormal(waypoints_New_Normal[0], waypoints_New_Normal[1], waypoints_New_Normal[2]);
        }
    }

    // CNC move
    for (GLKPOSITION cncPos = cnc->GetMeshList().GetHeadPosition(); cncPos;) {
        QMeshPatch* cncPatch = (QMeshPatch*)cnc->GetMeshList().GetNext(cncPos);

        if (!ui->checkBox_showCNC->isChecked()) {
            if (cncPatch->patchName == "c_axis" || cncPatch->patchName == "nozzle")
                cncPatch->drawThisPatch = true;
            else
                cncPatch->drawThisPatch = false;
        }
        else
            cncPatch->drawThisPatch = true;

        for (GLKPOSITION Pos = cncPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* node = (QMeshNode*)cncPatch->GetNodeList().GetNext(Pos);

            Eigen::Vector3d cnc_Base_Position; node->GetCoord3D_last(cnc_Base_Position[0], cnc_Base_Position[1], cnc_Base_Position[2]);
            Eigen::Vector3d cnc_New_Position;

            if (cncPatch->patchName == "b_axis" || cncPatch->patchName == "nozzle") {

                cnc_New_Position = _calPartGuesture(false, cnc_Base_Position, machine_X, machine_Y, machine_Z, machine_B, 0.0);
            }

            if (cncPatch->patchName == "c_axis") {

                cnc_New_Position = _calPartGuesture(true, cnc_Base_Position, 0.0, 0.0, 0.0, 0.0, machine_C);
            }

            if (cncPatch->patchName == "z_axis") {

                cnc_New_Position = _calPartGuesture(false, cnc_Base_Position, machine_X, machine_Y, machine_Z, 0.0, 0.0);
            }

            if (cncPatch->patchName == "y_axis") {

                cnc_New_Position = _calPartGuesture(false, cnc_Base_Position, machine_X, machine_Y, 0.0, 0.0, 0.0);
            }

            if (cncPatch->patchName == "x_axis") {

                cnc_New_Position = _calPartGuesture(false, cnc_Base_Position, machine_X, 0.0, 0.0, 0.0, 0.0);
            }

            if (cncPatch->patchName == "frame") {
                cnc_New_Position = _calPartGuesture(true, cnc_Base_Position, 0.0, 0.0, 0.0, 0.0, 0.0);
            }

            node->SetCoord3D(cnc_New_Position[0], cnc_New_Position[1], cnc_New_Position[2]);
        }
    }

    gcodetimerItertime++;
    ui->progressBar_GcodeSimulation->setValue(gcodetimerItertime);
    if (gcodetimerItertime >= Gcode_Table.rows()) {
        Gcode_timer.stop();
        std::cout << "------------------------------------------- G code Simulation Finish!" << endl;
    }
    pGLK->refresh(true);
    update_Layer_Display();
    bool stopSimulation_switch = ui->checkBox_stopSimulation->isChecked();
    if (stopSimulation_switch == true) {
        Gcode_timer.stop();
        ui->checkBox_stopSimulation->setCheckState(Qt::Unchecked);
        std::cout << "------------------------------------------- Quit G code Simulation !" << endl;
    }
}

Eigen::Vector3d MainWindow::_calPartGuesture(
    bool table_OR_head, Eigen::Vector3d printPos, double X, double Y, double Z, double B, double C) {

    Eigen::Vector3d temp = Eigen::Vector3d::Zero();
    Eigen::Vector4d temp_4d = Eigen::Vector4d::Zero();

    Eigen::Vector4d part_Postion = Eigen::Vector4d::Zero();
    part_Postion << printPos[0], printPos[1], printPos[2], 1.0;
    double rad_B = DEGREE_TO_ROTATE(B);
    double rad_C = DEGREE_TO_ROTATE(C);

    double h = ui->doubleSpinBox_toolLength->value();

    if (table_OR_head) {

        Eigen::Matrix4d Rot_C;
        Rot_C << cos(rad_C), -sin(rad_C), 0, 0,
            sin(rad_C), cos(rad_C), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

        temp_4d = Rot_C * part_Postion;

    }
    else {

        Eigen::Matrix4d Offset;
        Offset << 1, 0, 0, X,
            0, 1, 0, Y,
            0, 0, 1, Z + h,
            0, 0, 0, 1;

        Eigen::Matrix4d Rot_B;
        Rot_B << 1, 0, 0, 0,
            0, cos(rad_B), -sin(rad_B), 0,
            0, sin(rad_B), cos(rad_B), 0,
            0, 0, 0, 1;

        Eigen::Matrix4d Offset_back;
        Offset_back << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, -h,
            0, 0, 0, 1;

        temp_4d = Offset * Rot_B * Offset_back * part_Postion;
    }
    temp << temp_4d[0], temp_4d[1], temp_4d[2];

    return temp;
}
