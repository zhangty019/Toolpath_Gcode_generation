#pragma once

#include "QMeshPatch.h"
#include "PolygenMesh.h"
#include "QMeshNode.h"
#include "QMeshEdge.h"
#include "QMeshFace.h"

class spiralToolpath {

public:
	spiralToolpath(QMeshPatch* isoLayer, PolygenMesh* spiralPath, 
		double deltaWidth, double deltaDistance);
	~spiralToolpath();

	void generateSpiralToolPath();
	
private:

	void calIsoCurves();

	int autoComputeTPathNum();
	void generateBoundaryIsoNode(QMeshPatch* singlePath, double isoValue);
	void linkIsoCurve(QMeshPatch* isoCurve, double isoValue, QMeshNode* startNode);
	QMeshNode* findNextPathNode(QMeshNode* sNode, QMeshPatch* singlePath);
	QMeshEdge* buildNewEdgetoQMeshPatch(QMeshPatch* patch, QMeshNode* startNode, QMeshNode* endNode);
	void resampleIsoCurve(QMeshPatch* curvedPatch_i, QMeshPatch* curvedPatch_ii, 
		QMeshPatch* spiralCurvePatch);//C_i -> C_i+1
	double calCurveLength(QMeshPatch* isoCurve);
	void generateUniformIsoNode(QMeshPatch* each_isoCurve, QMeshPatch* uniform_isoCurve, double delta_Length, int secNum);
	void initialEdge_Condition(QMeshPatch* each_isoCurve);
	QMeshPatch* generateCutLine();
	QMeshNode* findNextCutNode(QMeshNode* sNode, QMeshPatch* singlePath);
	bool detectAll_CutNode_Processed(QMeshPatch* singlePath);
	QMeshNode* getCrossNode_cutIso(QMeshPatch* cutLine, double isoValue);
	QMeshFace* cutLineOnwhichFace(QMeshEdge* Edge);
	bool spiralToolpath::_lineIntersectSphere
	(Eigen::Vector3d& O, Eigen::Vector3d& T, Eigen::Vector3d& Center, double R, double& mu1, double& mu2);
	void _newPositionOnSphere(Eigen::Vector3d& before_Position, Eigen::Vector3d& after_Position);
	void _newPositionOnSurface(Eigen::Vector3d before_Position,
		Eigen::Vector3d& after_Position, Eigen::Vector3d& node_normal);
	bool IntersectTriangle(const Eigen::Vector3d& orig, const Eigen::Vector3d& dir,
		Eigen::Vector3d& v0, Eigen::Vector3d& v1, Eigen::Vector3d& v2, double& t);


	PolygenMesh* spiralPath;
	QMeshPatch* surfaceMesh;
	double toolpath_Width;
	double toolpath_Distance;
	bool m_runProject = true;
};