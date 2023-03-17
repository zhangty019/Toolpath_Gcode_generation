#include "spiralToolpath.h"

spiralToolpath::spiralToolpath(QMeshPatch* isoLayer, PolygenMesh* spiralTPath, double deltaWidth, double deltaDistance){

	surfaceMesh = isoLayer;
	spiralPath = spiralTPath;
	toolpath_Width = deltaWidth;
	toolpath_Distance = deltaDistance;
}

spiralToolpath::~spiralToolpath() {}

void spiralToolpath::generateSpiralToolPath() {

	calIsoCurves();

	std::vector <QMeshPatch*> isoCurveSet;
	for (GLKPOSITION posMesh = spiralPath->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshPatch* each_isoCurve = (QMeshPatch*)spiralPath->GetMeshList().GetNext(posMesh);

		if (each_isoCurve->curveType == 1) isoCurveSet.push_back(each_isoCurve);
	}

	QMeshPatch* spiralCurvePatch = new QMeshPatch;
	spiralPath->GetMeshList().AddTail(spiralCurvePatch);
	spiralCurvePatch->curveType = 3;	//normal iso-curve(spiral curve)

	for (int i = 0; i < isoCurveSet.size() - 1; i++) {

		resampleIsoCurve(isoCurveSet[i], isoCurveSet[i + 1], spiralCurvePatch);
	}

	
	//project the point onto the surface
	std::vector<QMeshNode*> NodeSet_spiral(spiralCurvePatch->GetNodeNumber());
	int temp = 0;
	for (GLKPOSITION Pos = spiralCurvePatch->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* Node = (QMeshNode*)spiralCurvePatch->GetNodeList().GetNext(Pos);

		NodeSet_spiral[temp] = Node;
		temp++;
	}

	if (m_runProject) {

#pragma omp parallel
		{
#pragma omp for  

			for (int nodeId = 0; nodeId < NodeSet_spiral.size(); nodeId++) {

				Eigen::Vector3d before_Position = Eigen::Vector3d::Zero();
				Eigen::Vector3d after_Position = Eigen::Vector3d::Zero();
				Eigen::Vector3d	node_Normal = Eigen::Vector3d::Zero();
				NodeSet_spiral[nodeId]->GetCoord3D(before_Position[0], before_Position[1], before_Position[2]);
				_newPositionOnSurface(before_Position, after_Position, node_Normal);

				NodeSet_spiral[nodeId]->SetCoord3D(after_Position[0], after_Position[1], after_Position[2]);// projecting
				NodeSet_spiral[nodeId]->SetNormal(node_Normal[0], node_Normal[1], node_Normal[2]);
				std::cout << "*";
			}
		}
	}
	std::cout << std::endl;


	//link each spiral Node
	for (int i = 0; i < NodeSet_spiral.size() - 1; i++) {
		buildNewEdgetoQMeshPatch(spiralCurvePatch, NodeSet_spiral[i], NodeSet_spiral[i + 1]);
	}
}

void spiralToolpath::calIsoCurves(){

	QMeshPatch* cutLine = generateCutLine();

	int curveNum = autoComputeTPathNum();
	double deltaIsoValue = 1.0 / curveNum; // deltaIsoValue in [0-1]
	std::vector<double> isoValue(curveNum);// store all isoValue
	std::vector<QMeshNode*> crossNodeSet(curveNum);// get cross (iso-cut) point into a vector<QMeshNode*>

	for (int i = 0; i < curveNum; i++) {
		isoValue[i] = 1 - (i + 0.5) * deltaIsoValue;// patches: [0.95 0.85 ... 0.05]

		//build Cut node with each isoValue without linkage
		crossNodeSet[i] = getCrossNode_cutIso(cutLine, isoValue[i]);
		//build isonode with each isoValue without linkage
		QMeshPatch* isoCurve = new QMeshPatch;
		generateBoundaryIsoNode(isoCurve, isoValue[i]);

		if (isoCurve->GetNodeNumber() == 0)
			std::cout << "Warning! "<< "isoValue: " << isoValue[i] << ". The iso-curve contains no isonode!" << std::endl;  //means no node in singlePath

		spiralPath->GetMeshList().AddTail(isoCurve);
		isoCurve->curveType = 1;	//normal iso-curve(non-uniformed)

		linkIsoCurve(isoCurve, isoValue[i], crossNodeSet[i]);
	}
}

int spiralToolpath::autoComputeTPathNum() {

	double distanceTPath = this->toolpath_Width;

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

			double a = Edge->GetStartPoint()->boundaryValue;
			double b = Edge->GetEndPoint()->boundaryValue;

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

void spiralToolpath::generateBoundaryIsoNode(QMeshPatch* singlePath, double isoValue) {

	//----build the node and install back to the "singlePath"
	for (GLKPOSITION Pos = surfaceMesh->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)surfaceMesh->GetEdgeList().GetNext(Pos);

		if (Edge->GetLeftFace() == NULL || Edge->GetRightFace() == NULL) continue;

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

void spiralToolpath::linkIsoCurve(QMeshPatch* isoCurve, double isoValue, QMeshNode* startNode) {

	// get a First Node(boundFirstNode) to start the toolPath
	isoCurve->GetNodeList().AddTail(startNode);

	// detect face containing iso-Node
	QMeshNode* boundFirstNode_onEdge = nullptr;
	bool nextNodeDetected = false;
	for (int i = 0; i < 3; i++) {
		QMeshEdge* thisEdge = startNode->relatedLayerFace->GetEdgeRecordPtr(i + 1);

		if (thisEdge->isLocateIsoNode_layerEdge) {

			for (int j = 0; j < thisEdge->installedIsoNode_layerEdge.size(); j++) {

				QMeshNode* bNode = thisEdge->installedIsoNode_layerEdge[j];
				double pp[3];
				bNode->GetCoord3D(pp[0], pp[1], pp[2]);

				if (bNode->connectTPathProcessed == false 
					&& bNode->isoValue == startNode->isoValue
					&& pp[1]>0.0 // control the link direction as anti-colockwise
					) {
					nextNodeDetected = true;
					boundFirstNode_onEdge = bNode;
					break;
				}
			}
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) {
		boundFirstNode_onEdge->connectTPathProcessed = true;
		buildNewEdgetoQMeshPatch(isoCurve, startNode, boundFirstNode_onEdge);
	}
	else {
		std::cout << "ERROR: cannot find boundFirstNode on the Edge, please check!" << std::endl;
	}
	
	//

	QMeshNode* sNode = boundFirstNode_onEdge;
	QMeshNode* eNode = findNextPathNode(sNode, isoCurve);
	/* Link all of iso-Node with the same isoValue*/
	do {

		// Start linking one ring with same iso-Value
		if (sNode == nullptr || eNode == nullptr) {
			std::cout << "NULL sNode or eNode" << std::endl;
			return;
		}// protection

		buildNewEdgetoQMeshPatch(isoCurve, sNode, eNode);

		sNode = eNode;
		eNode = findNextPathNode(sNode, isoCurve);

	} while (eNode != nullptr);

	//close the iso-Curve
	buildNewEdgetoQMeshPatch(isoCurve, sNode, startNode);

}

QMeshNode* spiralToolpath::findNextPathNode(QMeshNode* sNode, QMeshPatch* singlePath) {

	QMeshNode* eNode;
	bool nextNodeDetected = false;
	QMeshEdge* thisEdge = sNode->relatedLayerEdge;

	//detect left face
	for (int i = 0; i < 3; i++) {
		// avoid random link between faceSnode and edgeSnode
		if (thisEdge->GetLeftFace()->isLocateIsoNode_layerFace) continue;

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

		if (thisEdge->GetRightFace()->isLocateIsoNode_layerFace) continue;

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
	}
	else {
		//std::cout<<"Error, the next node is not found!"<<std::endl;
		return nullptr;
	}
}

QMeshEdge* spiralToolpath::buildNewEdgetoQMeshPatch(QMeshPatch* patch, QMeshNode* startNode, QMeshNode* endNode) {

	QMeshEdge* isoEdge = new QMeshEdge;
	isoEdge->SetStartPoint(startNode);
	isoEdge->SetEndPoint(endNode);

	isoEdge->SetMeshPatchPtr(patch);
	//isoEdge->SetIndexNo(patch->GetEdgeList().GetCount() + 1);
	isoEdge->SetIndexNo(patch->GetEdgeList().GetCount());

	(startNode->GetEdgeList()).AddTail(isoEdge);
	(endNode->GetEdgeList()).AddTail(isoEdge);
	patch->GetEdgeList().AddTail(isoEdge);
	//if (patch->curveType == 4) isoEdge->isheightlight = true;

	return isoEdge;
}

void spiralToolpath::resampleIsoCurve(QMeshPatch* curvedPatch_i, 
	QMeshPatch* curvedPatch_ii, QMeshPatch* spiralCurvePatch) {

	double curveLength_Ci = calCurveLength(curvedPatch_i);
	double curveLength_Cii = calCurveLength(curvedPatch_ii);

	int sectionNum = int(curveLength_Ci / toolpath_Distance) + 1;

	double deltaLen_Ci = curveLength_Ci / sectionNum;
	double deltaLen_Cii = curveLength_Cii / sectionNum;

	//std::cout << "**********************: " << std::endl;
	//std::cout << "curveLength_Ci: "<< curveLength_Ci << std::endl;
	//std::cout << "curveLength_Cii: " << curveLength_Cii << std::endl;
	//std::cout << "sectionNum: " << sectionNum << std::endl;
	//std::cout << "deltaLen_Ci: " << deltaLen_Ci << std::endl;
	//std::cout << "deltaLen_Cii: " << deltaLen_Cii << std::endl;

	//initial edge_processed flag and clear uniform iso-node on edge
	//initialEdge_Condition(curvedPatch_i);
	
	/*uniform IsoCurve Patch for C_i*/
	QMeshPatch* uniformIsoCurvePatch_i = new QMeshPatch;
	spiralPath->GetMeshList().AddTail(uniformIsoCurvePatch_i);
	uniformIsoCurvePatch_i->curveType = 2;	// 2 - uniform iso-curve
	//build uniform iso-node without linkage into "uniformIsoCurvePatch_i"
	generateUniformIsoNode(curvedPatch_i, uniformIsoCurvePatch_i, deltaLen_Ci, sectionNum);
	/*END*/

	/*uniform IsoCurve Patch for C_ii*/
	QMeshPatch* uniformIsoCurvePatch_ii = new QMeshPatch;
	spiralPath->GetMeshList().AddTail(uniformIsoCurvePatch_ii);
	uniformIsoCurvePatch_ii->curveType = 2;	// 2 - uniform iso-curve
	//build uniform iso-node without linkage into "uniformIsoCurvePatch_ii"
	generateUniformIsoNode(curvedPatch_ii, uniformIsoCurvePatch_ii, deltaLen_Cii, sectionNum);
	/*END*/

	/*test simple method for spiral Nodes*/

	if (uniformIsoCurvePatch_i->GetNodeNumber() != uniformIsoCurvePatch_ii->GetNodeNumber()) {

		std::cout << "****************************" << std::endl;
		std::cout << "the size of C_i and C_ii is different! please check" << std::endl;
		std::cout << "C_i  nodeNum = " << uniformIsoCurvePatch_i->GetNodeNumber() << std::endl;
		std::cout << "C_ii nodeNum = " << uniformIsoCurvePatch_ii->GetNodeNumber() << std::endl;
	}
	//construct a vector to easily find the needed node_pair
	std::vector<QMeshNode*> NodeSet_C_i(uniformIsoCurvePatch_i->GetNodeNumber());
	int temp = 0;
	for (GLKPOSITION Pos = uniformIsoCurvePatch_i->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)uniformIsoCurvePatch_i->GetNodeList().GetNext(Pos);
		NodeSet_C_i[temp] = thisNode;
		temp++;
	}

	std::vector<QMeshNode*> NodeSet_C_ii(uniformIsoCurvePatch_ii->GetNodeNumber());
	temp = 0;
	for (GLKPOSITION Pos = uniformIsoCurvePatch_ii->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)uniformIsoCurvePatch_ii->GetNodeList().GetNext(Pos);
		NodeSet_C_ii[temp] = thisNode;
		temp++;
	}

	for (int i = 0; i < NodeSet_C_i.size(); i++) {

		double Sorce_Node_Coord3D[3];
		NodeSet_C_i[i]->GetCoord3D(Sorce_Node_Coord3D[0], Sorce_Node_Coord3D[1], Sorce_Node_Coord3D[2]);
		double Target_Node_Coord3D[3];
		NodeSet_C_ii[i]->GetCoord3D(Target_Node_Coord3D[0],Target_Node_Coord3D[1], Target_Node_Coord3D[2]);

		double alpha = double(i) / NodeSet_C_i.size();
		double Spiral_Node_Coord3D[3];
		for (int j = 0; j < 3; j++) {
			//compute the position for this isonode --> spiral-like
			Spiral_Node_Coord3D[j] = (1.0 - alpha) * Sorce_Node_Coord3D[j] + alpha * Target_Node_Coord3D[j];
		}


		QMeshNode* spiralNode = new QMeshNode;

		spiralNode->isoValue = (1.0 - alpha) * NodeSet_C_i[i]->isoValue + alpha * NodeSet_C_ii[i]->isoValue;

		spiralNode->SetCoord3D(Spiral_Node_Coord3D[0], Spiral_Node_Coord3D[1], Spiral_Node_Coord3D[2]);// without projecting

		spiralNode->SetIndexNo(spiralCurvePatch->GetNodeList().GetCount()); //index should start from 0
		spiralCurvePatch->GetNodeList().AddTail(spiralNode);
	}

}

double spiralToolpath::calCurveLength(QMeshPatch* isoCurve) {

	double length = 0.0;

	for (GLKPOSITION posEdge = isoCurve->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)isoCurve->GetEdgeList().GetNext(posEdge);

		Edge->CalLength();
		length += Edge->GetLength();
	}

	if (length == 0.0) std::cout << "Length calculation of one curve ERROR!" << std::endl;

	return length;
}

void spiralToolpath::initialEdge_Condition(QMeshPatch* each_isoCurve) {

	for (GLKPOSITION posEdge = each_isoCurve->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)each_isoCurve->GetEdgeList().GetNext(posEdge);

		Edge->resample_processed = false;
		Edge->resample_installedIsoNode.clear();
	}
}

void spiralToolpath::generateUniformIsoNode
(QMeshPatch* each_isoCurve, QMeshPatch* uniform_isoCurve, double delta_Length, int secNum) {

	double patchIsoValue = -1.0;
	// collect the first Node into uniformIsoCurve patch (start Node of 1st edge)
	for (GLKPOSITION posEdge = each_isoCurve->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)each_isoCurve->GetEdgeList().GetNext(posEdge);

		patchIsoValue = Edge->GetStartPoint()->isoValue;
		//copy the message of startNode(cut_iso) for uniform isoCurve
		QMeshNode* uniform_isoNode = new QMeshNode;

		uniform_isoNode->isoValue = patchIsoValue;
		uniform_isoNode->relatedLayerFace = Edge->GetStartPoint()->relatedLayerFace;
		double pp[3];
		Edge->GetStartPoint()->GetCoord3D(pp[0], pp[1], pp[2]);
		uniform_isoNode->SetCoord3D(pp[0], pp[1], pp[2]);
		uniform_isoNode->isHeightlight = true;
		uniform_isoNode->SetIndexNo(uniform_isoCurve->GetNodeList().GetCount()); //index should start from 0

		//cal normal of iso point
		double n3[3];
		uniform_isoNode->relatedLayerFace->GetNormal(n3[0], n3[1], n3[2]);
		uniform_isoNode->SetNormal(n3[0], n3[1], n3[2]);

		//install this isoNode of layer to its Face(cross node iso-cut)
		uniform_isoNode->relatedLayerFace->installedIsoNode_layerFace.pop_back();// throw the copied Node link in before uniformed isoCurve
		uniform_isoNode->relatedLayerFace->installedIsoNode_layerFace.push_back(uniform_isoNode);
		uniform_isoNode->relatedLayerFace->isLocateIsoNode_layerFace = true;
		uniform_isoCurve->GetNodeList().AddTail(uniform_isoNode);
		
		//std::cout << "mesh->nodeNUM = " << uniform_isoCurve->GetNodeNumber() << std::endl;
		break;//only run once
	}

	// find the rest uniform isoNode
	double dis_container = 0.0;
	for (GLKPOSITION posEdge = each_isoCurve->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
		QMeshEdge* Edge = (QMeshEdge*)each_isoCurve->GetEdgeList().GetNext(posEdge);

		if (Edge->GetIndexNo() == each_isoCurve->GetEdgeNumber() - 1) break;

		Edge->CalLength();
		double dist = Edge->GetLength() + dis_container;
		
		if (dist > delta_Length) {

			double alpha = (delta_Length - dis_container) / Edge->GetLength();
			double p1[3], p2[3], pp[3];
			Edge->GetStartPoint()->GetCoord3D(p1[0], p1[1], p1[2]);
			Edge->GetEndPoint()->GetCoord3D(p2[0], p2[1], p2[2]);
			for (int j = 0; j < 3; j++) {
				//compute the position for this isonode
				pp[j] = (1.0 - alpha) * p1[j] + alpha * p2[j];
			}

			QMeshNode* uniform_isoNode = new QMeshNode;
			//isoNode->relatedLayerEdge = Edge;
			//isoNode->connectTPathProcessed = false;
			uniform_isoNode->isoValue = patchIsoValue;
			uniform_isoNode->relatedLayerFace = cutLineOnwhichFace(Edge);

			//uniform_isoNode->SetMeshPatchPtr(uniformIsoCurvePatch);
			uniform_isoNode->SetCoord3D(pp[0], pp[1], pp[2]);
			uniform_isoNode->isHeightlight = true;
			uniform_isoNode->SetIndexNo(uniform_isoCurve->GetNodeList().GetCount()); //index should start from 0

			//cal normal of iso point
			double n3[3];
			uniform_isoNode->relatedLayerFace->GetNormal(n3[0], n3[1], n3[2]);
			uniform_isoNode->SetNormal(n3[0], n3[1], n3[2]);

			//install this isoNode of layer to its Face(cross node iso-cut)
			uniform_isoNode->relatedLayerFace->installedIsoNode_layerFace.push_back(uniform_isoNode);
			uniform_isoNode->relatedLayerFace->isLocateIsoNode_layerFace = true;
			uniform_isoCurve->GetNodeList().AddTail(uniform_isoNode);

			dis_container = dist - delta_Length;
		}
		else {
			dis_container = dist;
		}
	}
}

QMeshPatch* spiralToolpath::generateCutLine() {// along X axis

	QMeshPatch* cutLine = new QMeshPatch;
	spiralPath->GetMeshList().AddTail(cutLine);
	cutLine->curveType = 4; // cutline
	//----build the node and install back to the "singlePath"
	for (GLKPOSITION Pos = surfaceMesh->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)surfaceMesh->GetEdgeList().GetNext(Pos);

		double p1[3], p2[3], pp[3];
		Edge->GetStartPoint()->GetCoord3D(p1[0], p1[1], p1[2]);
		Edge->GetEndPoint()->GetCoord3D(p2[0], p2[1], p2[2]);

		if (p1[0] < 0.0 && p2[0] < 0.0) continue;

		if (p1[1] * p2[1] < 0.0) {

			double alpha = (0.0 - p1[1]) / (p2[1] - p1[1]);

			for (int j = 0; j < 3; j++) {
				//compute the position for this isonode
				pp[j] = (1.0 - alpha) * p1[j] + alpha * p2[j];
			}

			QMeshNode* cutNode = new QMeshNode;
			cutNode->relatedLayerEdge = Edge;
			cutNode->connectTPathProcessed = false;
			cutNode->isoValue = (1.0 - alpha) * Edge->GetStartPoint()->boundaryValue
				+ alpha * Edge->GetEndPoint()->boundaryValue;

			cutNode->SetMeshPatchPtr(cutLine);
			cutNode->SetCoord3D(pp[0], pp[1], pp[2]);
			cutNode->SetIndexNo(cutLine->GetNodeList().GetCount()); //index should start from 0

			//cal normal of iso point
			double n1[3], n2[3], n3[3];

			QMeshFace* leftFace_edge = Edge->GetLeftFace();
			QMeshFace* rightFace_edge = Edge->GetRightFace();

			if (leftFace_edge == NULL && rightFace_edge == NULL) 
				std::cout << "ERROR: EDGE: no left and right face" << std::endl;

			if (leftFace_edge != NULL && rightFace_edge == NULL) {

				leftFace_edge->GetNormal(n1[0], n1[1], n1[2]);
				for (int i = 0; i < 3; i++) n3[i] = n1[i];
			}
			if (leftFace_edge == NULL && rightFace_edge != NULL) {

				rightFace_edge->GetNormal(n2[0], n2[1], n2[2]);
				for (int i = 0; i < 3; i++) n3[i] = n2[i];
			}
			if (leftFace_edge != NULL && rightFace_edge != NULL) {

				Edge->GetLeftFace()->GetNormal(n1[0], n1[1], n1[2]);
				Edge->GetRightFace()->GetNormal(n2[0], n2[1], n2[2]);
				for (int i = 0; i < 3; i++) n3[i] = (n1[i] + n2[i]) / 2;
			}

			cutNode->SetNormal(n3[0], n3[1], n3[2]);

			//install this cutNode of layer to its Edge
			Edge->installed_CutNode_layerEdge = cutNode;
			Edge->isLocate_CutNode_layerEdge = true;
			cutLine->GetNodeList().AddTail(cutNode);
		}
	}

	// get a First Node(cutline FirstNode) to start the toolPath
	QMeshNode* cutLineFirstNode = NULL;
	double minXX = 99999.99;
	for (GLKPOSITION Pos = cutLine->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)cutLine->GetNodeList().GetNext(Pos);

		if (thisNode->connectTPathProcessed == true) {

			std::cout << "this first node should not be processed! " << std::endl;
			continue;		
		}

		double xx, yy, zz;
		thisNode->GetCoord3D(xx, yy, zz);
		if (xx < minXX) {
			minXX = xx;
			cutLineFirstNode = thisNode;

		}
	}
	
	cutLineFirstNode->connectTPathProcessed = true;
	//cutLineFirstNode->isHeightlight = true;
	QMeshNode* sNode = cutLineFirstNode;
	QMeshNode* eNode = findNextCutNode(sNode, cutLine);
	/* Link all of cut-Node*/
	do {
		// Start linking one ring with same iso-Value
		if (eNode == nullptr) {
			std::cout << "NULL eNode" << std::endl;
			return cutLine;
		}// protection

		if (sNode == nullptr) {
			std::cout << "NULL sNode" << std::endl;
			return cutLine;
		}// protection

		buildNewEdgetoQMeshPatch(cutLine, sNode, eNode);

		sNode = eNode;
		eNode = findNextCutNode(sNode, cutLine);

	} while (!detectAll_CutNode_Processed(cutLine));
	buildNewEdgetoQMeshPatch(cutLine, sNode, eNode);

	return cutLine;
}

QMeshNode* spiralToolpath::findNextCutNode(QMeshNode* sNode, QMeshPatch* singlePath) {

	QMeshNode* eNode;
	bool nextNodeDetected = false;
	QMeshEdge* thisEdge = sNode->relatedLayerEdge;

	//detect left face
	for (int i = 0; i < 3; i++) {
		QMeshEdge* NeighborEdge = thisEdge->GetLeftFace()->GetEdgeRecordPtr(i + 1);
		if (NeighborEdge == thisEdge) continue;

		if (NeighborEdge->isLocate_CutNode_layerEdge) {

			QMeshNode* bNode = NeighborEdge->installed_CutNode_layerEdge;
			if (bNode->connectTPathProcessed == false) {
				nextNodeDetected = true;
				eNode = bNode;
				break;
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

		if (NeighborEdge->isLocate_CutNode_layerEdge) {

			QMeshNode* bNode = NeighborEdge->installed_CutNode_layerEdge;
			if (bNode->connectTPathProcessed == false) {
				nextNodeDetected = true;
				eNode = bNode;
				break;
			}
			
		}
		if (nextNodeDetected == true) break;
	}

	if (nextNodeDetected) {
		eNode->connectTPathProcessed = true;
		return eNode;
	}
	else {
		return nullptr; 
	}
}

bool spiralToolpath::detectAll_CutNode_Processed(QMeshPatch* singlePath) {
	//if all the node being processed return true, else return false.
	for (GLKPOSITION Pos = singlePath->GetNodeList().GetHeadPosition(); Pos;) {
		QMeshNode* thisNode = (QMeshNode*)singlePath->GetNodeList().GetNext(Pos);
		if (thisNode->connectTPathProcessed == false) return false;
	}
	return true;
}

QMeshNode* spiralToolpath::getCrossNode_cutIso(QMeshPatch* cutLine, double isoValue) {

	for (GLKPOSITION Pos = cutLine->GetEdgeList().GetHeadPosition(); Pos;) {
		QMeshEdge* Edge = (QMeshEdge*)cutLine->GetEdgeList().GetNext(Pos);

		double a = Edge->GetStartPoint()->isoValue;
		double b = Edge->GetEndPoint()->isoValue;

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
			//isoNode->relatedLayerEdge = Edge;
			//isoNode->connectTPathProcessed = false;
			isoNode->isoValue = isoValue;
			isoNode->relatedLayerFace = cutLineOnwhichFace(Edge);

			//isoNode->SetMeshPatchPtr(singlePath);
			isoNode->SetCoord3D(pp[0], pp[1], pp[2]);
			//isoNode->SetIndexNo(singlePath->GetNodeList().GetCount()); //index should start from 0

			//cal normal of iso point
			double n3[3];
			isoNode->relatedLayerFace->GetNormal(n3[0], n3[1], n3[2]);
			isoNode->SetNormal(n3[0], n3[1], n3[2]);

			//install this isoNode of layer to its Face(cross node iso-cut)
			isoNode->relatedLayerFace->installedIsoNode_layerFace.push_back(isoNode);
			isoNode->relatedLayerFace->isLocateIsoNode_layerFace = true;
			//cutLine->GetNodeList().AddTail(isoNode);
			return isoNode;
		}
	}
	std::cout << "ERROR: cannot find a isoNode on cutline, please check" << std::endl;
	return NULL;
}

QMeshFace* spiralToolpath::cutLineOnwhichFace(QMeshEdge* Edge) {

	QMeshFace* sNode_edge_leftFace = Edge->GetStartPoint()->relatedLayerEdge->GetLeftFace();
	QMeshFace* sNode_edge_rightFace = Edge->GetStartPoint()->relatedLayerEdge->GetRightFace();
	QMeshFace* eNode_edge_leftFace = Edge->GetEndPoint()->relatedLayerEdge->GetLeftFace();
	QMeshFace* eNode_edge_rightFace = Edge->GetEndPoint()->relatedLayerEdge->GetRightFace();

	if (sNode_edge_leftFace == eNode_edge_leftFace || sNode_edge_leftFace == eNode_edge_rightFace)
		return sNode_edge_leftFace;

	if (sNode_edge_rightFace == eNode_edge_leftFace || sNode_edge_rightFace == eNode_edge_rightFace)
		return sNode_edge_rightFace;

	std::cout << "ERROR: cannot find a FACE which owns a line, please check" << std::endl;
	return NULL;
}

// Line: O->T; Sphere: center,R; mu: Explicit argument
// reference: http://www.ambrsoft.com/TrigoCalc/Sphere/SpherLineIntersection_.htm
bool spiralToolpath::_lineIntersectSphere
(Eigen::Vector3d& O, Eigen::Vector3d& T, Eigen::Vector3d& Center, double R, double& mu1, double& mu2) {

	double a = (T - O).squaredNorm();
	double b = -2 * (T - O).dot(Center - O);
	double c = (Center - O).squaredNorm() - R * R;

	double bb4ac = b * b - 4 * a * c;

	if (fabs(a) == 0 || bb4ac <= 0) {
		mu1 = 0;
		mu2 = 0;
		return false;
	}

	mu1 = (-b + sqrt(bb4ac)) / (2 * a);
	mu2 = (-b - sqrt(bb4ac)) / (2 * a);

	// keep mu1 < mu2
	if (mu2 < mu1) {
		double temp_mu = mu1;
		mu1 = mu2;
		mu2 = temp_mu;
		//std::cout << "exchange mu1,mu2" << std::endl;
	}

	return true;
}

void spiralToolpath::_newPositionOnSphere(Eigen::Vector3d& before_Position, Eigen::Vector3d& after_Position) {

	Eigen::Vector3d O;	O << 0.0, 0.0, 0.0; // Vs
	Eigen::Vector3d T = before_Position; // Ve
	Eigen::Vector3d center; center << 0.0, 0.0, -7.5;
	double R = 19.5;
	double mu1, mu2;

	//test
	//std::cout << "O: " << O.transpose() << std::endl;
	//std::cout << "T: " << T.transpose() << std::endl;
	//std::cout << "------------------" << std::endl;

	bool intersection = _lineIntersectSphere(O, T, center, R, mu1, mu2);

	if (intersection == false) {
		std::cout << "ERROR: no intersection between line and sphere\n";
	}	// no intersection

	Eigen::Vector3d iPnt_1 = O + mu1 * (T - O); //p1
	Eigen::Vector3d iPnt_2 = O + mu2 * (T - O); //p2

	if(iPnt_1[2] * iPnt_2[2] >0.0) {
		std::cout << "ERROR: Z coordinate should be 1+1- \n";
	}

	if (iPnt_1[2] > 0.0) after_Position = iPnt_1;
	else after_Position = iPnt_2;
}

void spiralToolpath::_newPositionOnSurface(
	Eigen::Vector3d before_Position, Eigen::Vector3d& after_Position, Eigen::Vector3d& node_normal) {

	after_Position = { 1e10,1e10,1e10 };

	//find the nearest face
	Eigen::Vector3d faceCenter;
	double minDist = 1e10;
	QMeshFace* nearest_Face = NULL;
	for (GLKPOSITION Pos = surfaceMesh->GetFaceList().GetHeadPosition(); Pos != NULL;) {
		QMeshFace* face = (QMeshFace*)(surfaceMesh->GetFaceList().GetNext(Pos));

		face->CalCenterPos(faceCenter[0], faceCenter[1], faceCenter[2]);
		double dist = (faceCenter - before_Position).squaredNorm();
		if (dist > 9.0) continue;
		if (dist < minDist) {
			minDist = dist;
			nearest_Face = face;
		}
	}

	//get the normal of face neighbor
	if (nearest_Face == NULL) {
		std::cout << "There is no nearest face, please check! " << std::endl;
	}
	//nearest_Face->isHighlight = true;
	Eigen::Vector3d face_normal;
	nearest_Face->CalPlaneEquation();
	nearest_Face->GetNormal(face_normal[0], face_normal[1], face_normal[2]);

	////return before_Position;//DEBUG

	//project the node on to the surface
	Eigen::Vector3d dir = face_normal.normalized();
	node_normal = dir; // record the node normal
	Eigen::Vector3d orig = before_Position;

	for (GLKPOSITION Pos = surfaceMesh->GetFaceList().GetHeadPosition(); Pos;) {
		QMeshFace* each_face = (QMeshFace*)surfaceMesh->GetFaceList().GetNext(Pos);

		Eigen::Vector3d each_faceCenter;
		each_face->CalCenterPos(each_faceCenter[0], each_faceCenter[1], each_faceCenter[2]);
		double dist = (each_faceCenter - before_Position).squaredNorm();
		if (dist > 9.0) continue;

		double xx, yy, zz;
		each_face->GetNodeRecordPtr(0)->GetCoord3D(xx, yy, zz);
		Eigen::Vector3d v0 = { xx,yy,zz };

		each_face->GetNodeRecordPtr(1)->GetCoord3D(xx, yy, zz);
		Eigen::Vector3d v1 = { xx,yy,zz };

		each_face->GetNodeRecordPtr(2)->GetCoord3D(xx, yy, zz);
		Eigen::Vector3d v2 = { xx,yy,zz };

		double t;
		bool isIntersect = IntersectTriangle(orig, dir, v0, v1, v2, t);
		if (isIntersect) {
			after_Position = orig + dir * t;
			break;
		}

	}

	if (after_Position[0] == 1e10) {

		std::cout << "--\nThe input node: " << before_Position.transpose() << std::endl;
		std::cout << "The node dir: " << dir.transpose() << std::endl;
		std::cout << "The node orig: " << orig.transpose() << std::endl;
		std::cout << "The output node: " << after_Position.transpose() << std::endl;
		std::cout << "There is no intersection node, please check! " << std::endl;
	}
}

// Determine whether a ray intersect with a triangle
// Parameters
// orig: origin of the ray
// dir: direction of the ray
// v0, v1, v2: vertices of triangle
// t(out): weight of the intersection for the ray
// u(out), v(out): barycentric coordinate of intersection

bool spiralToolpath::IntersectTriangle(const Eigen::Vector3d& orig, const Eigen::Vector3d& dir,
	Eigen::Vector3d& v0, Eigen::Vector3d& v1, Eigen::Vector3d& v2, double& t) {
	// E1
	Eigen::Vector3d E1 = v1 - v0;

	// E2
	Eigen::Vector3d E2 = v2 - v0;

	// P
	Eigen::Vector3d P = dir.cross(E2);

	// determinant
	float det = E1.dot(P);

	// keep det > 0, modify T accordingly
	Eigen::Vector3d T;
	if (det > 0)
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det = -det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if (det < 0.000001f)
		return false;

	// Calculate u and make sure u <= 1
	double u, v;
	u = T.dot(P);
	if (u < 0.0f || u > det)
		return false;

	// Q
	Eigen::Vector3d Q = T.cross(E1);

	// Calculate v and make sure u + v <= 1
	v = dir.dot(Q);
	if (v < 0.0f || u + v > det)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	t = E2.dot(Q);

	float fInvDet = 1.0f / det;
	t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	//if (t < 0) return false;// comment it, as we use line-trangle intersection hear.

	return true;
}