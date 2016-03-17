/******************************************************************************

 @File         PVRTSVGObject.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  A class for storing a loaded SVG file

******************************************************************************/
#ifndef __PVRTSVGOBJECT_H__
#define __PVRTSVGOBJECT_H__

#include "PVRTContext.h"
#include "../PVRTVector.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

class CPVRTSVGNode;

#define DELETE_ARRAY(x) if(x){ delete[] x; x = 0; }

#if !defined(WIN32) && !defined(_stricmp)
	#define _stricmp strcasecmp
#endif

/*!****************************************************************************
 @Enum      EAssetType
 @Brief     Enum values for the different asset types
******************************************************************************/
enum EAssetType
{
	eAssetBase,
	eAssetFillPaint,
	eAssetLinearPaint,
	eAssetRadialPaint,
	eAssetPath,
	eAssetGradient,
	eAssetStop,
	eAssetStroke,
	eAssetSVGNode,
};

/*!****************************************************************************
 @Enum      EGradientUnits
 @Brief     Enum values for the gradient units
******************************************************************************/
enum EGradientUnits
{
	eUserSpaceOnUse,
	eUserSpace,
	eObjectBoundingBox
};

/*!****************************************************************************
 @Enum      EValue
 @Brief     Enum values for representing opacity values
******************************************************************************/
enum EValue
{
	eFillOpacity,
	eStrokeOpacity,
	eOpacity,
};

/*!****************************************************************************
 @Struct      SSVGValues
 @Brief       Holds a few loaded SVG values
******************************************************************************/
struct SSVGValues
{
	float fFillOpacity;
	float fStrokeOpacity;
	float fOpacity;
	bool bFillOpacitySet;
	bool bStrokeOpacitySet;
	bool bOpacitySet;

	SSVGValues()
	{
		fFillOpacity = 0;
		bFillOpacitySet = false;
		fStrokeOpacity = 0;
		bStrokeOpacitySet = false;
		fOpacity = 0;
		bOpacitySet = false;
	}
};

/*!****************************************************************************
 @Class       CPVRTSVGBase
 @Brief       A Base class that various other classes inherit from
******************************************************************************/
class CPVRTSVGBase
{
public:
	/*!***************************************************************************
	 @Function		CPVRTSVGBase
	 @Description	Constructor
	*****************************************************************************/
	CPVRTSVGBase(){ m_pNext = 0; m_pszID = 0;}

	/*!***************************************************************************
	 @Function		~CPVRTSVGBase
	 @Description	Deconstructor
	*****************************************************************************/
	virtual ~CPVRTSVGBase() { Destroy(); }

	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Frees the base class variables
	*****************************************************************************/
	virtual void Destroy() { delete[] m_pszID; m_pszID = 0; m_pNext = 0;}

	/*!***************************************************************************
	 @Function		GetType
	 @Returns		The class type
	 @Description	Returns the class type.
	*****************************************************************************/
	virtual EAssetType GetType() { return eAssetBase; }

	/*!***************************************************************************
	 @Function		SetNext
	 @Input			pNext A point to a CPVRTSVGBase
	 @Description	Sets the next base class in the list
	*****************************************************************************/
	virtual void SetNext(CPVRTSVGBase *pNext) { if(pNext) m_pNext = pNext; }

	/*!***************************************************************************
	 @Function		GetNext
	 @Returns		The next base class node
	 @Description	Returns the next base class node
	*****************************************************************************/
	virtual CPVRTSVGBase* GetNext() { return m_pNext; }

	/*!***************************************************************************
	 @Function		GetID
	 @Returns		The ID
	 @Description	Returns the ID
	*****************************************************************************/
	virtual char* GetID() { return m_pszID; }

	/*!***************************************************************************
	 @Function		SetID
	 @Input			pID The ID to set
	 @Returns		True on success
	 @Description	Sets the ID
	*****************************************************************************/
	virtual bool SetID(const char * const pID) { return Set(&m_pszID, pID, 0); }

protected:
	/*!***************************************************************************
	 @Function		Set
	 @Modified		pParam The output string
	 @Input			pStart The start of a string
	 @Input			pEnd	The end of a string. If 0 then it is assumed the string is null terminated.
	 @Returns 		True on success
	 @Description	Sets pParam to the string defined between pStart and pEnd
	*****************************************************************************/
	virtual bool Set(char** pParam, const char * const pStart, const char * const pEnd)
	{
		if(!pStart)
			return false;

		DELETE_ARRAY(*pParam);

		unsigned int ui32Length;

		if(!pEnd)
			ui32Length = (unsigned int) (strlen(pStart));
		else
			ui32Length = (unsigned int) (pEnd - pStart);

		if(ui32Length)
		{
			*pParam = new char[ui32Length + 1];
			memcpy(*pParam, pStart, ui32Length);
			(*pParam)[ui32Length] = '\0';
		}

		return true;
	}

protected:
	CPVRTSVGBase* m_pNext;
	char* m_pszID; // id attribute (for all tags, if provided)
};

/*!****************************************************************************
 @Class       CPVRTStroke
 @Brief       A class for storing SVG stroke data
******************************************************************************/
class CPVRTStroke: public CPVRTSVGBase
{
public:
	/*!***************************************************************************
	 @Function		CPVRTStroke
	 @Description	Constructor
	*****************************************************************************/
	CPVRTStroke();

	/*!***************************************************************************
	 @Function		~CPVRTStroke
	 @Description	Destructor
	*****************************************************************************/
	~CPVRTStroke();

	/*!***************************************************************************
	 @Function		GetType
	 @Returns		The class type
	 @Description	Returns the class type.
	*****************************************************************************/
	EAssetType GetType() { return eAssetStroke;}

	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Frees the class variables
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		=
	 @Input			Stroke A stroke
	 @Returns 		this
	 @Description	= operator
	*****************************************************************************/
	CPVRTStroke & operator=(const CPVRTStroke &Stroke);

public:
	float  m_fWidth;
	float* m_pafDashArray;
	float  m_fDashOffset;
	int    m_i32DashSize;
	int    m_i32LineCap;
	int	   m_i32LineJoin;
	float  m_fMiterLimit;
	float  m_fOpacity;
};

/*!****************************************************************************
 @Class       CPVRTPath
 @Brief       A class for storing SVG path data
******************************************************************************/
class CPVRTPath : public CPVRTSVGBase
{
public:
	/*!***************************************************************************
	 @Function		CPVRTPath
	 @Description	Constructor
	*****************************************************************************/
	CPVRTPath(void);

	/*!***************************************************************************
	 @Function		CPVRTPath
	 @Description	Constructor
	*****************************************************************************/
	~CPVRTPath(void);

	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Frees the class variables
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		GetType
	 @Returns		The class type
	 @Description	Returns the class type.
	*****************************************************************************/
	EAssetType GetType() { return eAssetPath; }

public:
	VGPath m_vgPath;

};

/*!****************************************************************************
 @Class       CPVRTPaint
 @Brief       A class for storing SVG paint data
******************************************************************************/
class CPVRTPaint : public CPVRTSVGBase
{
public:
	/*!***************************************************************************
	 @Function 		CPVRTPaint
	 @Description 	Constructor
	*****************************************************************************/
	CPVRTPaint(void);

	/*!***************************************************************************
	 @Function 		~CPVRTPaint
	 @Description 	Destructor
	*****************************************************************************/
	~CPVRTPaint(void);

	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Frees the class variables
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		GetType
	 @Returns		The class type
	 @Description	Returns the class type.
	*****************************************************************************/
	EAssetType GetType() { return m_eType; }

	/*!***************************************************************************
	 @Function		SetType
	 @Input			eType Sets the paints type
	 @Description	Sets the paints type.
	*****************************************************************************/
	void SetType(EAssetType eType) { m_eType = eType; }

	/*!***************************************************************************
	 @Function		=
	 @Input			Paint A paint
	 @Returns 		this
	 @Description	= operator
	*****************************************************************************/
	CPVRTPaint & operator=(const CPVRTPaint &Paint);

	/*!***************************************************************************
	 @Function		GetHref
	 @Returns		The paint's href
	 @Description	Returns the paint's href
	*****************************************************************************/
	char* GetHref() { return m_pHref; }

	/*!***************************************************************************
	 @Function		SetHref
	 @Input			pHref	Set the href for this paint
	 @Returns		True if successful
	 @Description	Sets the paints href
	*****************************************************************************/
	bool SetHref(const char * const pHref);

	/*!***************************************************************************
	 @Function		IsInit
	 @Returns		True if the paint is initialised
	 @Description	Returns true if the paint is initialised
	*****************************************************************************/
	bool IsInit() { return m_bInit; }

	/*!***************************************************************************
	 @Function		SetInit
	 @Input			bInit A boolean
	 @Description	Sets whether the paint is initialised
	*****************************************************************************/
	void SetInit( bool bInit) { m_bInit = bInit; }

	/*!***************************************************************************
	 @Function	SetTransform
	 @Input		pTransform A 3x3 matrix
	 @Description	Sets the paints transformation
	*****************************************************************************/
	void SetTransform(PVRTMat3 *pTransform);

	/*!***************************************************************************
	 @Function		GetTransform
	 @Returns		The paints transform
	 @Description	Returns the paints transform
	*****************************************************************************/
	PVRTMat3* GetTransform() { return &m_GradTransform; }

	/*!***************************************************************************
	 @Function		ApplyTransform
	 @Description	Apply the transformation to the paint
	*****************************************************************************/
	void ApplyTransform();

protected:
	EAssetType m_eType;

public:
	VGPaint m_vgPaint;
	float   m_afPoints[5];
	bool	m_bPointSet[5];
	EGradientUnits m_eGradientUnits;
	PVRTMat3 m_GradTransform;
	char *m_pHref;
	bool m_bInit;
	CPVRTSVGNode *m_pPrevNode;
};

/*!****************************************************************************
 @Class       CPVRTSVGNode
 @Brief       A class for storing SVG node data
******************************************************************************/
class CPVRTSVGNode : public CPVRTSVGBase
{
public:
	/*!***************************************************************************
	 @Function CPVRTSVGNode
	 @Description Constructor
	*****************************************************************************/
	CPVRTSVGNode(void);

	/*!***************************************************************************
	 @Function CPVRTSVGNode
	 @Description Destructor
	*****************************************************************************/
	~CPVRTSVGNode(void);

	/*!***************************************************************************
	 @Function	Destroy
	 @Description Frees the variables contained within the node
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		AddChild
	 @Input			pChild Add a child to this node
	 @Returns		True on success
	 @Description	Adds a child node to this node
	*****************************************************************************/
	bool AddChild(CPVRTSVGNode *pChild);

	/*!***************************************************************************
	 @Function		GetFirstChild
	 @Returns		The first child node
	 @Description	Returns the first child node
	*****************************************************************************/
	CPVRTSVGNode* GetFirstChild();

	/*!***************************************************************************
	 @Function		Draw
	 @Input			i32InheritedPaint The inherited paint mode
	 @Returns		True on success
	 @Description	Draw the node
	*****************************************************************************/
	bool Draw(int i32InheritedPaint = 0);

	/*!***************************************************************************
	 @Function		ApplyStrokes
	 @Input			i32Mode The paint mode
	 @Description	Apply stroking if the paint mode requests it
	*****************************************************************************/
	void ApplyStrokes(int i32Mode);

	/*!***************************************************************************
	 @Function		ApplyPaints
	 @Input			i32Mode The paint mode
	 @Input			pPath The path to apply the paints to
	 @Description	Applies the paints
	*****************************************************************************/
	void ApplyPaints(int i32Mode, CPVRTPath *pPath);

	/*!***************************************************************************
	 @Function		DrawPath
	 @Input			i32InheritedPaint The inherited paint mode
	 @Description	Draw a path
	*****************************************************************************/
	void DrawPath(int i32InheritedPaint);

	/*!***************************************************************************
	 @Function		ApplyTransform
	 @Description	Apply this node's transform
	*****************************************************************************/
	void ApplyTransform();

	/*!***************************************************************************
	 @Function		SetTransform
	 @Input			pTransform The nodes new transformation
	 @Description	Set the nodes transformation
	*****************************************************************************/
	void SetTransform(PVRTMat3 *pTransform);

	/*!***************************************************************************
	 @Function		GetTransform
	 @Returns		The node's transformation
	 @Description	Returns the node's transformation
	*****************************************************************************/
	PVRTMat3* GetTransform() { return &m_Transform; }

	/*!***************************************************************************
	 @Function		SetPath
	 @Input			pPath The path to set
	 @Description	Sets the node's path
	*****************************************************************************/
	void SetPath(CPVRTPath *pPath) { m_pPath = pPath; }

	/*!***************************************************************************
	 @Function		GetPath
	 @Returns		The node's path
	 @Description	Returns the node's path
	*****************************************************************************/
	CPVRTPath* GetPath() { return m_pPath; }

	/*!***************************************************************************
	 @Function		SetPaint
	 @Input			Mode	The paint mode to set the paint for
	 @Input			pPaint	The paint
	 @Description	Sets a paint for the node
	*****************************************************************************/
	void SetPaint(VGPaintMode Mode, CPVRTPaint *pPaint);

	/*!***************************************************************************
	 @Function		GetPaint
	 @Input			Mode The paint mode to get the paint for
	 @Returns		The paint
	 @Description	Returns the nodes paint for a particular mode
	*****************************************************************************/
	CPVRTPaint* GetPaint(VGPaintMode Mode);

	/*!***************************************************************************
	 @Function		SetStroke
	 @Input			pStroke The stroke to set
	 @Description	Sets the stroke for the node
	*****************************************************************************/
	void SetStroke(CPVRTStroke *pStroke);

	/*!***************************************************************************
	 @Function		GetStroke
	 @Returns		The stroke
	 @Description	Gets the node's stroke
	*****************************************************************************/
	CPVRTStroke* GetStroke() { return m_pStroke; }

	/*!***************************************************************************
	 @Function		SetPaintID
	 @Input			Mode The paint mode to set the paint ID for
	 @Input			pID The paint ID
	 @Returns		Returns true if successful
	 @Description	Sets the ID for a particular paint
	*****************************************************************************/
	bool SetPaintID(VGPaintMode Mode, const char *pID);

	/*!***************************************************************************
	 @Function		GetPaintID
	 @Input			Mode the paint mode to get the paint ID for
	 @Returns		The paint ID for a particular paint mode
	 @Description	Returns the paint ID for a particular paint
	*****************************************************************************/
	char* GetPaintID(VGPaintMode Mode);

	/*!***************************************************************************
	 @Function		SetPathID
	 @Input			pID The ID to set
	 @Returns		True on success
	 @Description	Sets the path ID for this node
	*****************************************************************************/
	bool SetPathID(const char *pID) { return Set(&m_pPathID, pID, 0); }

	/*!***************************************************************************
	 @Function		GetPathID
	 @Returns		The path ID
	 @Description	Returns the path ID
	*****************************************************************************/
	char* GetPathID() { return m_pPathID; }

	/*!***************************************************************************
	 @Function		SetFillRule
	 @Input			FillRule The fillrule to set
	 @Description	Sets the fill rule for this node
	*****************************************************************************/
	void SetFillRule(VGFillRule FillRule) { m_FillRule = FillRule; }

	/*!***************************************************************************
	 @Function		SetRender
	 @Input			bRender a boolean
	 @Description	Sets whether to render the node or not
	*****************************************************************************/
	void SetRender(bool bRender) { m_bRender = bRender; };

	/*!***************************************************************************
	 @Function		GetRender
	 @Returns		True if the node is to be rendered
	 @Description	Returns true if the node is to be rendered
	*****************************************************************************/
	bool GetRender() { return m_bRender; }

	/*!***************************************************************************
	 @Function		SetRenderChildren
	 @Input			bRenderChildren a boolean
	 @Description	set whether we should render this node's children
	 *****************************************************************************/
	void SetRenderChildren(bool bRenderChildren) { m_bRenderChildren = bRenderChildren; };

	/*!***************************************************************************
	 @Function		GetRenderChildren
	 @Returns		True if the node's children are to be rendered
	 @Description	Returns true if the node's children are to be rendered
	*****************************************************************************/
	bool GetRenderChildren() { return m_bRenderChildren; }

	/*!***************************************************************************
	 @Function		SetDef
	 @Input			pDef A node
	 @Description	Set the node that defines this one
	*****************************************************************************/
	void SetDef(CPVRTSVGNode* pDef) { m_pDefinedNode = pDef; }

    /*!***************************************************************************
	 @Function		GetDef
	 @Returns		The node that defines this one
	 @Description	Returns the node that defines this one
	*****************************************************************************/
	CPVRTSVGNode* GetDef() { return m_pDefinedNode; }

	/*!***************************************************************************
	 @Function		SetNone
	 @Input			Mode	The paint mode
	 @Input			bValue	true or false
	 @Description	Sets whether a particular paint mode has a paint or not
	*****************************************************************************/
	void SetNone(VGPaintMode Mode, bool bValue);

	/*!***************************************************************************
	 @Function		GetValue
	 @Input			eValue The variable you wish to return the value for
	 @Returns		A value
	 @Description	Returns a value
	*****************************************************************************/
	float* GetValue(EValue eValue);

	/*!***************************************************************************
	 @Function		SetValue
	 @Input			eValue The value you wish to set
	 @Input			fValue	The value
	 @Return		Returns true on success
	 @Description	Sets a value
	*****************************************************************************/
	bool SetValue(EValue eValue, float fValue);

protected:
	/*!***************************************************************************
	 @Function		ApplyPaint
	 @Input			pPaint Paint to apply
	 @Input			pPath The path the paint is getting applied to
	 @Input			i32Mode	The paint mode
	 @Description	Applies a paint
	*****************************************************************************/
	void ApplyPaint(CPVRTPaint *pPaint, CPVRTPath *pPath, VGPaintMode i32Mode);

protected:
	CPVRTSVGNode *m_pDefinedNode;
	CPVRTPath  *m_pPath;
	CPVRTPaint *m_pFillPaint;
	CPVRTPaint *m_pStrokePaint;
	CPVRTStroke *m_pStroke;

	SSVGValues *m_pValues;

	VGFillRule m_FillRule;
	CPVRTSVGNode* m_pFirstChild;
	CPVRTSVGNode* m_pLastChild;

	int m_i32PaintMode;
	char *m_pFillID;
	char *m_pStrokeID;
	char *m_pPathID;
	bool m_bRender;
	bool m_bRenderChildren;
	bool m_bFillNone;
	bool m_bStrokeNone;

	PVRTMat3	m_Transform;
};

/*!****************************************************************************
 @Class       CPVRTSVGObject
 @Brief       A class for storing and drawing loaded SVG data
******************************************************************************/
class CPVRTSVGObject
{
public:
	/*!***************************************************************************
	 @Function		CPVRTSVGObject
	 @Description	Constructor
	*****************************************************************************/
	CPVRTSVGObject(void);

	/*!***************************************************************************
	@Function		~CPVRTSVGObject
	@Description	Destructor
	*****************************************************************************/
	~CPVRTSVGObject(void);

	/*!***************************************************************************
	 @Function	Destroy
	 @Description Frees all the memory of the SVG object
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		AddAsset
	 @Input			pAsset The asset to add
	 @Returns		True if the asset was successfully added
	 @Description	Adds an asset to the list
	*****************************************************************************/
	bool AddAsset(CPVRTSVGBase* pAsset);

	/*!***************************************************************************
	 @Function		SetRoot
	 @Input			pRoot The root node
	 @Returns		true if successful
	 @Description	Sets the root node of the SVG Object
	*****************************************************************************/
	bool SetRoot(CPVRTSVGNode* pRoot);

	/*!***************************************************************************
	 @Function		GetRoot
	 @Returns		The root node
	 @Description	Returns the root node
	*****************************************************************************/
	CPVRTSVGNode* GetRoot();

	/*!***************************************************************************
	 @Function		Draw
	 @Returns		True on success
	 @Description	Draws the SVG Object
	*****************************************************************************/
	bool Draw();

	/*!***************************************************************************
	 @Function		FindAsset
	 @Input			pID		Asset to find
	 @Input			ui32Size Length of ID
	 @Returns		false if unsuccessful or a point to the asset
	 @Description	Finds an asset
	*****************************************************************************/
	CPVRTSVGBase* FindAsset(const char * pID, unsigned int ui32Size);

	/*!***************************************************************************
	 @Function		SetupReferences
	 @Description	Sets up the references
	*****************************************************************************/
	void SetupReferences();

	/*!***************************************************************************
	 @Function		SetupReferences
	 @Input			pNode	Setup the references for this particular node
	 @Input			bDoChildren	Setup the references for this Node's children
	 @Description	Sets up the references for a particular node
	*****************************************************************************/
	void SetupReferences(CPVRTSVGNode *pNode, bool bDoChildren = false);

	/*!***************************************************************************
	 @Function		FindNode
	 @Input			pRoot	The root of the list to search
	 @Input			pID		The ID to search for
	 @Input			ui32Size The size of the ID
	 @Returns		false if unsuccessful or a pointer to the node
	 @Description	Finds a node in a particular list
	*****************************************************************************/
	CPVRTSVGNode* FindNode(CPVRTSVGNode* pRoot, const char * pID, unsigned int ui32Size);

	/*!***************************************************************************
	 @Function		SetScreenDimensions
	 @Input			ui32Width	Width of the screen
	 @Input			ui32Height	Height of the screen
	 @Description	Sets the screen width and height for the SVG object
	*****************************************************************************/
	void SetScreenDimensions(unsigned int ui32Width, unsigned int ui32Height);

	/*!***************************************************************************
	 @Function SetViewBox
	 @Input fX	ViewBox x origin
	 @Input	fY	ViewBox y origin
	 @Input	fWidth	ViewBox width
	 @Input	fHeight	ViewBox height
	 @Description	Set the view box parameters
	*****************************************************************************/
	void SetViewBox(float fX, float fY, float fWidth, float fHeight);

	/*!***************************************************************************
	 @Function		SetToOrigin
	 @Description	Set the centre of the object to the origin
	*****************************************************************************/
	void SetToOrigin();

	/*!***************************************************************************
	 @Function		SetTransformation
	 @Input			pMatrix	The transformation
	 @Description	Sets the transformation for the whole scene
	*****************************************************************************/
	void SetTransformation(PVRTMat3 *pMatrix);

protected:
	/*!***************************************************************************
	 @Function		Draw
	 @Input			pNode The node to draw
	 @Returns		True on success
	 @Description	Draws a particular SVG Node
	*****************************************************************************/
	bool Draw(CPVRTSVGNode *pNode);

	/*!***************************************************************************
	 @Function		SetupAssetReferences
	 @Input			pNode	The node to set the asset references to
	 @Description	Setup the asset references (e.g. paints) for pNode
	*****************************************************************************/
	void SetupAssetReferences(CPVRTSVGBase *pNode);

	/*!***************************************************************************
	 @Function		SetupPaint
	 @Input			pNode Node to setup the paint for
	 @Input			Mode The paint to setup
	 @Description	Sets up a paint for a node
	*****************************************************************************/
	void SetupPaint(CPVRTSVGNode *pNode, VGPaintMode Mode);

protected:
	CPVRTSVGNode *m_pRoot;
	CPVRTSVGBase *m_pFirstAsset;
	CPVRTSVGBase *m_pLastAsset;
	PVRTMat3	m_ViewBoxTransform;
	PVRTMat3  m_InitialTransform;
	PVRTMat3	m_UserTransform;
	unsigned int m_ui32Width;
	unsigned int m_ui32Height;
	float m_fObjectWidth;
	float m_fObjectHeight;
	float m_fObjectX;
	float m_fObjectY;
};

#endif /*__PVRTSVGOBJECT_H__*/

/*****************************************************************************
 End of file (PVRTSVGObject.h)
*****************************************************************************/

