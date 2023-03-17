#include "toolpathgeneration.h"
#include <iostream>
#include <fstream>
#include "GLKGeometry.h"
#include "heatmethodfield.h"
#include "io.h"
#include "dirent.h"

toolpathGeneration::toolpathGeneration(PolygenMesh* isoLayerSet, PolygenMesh* toolPathSet,
	double deltaWidth, double deltaDistance){

	m_isoLayerSet = isoLayerSet;
	m_toolPathSet = toolPathSet;
	toolpath_Width = deltaWidth;
	toolpath_Distance = deltaDistance;
}

toolpathGeneration::~toolpathGeneration() {}

void toolpathGeneration::generate_all_toolPath() {

	if (toolpath_Width <= 0.0 || toolpath_Distance <= 0.0){

		std::cout << "width or distance is zero!" << std::endl;
		return;
	}

	int layerNum = m_isoLayerSet->GetMeshList().GetCount();
	std::vector<QMeshPatch*> sliceVector(layerNum);
	std::vector<QMeshPatch*> toolpathVector(layerNum);

	int tempInd = 0;
	for (GLKPOSITION posMesh = m_isoLayerSet->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshPatch* layer = (QMeshPatch*)m_isoLayerSet->GetMeshList().GetNext(posMesh);

		sliceVector[tempInd] = layer;
		tempInd++;
	}

#pragma omp parallel
	{
#pragma omp for  
		for (int i = 0; i < sliceVector.size(); i++) {

			/* ---- Generate boundary heat field ---- */
			QMeshPatch* layer = sliceVector[i];
			heatMethodField* heatField_layer = new heatMethodField(layer);

			//heatField_layer->meshRefinement();
			heatField_layer->compBoundaryHeatKernel();
			delete heatField_layer;
			/* ---- END ---- */

			QMeshPatch* singlePath = this->generate_each_bundaryToolPath(layer);
			this->resampleToolpath(singlePath);

			toolpathVector[layer->GetIndexNo()] = singlePath;
		}
	}

	std::cout << "\nThe number means Toolpath i is NULL, skip it." << std::endl;
	for (int i = 0; i < layerNum; i++) {

		if ((i + 1) % 100 == 0) std::cout << std::endl;
		if (toolpathVector[i] != NULL) std::cout << ".";
		else std::cout << i;

		if (toolpathVector[i] != NULL) {
			QMeshPatch* singlePath = toolpathVector[i];
			singlePath->SetIndexNo(i);
			m_toolPathSet->GetMeshList().AddTail(singlePath);
		}
	}
	std::cout << std::endl;
}

QMeshPatch* toolpathGeneration::generate_each_bundaryToolPath(QMeshPatch* surfaceMesh) {

	int curveNum = autoComputeTPathNum(surfaceMesh, true); int boundaryTp_num = 5;

	if (curveNum > 100) {
		std::cout << "layer = " << surfaceMesh->GetIndexNo() << " high curve num " << curveNum << "." << std::endl;
	}
	if (curveNum <= 0 || curveNum > 100) { return NULL; }
	
	if (curveNum < boundaryTp_num)	curveNum = boundaryTp_num; // small layers needs at lest 3 rings of boundary loop
	std::vector<double> isoValue(curveNum);// store all isoValue
	double deltaIsoValue = 1.0 / curveNum; // deltaIsoValue in [0-1]
	QMeshPatch* singlePath = new QMeshPatch;

	for (int i = 0; i < curveNum; i++) {

		isoValue[i] = (i + 0.5) * deltaIsoValue;
		//build isonode with each isoValue without linkage
		generateBoundaryIsoNode(singlePath, surfaceMesh, isoValue[i]);

		if (singlePath->GetNodeNumber() == 0)
			std::cout << "Warning! the boundary toolpath contains no isonode!" << std::endl;  //means no node in singlePath
	}

	//DEBUG
	//for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
	//	QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
	//	std::cout << thisNode->GetIndexNo() << std::endl;
	//}
	//

	double min_isoValue = isoValue[0];
	double max_isoValue = isoValue[curveNum - 1];
	double startIsoValue;
	//choose printing direction (inner -> outside(true)) or inverse(false)
	bool growDirction = true;
	if (growDirction)	startIsoValue = min_isoValue;
	else startIsoValue = max_isoValue;
	linkEachIsoNode(singlePath, startIsoValue);

	//DEBUG
	/*for (GLKPOSITION posEdge = singlePath->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)singlePath->GetEdgeList().GetNext(posEdge);
		std::cout << "start Node " << Edge->GetStartPoint()->GetIndexNo() << " end Node " << Edge->GetEndPoint()->GetIndexNo() << std::endl;
	}*/
	//END

	//printf("--> Bundary Toolpath Generation Finish\n");

	return singlePath;
}

//true->boundary; false->zigzag
int toolpathGeneration::autoComputeTPathNum(QMeshPatch* surfaceMesh, bool boundaryORzigzag) {
	
	double distanceTPath = this->toolpath_Width;
	if (!boundaryORzigzag) distanceTPath *= 1.00; //0.95

	int iter = 10;
	std::vector<Eigen::MatrixXd> isoPoint(10);
	Eigen::VectorXd isoPointNum(iter);

	int maxNodeNum = 10000;

	for (int i = 0; i < iter; i++) {
		isoPoint[i] = Eigen::MatrixXd::Zero(maxNodeNum, 3);

		double isoValue = (0.5 + i) * 1.0 / iter;

		int index = 0;
		for (GLKPOSITION Pos = surfaceMesh->GetEdgeList().GetHeadPosition(); Pos;) {
			QMeshEdge* Edge = (QMeshEdge*)surfaceMesh->GetEdgeList().GetNext(Pos);

			double a, b;
			if (boundaryORzigzag) {
				a = Edge->GetStartPoint()->boundaryValue;
				b = Edge->GetEndPoint()->boundaryValue;
			}
			else {
				a = Edge->GetStartPoint()->zigzagValue;
				b = Edge->GetEndPoint()->zigzagValue;
			}

			if ((isoValue - a) * (isoValue - b) < 0.0) {
				double alpha = (isoValue - a) / (b - a);
				double p1[3], p2[3], pp[3];
				Edge->GetStartPoint()->GetCoord3D(p1[0], p1[1], p1[2]);
				Edge->GetEndPoint()->GetCoord3D(p2[0], p2[1], p2[2]);

				for (int j = 0; j < 3; j++) {
					//compute the position for this isonode
					if (index > maxNodeNum) { printf("ERROR, node number too high!\n"); break; }
					isoPoint[i](index, j) = (1.0 - alpha) * p1[j] + alpha * p2[j];
				}
				index++;
			}
		}
		isoPointNum(i) = index;
		//std::cout << "isovalue " << isoValue << " layer has " << index << " points" << std::endl;
	}

	//fix bug
	double node_coord3D_sum = 0.0;
	for (int i = 0; i < iter - 1; i++) {

		node_coord3D_sum += isoPoint[i].sum();
	}
	//std::cout << surfaceMesh->GetIndexNo() << " " << node_coord3D_sum << std::endl;
	if (fabs(node_coord3D_sum) < 1e-5) return 0;


	Eigen::VectorXd distance(iter - 1);
	for (int i = 0; i < iter - 1; i++) {
		distance(i) = 1000000.0;

		for (int j = 0; j < isoPointNum(i); j++) {
			for (int k = 0; k < isoPointNum(i + 1); k++) {
				double dis = (isoPoint[i].row(j) - isoPoint[i + 1].row(k)).norm();
				if (dis < distance(i)) distance(i) = dis;
			}
		}
	}
	//std::cout << distance << std::endl; // return the number of cut
	return floor(distance.sum() / distanceTPath); 
}

void toolpathGeneration::generateBoundaryIsoNode(QMeshPatch* singlePath, QMeshPatch* surfaceMesh, double isoValue) {

	//when the node iso-value is equal to surface value, add this eps.
	/*double eps = 1.0e-5;
	for (GLKPOSITION Pos = surfaceMesh->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)surfaceMesh->GetNodeList().GetNext(Pos);
		if (fabs(Node->boundaryValue - isoValue) < eps) {
			if (Node->boundaryValue > isoValue) Node->boundaryValue = isoValue + eps;
			else Node->boundaryValue = isoValue - eps;
		}
	}*/

	//----build the node and install back to the "singlePath"
	for (GLKPOSITION Pos = surfaceMesh->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)surfaceMesh->GetEdgeList().GetNext(Pos);

		double a = Edge->GetStartPoint()->boundaryValue;
		double b = Edge->GetEndPoint()->boundaryValue;

		if ((isoValue - a) * (isoValue - b) < 0.0) {
			double alpha = (isoValue - a) / (b - a);
			double p1[3], p2[3], pp[3];
			Edge->GetStartPoint()->GetCoord3D(p1[0], p1[1], p1[2]);
			Edge->GetEndPoint()->GetCoord3D(p2[0], p2[1], p2[2]);

			for (int j = 0; j < 3; j++) {
				//compute the position for this isonode
				pp[j] = (1.0 - alpha) * p1[j] + alpha * p2[j];
			}

			QMeshNode* isoNode = new QMeshNode;
			isoNode->relatedLayerEdge = Edge;
			isoNode->connectTPathProcessed = false;
			isoNode->isoValue = isoValue;

			isoNode->SetMeshPatchPtr(singlePath);
			isoNode->SetCoord3D(pp[0], pp[1], pp[2]);
			isoNode->SetIndexNo(singlePath->GetNodeList().GetCount()); //index should start from 0

			//cal normal of iso point
			double n1[3], n2[3], n3[3];
			Edge->GetLeftFace()->GetNormal(n1[0], n1[1], n1[2]);
			Edge->GetRightFace()->GetNormal(n2[0], n2[1], n2[2]);
			for (int i = 0; i < 3; i++) n3[i] = (n1[i] + n2[i]) / 2;
			isoNode->SetNormal(n3[0], n3[1], n3[2]);

			//install this isoNode of layer to its Edge
			Edge->installedIsoNode_layerEdge.push_back(isoNode);
			Edge->isLocateIsoNode_layerEdge = true;
			singlePath->GetNodeList().AddTail(isoNode);
		}
	}
}

void toolpathGeneration::linkEachIsoNode(QMeshPatch* singlePath, double startIsoValue) {

	/*DEGUG*/
	//for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
	//	QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
	//	std::cout << "thisNode->connectTPathProcessed = " << thisNode->connectTPathProcessed 
	//		<< " thisNode->isoValue = " << thisNode->isoValue << std::endl;
	//}
	/*END*/

	// Method 1: find the First Node(boundFirstNode) which is nearest with x axis
	QMeshNode* X_nearestFirstNode = nullptr;
	double minX = 1e10;
	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (thisNode->connectTPathProcessed == false && startIsoValue == thisNode->isoValue) {
			
			double px = 0.0; double py = 0.0; double pz = 0.0;
			thisNode->GetCoord3D(px, py, pz);
			if (px < minX) { 
				minX = px; 
				X_nearestFirstNode = thisNode;
			}
		}
	}
	QMeshNode* boundFirstNode = X_nearestFirstNode;
	boundFirstNode->connectTPathProcessed = true;

	// Method 2: get a First Node(boundFirstNode) to start the toolPath (randomly)
	//QMeshNode* boundFirstNode = nullptr;
	//for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
	//	QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
	//	if (thisNode->connectTPathProcessed == false && startIsoValue == thisNode->isoValue) {
	//		thisNode->connectTPathProcessed = true;
	//		boundFirstNode = thisNode;
	//		break;
	//	}
	//}

	// protect output
	if (boundFirstNode == NULL) std::cout << "Cannot find the start point!, please check." << std::endl;

	QMeshNode* sNode = boundFirstNode;
	QMeshNode* eNode = findNextBoundaryToolPath(sNode, singlePath);

	/* Link all of iso-Node in one Layer*/
	do {
		
		// Start linking one ring with same iso-Value
		QMeshNode* unlinked_eNode = link_OneRing_isoNode(singlePath, sNode, eNode);
		if (unlinked_eNode == nullptr) return;
		
		if (detectAll_isoNode_Processed(singlePath)) return;
		
		QMeshNode* link_sNode = unlinked_eNode;
		QMeshNode* link_eNode = findNextNearestPoint(link_sNode, singlePath);
		if (link_eNode == nullptr) std::cout << "Error: Cannot find link_eNode" << std::endl;
		QMeshEdge* link_Edge = buildNewEdgetoQMeshPatch(singlePath, link_sNode, link_eNode);
		link_Edge->isConnectEdge = true;

		sNode = link_eNode;
		eNode = findNextBoundaryToolPath(sNode, singlePath);
		if (eNode == nullptr) std::cout << "Error: Cannot find next toolpath Node" << std::endl;

	} while (detectAll_isoNode_Processed(singlePath) == false);
}

QMeshNode* toolpathGeneration::findNextBoundaryToolPath(QMeshNode* sNode, QMeshPatch* singlePath) {

	QMeshNode* eNode;
	bool nextNodeDetected = false;
	QMeshEdge* thisEdge = sNode->relatedLayerEdge;

	//detect left face
	for (int i = 0; i < 3; i++) {
		QMeshEdge* NeighborEdge = thisEdge->GetLeftFace()->GetEdgeRecordPtr(i + 1);
		if (NeighborEdge == thisEdge) continue;

		if (NeighborEdge->isLocateIsoNode_layerEdge) {

			for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge.size(); j++) {

				QMeshNode* bNode = NeighborEdge->installedIsoNode_layerEdge[j];
				if (bNode->connectTPathProcessed == false && bNode->isoValue == sNode->isoValue) {
					nextNodeDetected = true;
					eNode = bNode;
					break;
				}
			}
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) {
		eNode->connectTPathProcessed = true;
		return eNode;
	}

	//detect right face
	for (int i = 0; i < 3; i++) {
		QMeshEdge* NeighborEdge = thisEdge->GetRightFace()->GetEdgeRecordPtr(i + 1);
		if (NeighborEdge == thisEdge) continue;

		if (NeighborEdge->isLocateIsoNode_layerEdge) {

			for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge.size(); j++) {

				QMeshNode* bNode = NeighborEdge->installedIsoNode_layerEdge[j];
				if (bNode->connectTPathProcessed == false && bNode->isoValue == sNode->isoValue) {
					nextNodeDetected = true;
					eNode = bNode;
					break;
				}
			}
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) { 
		eNode->connectTPathProcessed = true;
		return eNode; 
	}else {
		//std::cout<<"Error, the next node is not found!"<<std::endl;
		return nullptr;
	}
}

QMeshEdge* toolpathGeneration::buildNewEdgetoQMeshPatch(QMeshPatch* patch, QMeshNode* startNode, QMeshNode* endNode) {

	QMeshEdge* isoEdge = new QMeshEdge;

	//std::cout << "start Node " << startNode->GetIndexNo() << " end Node " << endNode->GetIndexNo() << std::endl;

	isoEdge->SetStartPoint(startNode);
	isoEdge->SetEndPoint(endNode);

	isoEdge->SetMeshPatchPtr(patch);
	//isoEdge->SetIndexNo(patch->GetEdgeList().GetCount() + 1);
	isoEdge->SetIndexNo(patch->GetEdgeList().GetCount());

	(startNode->GetEdgeList()).AddTail(isoEdge);
	(endNode->GetEdgeList()).AddTail(isoEdge);
	patch->GetEdgeList().AddTail(isoEdge);

	return isoEdge;
}

QMeshNode* toolpathGeneration::findNextNearestPoint(QMeshNode* sNode, QMeshPatch* singlePath){

	GLKGeometry geo;
	double pp[3]; sNode->GetCoord3D(pp[0], pp[1], pp[2]);
	double p1[3];
	double dist = 1000.0;
	QMeshNode* nextNode = nullptr;

	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (Node->connectTPathProcessed == true) continue;
		Node->GetCoord3D(p1[0],p1[1],p1[2]);
		double distancePP = geo.Distance_to_Point(pp, p1);
		if (distancePP < dist) {
			nextNode = Node;
			dist = distancePP;
		}
	}
	if (nextNode == nullptr) { 
		std::cout << "There is no isoNode need to link between different isoValue!" <<std::endl;
		return nullptr;
	}
	else {
		nextNode->connectTPathProcessed = true;
		return nextNode;
	}
}

bool toolpathGeneration::detectAll_isoNode_Processed(QMeshPatch* singlePath) {
	//if all the node being processed return true, else return false.
	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (thisNode->connectTPathProcessed == false) return false;
	}
	return true;
}

QMeshNode* toolpathGeneration::link_OneRing_isoNode(QMeshPatch* singlePath, QMeshNode* sNode, QMeshNode* eNode) {

	do {

		if (sNode == nullptr || eNode == nullptr) {
			std::cout << "NULL sNode or eNode" << std::endl;
			return nullptr;
		}

		buildNewEdgetoQMeshPatch(singlePath, sNode, eNode);

		sNode = eNode;
		eNode = findNextBoundaryToolPath(sNode, singlePath);

	} while (eNode != nullptr);

	return sNode;// return the sNode for the link opration between isoNode with different isoValue 
}

void toolpathGeneration::resampleToolpath(QMeshPatch* patch) {

	if (patch == NULL) return;

	double length = this->toolpath_Distance;

	for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);
		Node->resampleChecked = false;
	}

	QMeshEdge* sEdge = (QMeshEdge*)patch->GetEdgeList().GetHead();
	QMeshNode* sNode = sEdge->GetStartPoint(); // the first Node of Toolpath
	if (sEdge->GetStartPoint()->GetEdgeNumber() > 1) sNode = sEdge->GetEndPoint();

	QMeshNode* sPoint = sNode;	QMeshNode* ePoint;	double lsum = 0.0;// temp distance record
	// mark the resampleChecked nodes
	for (GLKPOSITION Pos = patch->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)patch->GetEdgeList().GetNext(Pos);

		ePoint = Edge->GetStartPoint();
		if (ePoint == sPoint) ePoint = Edge->GetEndPoint();

		// give the ancor points (END point)
		if (Edge == patch->GetEdgeList().GetTail()) {

			ePoint->resampleChecked = true;
			sPoint->resampleChecked = true;
			break;
		}
		// give the ancor points (Linkage point)
		if (Edge->isConnectEdge || Edge->isConnectEdge_zigzag) {

			sPoint->resampleChecked = true;	ePoint->resampleChecked = true;

			sPoint = ePoint;
			lsum = 0;
			continue;
		}

		lsum += Edge->CalLength();
		if (lsum > length) {
			
			ePoint->resampleChecked = true;
			sPoint = ePoint;
			lsum = 0;
		}
		else {
			sPoint = ePoint;
		}
	}

	// reorganize the toolpath node order(toolpath_NodeSet)
	std::vector<QMeshNode*> toolpath_NodeSet;
	toolpath_NodeSet.push_back(sNode);
	for (GLKPOSITION Pos = patch->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)patch->GetEdgeList().GetNext(Pos);

		ePoint = Edge->GetStartPoint();
		if (ePoint == sNode) ePoint = Edge->GetEndPoint();

		if (ePoint->resampleChecked)	toolpath_NodeSet.push_back(ePoint);

		sNode = ePoint;
	}

	////DEBUG
	//std::cout << "---------------------------------- " << std::endl;
	//for (int i = 0; i < toolpath_NodeSet.size(); i++) {
	//	std::cout << "Node " << toolpath_NodeSet[i]->GetIndexNo() << std::endl;
	//}
	////END

	//rebuild the edge
	patch->GetEdgeList().RemoveAll();
	for (int i = 0; i < ( toolpath_NodeSet.size() - 1); i++)
		buildNewEdgetoQMeshPatch(patch, toolpath_NodeSet[i], toolpath_NodeSet[i + 1]);

	//printf("--> Resample Toolpath Finish\n");
}


// hybrid toolpath generation
// use heat source for zigzag also
void toolpathGeneration::generate_all_hybrid_toolPath() {

	if (toolpath_Width <= 0.0 || toolpath_Distance <= 0.0) {

		std::cout << "width or distance is zero!" << std::endl;
		return;
	}

	int layerNum = m_isoLayerSet->GetMeshList().GetCount();
	std::vector<QMeshPatch*> sliceVector(layerNum);
	std::vector<QMeshPatch*> toolpathVector(layerNum);

	int tempInd = 0;
	for (GLKPOSITION posMesh = m_isoLayerSet->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshPatch* layer = (QMeshPatch*)m_isoLayerSet->GetMeshList().GetNext(posMesh);

		sliceVector[tempInd] = layer;
		tempInd++;
	}

#pragma omp parallel
	{
#pragma omp for  
		for (int i = 0; i < sliceVector.size(); i++) {

			/* ---- Generate boundary heat field ---- */
			QMeshPatch* layer = sliceVector[i];
			heatMethodField* heatField_layer = new heatMethodField(layer);

			heatField_layer->meshRefinement(); //it will break original flag
			heatField_layer->compBoundaryHeatKernel();
			heatField_layer->compZigZagFieldValue();// use vector/scalar to calculate zigzag value
			delete heatField_layer;
			/* ---- END ---- */

			QMeshPatch* singlePath = this->generate_each_hybrid_bundaryToolPath(layer);
			this->resampleToolpath(singlePath);

			toolpathVector[layer->GetIndexNo()] = singlePath;
		}
	}

	std::cout << "\nThe number means Toolpath i is NULL, skip it." << std::endl;
	for (int i = 0; i < layerNum; i++) {

		if ((i + 1) % 100 == 0) std::cout << std::endl;
		if (toolpathVector[i] != NULL) std::cout << ".";
		else std::cout << i;

		if (toolpathVector[i] != NULL) {
			QMeshPatch* singlePath = toolpathVector[i];
			singlePath->SetIndexNo(i);
			m_toolPathSet->GetMeshList().AddTail(singlePath);
		}
	}
	std::cout << std::endl;
}

QMeshPatch* toolpathGeneration::generate_each_hybrid_bundaryToolPath(QMeshPatch* surfaceMesh) {

	int curveNum = autoComputeTPathNum(surfaceMesh, true);	int boundaryTp_num = 2;

	bool only_boundary = false;

	if (curveNum > 500) {
		std::cout << "layer = " << surfaceMesh->GetIndexNo() << " high curve num " << curveNum << "." << std::endl;
	}
	if (curveNum <= 0 || curveNum > 500) { return NULL; }

	if (curveNum < boundaryTp_num) {
		curveNum = boundaryTp_num; // small layers needs at lest boundaryTp_num rings of boundary loop
		only_boundary = true;
	}
	std::vector<double> isoValue_boundary(curveNum);// store all isoValue
	double deltaIsoValue_boundary = 1.0 / curveNum; // deltaIsoValue in [0-1]
	QMeshPatch* singlePath = new QMeshPatch;
	singlePath->SetIndexNo(surfaceMesh->GetIndexNo());

	for (int i = (curveNum - boundaryTp_num); i < curveNum; i++) {

		isoValue_boundary[i] = (i + 0.5) * deltaIsoValue_boundary;
		//build isonode with each isoValue without linkage
		generateBoundaryIsoNode(singlePath, surfaceMesh, isoValue_boundary[i]);
	}
	if (singlePath->GetNodeNumber() == 0)
		std::cout << "Warning! the boundary toolpath contains no isonode!" << std::endl;  //means no node in singlePath

	//DEBUG
	//for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
	//	QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
	//	std::cout << thisNode->GetIndexNo() << std::endl;
	//}

	double min_isoValue = isoValue_boundary[curveNum - boundaryTp_num];
	double max_isoValue = isoValue_boundary[curveNum - 1];
	double startIsoValue;
	//choose printing direction (inner -> outside(true)) or inverse(false)
	bool growDirction = true;
	if (growDirction)	startIsoValue = min_isoValue;
	else startIsoValue = max_isoValue;
	linkEachIsoNode(singlePath, startIsoValue);

	//DEBUG
	/*for (GLKPOSITION posEdge = singlePath->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)singlePath->GetEdgeList().GetNext(posEdge);
		std::cout << "start Node " << Edge->GetStartPoint()->GetIndexNo() << " end Node " << Edge->GetEndPoint()->GetIndexNo() << std::endl;
	}*/
	//END

	if (only_boundary ) return singlePath;
	//if (singlePath->GetIndexNo() > 7) return singlePath; // for tube_new
	//return singlePath;
	//printf("--> Bundary Toolpath Generation Finish\n");


	/*Zigzag toolpath generation*/
	// toolpath node
	int contourNode_number = singlePath->GetNodeNumber();
	int zigzagNum = autoComputeTPathNum(surfaceMesh, false);
	if (zigzagNum > 500) {
		std::cout << "layer = " << surfaceMesh->GetIndexNo()
			<< " high zigzag num " << zigzagNum << "." << std::endl;
	}
	if (zigzagNum <= 0 || zigzagNum > 500) { return singlePath; }

	std::vector<double> isoValue_zigzag(zigzagNum);		// store all isoValue
	double deltaIsoValue_zigzag = 1.0 / zigzagNum;		// deltaIsoValue in [0-1]
	double boundaryField_threshold = 1.0 - boundaryTp_num * deltaIsoValue_boundary;

	for (int i = 0; i < zigzagNum; i++) {

		isoValue_zigzag[i] = (i + 0.5) * deltaIsoValue_zigzag;
		//build isonode with each isoValue without linkage
		generateZigzagIsoNode(singlePath, surfaceMesh, isoValue_zigzag[i], boundaryField_threshold);
	}
	if (singlePath->GetNodeNumber() == contourNode_number) {
		std::cout << "Warning! the zigzag toolpath of layer " 
			<< singlePath->GetIndexNo() <<" contains no isonode!" << std::endl;
		return singlePath;
	}


	// link each zigzag node
	// find the link node between boundary and zigzag (last_boundary_node)
	QMeshEdge* last_Edge = (QMeshEdge*)singlePath->GetEdgeList().GetTail();
	QMeshNode* startNode_lastEdge = last_Edge->GetStartPoint();
	QMeshNode* endNode_lastEdge = last_Edge->GetEndPoint();
	QMeshNode* last_boundaryNode = NULL;
	if ((startNode_lastEdge->GetEdgeNumber() == 2) && (endNode_lastEdge->GetEdgeNumber() == 1))	
		last_boundaryNode = endNode_lastEdge;
	else if((startNode_lastEdge->GetEdgeNumber() == 1) && (endNode_lastEdge->GetEdgeNumber() == 2))
		last_boundaryNode = startNode_lastEdge;
	else { std::cout << "error: there should be only one node without two neighbor." << std::endl; }
	
	linkEachIsoNode_zigzag(singlePath, last_boundaryNode);


	return singlePath;
}

void toolpathGeneration::generateZigzagIsoNode(QMeshPatch* singlePath, 
	QMeshPatch* surfaceMesh, double isoValue, double boundaryField_threshold) {

	//----build the node and install back to the "singlePath"
	for (GLKPOSITION Pos = surfaceMesh->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)surfaceMesh->GetEdgeList().GetNext(Pos);

		double a = Edge->GetStartPoint()->zigzagValue;
		double b = Edge->GetEndPoint()->zigzagValue;
		double boundaryValue = 0;

		if ((isoValue - a) * (isoValue - b) < 0.0) {
			double alpha = (isoValue - a) / (b - a);
			double p1[3], p2[3], pp[3];
			Edge->GetStartPoint()->GetCoord3D(p1[0], p1[1], p1[2]);
			Edge->GetEndPoint()->GetCoord3D(p2[0], p2[1], p2[2]);

			for (int j = 0; j < 3; j++) {
				//compute the position for this isonode
				pp[j] = (1.0 - alpha) * p1[j] + alpha * p2[j];
				//compute boundary value for this isonode
				boundaryValue = (1.0 - alpha) * Edge->GetStartPoint()->boundaryValue +
					alpha * Edge->GetEndPoint()->boundaryValue;
			}

			//detect if the node inside the boundary region // key step for zigzag shrink
			if (boundaryValue > boundaryField_threshold) continue;

			QMeshNode* isoNode = new QMeshNode;
			isoNode->relatedLayerEdge = Edge;
			isoNode->connectTPathProcessed = false;
			isoNode->isoValue = isoValue;
			isoNode->isZigzagNode = true;

			isoNode->SetMeshPatchPtr(singlePath);
			isoNode->SetCoord3D(pp[0], pp[1], pp[2]);
			//index start from boundaryNode num
			isoNode->SetIndexNo(singlePath->GetNodeList().GetCount()); 

			//cal normal of iso point
			double n1[3], n2[3]; Eigen::Vector3d n3;
			if (Edge->GetLeftFace() != NULL) Edge->GetLeftFace()->GetNormal(n1[0], n1[1], n1[2]);
			if (Edge->GetRightFace() != NULL) Edge->GetRightFace()->GetNormal(n2[0], n2[1], n2[2]);
			for (int i = 0; i < 3; i++) n3[i] = (n1[i] + n2[i]) / 2;
			if (n3.norm() < 0.01) std::cerr << "toolpath - zigzag - NORMAL ERROR!" << std::endl;
			n3 = n3.normalized();
			isoNode->SetNormal(n3[0], n3[1], n3[2]);

			//install this isoNode of layer to its Edge
			Edge->installedIsoNode_layerEdge_zigzag.push_back(isoNode);
			Edge->isLocateIsoNode_layerEdge_zigzag = true;
			singlePath->GetNodeList().AddTail(isoNode);
		}
	}

	//----Detect the iso node located at the two side of single tool-path, used for connection
	//Detect the side node by recording the nuber of isonode in the edges of neighbor face. 
	//If it is 2, middle node. If it is 1, side node.
	//Notice that, the edge contains zig-zag node normally not located at the boundary. If its a boundary edge, we directly
	//set this isonode as boundary of zig-zag tool-path.

	int zigzagPath_sideNode = 0;
	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);

		if (!thisNode->isZigzagNode) continue;
		QMeshEdge* thisEdge = thisNode->relatedLayerEdge;

		thisNode->isZigzag_sideNode = false;

		if (thisEdge->IsBoundaryEdge()) {
			thisNode->isZigzag_sideNode = true; zigzagPath_sideNode++;
			std::cout << "Warning:: Notice that, the zigzag toolpath node related to a boundary edge! " <<
				"You can try to make the boundary tool-path offset higher." << std::endl;
			continue;
		}

		int lFaceIsoNodeNum = 0; //left face
		for (int i = 0; i < 3; i++) {
			QMeshEdge* NeighborEdge = thisEdge->GetLeftFace()->GetEdgeRecordPtr(i + 1);
			if (NeighborEdge == thisEdge) continue;
			if (NeighborEdge->isLocateIsoNode_layerEdge_zigzag) {
				for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge_zigzag.size(); j++) {
					if (NeighborEdge->installedIsoNode_layerEdge_zigzag[j]->isoValue
						== thisNode->isoValue) {
						lFaceIsoNodeNum++;
					}
				}
			}
		}
		int rFaceIsoNodeNum = 0; //right face
		for (int i = 0; i < 3; i++) {
			QMeshEdge* NeighborEdge = thisEdge->GetRightFace()->GetEdgeRecordPtr(i + 1);
			if (NeighborEdge == thisEdge) continue;
			if (NeighborEdge->isLocateIsoNode_layerEdge_zigzag) {
				for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge_zigzag.size(); j++) {
					if (NeighborEdge->installedIsoNode_layerEdge_zigzag[j]->isoValue
						== thisNode->isoValue) {
						rFaceIsoNodeNum++;
					}
				}
			}
		}
		// isolated node, with no left and right boundary node connected, we ignore this region.
		if (lFaceIsoNodeNum == 0 && rFaceIsoNodeNum == 0) {
			thisNode->connectTPathProcessed = true; continue;
		}

		if (lFaceIsoNodeNum + rFaceIsoNodeNum < 2) {
			thisNode->isZigzag_sideNode = true;
			zigzagPath_sideNode++;
		}
	}
	if (zigzagPath_sideNode % 2 != 0) printf("zigzag path boundary node number error! should be even number\n");
}

void toolpathGeneration::linkEachIsoNode_zigzag(QMeshPatch* singlePath, QMeshNode* last_boundaryNode) {

	// get a First Node(zigzag FirstNode) to start the toolPath
	QMeshNode* zigzagFirstNode = nullptr;		double min_dist_L2 = 1.0e10;
	Eigen::Vector3d coord3d; last_boundaryNode->GetCoord3D(coord3d[0], coord3d[1], coord3d[2]);
	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (!thisNode->isZigzag_sideNode) continue;
		if (thisNode->connectTPathProcessed == false) {

			Eigen::Vector3d inquire_coord3d; 
			thisNode->GetCoord3D(inquire_coord3d[0], inquire_coord3d[1], inquire_coord3d[2]);
			double dist_L2 = (inquire_coord3d - coord3d).squaredNorm();
			if (min_dist_L2 > dist_L2) { 
				min_dist_L2 = dist_L2;
				zigzagFirstNode = thisNode;
			}
		}
	}

	zigzagFirstNode->connectTPathProcessed = true;
	// protect output
	if (zigzagFirstNode == NULL) std::cout << "Cannot find the start point!, please check." << std::endl;

	QMeshEdge* link_Edge = buildNewEdgetoQMeshPatch(singlePath, last_boundaryNode, zigzagFirstNode);
	link_Edge->isConnectEdge = true;
	QMeshNode* sNode = zigzagFirstNode;
	QMeshNode* eNode = findNextZigzagToolPath(sNode, singlePath);

	/* Link all of iso-Node in one Layer*/
	do {

		// Start linking one ring with same iso-Value
		QMeshNode* unlinked_eNode = link_OneLine_isoNode(singlePath, sNode, eNode);
		if (unlinked_eNode == nullptr) return;

		if (detectAll_isoNode_Processed(singlePath)) return;

		QMeshNode* link_sNode = unlinked_eNode;
		QMeshNode* link_eNode = findNextNearest_sidePoint(link_sNode, singlePath);
		if (link_eNode == nullptr) {
			std::cout << "Error: Cannot find link_eNode: Layer: "<< singlePath->GetIndexNo() << std::endl;
			return;
		}
		QMeshEdge* link_Edge = buildNewEdgetoQMeshPatch(singlePath, link_sNode, link_eNode);

		link_Edge->isConnectEdge = true;

		sNode = link_eNode;
		eNode = findNextZigzagToolPath(sNode, singlePath);
		if (eNode == nullptr) std::cout << "Error: Cannot find next toolpath Node." << std::endl;

	} while (detectAll_isoNode_Processed(singlePath) == false);
}

QMeshNode* toolpathGeneration::findNextZigzagToolPath(QMeshNode* sNode, QMeshPatch* singlePath) {

	QMeshNode* eNode;
	bool nextNodeDetected = false;
	QMeshEdge* thisEdge = sNode->relatedLayerEdge;

	//detect left face
	for (int i = 0; i < 3; i++) {
		QMeshEdge* NeighborEdge = thisEdge->GetLeftFace()->GetEdgeRecordPtr(i + 1);
		if (NeighborEdge == thisEdge) continue;

		if (NeighborEdge->isLocateIsoNode_layerEdge_zigzag) {

			for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge_zigzag.size(); j++) {

				QMeshNode* bNode = NeighborEdge->installedIsoNode_layerEdge_zigzag[j];
				if (bNode->connectTPathProcessed == false && bNode->isoValue == sNode->isoValue) {
					nextNodeDetected = true;
					eNode = bNode;
					break;
				}
			}
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) {
		eNode->connectTPathProcessed = true;
		return eNode;
	}

	//detect right face
	for (int i = 0; i < 3; i++) {
		QMeshEdge* NeighborEdge = thisEdge->GetRightFace()->GetEdgeRecordPtr(i + 1);
		if (NeighborEdge == thisEdge) continue;

		if (NeighborEdge->isLocateIsoNode_layerEdge_zigzag) {

			for (int j = 0; j < NeighborEdge->installedIsoNode_layerEdge_zigzag.size(); j++) {

				QMeshNode* bNode = NeighborEdge->installedIsoNode_layerEdge_zigzag[j];
				if (bNode->connectTPathProcessed == false && bNode->isoValue == sNode->isoValue) {
					nextNodeDetected = true;
					eNode = bNode;
					break;
				}
			}
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) {
		eNode->connectTPathProcessed = true;
		return eNode;
	}
	else {
		//std::cout<<"Error, the next node is not found!"<<std::endl;
		return nullptr;
	}
}

QMeshNode* toolpathGeneration::link_OneLine_isoNode(QMeshPatch* singlePath, QMeshNode* sNode, QMeshNode* eNode) {

	do {

		if (sNode == nullptr || eNode == nullptr) {
			std::cout << "NULL sNode or eNode" << std::endl;
			return nullptr;
		}

		buildNewEdgetoQMeshPatch(singlePath, sNode, eNode);

		sNode = eNode;
		eNode = findNextZigzagToolPath(sNode, singlePath);

	} while (eNode != nullptr);

	return sNode;// return the sNode for the link opration between isoNode with different isoValue 
}

QMeshNode* toolpathGeneration::findNextNearest_sidePoint(QMeshNode* sNode, QMeshPatch* singlePath) {

	GLKGeometry geo;
	double pp[3]; sNode->GetCoord3D(pp[0], pp[1], pp[2]);
	double p1[3];
	double dist = 1000.0;
	QMeshNode* nextNode = nullptr;

	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (Node->connectTPathProcessed) continue;
		if (!Node->isZigzag_sideNode) continue;
		Node->GetCoord3D(p1[0], p1[1], p1[2]);
		double distancePP = geo.Distance_to_Point(pp, p1);
		if (distancePP < dist) {
			nextNode = Node;
			dist = distancePP;
		}
	}
	if (nextNode == nullptr) {
		std::cout << "There is no side isoNode to link!" << std::endl;
		return nullptr;
	}
	else {
		nextNode->connectTPathProcessed = true;
		return nextNode;
	}
}

//Smooth for the layers which is used to generate toolpath
//temp operation
void toolpathGeneration::smoothingIsoSurface() {

	std::cout << "----------------------------------" << std::endl;

	int smoothLoop_time = 1;
	for (int i = 0; i < smoothLoop_time; i++) {

		for (GLKPOSITION posMesh = m_isoLayerSet->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
			QMeshPatch* layer = (QMeshPatch*)m_isoLayerSet->GetMeshList().GetNext(posMesh);

			for (GLKPOSITION Pos = layer->GetEdgeList().GetHeadPosition(); Pos;) {
				QMeshEdge* thisEdge = (QMeshEdge*)layer->GetEdgeList().GetNext(Pos);
				if (thisEdge->IsBoundaryEdge()) {
					thisEdge->GetStartPoint()->isoSurfaceBoundary = true;
					thisEdge->GetEndPoint()->isoSurfaceBoundary = true;
				}
			}

			//laplacian smoothness
			for (GLKPOSITION Pos = layer->GetNodeList().GetHeadPosition(); Pos;) {
				QMeshNode* thisNode = (QMeshNode*)layer->GetNodeList().GetNext(Pos);

				if (thisNode->isoSurfaceBoundary) continue;
				else {
					double pp[3] = { 0 }; int neighNum = 0;
					for (GLKPOSITION Pos = thisNode->GetEdgeList().GetHeadPosition(); Pos;) {
						QMeshEdge* neighEdge = (QMeshEdge*)thisNode->GetEdgeList().GetNext(Pos);

						QMeshNode* neighNode = neighEdge->GetStartPoint();
						if (neighNode == thisNode) neighNode = neighEdge->GetEndPoint();

						double p1[3];
						neighNode->GetCoord3D(p1[0], p1[1], p1[2]);

						for (int i = 0; i < 3; i++) pp[i] += p1[i];
						neighNum++;
					}
					for (int i = 0; i < 3; i++) pp[i] /= neighNum;
					thisNode->SetCoord3D(pp[0], pp[1], pp[2]);
				}
			}
		}
		std::cout << "smooth the layers once before toolpath generation" << std::endl;
	}
	std::cout << "----------------------------------" << std::endl;
}


















