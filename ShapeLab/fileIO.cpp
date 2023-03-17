#include "fileIO.h"
#include <fstream>
#include "alphanum.hpp"
#include "dirent.h"
#include "io.h"

void fileIO::input_remeshed_Layer(PolygenMesh* isoLayerSet, std::string path) {

	std::string remeshed_isoLayer_dir = path;
	std::vector<std::string> remeshedLayer_FileCell;// File name table
	this->_natSort(remeshed_isoLayer_dir, remeshedLayer_FileCell);

	//read slice files and build mesh_patches
	char file_Dir[1024];
	for (int i = 0; i < remeshedLayer_FileCell.size(); i++)
	{
		sprintf(file_Dir, "%s%s%s", remeshed_isoLayer_dir.c_str(), "/", remeshedLayer_FileCell[i].data());
		//std::cout << file_Dir << std::endl;

		QMeshPatch* slice = new QMeshPatch;
		slice->SetIndexNo(isoLayerSet->GetMeshList().GetCount()); //index begin from 0
		isoLayerSet->GetMeshList().AddTail(slice);
		slice->patchName = remeshedLayer_FileCell[i].data();

		slice->inputOBJFile(file_Dir);
	}
}

void fileIO::_natSort(std::string dirctory, std::vector<std::string>& fileNameCell) {

	if (fileNameCell.empty() == false) return;

	DIR* dp;
	struct dirent* ep;
	std::string fullDir = dirctory;
	//cout << fullDir << endl;
	dp = opendir(fullDir.c_str());
	//dp = opendir("../Waypoints");

	if (dp != NULL) {
		while (ep = readdir(dp)) {
			//cout << ep->d_name << endl;
			if ((std::string(ep->d_name) != ".") && (std::string(ep->d_name) != "..")) {
				//cout << ep->d_name << endl;
				fileNameCell.push_back(std::string(ep->d_name));
			}
		}
		(void)closedir(dp);
	}
	else {
		perror("Couldn't open the directory");
	}
	//resort the files with nature order
	sort(fileNameCell.begin(), fileNameCell.end(), doj::alphanum_less<std::string>());

}

void fileIO::spiralToolpath_Output(PolygenMesh* spiralTPath, std::string dir, int index, bool OnOff) {

	if (OnOff == false) return;

	if (index == 0) this->_remove_allFile_in_Dir(dir);

	for (GLKPOSITION posMesh = spiralTPath->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshPatch* each_toolpath = (QMeshPatch*)spiralTPath->GetMeshList().GetNext(posMesh);

		if (each_toolpath->curveType != 3) continue;

		std::string TOOLPATH_dir;
		TOOLPATH_dir = dir + "/" + std::to_string(index) + ".txt";

		std::cout << "Output File: " << TOOLPATH_dir << std::endl;

		std::ofstream toolpathFile(TOOLPATH_dir);

		double pp[3],n[3];

		for (GLKPOSITION Pos = each_toolpath->GetNodeList().GetHeadPosition(); Pos;) {
			QMeshNode* Node = (QMeshNode*)each_toolpath->GetNodeList().GetNext(Pos);

			Node->GetCoord3D(pp[0], pp[1], pp[2]);
			Node->GetNormal(n[0], n[1], n[2]);
			// the normal is inversed to make it towards up
			toolpathFile << pp[0] << " " << pp[1] << " " << pp[2] << " " << -n[0] << " " << -n[1] << " " << -n[2] << std::endl;
		}
		toolpathFile.close();

	}
	std::cout << "Output Files Finish!" << std::endl;

}

void fileIO::normalToolpath_Output(PolygenMesh* toolPath, std::string TOOLPATH_waypoint_dir) {

	this->_remove_allFile_in_Dir(TOOLPATH_waypoint_dir);

	std::vector<QMeshPatch*> toolpath_list;
	for (GLKPOSITION Pos = toolPath->GetMeshList().GetHeadPosition(); Pos;) {
		QMeshPatch* each_toolpath = (QMeshPatch*)toolPath->GetMeshList().GetNext(Pos);

		if (each_toolpath == NULL) {
			std::cout << "Toolpath " << each_toolpath->GetIndexNo() << " is a NULL patch." << std::endl;
			continue;
		}
		toolpath_list.push_back(each_toolpath);
	}

	//output waypoints
	for (int i = 0; i < toolpath_list.size(); i++) {

		QMeshPatch* each_toolpath = toolpath_list[i];

		std::string eachToolpath_dir;
		eachToolpath_dir = TOOLPATH_waypoint_dir + std::to_string(i) + ".txt";

		//std::cout << "Output File: " << TOOLPATH_dir << std::endl;

		std::ofstream toolpathFile(eachToolpath_dir);

		double pp[3]; double n[3];
		QMeshEdge* sEdge = (QMeshEdge*)each_toolpath->GetEdgeList().GetHead(); // the first Edge of Toolpath
		QMeshNode* sNode = sEdge->GetStartPoint();
		sNode->GetCoord3D(pp[0], pp[1], pp[2]); sNode->GetNormal(n[0], n[1], n[2]);
		// the normal is inversed to make it towards up
		toolpathFile << pp[0] << " " << pp[1] << " " << pp[2] << " " << -n[0] << " " << -n[1] << " " << -n[2] << std::endl;

		for (GLKPOSITION posEdge = each_toolpath->GetEdgeList().GetHeadPosition(); posEdge != nullptr;) {
			QMeshEdge* Edge = (QMeshEdge*)each_toolpath->GetEdgeList().GetNext(posEdge);

			//std::cout << "start Node " << Edge->GetStartPoint()->GetIndexNo() << " end Node " << Edge->GetEndPoint()->GetIndexNo() << std::endl;

			QMeshNode* eNode = Edge->GetStartPoint();
			if (eNode == sNode) eNode = Edge->GetEndPoint();

			eNode->GetCoord3D(pp[0], pp[1], pp[2]); eNode->GetNormal(n[0], n[1], n[2]);
			// the normal is inversed to make it towards up
			toolpathFile << pp[0] << " " << pp[1] << " " << pp[2] << " " << -n[0] << " " << -n[1] << " " << -n[2] << std::endl;

			sNode = eNode;
		}
		toolpathFile.close();
	}
	std::cout << "Finish output toolpath into : " << ".. / DataSet / TOOL_PATH /" << std::endl;
}

int fileIO::_remove_allFile_in_Dir(std::string dirPath) {

	struct _finddata_t fb;   //find the storage structure of the same properties file.
	std::string path;
	intptr_t    handle;
	int  resultone;
	int   noFile;            // the tag for the system's hidden files

	noFile = 0;
	handle = 0;

	path = dirPath + "/*";

	handle = _findfirst(path.c_str(), &fb);

	//find the first matching file
	if (handle != -1)
	{
		//find next matching file
		while (0 == _findnext(handle, &fb))
		{
			// "." and ".." are not processed
			noFile = strcmp(fb.name, "..");

			if (0 != noFile)
			{
				path.clear();
				path = dirPath + "/" + fb.name;

				//fb.attrib == 16 means folder
				if (fb.attrib == 16)
				{
					_remove_allFile_in_Dir(path);
				}
				else
				{
					//not folder, delete it. if empty folder, using _rmdir instead.
					remove(path.c_str());
				}
			}
		}
		// close the folder and delete it only if it is closed. For standard c, using closedir instead(findclose -> closedir).
		// when Handle is created, it should be closed at last.
		_findclose(handle);
		return 0;
	}
}

void fileIO::outputIsoSurface(PolygenMesh* isoSurface, std::string path) {

	this->_remove_allFile_in_Dir(path);

	int layer_index = 0; std::string LAYER_dir;

	for (GLKPOSITION posMesh = isoSurface->GetMeshList().GetHeadPosition(); posMesh != nullptr;) {
		QMeshPatch* each_layer = (QMeshPatch*)isoSurface->GetMeshList().GetNext(posMesh);

		LAYER_dir = path + std::to_string(layer_index);

		this->_output_OneSurfaceMesh(each_layer, LAYER_dir);
		layer_index++;

	}
	std::cout << "Finish output layers into : " << ".. / DataSet / CURVED_LAYER /" << std::endl;
}

void fileIO::_output_OneSurfaceMesh(QMeshPatch* eachLayer, std::string path) {

	double pp[3];
	path += ".obj";
	std::ofstream nodeSelection(path);

	int index = 0;
	for (GLKPOSITION posNode = eachLayer->GetNodeList().GetHeadPosition(); posNode != nullptr;) {
		QMeshNode* node = (QMeshNode*)eachLayer->GetNodeList().GetNext(posNode);
		node->GetCoord3D(pp[0], pp[1], pp[2]);
		nodeSelection << "v " << pp[0] << " " << pp[1] << " " << pp[2] << std::endl;
		index++; node->SetIndexNo(index);

	}
	for (GLKPOSITION posFace = eachLayer->GetFaceList().GetHeadPosition(); posFace != nullptr;) {
		QMeshFace* face = (QMeshFace*)eachLayer->GetFaceList().GetNext(posFace);
		nodeSelection << "f " << face->GetNodeRecordPtr(0)->GetIndexNo()
			<< " " << face->GetNodeRecordPtr(1)->GetIndexNo()
			<< " " << face->GetNodeRecordPtr(2)->GetIndexNo() << std::endl;
	}
	nodeSelection.close();
}

int fileIO::read_layer_toolpath_cnc_files(
	PolygenMesh* Slices, PolygenMesh* Waypoints, PolygenMesh* CncPart,
	std::string Dir, std::string modelName) {

	std::string PosNorFileDir = Dir + modelName + "/waypoint";
	std::string LayerFileDir = Dir + modelName + "/layer_Simplified";
	std::string CncFileDir = Dir + "cnc_prt";

	this->_natSort(PosNorFileDir, wayPointFileCell);
	this->_natSort(LayerFileDir, sliceSetFileCell);
	this->_natSort(CncFileDir, cncFileCell);

	if (wayPointFileCell.size() != sliceSetFileCell.size()) {
		std::cout << "The file number of slics and toolpath is not the same, please check." << std::endl;
		return 0;
	}

	this->_readWayPointData(Waypoints, PosNorFileDir);
	this->_readSliceData(Slices, LayerFileDir);
	this->_readCncData(CncPart, CncFileDir);

	return wayPointFileCell.size();
}

void fileIO::_readWayPointData(PolygenMesh* Waypoints, std::string packName) {

	//read waypoint files and build mesh_patches
	char filename[1024];
	for (int i = 0; i < wayPointFileCell.size(); i++) {

		sprintf(filename, "%s%s%s", packName.c_str(), "/", wayPointFileCell[i].data());

		QMeshPatch* waypoint = new QMeshPatch;
		waypoint->SetIndexNo(Waypoints->GetMeshList().GetCount()); //index begin from 0
		Waypoints->GetMeshList().AddTail(waypoint);
		waypoint->patchName = wayPointFileCell[i].data();

		waypoint->inputPosNorFile(filename);
	}
	std::cout << "------------------------------------------- WayPoints Load Finish!" << std::endl;
}

void fileIO::_readSliceData(PolygenMesh* Slices, std::string packName) {

	//read slice files and build mesh_patches
	char filename[1024];
	for (int i = 0; i < sliceSetFileCell.size(); i++) {

		sprintf(filename, "%s%s%s", packName.c_str(), "/", sliceSetFileCell[i].data());

		QMeshPatch* layers = new QMeshPatch;
		layers->SetIndexNo(Slices->GetMeshList().GetCount()); //index begin from 0
		Slices->GetMeshList().AddTail(layers);
		layers->patchName = sliceSetFileCell[i].data();

		layers->inputOBJFile(filename);
	}
	std::cout << "------------------------------------------- Slices Load Finish!" << std::endl;
}

void fileIO::_readCncData(PolygenMesh* CncPart, std::string packName) {

	std::vector<std::string> CNCfileSet;
	CNCfileSet = { "frame", "x_axis", "y_axis", "z_axis", "b_axis", "c_axis", "nozzle" };

	//read CNC files and build mesh_patches
	char filename[1024];

	for (int i = 0; i < CNCfileSet.size(); i++) {
		sprintf(filename, "%s%s%s%s", packName.c_str(), "/cnc_assembly_", CNCfileSet[i].c_str(), ".obj");
		std::cout << "input " << CNCfileSet[i].data() << " from: " << filename << std::endl;

		QMeshPatch* cncPatch = new QMeshPatch;
		cncPatch->SetIndexNo(CncPart->GetMeshList().GetCount()); //index begin from 0
		CncPart->GetMeshList().AddTail(cncPatch);
		cncPatch->inputOBJFile(filename);
		cncPatch->patchName = CNCfileSet[i].data();
	}
}