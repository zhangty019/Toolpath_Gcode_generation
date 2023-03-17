#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QStandardItemModel>
#include "../GLKLib/GLKLib.h"
#include "../QMeshLib/PolygenMesh.h"
#include <omp.h>
#include <QTimer>
#include <QLabel>
#include "GcodeGeneration.h"

#define PI		3.141592654
#define DEGREE_TO_ROTATE(x)		0.0174532922222*x
#define ROTATE_TO_DEGREE(x)		57.295780490443*x

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	// Qtimer - defined function
    void doTimerGcodeMoving();

private:
    Ui::MainWindow *ui;
    GLKLib *pGLK;

    /* add for Gcode generation */
    QTimer Gcode_timer; //Gcode Simulation timer
    int gcodetimerItertime;
    int simuLayerInd;
    Eigen::MatrixXf Gcode_Table;
    //unsigned int operationTime = 0;
    /* ------------------------ */
	GLKObList polygenMeshList;

private:
    void createActions();
    void createTreeView();
	void showTetraDeformationRatio();
	void MoveHandleRegion();
	void QTgetscreenshoot();

    PolygenMesh *getSelectedPolygenMesh();

    QSignalMapper *signalMapper;
    QStandardItemModel *treeModel;

    GcodeGeneration* GcodeGene;

private:
    PolygenMesh* _buildPolygenMesh(mesh_type type, std::string name);
    PolygenMesh* _detectPolygenMesh(mesh_type type);
    // tianyu: 16/3/2023
    // developted for Undergraduate student project
    void _rot_And_Move_Model(QMeshPatch* m_tetModel, double xRot, double yRot, double zRot,
        double xMove, double yMove, double zMove, bool isUpdate_lastCoord3D);
    Eigen::Vector3d _calPartGuesture(bool table_OR_head, Eigen::Vector3d printPos,
        double X, double Y, double Z, double B, double C);



protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void open();
    void save();
	void saveSelection();
	void readSelection();

    void signalNavigation(int flag);
    void shiftToOrigin();
    void updateTree();
	void mouseMoveEvent(QMouseEvent *event);
    void on_pushButton_clearAll_clicked();
    void on_treeView_clicked(const QModelIndex &index);

	/*This is for toolpath generation*/
    void readSourceLayer();
    void update_model_postion_orientation();
    void spiralToolpath_generation();
    void contourToolPath_Generation();
    void zigzigToolPath_Generation();

    /*This is for G code generation and simulation*/
    void readGcodeSourceData();
    void runDHWcalculation();
    void runSingularityOpt();
    void runWriteGcode();
    void runGcodeSimulation();

    /*This is for display*/
    void update_Layer_Display();
    void all_Display();
};

#endif // MAINWINDOW_H
