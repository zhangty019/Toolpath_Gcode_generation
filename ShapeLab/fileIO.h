#pragma once
#include "../QMeshLib/PolygenMesh.h"

class fileIO
{
public:
	fileIO() {};
	~fileIO() {};

	void input_remeshed_Layer(PolygenMesh* isoLayerSet, std::string path);
	void spiralToolpath_Output(PolygenMesh* spiralTPath, std::string dir,
		int index, bool OnOff);
	void normalToolpath_Output(PolygenMesh* toolPath, std::string dir);
	void outputIsoSurface(PolygenMesh* isoSurface, std::string path);
	int read_layer_toolpath_cnc_files(
		PolygenMesh* Slices, PolygenMesh* Waypoints, PolygenMesh* CncPart,
		std::string Dir, std::string modelName);
	int read_layer_toolpath_files(
		PolygenMesh* Slices, PolygenMesh* Waypoints, std::string Dir);
	void output_toolpath_UR5e(PolygenMesh* toolPath, std::string FileDir);

private:

	void _natSort(std::string dirctory, std::vector<std::string>& fileNameCell);
	int _remove_allFile_in_Dir(std::string dirPath);
	void _output_OneSurfaceMesh(QMeshPatch* eachLayer, std::string path);

	void _readWayPointData(PolygenMesh* Waypoints, std::string packName);
	void _readSliceData(PolygenMesh* Slices, std::string packName);
	void _readCncData(PolygenMesh* CncPart, std::string packName);


	std::vector<std::string> wayPointFileCell;// Waypoints Dir Files
	std::vector<std::string> sliceSetFileCell;// Layers Dir Files
	std::vector<std::string> cncFileCell;// Cnc Dir Files
	int threshold_nodeNum = 0;

};