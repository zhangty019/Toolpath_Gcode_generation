// QMeshEdge.h: interface for the QMeshEdge class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _QMESHEDGE
#define _QMESHEDGE

#include "../GLKLib/GLKObList.h"
#include <vector>
class QMeshPatch;
class QMeshNode;
class QMeshFace;

class QMeshEdge : public GLKObject  
{
public:
	QMeshEdge();
	virtual ~QMeshEdge();

public:
	bool GetAttribFlag( const int whichBit );
	void SetAttribFlag( const int whichBit, const bool toBe = true );

	int GetIndexNo();		//from 1 to n
	void SetIndexNo( const int _index = 1 );

	bool IsBoundaryEdge();

	QMeshNode * GetStartPoint();
    void SetStartPoint( QMeshNode * _pStartPoint = nullptr );

	QMeshNode * GetEndPoint();
    void SetEndPoint( QMeshNode * _pEndPoint = nullptr );

	QMeshFace * GetLeftFace();
    void SetLeftFace( QMeshFace * _pLeftFace = nullptr );

	QMeshFace * GetRightFace();
    void SetRightFace( QMeshFace * _pRightFace = nullptr );

	void SetMeshPatchPtr(QMeshPatch* _mesh);
	QMeshPatch* GetMeshPatchPtr();

	void CalNormal(double normal[]);

	void SetSharpFactor(int factor);
	int GetSharpFactor();

	double CalLength();
	double GetLength() {return m_edgeLength;};
	double Cal2DLength();
	double Get2DLength() {return m_edge2DLength;};

    GLKObList& GetAttachedList() {return attachedList;};

	////******************************************************************
	////	Interpolate Points List
	//int attrPntNum;
	//float **attrPnt;		//	attrPnt[attrPntNum][3]
	//float *attrLength;		//	attrLength[attrPntNum-1]
	//float attrTotalLength;	

	void *attachedPointer;
    bool boundary, boundary1, boundary2;
    int SystemLineID;

    bool inner;
    //int seamIndex;
    bool selected;
    //int cableIndex = -1;
    bool IsMoreWeight;
    bool bVisit;
	int index_forAbaqus = 0;
    double weight;

	bool isConnectEdge = false;
	std::vector<QMeshNode*> installedIsoNode_layerEdge;//for layer Iso-curve generation, indicate which isoNode is on this edge of Layer
	bool isLocateIsoNode_layerEdge = false;//indicate the wether have a isoNode on this Edge of Layer
	std::vector<QMeshNode*> installedIsoNode_layerEdge_zigzag;
	bool isLocateIsoNode_layerEdge_zigzag = false;
	
	bool resample_processed = false; // indicate the edge was resampled
	std::vector<QMeshNode*> resample_installedIsoNode;// install the resampled Nodes
	QMeshNode* installed_CutNode_layerEdge; // for cutline generation
	bool isLocate_CutNode_layerEdge = false;//indicate whether have a cut Node on this Edge of Layer
	int refineNodeIndex = -1;//for surface mesh refinement when insert new Node in the middle of this Edge
	bool isConnectEdge_zigzag = false;

private:
	int indexno;
	bool flags[8];
				// bit 0 -- TRUE for boundary edge
				// bit 1 -- TRUE for sharp-feature edge	(or edges on key feature curves)
				// bit 2 -- TRUE for sharp-feature-region edge (or edges on accessory feature curves)
				// bit 4 -- TRUE for the edge needs to be splitted
				// bit 6 -- TRUE for edges need to be cut off
				// bit 7 -- temp use

                                     //*** Edge vector definition ***
                                     //                             *
		                             //         end point           *
	QMeshNode * pStartPoint;		 //           /|\               *
	QMeshNode * pEndPoint;			 //            |                *
                                     //  left face | right face     *
	QMeshFace * pLeftFace = nullptr; //            |                *
	QMeshFace * pRightFace = nullptr;//            |                *
		                             //       start point           *
		                             //                             *
		                             //******************************

	QMeshPatch *meshSurface;		// QMesh contain this edge

	GLKObList attachedList;			// a list of attached object

	int m_sharpFactor;	
	double m_edgeLength,m_edge2DLength;

	//for volume mesh
	GLKObList FaceList;	// a list of triangle's faces (TRGLFACE)

public:
	GLKObList& GetFaceList();
};

#endif
