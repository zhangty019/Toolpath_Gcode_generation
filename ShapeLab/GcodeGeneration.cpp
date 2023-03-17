#include "GcodeGeneration.h"
#include "../ThirdPartyDependence/PQPLib/PQP.h"
#include "GLKGeometry.h"

void GcodeGeneration::initial(
	PolygenMesh* Slices, PolygenMesh* Waypoints, PolygenMesh* CncPart) {

	m_Slices = Slices;
	m_Waypoints = Waypoints;
	m_CncPart = CncPart;
}

void GcodeGeneration::updateParameter(int FromIndex, int ToIndex, double lambdaValue, 
    double width, double tool_length) {

    m_FromIndex = FromIndex;
    m_ToIndex = ToIndex;
    m_lambdaValue = lambdaValue;
    m_width = width; // toolpath width
    h = tool_length; // tool length
}

void GcodeGeneration::calDHW() {

    this->_cal_Dist();
    this->_initialSmooth(10);
    this->_cal_Height();
}

void GcodeGeneration::_cal_Dist() {

    std::cout << "------------------------------------------- Waypoint Distance Calculation Running ..." << std::endl;
    long time = clock();

    std::cout << "--> largeJ_Length: " << m_jump_detection_threshold << std::endl;

#pragma omp parallel
    {
#pragma omp for  
        for (int omptime = 0; omptime < Core; omptime++) {

            for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
                QMeshPatch* layer = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

                if (layer->GetIndexNo() < m_FromIndex || layer->GetIndexNo() > m_ToIndex) continue;

                if (layer->GetIndexNo() % Core != omptime) continue;

                for (GLKPOSITION nodePos = layer->GetNodeList().GetHeadPosition(); nodePos;) {
                    QMeshNode* Node = (QMeshNode*)layer->GetNodeList().GetNext(nodePos);

                    double D = 0.0;
                    int lines = layer->GetNodeNumber();
                    if (Node->GetIndexNo() == (lines - 1)) { D = 0.0; }
                    else {

                        GLKPOSITION nextPos = layer->GetNodeList().Find(Node)->next;
                        QMeshNode* nextNode = (QMeshNode*)layer->GetNodeList().GetAt(nextPos);

                        D = (Node->m_printPos - nextNode->m_printPos).norm();

                        if (D > m_jump_detection_threshold) {
                            D = 0.0;								// inject the D to the Node/startPnt of Edge
                            Node->Jump_preSecEnd = true;			// end of prev section
                            nextNode->Jump_nextSecStart = true;		// start of next section
                        }
                    }
                    Node->m_DHW(0) = D;
                }
            }
        }
    }
    printf("TIMER -- Distance Calculation takes %ld ms.\n", clock() - time);
    std::cout << "------------------------------------------- Waypoint Distance Calculation Finish!\n" << std::endl;
}

void GcodeGeneration::_initialSmooth(int loopTime) {

    for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
        QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

        if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;

        int patch_NodeNum = WayPointPatch->GetNodeNumber();
        std::vector<bool> fix_Flag(patch_NodeNum);
        std::vector<Eigen::Vector3d> NodeNormal_temp(patch_NodeNum); // [Nx Ny Nz fix_flag]

        for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

            /*fixed at first / end / jump_start / jump_end points*/
            if (Node->GetIndexNo() == 0 || Node->GetIndexNo() == patch_NodeNum - 1
                || Node->Jump_preSecEnd || Node->Jump_nextSecStart) {
                fix_Flag[Node->GetIndexNo()] = true;
            }
            else { fix_Flag[Node->GetIndexNo()] = false; }

            NodeNormal_temp[Node->GetIndexNo()] = Node->m_printNor;

        }

        //smooth normal by (n-1) + X*(n) + (n+1)
        for (int loop = 0; loop < loopTime; loop++) {
            for (int i = 0; i < fix_Flag.size(); i++) {

                if (fix_Flag[i] == false) {
                    NodeNormal_temp[i] = (NodeNormal_temp[i - 1] + 0.5 * NodeNormal_temp[i] + NodeNormal_temp[i + 1]).normalized();
                }

            }
        }

        for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

            Node->m_printNor = NodeNormal_temp[Node->GetIndexNo()];
            Node->SetNormal(Node->m_printNor(0), Node->m_printNor(1), Node->m_printNor(2));
        }
    }
    std::cout << "------------------------------------------- Initial Smooth Finish!\n" << std::endl;
}

void GcodeGeneration::_cal_Height() {

    std::cout << "------------------------------------------- Waypoint Height Calculation Running ..." << std::endl;
    long time = clock();

    // get the patch polygenMesh_PrintPlatform
    QMeshPatch* patch_PrintPlatform = NULL;
    for (GLKPOSITION posMesh = m_CncPart->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
        QMeshPatch* thisPatch = (QMeshPatch*)m_CncPart->GetMeshList().GetNext(posMesh);

        if (thisPatch->patchName == "c_axis") {
            patch_PrintPlatform = thisPatch;
            break;
        }
    }

    if (patch_PrintPlatform == NULL) {
        std::cout << "patch_PrintPlatform is NULL, please check." << std::endl;
        return;
    }

#pragma omp parallel
    {
#pragma omp for  
        for (int omptime = 0; omptime < Core; omptime++) {

            // topLayer --> layer on the highest place [travel head to tail]
            for (GLKPOSITION Pos = m_Slices->GetMeshList().GetHeadPosition(); Pos;) {
                QMeshPatch* topLayer = (QMeshPatch*)m_Slices->GetMeshList().GetNext(Pos); // order: get data -> pnt move

                if (topLayer->GetIndexNo() < m_FromIndex || topLayer->GetIndexNo() > m_ToIndex) continue;

                if (topLayer->GetIndexNo() % Core != omptime) continue;

                std::vector<QMeshPatch*> bottomLayers;

                bottomLayers.push_back(patch_PrintPlatform);
                // construct a bottomLayers[i] to store the point of bottom layers for every toplayer
                for (GLKPOSITION beforePos = m_Slices->GetMeshList().Find(topLayer)->prev; beforePos;) {
                    QMeshPatch* beforePatch = (QMeshPatch*)m_Slices->GetMeshList().GetPrev(beforePos);

                    bottomLayers.push_back(beforePatch);
                    if (bottomLayers.size() > layerNum) break;
                }


                //--build PQP model
                std::vector<PQP_Model*> bLayerPQP;
                bLayerPQP.resize(bottomLayers.size());
                for (int i = 0; i < bottomLayers.size(); i++) {
                    if (bottomLayers[i]->GetNodeNumber() < 3) continue;
                    // build PQP model for bottom layers
                    PQP_Model* pqpModel = new PQP_Model();
                    pqpModel->BeginModel();  int index = 0;
                    PQP_REAL p1[3], p2[3], p3[3];

                    for (GLKPOSITION Pos = bottomLayers[i]->GetFaceList().GetHeadPosition(); Pos;) {
                        QMeshFace* Face = (QMeshFace*)bottomLayers[i]->GetFaceList().GetNext(Pos);

                        Face->GetNodeRecordPtr(0)->GetCoord3D(p1[0], p1[1], p1[2]);
                        Face->GetNodeRecordPtr(1)->GetCoord3D(p2[0], p2[1], p2[2]);
                        Face->GetNodeRecordPtr(2)->GetCoord3D(p3[0], p3[1], p3[2]);

                        pqpModel->AddTri(p1, p2, p3, index);
                        index++;

                    }
                    pqpModel->EndModel();
                    bLayerPQP[i] = pqpModel;
                }//--build PQP model END

                int layerIndex = topLayer->GetIndexNo();

                GLKPOSITION WayPointPatch_Pos = m_Waypoints->GetMeshList().FindIndex(layerIndex);
                QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetAt(WayPointPatch_Pos);

                for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
                    QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

                    double minHeight = 99999.99;
                    for (int i = 0; i < bottomLayers.size(); i++) {
                        if (bottomLayers[i]->GetNodeNumber() < 3) continue;

                        PQP_DistanceResult dres; dres.last_tri = bLayerPQP[i]->last_tri;
                        PQP_REAL p[3];
                        p[0] = Node->m_printPos(0); p[1] = Node->m_printPos(1); p[2] = Node->m_printPos(2);
                        PQP_Distance(&dres, bLayerPQP[i], p, 0.0, 0.0);

                        double Height = dres.Distance(); // Height of this layer
                        //int minTriIndex = dres.last_tri->id;	// closest triangle
                        if (minHeight > Height) minHeight = Height;
                    }
                    //cout << minHeight << endl;
                    Node->m_DHW(1) = minHeight;
                }

                //	free memory
                for (int i = 0; i < bottomLayers.size(); i++) { delete bLayerPQP[i]; }
            }
        }
    }

    std::printf("TIMER -- Height Calculation takes %ld ms.\n", clock() - time);
    std::cout << "------------------------------------------- Waypoint Height Calculation Finish!\n" << std::endl;
}

void GcodeGeneration::singularityOpt() {

    std::cout << "------------------------------------------- XYZBCE Calculation running ... " << std::endl;
    long time = clock();

#pragma omp parallel
    {
#pragma omp for  
        for (int omptime = 0; omptime < Core; omptime++) {

            for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
                QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

                if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;
                if (WayPointPatch->GetIndexNo() % Core != omptime) continue;

                std::vector<QMeshPatch*> layerJumpPatchSet = _getJumpSection_patchSet(WayPointPatch);

                Eigen::RowVector2d prevBC = { 0.0,0.0 };
                // give the message of BC for the first Node (only one)
                for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
                    QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

                    // solve 1
                    prevBC(0) = ROTATE_TO_DEGREE(-_safe_acos(Node->m_printNor(2)));
                    prevBC(1) = ROTATE_TO_DEGREE(atan2(Node->m_printNor(0), Node->m_printNor(1)));

                    // solve 2
                    double C2temp = prevBC(1) + 180.0;
                    if (C2temp > 180.0)	C2temp -= 360.0; // control the range of C2 into the (-180,180]

                    //// prevBC always BC1
                    //if (fabs(C2temp) < fabs(prevBC(1))) {
                    //    prevBC(0) = -prevBC(0);
                    //    prevBC(1) = C2temp;
                    //}

                    break;
                }
                //cout << "prevBC: " << prevBC << endl;

                for (int Index = 0; Index < layerJumpPatchSet.size(); Index++) {
                    //1.0 find the singularity waypoints
                    _markSingularNode(layerJumpPatchSet[Index]);
                    //1.1 filter single singular waypoint (XXOXX -> XXXXX)
                    _filterSingleSingularNode(layerJumpPatchSet[Index]);

                    Eigen::MatrixXd sectionTable, B1C1table, B2C2table;
                    //2.0 get the range of singularity Sections
                    _getSingularSec(layerJumpPatchSet[Index], sectionTable);
                    //2.1 project normal to the singular region boundary and check
                    _projectAnchorPoint(layerJumpPatchSet[Index]);

                    //3. calculate the 2 solves baced on kinematics of CNC
                    _getBCtable2(layerJumpPatchSet[Index], B1C1table, B2C2table);
                    //4. Main singularity optimization algorithm
                    _motionPlanning3(layerJumpPatchSet[Index], sectionTable, B1C1table, B2C2table, prevBC);
                    //5. reset steps: CNC XYZ calculation and E(=DHW) calculation
                    _getXYZ(layerJumpPatchSet[Index]);
                    _calDHW2E(layerJumpPatchSet[Index], true);
                }

                //aim to eliminate the -pi to pi sharp change
                _optimizationC(WayPointPatch);

                // from delta_E of each point to E in Gcode
                double E = 0.0;
                for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
                    QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
                    E = E + Node->m_XYZBCE(5);
                    Node->m_XYZBCE(5) = E;
                }
            }
        }
    }

    std::cout << "-------------------------------------------" << std::endl;
    std::printf("TIMER -- XYZBCE Calculation takes %ld ms.\n", clock() - time);
    std::cout << "------------------------------------------- XYZBCE Calculation Finish!\n " << std::endl;

    this->_verifyPosNor();
}

std::vector<QMeshPatch*> GcodeGeneration::_getJumpSection_patchSet(QMeshPatch* patch) {

    // Get the Jump section Num
    int JumpPatchNum = 1;
    for (GLKPOSITION Pos_Node = patch->GetNodeList().GetHeadPosition(); Pos_Node;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos_Node);

        if (Node->Jump_nextSecStart == true) JumpPatchNum++;
    }

    // molloc the space for each jumpPatch
    std::vector<QMeshPatch*> layerJumpPatchSet(JumpPatchNum);
    for (int i = 0; i < JumpPatchNum; i++) {
        layerJumpPatchSet[i] = new QMeshPatch();
        layerJumpPatchSet[i]->rootPatch_jumpPatch = patch;
    }

    // Add node into each JumpPatch
    int Jump_PatchIndex = 0;
    int JumpPatch_NodeIndex = 0;
    for (GLKPOSITION Pos_Node = patch->GetNodeList().GetHeadPosition(); Pos_Node;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos_Node);

        if (Node->Jump_nextSecStart == true) {
            Jump_PatchIndex++;
            JumpPatch_NodeIndex = 0;
        }

        layerJumpPatchSet[Jump_PatchIndex]->GetNodeList().AddTail(Node);
        Node->Jump_SecIndex = JumpPatch_NodeIndex;
        JumpPatch_NodeIndex++;
    }
    //std::cout << "-----------------------------------" << std::endl;
    //std::cout << "--> Split ToolPath into JumpSection" << std::endl;

    return layerJumpPatchSet;
}

double GcodeGeneration::_safe_acos(double value) {
    if (value <= -1.0) {
        return PI;
    }
    else if (value >= 1.0) {
        return 0;
    }
    else {
        return acos(value);
    }
}

void GcodeGeneration::_markSingularNode(QMeshPatch* patch) {

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        Eigen::Vector2d Cspece_Coord;
        // Cal C space Coordinate : Cspece = Nx/Nz, Ny/Nz;
        Cspece_Coord << Node->m_printNor(0) / Node->m_printNor(2),
            Node->m_printNor(1) / Node->m_printNor(2);

        double R = Cspece_Coord.norm();
        double radLambda = DEGREE_TO_ROTATE(m_lambdaValue);

        if (R < tan(radLambda)) Node->isSingularNode = true;
    }
}

void GcodeGeneration::_filterSingleSingularNode(QMeshPatch* patch) {

    //protect
    if (patch->GetNodeNumber() < 4) return;

    std::vector<QMeshNode*> nodeSet(patch->GetNodeNumber());
    std::vector<bool> kept_Singular_Flag(patch->GetNodeNumber());

    int tempIndex = 0;
    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        nodeSet[tempIndex] = Node;
        kept_Singular_Flag[tempIndex] = Node->isSingularNode;
        tempIndex++;
    }


    // remove OXX ... XOX ... XXO
    for (int i = 0; i < kept_Singular_Flag.size(); i++) {

        if (i == 0) {
            if (kept_Singular_Flag[i] == false && kept_Singular_Flag[i + 1] == true && kept_Singular_Flag[i + 2] == true) {
                nodeSet[i]->isSingularNode = true;
            }
        }
        else if (i == (kept_Singular_Flag.size() - 1)) {
            if (kept_Singular_Flag[i - 2] == true && kept_Singular_Flag[i - 1] == true && kept_Singular_Flag[i] == false) {
                nodeSet[i]->isSingularNode = true;
            }
        }
        else {
            if (kept_Singular_Flag[i - 1] == true && kept_Singular_Flag[i] == false && kept_Singular_Flag[i + 1] == true) {
                nodeSet[i]->isSingularNode = true;
            }
        }
    }

    // remove XOOX
    if (patch->GetNodeNumber() < 5) return;
    for (int i = 0; i < kept_Singular_Flag.size(); i++) {
        kept_Singular_Flag[i] = nodeSet[i]->isSingularNode;
    }
    for (int i = 0; i < kept_Singular_Flag.size() - 3; i++) {

        if (kept_Singular_Flag[i] == true
            && kept_Singular_Flag[i + 1] == false
            && kept_Singular_Flag[i + 2] == false
            && kept_Singular_Flag[i + 3] == true) {
            nodeSet[i + 1]->isSingularNode = true;
            nodeSet[i + 2]->isSingularNode = true;
        }
    }
    // remove XOOOX
    if (patch->GetNodeNumber() < 6) return;
    for (int i = 0; i < kept_Singular_Flag.size(); i++) {
        kept_Singular_Flag[i] = nodeSet[i]->isSingularNode;
    }
    for (int i = 0; i < kept_Singular_Flag.size() - 4; i++) {

        if (kept_Singular_Flag[i] == true
            && kept_Singular_Flag[i + 1] == false
            && kept_Singular_Flag[i + 2] == false
            && kept_Singular_Flag[i + 3] == false
            && kept_Singular_Flag[i + 4] == true) {
            nodeSet[i + 1]->isSingularNode = true;
            nodeSet[i + 2]->isSingularNode = true;
            nodeSet[i + 3]->isSingularNode = true;
        }
    }
}

void GcodeGeneration::_getSingularSec(QMeshPatch* patch, Eigen::MatrixXd& sectionTable) {

    int lines = patch->GetNodeNumber();
    std::vector<int> srtPntIndTable, endPntIndTable;

    for (int i = 0; i < lines - 1; i++) {

        GLKPOSITION Node_Pos = patch->GetNodeList().FindIndex(i);
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetAt(Node_Pos);

        GLKPOSITION nextNode_Pos = patch->GetNodeList().FindIndex(i)->next;
        QMeshNode* nextNode = (QMeshNode*)patch->GetNodeList().GetAt(nextNode_Pos);


        if ((Node->isSingularNode == false && nextNode->isSingularNode == true)
            || (Node->isSingularNode == true && Node->Jump_SecIndex == 0)) {
            srtPntIndTable.push_back(Node->Jump_SecIndex);
            Node->isSingularSecStartNode = true;
        }
        if ((Node->isSingularNode == true && nextNode->isSingularNode == false)
            || (nextNode->isSingularNode == true && nextNode->Jump_SecIndex == lines - 1)) {
            endPntIndTable.push_back(nextNode->Jump_SecIndex);
            nextNode->isSingularSecEndNode = true;
        }
    }

    if (srtPntIndTable.size() == endPntIndTable.size()) sectionTable.resize(srtPntIndTable.size(), 2);
    else std::cout << "ERROR : srtPntIndTable.size() != endPntIndTable.size()" << std::endl;

    for (int i = 0; i < srtPntIndTable.size(); i++) {
        sectionTable(i, 0) = srtPntIndTable[i];
        sectionTable(i, 1) = endPntIndTable[i];
    }
    //std::cout << "sectionTable:\n"<<sectionTable << std::endl;
}

void GcodeGeneration::_projectAnchorPoint(QMeshPatch* patch) {

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        if (Node->isSingularSecStartNode == true && Node->isSingularSecEndNode == true) {
            std::cout << "Error: the normal of anchor point cannot move to the boundary of singular region" << std::endl;
            std::cout << "Error: as one normal cannot move to double directions" << std::endl;
        }

        if (Node->GetIndexNo() == 0 || Node->GetIndexNo() == (patch->GetNodeNumber() - 1)) continue;


        if (Node->isSingularSecStartNode == true || Node->isSingularSecEndNode == true) {

            Eigen::Vector3d m_printNor_before = Node->m_printNor;

            double anchor_Nz = cos(DEGREE_TO_ROTATE(m_lambdaValue));
            double temp_k = Node->m_printNor(1) / Node->m_printNor(0);
            double anchor_Nx = sqrt((1 - anchor_Nz * anchor_Nz) / (1 + temp_k * temp_k));
            if (Node->m_printNor(0) < 0.0) anchor_Nx = -anchor_Nx;
            double anchor_Ny = anchor_Nx * temp_k;

            Node->SetNormal(anchor_Nx, anchor_Ny, anchor_Nz);
            Node->SetNormal_last(anchor_Nx, anchor_Ny, anchor_Nz);
            Node->m_printNor << anchor_Nx, anchor_Ny, anchor_Nz;

            //cal the angle of before and after of anchor normal
            if (false) {
                double change = ROTATE_TO_DEGREE(
                    _safe_acos(
                        m_printNor_before.dot(Node->m_printNor)
                        / m_printNor_before.norm() / Node->m_printNor.norm()));
                std::cout << " the angle of before and after of anchor normal is " << change << std::endl;
            }
        }
    }
}

void GcodeGeneration::_getBCtable2(QMeshPatch* patch, Eigen::MatrixXd& B1C1table, Eigen::MatrixXd& B2C2table) {

    int lines = patch->GetNodeNumber();

    B1C1table = Eigen::MatrixXd::Zero(lines, 2);	B2C2table = Eigen::MatrixXd::Zero(lines, 2);

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        int i = Node->Jump_SecIndex;

        // solve 1
        // B1 deg // -acos(Nz)
        B1C1table(i, 0) = ROTATE_TO_DEGREE(-_safe_acos(Node->m_printNor(2)));
        // C1 deg //atan2(Nx, Ny)
        B1C1table(i, 1) = ROTATE_TO_DEGREE(atan2(Node->m_printNor(0), Node->m_printNor(1)));

        // solve 2
        // B2 deg // acos(Nz)
        B2C2table(i, 0) = -B1C1table(i, 0);
        // C2 deg //atan2(Ny, Nx) +/- 180
        double C2temp = B1C1table(i, 1) + 180.0;
        if (C2temp > 180.0)C2temp -= 360.0; // control the range of C2 into the (-180,180]
        B2C2table(i, 1) = C2temp;

        // only use solve 2
        // modified by tianyu 10/04/2022
        //B1C1table(i, 0) = B2C2table(i, 0);
        //B1C1table(i, 1) = B2C2table(i, 1);
    }
}

void GcodeGeneration::_motionPlanning3(
    QMeshPatch* patch, const Eigen::MatrixXd& sectionTable, const Eigen::MatrixXd& B1C1table,
    const Eigen::MatrixXd& B2C2table, Eigen::RowVector2d& prevBC) {

    int lines = patch->GetNodeNumber();
    Eigen::MatrixXd BC_Matrix(lines, 2); BC_Matrix = Eigen::MatrixXd::Zero(lines, 2);
    std::vector<int> solveFlag(lines);// 1 -> solve 1 // 2 -> solve 2
    // std::vector<int> insertNum(lines);// insertNum for large BC change at the beginning point
    int sectionNumber = 0;
    int sectionAmount = sectionTable.rows();

    int i = 0;

    while (i < lines) {
        //all points of current path are OUT of the sigularity region
        if (sectionAmount == 0) {

            if (_chooseB1C1(B1C1table.row(i), B2C2table.row(i), prevBC)) {
                prevBC << B1C1table.row(i);
                solveFlag[i] = 1;
            }
            else {
                prevBC << B2C2table.row(i);
                solveFlag[i] = 2;
            }

            BC_Matrix.row(i) = prevBC;
            i = i + 1;
        }
        else {
            Eigen::RowVector2d tempBC;
            //all points of current path are IN the sigularity region
            if (i == sectionTable(sectionNumber, 0) && i == 0 && sectionTable(sectionNumber, 1) == (lines - 1)) {
                for (int secLineIndex = i; secLineIndex < lines; secLineIndex++) {

                    tempBC = { 0.0, prevBC(1) };
                    prevBC = tempBC;
                    solveFlag[secLineIndex] = 1;
                    BC_Matrix.row(secLineIndex) = prevBC;
                }
                i = lines;
            }
            // start from the singularity region (end in singularity region or not)
            else if (i == sectionTable(sectionNumber, 0) && i == 0 && sectionTable(sectionNumber, 1) != (lines - 1)) {

                int secEndIndex = sectionTable(sectionNumber, 1);

                for (int secLineIndex = i; secLineIndex < secEndIndex; secLineIndex++) {

                    if (_chooseB1C1(B1C1table.row(secEndIndex), B2C2table.row(secEndIndex), prevBC)
                        || (secLineIndex == 0) // this can make sure start from solution 1
                        ) {
                        tempBC << B1C1table.row(secEndIndex);
                        solveFlag[secLineIndex] = 1;
                    }
                    else {
                        tempBC << B2C2table.row(secEndIndex);
                        solveFlag[secLineIndex] = 2;
                    }

                    prevBC = tempBC;
                    BC_Matrix.row(secLineIndex) = prevBC;

                }

                i = secEndIndex;
                if (sectionNumber != (sectionAmount - 1))	sectionNumber++;
            }
            // end in the singularity region / finish path
            else if (i == sectionTable(sectionNumber, 0) && i != 0 && sectionTable(sectionNumber, 1) == (lines - 1)) {

                int secStartIndex = sectionTable(sectionNumber, 0);

                for (int secLineIndex = i; secLineIndex < lines; secLineIndex++) {

                    if (_chooseB1C1(B1C1table.row(secStartIndex), B2C2table.row(secStartIndex), prevBC)) {
                        tempBC << B1C1table.row(secStartIndex);
                        solveFlag[secLineIndex] = 1;
                    }
                    else {
                        tempBC << B2C2table.row(secStartIndex);
                        solveFlag[secLineIndex] = 2;
                    }

                    prevBC = tempBC;
                    BC_Matrix.row(secLineIndex) = prevBC;
                }

                i = lines;
            }
            // path passes through the sigularity region
            else if (i == sectionTable(sectionNumber, 0) && i != 0 && sectionTable(sectionNumber, 1) != (lines - 1)) {

                // give the message to anchor point (start point)
                int secStartIndex = sectionTable(sectionNumber, 0);
                if (_chooseB1C1(B1C1table.row(secStartIndex), B2C2table.row(secStartIndex), prevBC)) {
                    prevBC << B1C1table.row(secStartIndex);
                    solveFlag[secStartIndex] = 1;
                }
                else {
                    prevBC << B2C2table.row(secStartIndex);
                    solveFlag[secStartIndex] = 2;
                }

                // record the deg_BC of secStart point
                Eigen::RowVector2d startPntBC = prevBC;

                // decide the solve of End point
                int secEndIndex = sectionTable(sectionNumber, 1);
                int pointNum = secEndIndex - secStartIndex;

                double rad_end_B1 = DEGREE_TO_ROTATE(B1C1table(secEndIndex, 0));	double rad_end_C1 = DEGREE_TO_ROTATE(B1C1table(secEndIndex, 1));
                double rad_end_B2 = DEGREE_TO_ROTATE(B2C2table(secEndIndex, 0));	double rad_end_C2 = DEGREE_TO_ROTATE(B2C2table(secEndIndex, 1));
                double rad_start_B = DEGREE_TO_ROTATE(startPntBC(0));				double rad_start_C = DEGREE_TO_ROTATE(startPntBC(1));

                Eigen::Vector2d v_start_C = { cos(rad_start_C),sin(rad_start_C) };
                Eigen::Vector2d v_end_C1 = { cos(rad_end_C1),sin(rad_end_C1) };
                Eigen::Vector2d v_end_C2 = { cos(rad_end_C2),sin(rad_end_C2) };
                //compute the actural angle of 2 vectors
                double rad_end_C1_start_C = _safe_acos(v_end_C1.dot(v_start_C));		double rad_end_B1_start_B = rad_end_B1 - rad_start_B;
                double rad_end_C2_start_C = _safe_acos(v_end_C2.dot(v_start_C));		double rad_end_B2_start_B = rad_end_B2 - rad_start_B;
                //get rad_C/B_start_end
                double rad_C_start_end = 0.0;
                double rad_B_start_end = 0.0;
                Eigen::Vector2d v_end_C = { 0.0,0.0 };

                int solveFlag_passThrough = 0; // 1 -> solve 1 // 2 -> solve 2

                if ((rad_end_C1_start_C) <= (rad_end_C2_start_C)) {
                    rad_C_start_end = rad_end_C1_start_C;
                    rad_B_start_end = rad_end_B1_start_B;
                    v_end_C = v_end_C1;
                    solveFlag_passThrough = 1;
                }
                else {
                    rad_C_start_end = rad_end_C2_start_C;
                    rad_B_start_end = rad_end_B2_start_B;
                    v_end_C = v_end_C2;
                    solveFlag_passThrough = 2;
                }

                //decide the rotation direction of C axis
                double sign = _toLeft({ 0.0,0.0 }, v_start_C, v_end_C);

                //get tha delta Angel of deg_B/C
                double C_delta_Angle = ROTATE_TO_DEGREE(rad_C_start_end) / pointNum;
                double B_delta_Angle = ROTATE_TO_DEGREE(rad_B_start_end) / pointNum;

                unsigned int times = 0;
                for (int secLineIndex = secStartIndex; secLineIndex < secEndIndex; secLineIndex++) {

                    tempBC(0) = startPntBC(0) + times * B_delta_Angle;
                    tempBC(1) = startPntBC(1) + sign * times * C_delta_Angle;

                    prevBC = tempBC;

                    if (prevBC(1) > 180.0) prevBC(1) -= 360.0;
                    if (prevBC(1) < -180.0) prevBC(1) += 360.0;

                    solveFlag[secLineIndex] = solveFlag_passThrough;
                    BC_Matrix.row(secLineIndex) = prevBC;

                    times++;
                }

                i = secEndIndex;

                if (sectionNumber != (sectionAmount - 1))	sectionNumber = sectionNumber + 1;

            }
            // other points out of the singularity region
            else {

                if (_chooseB1C1(B1C1table.row(i), B2C2table.row(i), prevBC)) {

                    prevBC << B1C1table.row(i);
                    solveFlag[i] = 1;
                }
                else {
                    prevBC << B2C2table.row(i);
                    solveFlag[i] = 2;
                }

                BC_Matrix.row(i) = prevBC;
                i = i + 1;
            }
        }
    }

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        int nodeIndex = Node->Jump_SecIndex;
        Node->m_XYZBCE(3) = BC_Matrix(nodeIndex, 0); //deg
        Node->m_XYZBCE(4) = BC_Matrix(nodeIndex, 1); //deg

        Node->solveSeclct = solveFlag[nodeIndex];
        //cout << Node->solveSeclct << endl;
    }
}

bool GcodeGeneration::_chooseB1C1(
    const Eigen::RowVector2d& B1C1, const Eigen::RowVector2d& B2C2, Eigen::RowVector2d& prevBC) {

    double rad_B1 = DEGREE_TO_ROTATE(B1C1(0));	double rad_C1 = DEGREE_TO_ROTATE(B1C1(1));
    double rad_B2 = DEGREE_TO_ROTATE(B2C2(0));	double rad_C2 = DEGREE_TO_ROTATE(B2C2(1));
    double rad_Bp = DEGREE_TO_ROTATE(prevBC(0)); double rad_Cp = DEGREE_TO_ROTATE(prevBC(1));

    Eigen::Vector2d v_Cp = { cos(rad_Cp),sin(rad_Cp) };
    Eigen::Vector2d v_C1 = { cos(rad_C1),sin(rad_C1) };
    Eigen::Vector2d v_C2 = { cos(rad_C2),sin(rad_C2) };
    //compute the actural angle

    double rad_v_C1_v_Cp = _safe_acos(v_C1.dot(v_Cp));		double rad_B1_rad_Bp = fabs(rad_B1 - rad_Bp);
    double rad_v_C2_v_Cp = _safe_acos(v_C2.dot(v_Cp));		double rad_B2_rad_Bp = fabs(rad_B2 - rad_Bp);

    bool isB1C1 = true;

    if ((rad_v_C1_v_Cp + rad_B1_rad_Bp) > (rad_v_C2_v_Cp + rad_B2_rad_Bp)) {
        isB1C1 = false;

        //std::cout << "----------------------------\n use 2 solve" << std::endl;
        //std::cout << "B1C1 = " << B1C1 << std::endl;
        //std::cout << "B2C2 = " << B2C2 << std::endl;
        //std::cout << "prevBC = " << prevBC << std::endl;
        //std::cout << "rad_v_C1_v_Cp = " << rad_v_C1_v_Cp << std::endl;
        //std::cout << "rad_B1_rad_Bp = " << rad_B1_rad_Bp << std::endl;
        //std::cout << "rad_v_C2_v_Cp = " << rad_v_C2_v_Cp << std::endl;
        //std::cout << "rad_B2_rad_Bp = " << rad_B2_rad_Bp << std::endl;
    }
    return isB1C1;
}

double GcodeGeneration::_toLeft(
    const Eigen::RowVector2d& origin_p, const Eigen::RowVector2d& startPnt_q, const Eigen::RowVector2d& endPnt_s) {

    double Area2 = origin_p(0) * startPnt_q(1) - origin_p(1) * startPnt_q(0)
        + startPnt_q(0) * endPnt_s(1) - startPnt_q(1) * endPnt_s(0)
        + endPnt_s(0) * origin_p(1) - endPnt_s(1) * origin_p(0);

    double isLeft = -1.0;
    if (Area2 > 0.0) isLeft = 1.0;

    return isLeft;
}

void GcodeGeneration::_getXYZ(QMeshPatch* patch) {

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        double px = Node->m_printPos(0);
        double py = Node->m_printPos(1);
        double pz = Node->m_printPos(2);

        double rad_B = DEGREE_TO_ROTATE(Node->m_XYZBCE(3));// rad
        double rad_C = DEGREE_TO_ROTATE(Node->m_XYZBCE(4));// rad

        if (is_planar_printing) {
            rad_B = 0.0; rad_C = 0.0;
            Node->m_XYZBCE(3) = 0.0; Node->m_XYZBCE(4) = 0.0;
        }

        //cal XYZ
        Node->m_XYZBCE(0) = px * cos(rad_C) - py * sin(rad_C);
        Node->m_XYZBCE(1) = px * sin(rad_C) + py * cos(rad_C) - h * sin(rad_B);
        Node->m_XYZBCE(2) = pz - h * (1 - cos(rad_B));

        //if (Node->m_XYZBCE(2) < 0.0) { Node->negativeZ = true; }
    }
}

void GcodeGeneration::_calDHW2E(QMeshPatch* patch, bool hysteresis_switch) {

    // E = E + ratio * height * length * width;
    // Dicided by CNC W.R.T (E:Volume:E = 0.45)

    double D, H, W;

    // optimize the Hysteresis of extruder
    std::vector<double> Hysteresis_Ks = { 2.0,2.0,1.7,1.5,1.25 };
    std::vector<double> Hysteresis_Ke = { 0.0,0.0,0.0,0.0,0.0,0.0 };
    int lines = patch->GetNodeNumber();
    std::vector<double> deltaE(lines);

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        D = Node->m_DHW(0);

        H = Node->m_DHW(1);

        Node->m_DHW(2) = m_width;
        W = Node->m_DHW(2);

        deltaE[Node->Jump_SecIndex] = ratio * H * D * W;
    }

    if (lines >= (Hysteresis_Ks.size() + Hysteresis_Ke.size()) && hysteresis_switch) {

        for (int i = 0; i < lines; i++) {

            if (i >= 0 && i < Hysteresis_Ks.size()) {
                deltaE[i] = deltaE[i] * Hysteresis_Ks[i];
            }

            if (i >= (lines - Hysteresis_Ke.size()) && i < lines) {
                deltaE[i] = deltaE[i] * Hysteresis_Ke[i - lines + Hysteresis_Ke.size()];
            }
        }
    }

    for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

        Node->m_XYZBCE(5) = deltaE[Node->Jump_SecIndex];
    }
}

void GcodeGeneration::_optimizationC(QMeshPatch* patch) {

    for (int loop = 0; loop < 50; loop++) {

        double threshhold = 180.0;

        for (GLKPOSITION Pos = patch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)patch->GetNodeList().GetNext(Pos);

            double C = Node->m_XYZBCE(4); // deg

            if (Node->GetIndexNo() == 0) continue;
            GLKPOSITION prevPos = patch->GetNodeList().Find(Node)->prev;
            QMeshNode* prevNode = (QMeshNode*)patch->GetNodeList().GetAt(prevPos);
            double preC = prevNode->m_XYZBCE(4);

            if (C - preC < -threshhold) {
                C = C + 360;
            }
            else if (C - preC > threshhold) {
                C = C - 360;
            }
            else {}

            Node->m_XYZBCE(4) = C;
        }
    }
}

void GcodeGeneration::_verifyPosNor() {

    std::cout << "------------------------------------------- PosNor verification running ... " << std::endl;
    for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
        QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

        if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;

        for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);

            double X = Node->m_XYZBCE(0);	double Y = Node->m_XYZBCE(1);	double Z = Node->m_XYZBCE(2);
            double rad_B = DEGREE_TO_ROTATE(Node->m_XYZBCE(3));
            double rad_C = DEGREE_TO_ROTATE(Node->m_XYZBCE(4));

            double finalNx = -sin(rad_B) * sin(rad_C);
            double finalNy = -sin(rad_B) * cos(rad_C);
            double finalNz = cos(rad_B);
            double finalPx = X * cos(rad_C) + sin(rad_C) * (h * sin(rad_B) + Y);
            double finalPy = -X * sin(rad_C) + cos(rad_C) * (h * sin(rad_B) + Y);
            double finalPz = Z + h - h * cos(rad_B);

            bool equalPx = (fabs(finalPx - Node->m_printPos(0)) < 0.001) ? true : false;
            bool equalPy = (fabs(finalPy - Node->m_printPos(1)) < 0.001) ? true : false;
            bool equalPz = (fabs(finalPz - Node->m_printPos(2)) < 0.001) ? true : false;

            if ((equalPx == false) || (equalPy == false) || (equalPz == false)) {
                std::cout << "++++++++++++++++++++++++++++++++" << std::endl;
                std::cout << "Layer " << WayPointPatch->GetIndexNo() << " Point Index " << Node->GetIndexNo() << std::endl;
                std::cout << "final Position" << std::endl;
                std::cout << finalPx << "\n" << finalPy << "\n" << finalPz << std::endl;
                std::cout << "print Position\n" << Node->m_printPos << std::endl;
            }

            Eigen::Vector3d finalNormal = { finalNx, finalNy, finalNz };
            double angle = _getAngle3D(finalNormal, Node->m_printNor, true);
            //if (angle >= 0.0001) {
            if (angle > (m_lambdaValue * 2 + 1.0)) {
                //if (Node->isSingularNode) cout << "this is a singular node";
                std::cout << "--------------------------------" << std::endl;
                std::cout << "Layer " << WayPointPatch->GetIndexNo() << " Point Index " << Node->GetIndexNo() << std::endl;
                std::cout << "The angle is " << angle << std::endl;
                std::cout << "final Normal\n" << finalNormal.transpose() << std::endl;
                std::cout << "print Normal\n" << Node->m_printNor.transpose() << std::endl;
            }

            //update the normal after singular optimization
            Node->SetNormal(finalNormal(0), finalNormal(1), finalNormal(2));// for show the final normal on the GUI
            Node->SetNormal_last(finalNormal(0), finalNormal(1), finalNormal(2));
            Node->m_printNor = finalNormal;
        }

    }

    std::cout << "------------------------------------------- PosNor verification Finish!\n" << std::endl;

}

double GcodeGeneration::_getAngle3D(const Eigen::Vector3d& v1, const Eigen::Vector3d& v2, const bool in_degree) {
    //compute the actural angle
    double rad = v1.normalized().dot(v2.normalized());//dot product
    if (rad < -1.0)
        rad = -1.0;
    else if (rad > 1.0)
        rad = 1.0;
    return (in_degree ? _safe_acos(rad) * 180.0 / PI : _safe_acos(rad));
}

//Function for feedrate optimization
void GcodeGeneration::feedrateOpt() {

    double base_Feedrate = 600.0;
    double max_Feedrate = 2500.0;
    double min_Feedrate = 500.0;

    for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
        QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

        if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;

        double last_feedRate = 0.0;
        for (GLKPOSITION nodePos = WayPointPatch->GetNodeList().GetHeadPosition(); nodePos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(nodePos);

            double feedRate = 0.0;
            int lines = WayPointPatch->GetNodeNumber();
            if (Node->GetIndexNo() == (lines - 1)) { feedRate = last_feedRate; }
            else {

                GLKPOSITION nextPos = WayPointPatch->GetNodeList().Find(Node)->next;
                QMeshNode* nextNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(nextPos);

                double l = (Node->m_printPos - nextNode->m_printPos).norm(); //(l: Euclidean distance)    
                double d = (Node->m_XYZBCE - nextNode->m_XYZBCE).norm(); //(d: Joint space distance)

                if (Node->Jump_preSecEnd) {
                    feedRate = last_feedRate;
                }
                else {
                    //feedRate = base_Feedrate * d / l;
                    feedRate = base_Feedrate * sqrt(d);
                }

                if (Node->Jump_nextSecStart && Node->Jump_preSecEnd)
                {
                    std::cout << "Error: the node cannot be start and end at the same time!" << std::endl;
                    return;
                }

                if (feedRate >= max_Feedrate) {
                    feedRate = max_Feedrate;
                    //std::cout << "more than max: node ind: "<< Node->GetIndexNo() << " l: " << l << " d: " << d << std::endl;
                }
                if (feedRate <= min_Feedrate) {
                    feedRate = min_Feedrate;
                    //std::cout << "less than min: node ind: " << Node->GetIndexNo() << " l: " << l << " d: " << d << std::endl;
                }

            }

            //if (is_planar_printing) feedRate = 500;
            Node->m_F = feedRate;
            last_feedRate = feedRate;
        }
    }
}

void GcodeGeneration::writeGcode(std::string GcodeDir) {
    std::cout << "------------------------------------------- " << GcodeDir << " Gcode Writing ..." << std::endl;

    // First varify the tip height is larger than 0.0; IMPORTANT
    for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
        QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

        if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;

        Eigen::Vector4d tipPos_4d_initial = Eigen::Vector4d::Zero();    tipPos_4d_initial << 0.0, 0.0, 0.0, 1.0;
        Eigen::Vector4d tipPos_4d = Eigen::Vector4d::Zero();

        for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
            double X = Node->m_XYZBCE(0); double Y = Node->m_XYZBCE(1); double Z = Node->m_XYZBCE(2);
            double B = Node->m_XYZBCE(3); double C = Node->m_XYZBCE(4); double E = Node->m_XYZBCE(5);

            double rad_B = DEGREE_TO_ROTATE(B);
            double rad_C = DEGREE_TO_ROTATE(C);

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

            tipPos_4d = Offset * Rot_B * Offset_back * tipPos_4d_initial;

            if (tipPos_4d[2] < 0.0) {

                std::cout << "error: Tip will hit the bottom, DANGER!!!" << std::endl;
                return;
            }
        }
    }

    // Define the basic parameter
    double Z_home = 250;						// The hight of Home point; / mm
    double Z_high = 3;// h;  					// The hight of G1 point(for safety); / mm
    if (is_planar_printing) Z_high = 2.0;
    double Z_compensateUpDistance = 2;			// The hight of waiting distance of extruder; / mm

    int F_G1_move = 3000;                       // Speed of move
    int F_G1_1stlayer = 650;				    // Speed of G1(special 1st layer)
    int F_G1_original = 750;					// Speed of G1 original material (normal 2ed~layers)
    int F_G1_support = F_G1_original;			// Speed of G1 support material (normal 2ed~layers)
    int F_PumpBack = 6000;						// Speed of F_PumpBack
    int F_PumpCompensate = 900;				    // Speed of PumpCompensate

    double E_PumpBack = -6.0; 					// The extruder pump back Xmm
    double E_PumpCompensate = 6.0;				// The extruder pump compensate Xmm
    double E_PumpCompensateL1 = 4;				// The extruder pump compensate for 1st layer Xmm
    double E_PumpCompensateNewE = 4;			// The extruder pump compensate for new type layer Xmm

    char targetFilename[1024];
    std::sprintf(targetFilename, "%s%s", "../DataSet/G_CODE/", GcodeDir.c_str());
    FILE* fp = fopen(targetFilename, "w");
    if (!fp) {
        perror("Couldn't open the directory");
        return;
    }

    // Record the max Z for security consideration (the first printed layer)
    GLKPOSITION layer1st_Pos = m_Waypoints->GetMeshList().FindIndex(m_FromIndex);
    QMeshPatch* layer1st_WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetAt(layer1st_Pos);

    double Z_max = -99999.9;
    for (GLKPOSITION Pos = layer1st_WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
        QMeshNode* Node = (QMeshNode*)layer1st_WayPointPatch->GetNodeList().GetNext(Pos);

        if (Node->m_XYZBCE(2) > Z_max) { Z_max = Node->m_XYZBCE(2); }
    }

    // Give the start message of G_code
    std::fprintf(fp, "G21\n");
    std::fprintf(fp, "G40\n");
    std::fprintf(fp, "G49\n");
    std::fprintf(fp, "G80\n");
    std::fprintf(fp, "G90\n");
    std::fprintf(fp, "M5\n");
    std::fprintf(fp, "T1 M6\n");
    std::fprintf(fp, "G54\n");
    std::fprintf(fp, "(Position 1)\n");
    std::fprintf(fp, "G94\n");
    std::fprintf(fp, "G1 X0.000 Y0.000 Z%.2f B0.000 C0.000 F%d\n", Z_home, F_G1_move);

    for (GLKPOSITION Pos = m_Waypoints->GetMeshList().GetHeadPosition(); Pos;) {
        QMeshPatch* WayPointPatch = (QMeshPatch*)m_Waypoints->GetMeshList().GetNext(Pos);

        if (WayPointPatch->GetIndexNo() < m_FromIndex || WayPointPatch->GetIndexNo() > m_ToIndex) continue;

        bool showOnece = true; // show the Z < 0 once

        for (GLKPOSITION Pos = WayPointPatch->GetNodeList().GetHeadPosition(); Pos;) {
            QMeshNode* Node = (QMeshNode*)WayPointPatch->GetNodeList().GetNext(Pos);
            double X = Node->m_XYZBCE(0); double Y = Node->m_XYZBCE(1); double Z = Node->m_XYZBCE(2);
            double B = Node->m_XYZBCE(3); double C = Node->m_XYZBCE(4); double E = Node->m_XYZBCE(5);
            int F = (int)Node->m_F;

            // increase extrusion of 1st layer
            if (WayPointPatch->GetIndexNo() == 0) E *= 2.0; 
            // // increase the extrusion of the first printed layer
            //if (WayPointPatch->GetIndexNo() == m_FromIndex) E *= 1.0;

            // check the huge change of C angle
            if (Node->GetIndexNo() != 0)
            {
                GLKPOSITION prevPos = WayPointPatch->GetNodeList().Find(Node)->prev;
                QMeshNode* prevNode = (QMeshNode*)WayPointPatch->GetNodeList().GetAt(prevPos);

                double C_prev = prevNode->m_XYZBCE(4);

                if (fabs(C - C_prev) > 300
                    && prevNode->Jump_preSecEnd == false && Node->Jump_nextSecStart == false) {

                    std::cerr << "fabs(C - C_prev) ERROR! " << fabs(C - C_prev) << std::endl;
                    std::cout << "WayPointPatch->GetIndexNo() " << WayPointPatch->GetIndexNo() << std::endl;
                }

            }
            // check the negative Z motion
            if (Z < -(h - 10) && showOnece) {
                std::cout << "Layer: " << WayPointPatch->GetIndexNo() << " Z < -h hit the bottom " << std::endl;
                showOnece = false;
            }

            // Record the max Z for security consideration (rest layers)
            if (Z > Z_max) { Z_max = Z; }

            //---------------------------------------------------------------------------------------------------------------------------------
            // special change for 5AM maching
            B = -B; C = -C;
            //---------------------------------------------------------------------------------------------------------------------------------

            // Add some auxiliary G code
            if (Node->GetIndexNo() == 0) {// for the 1st point
                // for the 1st(LayerInd_From) printed layer
                if (WayPointPatch->GetIndexNo() == m_FromIndex) {
                    // move to start of printing location
                    std::fprintf(fp, "G1 X%.2f Y%.2f B%.2f C%.2f F%d\n", X, Y, B, C, F_G1_move);
                    // slowly lower for printing
                    std::fprintf(fp, "G1 Z%.2f F%d\n", (Z_max + Z_compensateUpDistance), F_G1_move);
                    // zero extruded length(set E axis to 0)
                    std::fprintf(fp, "G92 A0\n");
                    std::fprintf(fp, "G1 A%.2f F%d\n", E_PumpCompensateL1, F_PumpCompensate);
                    std::fprintf(fp, "G92 A0\n");
                    std::fprintf(fp, "G1 F%d\n", F_G1_1stlayer);
                }
                //
                else {

                    // return to the safe point Z_max + Z_high
                    std::fprintf(fp, "G1 Z%.2f F%d\n", (Z_max + Z_high), F_G1_move);

                    std::fprintf(fp, "G92 A0\n");
                    std::fprintf(fp, "G1 A%.2f F%d\n", E_PumpBack, F_PumpBack);
                    std::fprintf(fp, "G92 A0\n");
                    //// return to the safe point Z_max + Z_high (move this to front of retraction of extrusion)
                    //std::fprintf(fp, "G1 Z%.2f F%d\n", (Z_max + Z_high), F_G1_move);
                    // move to start of printing location
                    std::fprintf(fp, "G1 X%.2f Y%.2f B%.2f C%.2f F%d\n", X, Y, B, C, F_G1_move);
                    // slowly lower for printing
                    std::fprintf(fp, "G1 Z%.2f F%d\n", (Z + Z_compensateUpDistance), F_G1_move);
                    // compensate extrusion
                    std::fprintf(fp, "G1 A%.2f F%d\n", E_PumpCompensate, F_PumpCompensate);
                    std::fprintf(fp, "G92 A0\n");
                    std::fprintf(fp, "G1 F%d\n", F_G1_original);

                }

                std::fprintf(fp, "G1 X%.2f Y%.2f Z%.2f B%.2f C%.2f A%.2f F%d\n", X, Y, Z, B, C, E, F);

            }
            else {
                // Consider the waypoints with too large Length //OR large Singularity areas
                if (Node->Jump_nextSecStart) {

                        std::fprintf(fp, "G1 A%.2f F%d\n", (E + E_PumpBack), F_PumpBack);
                        std::fprintf(fp, "G1 Z%.2f F%d\n", (Z_max + Z_high), F_G1_move);
                        std::fprintf(fp, "G1 X%.2f Y%.2f B%.2f C%.2f F%d\n", X, Y, B, C, F_G1_move);
                        std::fprintf(fp, "G1 Z%.2f F%d\n", (Z + Z_compensateUpDistance), F_G1_move);
                        std::fprintf(fp, "G1 A%.2f F%d\n", (E - 0.1), F_PumpCompensate);
                        std::fprintf(fp, "G1 Z%.2f A%.2f F%d\n", Z, E, F_G1_original);
                    
                }
                std::fprintf(fp, "G1 X%.2f Y%.2f Z%.2f B%.2f C%.2f A%.2f F%d\n", X, Y, Z, B, C, E, F);
            }
        }
    }

    std::fprintf(fp, "G92 A0\n");
    std::fprintf(fp, "G1 A%.2f F%d\n", E_PumpBack, F_G1_move); // PumpBack
    std::fprintf(fp, "G1 Z%.2f F%d\n", Z_home, F_G1_move); // return to the home point Z_home
    std::fprintf(fp, "G1 X0.0 Y0.0 B0.0 C0.0 F%d\n", F_G1_move);
    std::fprintf(fp, "M30\n");// Stop all of the motion

    std::fclose(fp);

    std::cout << "------------------------------------------- " << GcodeDir << " Gcode Write Finish!\n" << std::endl;
}

void GcodeGeneration::readGcodeFile(Eigen::MatrixXf& Gcode_Table, std::string FileName) {

    char targetFilename[1024];
    std::sprintf(targetFilename, "%s%s", "../DataSet/G_CODE/", FileName.c_str());
    FILE* fp; char linebuf[2048];
    double machine_X = 0.0, machine_Y = 0.0, machine_Z = 300.0, machine_B = 0.0, machine_C = 0.0;
    fp = fopen(targetFilename, "r");
    if (!fp) {
        printf("===============================================\n");
        printf("Can not open the data file - Gcode File Import!\n");
        printf("===============================================\n");
        return;
    }

    //get the num of lines in Gcode file.
    int lines = 0;
    while (true) {
        fgets(linebuf, 255, fp);
        if (feof(fp)) break;
        lines++;
    }
    std::fclose(fp);
    //std::cout << lines << std::endl;

    fp = fopen(targetFilename, "r");
    Gcode_Table = Eigen::MatrixXf::Zero(lines, 6);
    bool T3_flag = false;
    for (int i = 0; i < lines; i++) {

        double newLayerFlag = 0.0;// DOUBLE type is for the compactness of data structure

        fgets(linebuf, 255, fp);
        //std::cout << linebuf;

        std::string str = linebuf;
        std::string::size_type position_X = str.find("X");  std::string::size_type position_Y = str.find("Y");
        std::string::size_type position_Z = str.find("Z");
        std::string::size_type position_B = str.find("B");	std::string::size_type position_C = str.find("C");
        std::string::size_type position_E = str.find("A");	std::string::size_type position_F = str.find("F");

        //std::cout << position_X << " " << position_Y << " " << position_Z << " " << position_B << " " << position_C << std::endl;

        std::string::size_type GFlag = str.find("G1");
        if (GFlag != std::string::npos) {
            // G1 X0.000 Y0.000 Z250.000 B0.000 C0.000 F2000
            if (position_X != str.npos && position_Y != str.npos && position_Z != str.npos
                && position_B != str.npos && position_C != str.npos
                && position_E == str.npos && position_F != str.npos) {

                std::string X_temp = str.substr(position_X + 1, position_Y - position_X - 2);
                std::string Y_temp = str.substr(position_Y + 1, position_Z - position_Y - 2);
                std::string Z_temp = str.substr(position_Z + 1, position_B - position_Z - 2);
                std::string B_temp = str.substr(position_B + 1, position_C - position_B - 2);
                std::string C_temp = str.substr(position_C + 1, position_F - position_C - 2);

                machine_X = atof(X_temp.c_str());	machine_Y = atof(Y_temp.c_str());	machine_Z = atof(Z_temp.c_str());
                machine_B = atof(B_temp.c_str());	machine_C = atof(C_temp.c_str());

            }
            // G1 X-2.207 Y88.771 Z114.490 B55.324 C-861.683 A782.472 F2000
            // the most common case
            else if (position_X != str.npos && position_Y != str.npos && position_Z != str.npos
                && position_B != str.npos && position_C != str.npos
                && position_E != str.npos && position_F != str.npos) {

                std::string X_temp = str.substr(position_X + 1, position_Y - position_X - 2);
                std::string Y_temp = str.substr(position_Y + 1, position_Z - position_Y - 2);
                std::string Z_temp = str.substr(position_Z + 1, position_B - position_Z - 2);
                std::string B_temp = str.substr(position_B + 1, position_C - position_B - 2);
                std::string C_temp = str.substr(position_C + 1, position_E - position_C - 2);

                machine_X = atof(X_temp.c_str());	machine_Y = atof(Y_temp.c_str());	machine_Z = atof(Z_temp.c_str());
                machine_B = atof(B_temp.c_str());	machine_C = atof(C_temp.c_str());

            }
            // G1 X2.003 Y87.702 B55.445 C51.857 F2000
            else if (position_X != str.npos && position_Y != str.npos && position_Z == str.npos
                && position_B != str.npos && position_C != str.npos
                && position_E == str.npos && position_F != str.npos) {

                std::string X_temp = str.substr(position_X + 1, position_Y - position_X - 2);
                std::string Y_temp = str.substr(position_Y + 1, position_B - position_Y - 2);
                std::string B_temp = str.substr(position_B + 1, position_C - position_B - 2);
                std::string C_temp = str.substr(position_C + 1, position_F - position_C - 2);

                machine_X = atof(X_temp.c_str());	machine_Y = atof(Y_temp.c_str());
                machine_B = atof(B_temp.c_str());	machine_C = atof(C_temp.c_str());
            }
            // G1 X-1.64 Y138.66 Z189.29 F2000
            else if (position_X != str.npos && position_Y != str.npos && position_Z != str.npos
                && position_B == str.npos && position_C == str.npos
                && position_E == str.npos && position_F != str.npos) {

                std::string X_temp = str.substr(position_X + 1, position_Y - position_X - 2);
                std::string Y_temp = str.substr(position_Y + 1, position_Z - position_Y - 2);
                std::string Z_temp = str.substr(position_Z + 1, position_F - position_Z - 2);

                machine_X = atof(X_temp.c_str());	machine_Y = atof(Y_temp.c_str());
                machine_Z = atof(Z_temp.c_str());
            }
            // G1 Z2.940 F4750
            else if (position_X == str.npos && position_Y == str.npos && position_Z != str.npos
                && position_B == str.npos && position_C == str.npos
                && position_E == str.npos && position_F != str.npos) {

                std::string Z_temp = str.substr(position_Z + 1, position_F - position_Z - 2);

                machine_Z = atof(Z_temp.c_str());
            }
            // G1 Z3.022 E562.31 F4750
            else if (position_X == str.npos && position_Y == str.npos && position_Z != str.npos
                && position_B == str.npos && position_C == str.npos
                && position_E != str.npos && position_F != str.npos) {

                std::string Z_temp = str.substr(position_Z + 1, position_E - position_Z - 2);

                machine_Z = atof(Z_temp.c_str());

            }
            // G1 F1500 // for new layer flag
            else if (position_X == str.npos && position_Y == str.npos && position_Z == str.npos
                && position_B == str.npos && position_C == str.npos
                && position_E == str.npos && position_F != str.npos) {

                newLayerFlag = 1.0;
            }
            // test for special case
            // else { cout << "------------------------------------------ some special case" << endl; }

        }
        // test for special case
        // else { cout << "------------------------------------------ some special case" << endl; }
        //std::cout << "X: " << machine_X << " Y: " << machine_Y << " Z: " << machine_Z
        //	<< " B: " << machine_B << " C: " << machine_C << std::endl << std::endl;

        Gcode_Table.row(i) << machine_X, machine_Y, machine_Z, -machine_B, -machine_C, newLayerFlag;
    }
    std::fclose(fp);

    std::cout << "Value range of X axis: [" << Gcode_Table.col(0).maxCoeff()
        << ", " << Gcode_Table.col(0).minCoeff() << "]" << std::endl;
    std::cout << "Value range of Y axis: [" << Gcode_Table.col(1).maxCoeff()
        << ", " << Gcode_Table.col(1).minCoeff() << "]" << std::endl;
    std::cout << "Value range of Z axis: [" << Gcode_Table.col(2).maxCoeff()
        << ", " << Gcode_Table.col(2).minCoeff() << "]" << std::endl;
    std::cout << "Value range of B axis: [" << Gcode_Table.col(3).maxCoeff()
        << ", " << Gcode_Table.col(3).minCoeff() << "]" << std::endl;
    std::cout << "Value range of C axis: [" << Gcode_Table.col(4).maxCoeff()
        << ", " << Gcode_Table.col(4).minCoeff() << "]" << std::endl;

    std::cout << "------------------------------------------- Gcode Load Finish!" << std::endl;
}