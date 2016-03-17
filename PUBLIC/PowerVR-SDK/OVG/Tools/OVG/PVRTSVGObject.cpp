/******************************************************************************

 @File         PVRTSVGObject.cpp

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  A class for storing a loaded SVG file

******************************************************************************/
#include "PVRTSVGObject.h"

/*!***************************************************************************
 @Function CPVRTSVGNode
 @Description Constructor
*****************************************************************************/
CPVRTSVGNode::CPVRTSVGNode(void)
{
	m_pFirstChild = 0;
	m_pFillID = 0;
	m_pStrokeID = 0;
	m_pPathID = 0;
	m_pValues = 0;

	Destroy();
}

/*!***************************************************************************
 @Function CPVRTSVGNode
 @Description Destructor
*****************************************************************************/
CPVRTSVGNode::~CPVRTSVGNode(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function	Destroy
 @Description Frees the variables contained within the node
*****************************************************************************/
void CPVRTSVGNode::Destroy()
{
	m_pDefinedNode = 0;
	m_pPath = 0;

	CPVRTSVGNode *pNode = m_pFirstChild;
	CPVRTSVGNode *pNext;

	while(pNode)
	{
		pNext = (CPVRTSVGNode*) pNode->GetNext();
		delete pNode;
		pNode = pNext;
	}

	m_pFirstChild = 0;
	m_pLastChild = 0;

	m_i32PaintMode = 0;
	m_pFillPaint = 0;
	m_pStrokePaint = 0;
	m_pStroke= 0;

	delete[] m_pFillID;
	m_pFillID = 0;

	delete[] m_pStrokeID;
	m_pStrokeID = 0;

	delete[] m_pPathID;
	m_pPathID = 0;

	SetFillRule(VG_NON_ZERO);

	CPVRTSVGBase::Destroy();

	m_bRender = true;
	m_bRenderChildren = true;
	m_bFillNone = false;
	m_bStrokeNone = false;

	delete m_pValues;

	m_Transform = PVRTMat3::Identity();
}

/*!***************************************************************************
 @Function		ApplyTransform
 @Description	Apply this node's transform
*****************************************************************************/
void CPVRTSVGNode::ApplyTransform()
{
	vgMultMatrix(m_Transform.f);
}

/*!***************************************************************************
 @Function		SetTransform
 @Input			pTransform The nodes new transformation
 @Description	Set the nodes transformation
*****************************************************************************/
void CPVRTSVGNode::SetTransform(PVRTMat3 *pTransform)
{
	m_Transform = *pTransform;
}

/*!***************************************************************************
 @Function		GetValue
 @Input			eValue The variable you wish to return the value for
 @Returns		A value
 @Description	Returns a value
*****************************************************************************/
float* CPVRTSVGNode::GetValue(EValue eValue)
{
	if(m_pValues)
	{
		switch(eValue)
		{
			case eFillOpacity:
				if(m_pValues->bFillOpacitySet)
					return &m_pValues->fFillOpacity;
			break;
			case eStrokeOpacity:
				if(m_pValues->bStrokeOpacitySet)
					return &m_pValues->fStrokeOpacity;
			break;
			case eOpacity:
				if(m_pValues->bOpacitySet)
					return &m_pValues->fOpacity;
			break;
		}
	}

	return 0;
}

/*!***************************************************************************
 @Function		SetValue
 @Input			eValue The value you wish to set
 @Input			fValue	The value
 @Return		Returns true on success
 @Description	Sets a value
*****************************************************************************/
bool CPVRTSVGNode::SetValue(EValue eValue, float fValue)
{
	if(!m_pValues)
		m_pValues = new SSVGValues;

	if(m_pValues)
	{
		switch(eValue)
		{
			case eFillOpacity:
				m_pValues->fFillOpacity = fValue;
				m_pValues->bFillOpacitySet = true;
			break;
			case eStrokeOpacity:
				m_pValues->fStrokeOpacity = fValue;
				m_pValues->bStrokeOpacitySet = true;
			break;
			case eOpacity:
				m_pValues->fOpacity = fValue;
				m_pValues->bOpacitySet = true;
			break;
		}

		return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		SetPaint
 @Input			Mode	The paint mode to set the paint for
 @Input			pPaint	The paint
 @Description	Sets a paint for the node
*****************************************************************************/
void CPVRTSVGNode::SetPaint(VGPaintMode Mode, CPVRTPaint *pPaint)
{
	if(pPaint)
	{
		if(Mode & VG_FILL_PATH)
			m_pFillPaint = pPaint;

		if(Mode & VG_STROKE_PATH)
			m_pStrokePaint = pPaint;

		m_i32PaintMode |= (int) Mode;
	}
}

/*!***************************************************************************
 @Function		SetNone
 @Input			Mode	The paint mode
 @Input			bValue	true or false
 @Description	Sets whether a particular paint mode has a paint or not
*****************************************************************************/
void CPVRTSVGNode::SetNone(VGPaintMode Mode, bool bValue)
{
	if(Mode & VG_FILL_PATH)
		m_bFillNone = bValue;

	if(Mode & VG_STROKE_PATH)
		m_bStrokeNone = bValue;
}

/*!***************************************************************************
 @Function		GetPaint
 @Input			Mode The paint mode to get the paint for
 @Returns		The paint
 @Description	Returns the nodes paint for a particular mode
*****************************************************************************/
CPVRTPaint* CPVRTSVGNode::GetPaint(VGPaintMode Mode)
{
	if(Mode == VG_FILL_PATH)
		return m_pFillPaint;
	else if(Mode == VG_STROKE_PATH)
		return m_pStrokePaint;
	else
		return 0;
}

/*!***************************************************************************
 @Function		SetPaintID
 @Input			Mode The paint mode to set the paint ID for
 @Input			pID The paint ID
 @Returns		Returns true if successful
 @Description	Sets the ID for a particular paint
*****************************************************************************/
bool CPVRTSVGNode::SetPaintID(VGPaintMode Mode, const char *pID)
{
	if(Mode == VG_FILL_PATH)
		return Set(&m_pFillID, pID, 0);
	else if(Mode == VG_STROKE_PATH)
		return Set(&m_pStrokeID, pID, 0);
	else
		return false;
}

/*!***************************************************************************
 @Function		GetPaintID
 @Input			Mode the paint mode to get the paint ID for
 @Returns		The paint ID for a particular paint mode
 @Description	Returns the paint ID for a particular paint
*****************************************************************************/
char* CPVRTSVGNode::GetPaintID(VGPaintMode Mode)
{
	if(Mode == VG_FILL_PATH)
		return m_pFillID;
	else if(Mode == VG_STROKE_PATH)
		return m_pStrokeID;
	else
		return 0;
}

/*!***************************************************************************
 @Function		SetStroke
 @Input			pStroke The stroke to set
 @Description	Sets the stroke for the node
*****************************************************************************/
void CPVRTSVGNode::SetStroke(CPVRTStroke *pStroke)
{
	if(pStroke)
	{
		m_pStroke = pStroke;

		if(m_pStrokePaint)
			m_i32PaintMode |= VG_STROKE_PATH;
	}
}

/*!***************************************************************************
 @Function		Draw
 @Input			i32InheritedPaint The inherited paint mode
 @Returns		True on success
 @Description	Draw the node
*****************************************************************************/
bool CPVRTSVGNode::Draw(int i32InheritedPaint)
{
	// Setup Transformation
	ApplyTransform();

	// Setup the stroke
	ApplyStrokes(m_i32PaintMode | i32InheritedPaint);

	// Setup Paints
	ApplyPaints(VG_FILL_PATH | VG_STROKE_PATH, m_pPath);

	DrawPath(i32InheritedPaint);

	return true;
}

/*!***************************************************************************
 @Function		ApplyStrokes
 @Input			i32Mode The paint mode
 @Description	Apply stroking if the paint mode requests it
*****************************************************************************/
void CPVRTSVGNode::ApplyStrokes(int i32Mode)
{
	if(i32Mode & VG_STROKE_PATH)
	{
		if(m_pStroke)
		{
			vgSetfv(VG_STROKE_DASH_PATTERN, m_pStroke->m_i32DashSize,  m_pStroke->m_pafDashArray);
			vgSetf( VG_STROKE_DASH_PHASE  , m_pStroke->m_fDashOffset);
			vgSetf( VG_STROKE_LINE_WIDTH  , m_pStroke->m_fWidth);
			vgSeti( VG_STROKE_CAP_STYLE   , m_pStroke->m_i32LineCap);
			vgSeti( VG_STROKE_JOIN_STYLE  , m_pStroke->m_i32LineJoin);
			vgSetf( VG_STROKE_MITER_LIMIT , m_pStroke->m_fMiterLimit);
		}
		else // Use defaults
		{
			vgSetfv(VG_STROKE_DASH_PATTERN, 0,  0);
			vgSetf( VG_STROKE_DASH_PHASE  , 0);
			vgSetf( VG_STROKE_LINE_WIDTH  , 1.0f);
			vgSeti( VG_STROKE_CAP_STYLE   , VG_CAP_BUTT);
			vgSeti( VG_STROKE_JOIN_STYLE  , VG_JOIN_MITER);
			vgSetf( VG_STROKE_MITER_LIMIT , 4.0f);
		}
	}
}

/*!***************************************************************************
 @Function		ApplyPaint
 @Input			pPaint Paint to apply
 @Input			pPath The path the paint is getting applied to
 @Input			i32Mode	The paint mode
 @Description	Applies a paint
*****************************************************************************/
void CPVRTSVGNode::ApplyPaint(CPVRTPaint *pPaint, CPVRTPath *pPath, VGPaintMode i32Mode)
{
	if(!pPaint)
		return;

	float af[5];

	if(pPaint->m_pPrevNode != this) // Has this paint been applied to something else?
	{
		float fMinX,fMinY,fWidth,fHeight;
		pPaint->m_pPrevNode = this;

		if(pPaint->GetType() == eAssetLinearPaint)
			vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, pPaint->m_afPoints);
		else
			vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, pPaint->m_afPoints);

		if(pPath && (pPaint->m_eGradientUnits != eUserSpaceOnUse || !pPaint->m_bPointSet[0]
				|| !pPaint->m_bPointSet[1] || !pPaint->m_bPointSet[2] || !pPaint->m_bPointSet[3] || !pPaint->m_bPointSet[4])
		)
		{
			vgPathBounds(pPath->m_vgPath,&fMinX,&fMinY,&fWidth,&fHeight);

			switch(pPaint->GetType())
			{
				case eAssetLinearPaint:
					vgGetParameterfv(pPaint->m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, af);

					if(!pPaint->m_bPointSet[2])
						af[2] = pPaint->m_afPoints[2] * fWidth;

					if(pPaint->m_eGradientUnits != eUserSpaceOnUse)
					{
						af[0] = (1.0f - pPaint->m_afPoints[0]) * fWidth  + fMinX;
						af[1] = (1.0f - pPaint->m_afPoints[1]) * fHeight + fMinY;
						af[2] = (1.0f - pPaint->m_afPoints[2]) * fWidth  + fMinX;
						af[3] = (1.0f - pPaint->m_afPoints[3]) * fHeight + fMinY;
					}

					vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, af);
				break;
				case eAssetRadialPaint:
						vgGetParameterfv(pPaint->m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 5, af);

					if(!pPaint->m_bPointSet[0])
						af[0] = pPaint->m_afPoints[0] * fWidth;

					if(!pPaint->m_bPointSet[1])
						af[1] = pPaint->m_afPoints[1] * fHeight;

					if(!pPaint->m_bPointSet[2])
						af[2] = pPaint->m_afPoints[0];

					if(!pPaint->m_bPointSet[3])
						af[3] = pPaint->m_afPoints[1];

					if(!pPaint->m_bPointSet[4])
						af[4] = pPaint->m_afPoints[4] * fWidth;

					if(pPaint->m_eGradientUnits != eUserSpaceOnUse)
					{
						af[0] = (1.0f - pPaint->m_afPoints[0]) * fWidth  + fMinX;
						af[1] = (1.0f - pPaint->m_afPoints[1]) * fHeight + fMinY;
						af[2] = (1.0f - pPaint->m_afPoints[2]) * fWidth  + fMinX;
						af[3] = (1.0f - pPaint->m_afPoints[3]) * fHeight + fMinY;
						af[4] = pPaint->m_afPoints[4];
					}

					vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, af);
				break;
				default:
				break;
			}
		}

		pPaint->ApplyTransform();

	}

	vgSetPaint(pPaint->m_vgPaint, i32Mode);
}

/*!***************************************************************************
 @Function		ApplyPaints
 @Input			i32Mode The paint mode
 @Input			pPath The path to apply the paints to
 @Description	Applies the paints
*****************************************************************************/
void CPVRTSVGNode::ApplyPaints(int i32Mode, CPVRTPath *pPath)
{
	if(i32Mode & VG_FILL_PATH)
	{
		if(m_bFillNone)
			vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
		else if(m_pFillPaint)
		{
			ApplyPaint(m_pFillPaint, pPath, VG_FILL_PATH);
			vgSeti(VG_FILL_RULE, m_FillRule);
		}
	}

	if(i32Mode & VG_STROKE_PATH)
	{
		if(m_bStrokeNone)
			vgSetPaint(VG_INVALID_HANDLE, VG_STROKE_PATH);
		else if(m_pStrokePaint)
			ApplyPaint(m_pStrokePaint, pPath, VG_STROKE_PATH);
	}
}

/*!***************************************************************************
 @Function		DrawPath
 @Input			i32InheritedPaint The inherited paint mode
 @Description	Draw a path
*****************************************************************************/
void CPVRTSVGNode::DrawPath(int i32InheritedPaint)
{
	int iValue = i32InheritedPaint | m_i32PaintMode;

	// If we are referencing another Node then use that one
	if(m_pDefinedNode)
	{
		m_pDefinedNode->ApplyTransform();

		if(!m_pFillPaint)
			m_pDefinedNode->ApplyPaints(VG_FILL_PATH, m_pDefinedNode->GetPath());

		if(!m_pStrokePaint)
			m_pDefinedNode->ApplyPaints(VG_STROKE_PATH, m_pDefinedNode->GetPath());

		if(!m_pStroke)
			m_pDefinedNode->ApplyStrokes(iValue);

		m_pDefinedNode->DrawPath(m_i32PaintMode);
	}
	else if(m_pPath && m_i32PaintMode)
		vgDrawPath(m_pPath->m_vgPath, m_i32PaintMode);
}

/*!***************************************************************************
 @Function		AddChild
 @Input			pChild Add a child to this node
 @Returns		True on success
 @Description	Adds a child node to this node
*****************************************************************************/
bool CPVRTSVGNode::AddChild(CPVRTSVGNode *pChild)
{
	if(pChild)
	{
		if(m_pFirstChild)
			m_pLastChild->SetNext(pChild);
		else
			m_pFirstChild = pChild;

		m_pLastChild = pChild;
		return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		GetFirstChild
 @Returns		The first child node
 @Description	Returns the first child node
*****************************************************************************/
CPVRTSVGNode* CPVRTSVGNode::GetFirstChild()
{
	return m_pFirstChild;
}


/*!***************************************************************************
 @Function		CPVRTPath
 @Description	Constructor
*****************************************************************************/
CPVRTPath::CPVRTPath()
{
}

/*!***************************************************************************
 @Function		~CPVRTPath
 @Description	Destructor
*****************************************************************************/
CPVRTPath::~CPVRTPath()
{
	Destroy();
}

/*!***************************************************************************
 @Function		Destroy
 @Description	Frees the class variables
*****************************************************************************/
void CPVRTPath::Destroy()
{
	vgDestroyPath(m_vgPath);

	CPVRTSVGBase::Destroy();
}

/*!***************************************************************************
 @Function 		CPVRTPaint
 @Description 	Constructor
*****************************************************************************/
CPVRTPaint::CPVRTPaint(void)
{
	m_pHref = 0;

	Destroy();
}

/*!***************************************************************************
 @Function 		~CPVRTPaint
 @Description 	Destructor
*****************************************************************************/
CPVRTPaint::~CPVRTPaint(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function		Destroy
 @Description	Frees the class variables
*****************************************************************************/
void CPVRTPaint::Destroy()
{
	m_eGradientUnits = eUserSpaceOnUse;

	vgDestroyPaint(m_vgPaint);
	m_vgPaint = 0;

	SetType(eAssetFillPaint);

	for(unsigned int i = 0; i < 5; ++i)
	{
		m_afPoints[i] = 0.0f;
		m_bPointSet[i] = false;
	}

	delete[] m_pHref;
	m_pHref = 0;

	m_bInit = true;
	m_pPrevNode = 0;

	m_GradTransform = PVRTMat3::Identity();

	CPVRTSVGBase::Destroy();
}

/*!***************************************************************************
 @Function		SetHref
 @Input			pHref	Set the href for this paint
 @Returns		True if successful
 @Description	Sets the paints href
*****************************************************************************/
bool CPVRTPaint::SetHref(const char * const pHref)
{
	if(Set(&m_pHref, pHref, 0))
	{
		m_bInit = false;
		return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		=
 @Input			Paint A paint
 @Returns 		this
 @Description	= operator
*****************************************************************************/
CPVRTPaint & CPVRTPaint::operator=(const CPVRTPaint &Paint)
{
	vgDestroyPaint(m_vgPaint);
	m_eGradientUnits = Paint.m_eGradientUnits;

	for(unsigned int i = 0; i < 5; ++i)
	{
		m_afPoints[i] = Paint.m_afPoints[i];
		m_bPointSet[i] = Paint.m_bPointSet[i];
	}

	m_GradTransform = Paint.m_GradTransform;

	m_eType = Paint.m_eType;

	if(Paint.m_vgPaint) // Copy the paint
	{
		float fValues[100];
		int i32Value = 0;
		bool bGradient = false;

		m_vgPaint = vgCreatePaint();

		switch(vgGetParameteri(Paint.m_vgPaint, VG_PAINT_TYPE))
		{
			case VG_PAINT_TYPE_COLOR:
				vgSetParameteri(m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);

				vgGetParameterfv(Paint.m_vgPaint, VG_PAINT_COLOR, 4, fValues);
				vgSetParameterfv(m_vgPaint, VG_PAINT_COLOR, 4, fValues);
				break;
			case VG_PAINT_TYPE_LINEAR_GRADIENT:
				vgSetParameteri(m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);

				vgGetParameterfv(Paint.m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, fValues);
				vgSetParameterfv(m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, fValues);
				bGradient = true;
				break;
			case VG_PAINT_TYPE_RADIAL_GRADIENT:
				vgSetParameteri(m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);

				vgGetParameterfv(Paint.m_vgPaint, VG_PAINT_TYPE_RADIAL_GRADIENT, 5, fValues);
				vgSetParameterfv(m_vgPaint, VG_PAINT_TYPE_RADIAL_GRADIENT, 5, fValues);
				bGradient = true;
				break;
		}

		// Do common gradient parameters
		if(bGradient)
		{
			i32Value = vgGetParameterVectorSize(Paint.m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS);

			if(i32Value < 100)
			{
				vgGetParameterfv(Paint.m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);
				vgSetParameterfv(m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);
			}

			i32Value = vgGetParameteri(Paint.m_vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE);
			vgSetParameteri(m_vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, i32Value);
		}
	}

	return *this;
}

/*!***************************************************************************
 @Function	SetTransform
 @Input		pTransform A 3x3 matrix
 @Description	Sets the paints transformation
*****************************************************************************/
void CPVRTPaint::SetTransform(PVRTMat3 *pTransform)
{
	m_GradTransform = *pTransform;
}

/*!***************************************************************************
 @Function		ApplyTransform
 @Description	Apply the transformation to the paint
*****************************************************************************/
void CPVRTPaint::ApplyTransform()
{
	int i32Type = vgGetParameteri(m_vgPaint, VG_PAINT_TYPE);

	if(i32Type != VG_PAINT_TYPE_LINEAR_GRADIENT && i32Type != VG_PAINT_TYPE_RADIAL_GRADIENT)
		return;

	float af[5];
	float aCurrent[5];

	switch(i32Type)
	{
		case VG_PAINT_TYPE_LINEAR_GRADIENT:
			vgGetParameterfv(m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, aCurrent);
		break;
		case VG_PAINT_TYPE_RADIAL_GRADIENT:
			vgGetParameterfv(m_vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, aCurrent);
		break;
		default:
			break;
	}

	af[0] = aCurrent[0] * m_GradTransform.f[0] + aCurrent[1] * m_GradTransform.f[3] + m_GradTransform.f[6];
	af[1] = aCurrent[0] * m_GradTransform.f[1] + aCurrent[1] * m_GradTransform.f[4] + m_GradTransform.f[7];

	af[2] = aCurrent[2] * m_GradTransform.f[0] + aCurrent[3] * m_GradTransform.f[3] + m_GradTransform.f[6];
	af[3] = aCurrent[2] * m_GradTransform.f[1] + aCurrent[3] * m_GradTransform.f[4] + m_GradTransform.f[7];

	switch(i32Type)
	{
		case VG_PAINT_TYPE_LINEAR_GRADIENT:
			vgSetParameterfv(m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, af);
		break;
		case VG_PAINT_TYPE_RADIAL_GRADIENT:
			af[4] = aCurrent[4] * m_GradTransform.f[1] +  aCurrent[4] * m_GradTransform.f[4] + m_GradTransform.f[6];

			if(af[4] < 0)
				af[4] *= -1.0f;

			vgSetParameterfv(m_vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, af);
		break;
		default:
			break;
	}
}

/*!***************************************************************************
 @Function		CPVRTStroke
 @Description	Constructor
*****************************************************************************/
CPVRTStroke::CPVRTStroke()
{
	m_pafDashArray = 0;
	Destroy();
}

/*!***************************************************************************
 @Function		~CPVRTStroke
 @Description	Destructor
*****************************************************************************/
CPVRTStroke::~CPVRTStroke()
{
	Destroy();
}

/*!***************************************************************************
 @Function		Destroy
 @Description	Frees the class variables
*****************************************************************************/
void CPVRTStroke::Destroy()
{
	delete[] m_pafDashArray;
	m_pafDashArray = 0;

	m_fWidth = 1.0f;
	m_fDashOffset = 0.0f;
	m_i32DashSize = 0;
	m_i32LineCap  = (int) VG_CAP_BUTT;
	m_i32LineJoin = (int) VG_JOIN_MITER;
	m_fMiterLimit = 4.0f;
	m_fOpacity = 1.0f;

	CPVRTSVGBase::Destroy();
}

/*!***************************************************************************
 @Function		=
 @Input			Stroke A stroke
 @Returns 		this
 @Description	= operator
*****************************************************************************/
CPVRTStroke &CPVRTStroke::operator=(const CPVRTStroke &Stroke)
{
	delete[] m_pafDashArray;
	m_pafDashArray = 0;

	m_fDashOffset = Stroke.m_fDashOffset;
	m_fWidth = Stroke.m_fWidth;
	m_fDashOffset = Stroke.m_fDashOffset;
	m_i32DashSize = Stroke.m_i32DashSize;
	m_i32LineCap  = Stroke.m_i32LineCap;
	m_i32LineJoin = Stroke.m_i32LineJoin;
	m_fMiterLimit = Stroke.m_fMiterLimit;
	m_fOpacity    = Stroke.m_fOpacity;

	if(Stroke.m_pafDashArray)
	{
		int i32Size = sizeof(Stroke.m_pafDashArray);
		m_pafDashArray = (float*) new unsigned char[i32Size];

		if(m_pafDashArray)
			memcpy(m_pafDashArray, Stroke.m_pafDashArray, i32Size);
	}


	return *this;
}

/////////
// CPVRTSVGObject
////////
/*!***************************************************************************
@Function		CPVRTSVGObject
@Description	Constructor
*****************************************************************************/
CPVRTSVGObject::CPVRTSVGObject(void)
{
	m_pRoot = 0;

	m_pFirstAsset = 0;
	m_pLastAsset = 0;

	m_ui32Width = 240;
	m_ui32Height = 320;

	m_ViewBoxTransform = PVRTMat3::Identity();
	m_UserTransform    = PVRTMat3::Identity();
	m_InitialTransform = PVRTMat3::Identity();

	m_fObjectWidth = 0.0f;
	m_fObjectHeight= 0.0f;

	m_fObjectX = 0.0f;
	m_fObjectY = 0.0f;
}

/*!***************************************************************************
@Function		~CPVRTSVGObject
@Description	Destructor
*****************************************************************************/
CPVRTSVGObject::~CPVRTSVGObject(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function SetViewBox
 @Input fX	ViewBox x origin
 @Input	fY	ViewBox y origin
 @Input	fWidth	ViewBox width
 @Input	fHeight	ViewBox height
 @Description	Set the view box parameters
*****************************************************************************/
void CPVRTSVGObject::SetViewBox(float fX, float fY, float fWidth, float fHeight)
{
	m_ViewBoxTransform = PVRTMat3::Identity();

	float fSx = 1.0f, fSy = 1.0f;

	if(fWidth != 0.0f)
		fSx = m_ui32Width / fWidth;

	if(fHeight != 0.0f)
		fSy = m_ui32Height / fHeight;

	float fS = fSx < fSy ? fSx : fSy;

	m_ViewBoxTransform = PVRTMat3::Scale(fS,fS,1.0f);

	m_fObjectWidth = fWidth * fS;
	m_fObjectHeight= fHeight* fS;

	m_fObjectX = fX;
	m_fObjectY = fY;
}

/*!***************************************************************************
 @Function		SetToOrigin
 @Description	Set the centre of the object to the origin
*****************************************************************************/
void CPVRTSVGObject::SetToOrigin()
{
	m_InitialTransform = PVRTMat3::Translation((m_fObjectWidth * -0.5f) + m_fObjectX, (m_fObjectHeight * 0.5f) - m_ui32Height + m_fObjectY);
}

/*!***************************************************************************
 @Function		SetupReferences
 @Description	Sets up the references
*****************************************************************************/
void CPVRTSVGObject::SetupReferences()
{
	CPVRTSVGNode *pNode = m_pRoot;

	while(pNode)
	{
		SetupReferences(pNode, true);
		pNode = (CPVRTSVGNode *) pNode->GetNext();
	}

	CPVRTSVGBase *pAsset = m_pFirstAsset;

	while(pAsset)
	{
		SetupAssetReferences(pAsset);
		pAsset = pAsset->GetNext();
	}
}

/*!***************************************************************************
 @Function		SetupAssetReferences
 @Input			pNode	The node to set the asset references to
 @Description	Setup the asset references (e.g. paints) for pNode
*****************************************************************************/
void CPVRTSVGObject::SetupAssetReferences(CPVRTSVGBase *pNode)
{
	if(pNode)
	{
		CPVRTPaint *pPaint;
		CPVRTPaint *pPaintRef;
		char *pHref = 0;

		float fValues[100];
		int i32Value = 0;

		switch(pNode->GetType())
		{
			case eAssetLinearPaint:
			case eAssetRadialPaint:
				pPaint = (CPVRTPaint*) pNode;

				if(!pPaint->IsInit())
				{
					pHref = pPaint->GetHref();

					if(pHref)
					{
						++pHref; // skip #

						pPaintRef = (CPVRTPaint*) FindAsset(pHref, (unsigned int) strlen(pHref));

 						if(pPaintRef)
						{
							SetupAssetReferences((CPVRTSVGBase*) pPaintRef); // Make sure the reference is setup fully

							// Inherit stops if we need to
							i32Value = vgGetParameterVectorSize(pPaint->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS);

							if(i32Value == 0) // We have no stops so we should inherit them
							{
								i32Value = vgGetParameterVectorSize(pPaintRef->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS);

								if(i32Value < 100)
								{
									vgGetParameterfv(pPaintRef->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);
									vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);
								}
							}

							pPaint->SetInit(true);
						}
					}else
						pPaint->SetInit(true);
				}
			break;
			default:
			break;
		}
	}
}

/*!***************************************************************************
 @Function		SetupPaint
 @Input			pNode Node to setup the paint for
 @Input			Mode The paint to setup
 @Description	Sets up a paint for a node
*****************************************************************************/
void CPVRTSVGObject::SetupPaint(CPVRTSVGNode *pNode, VGPaintMode Mode)
{
	unsigned int ui32Length;
	char *pID = pNode->GetPaintID(Mode);

	if(!pNode->GetPaint(Mode) && pID)
	{
		if(strstr(pID, "url("))
		{
			pID += 5; // Bypass the url(#
			ui32Length = (unsigned int) strlen(pID);

			CPVRTPaint* pPaint = (CPVRTPaint*) FindAsset(pID, ui32Length - 1);
			pNode->SetPaint(Mode, pPaint);

			if(pPaint)
				SetupAssetReferences(pPaint);
		}
	}
}

/*!***************************************************************************
 @Function		SetupReferences
 @Input			pNode	Setup the references for this particular node
 @Input			bDoChildren	Setup the references for this Node's children
 @Description	Sets up the references for a particular node
*****************************************************************************/
void CPVRTSVGObject::SetupReferences(CPVRTSVGNode *pNode, bool bDoChildren)
{
	if(pNode)
	{
		unsigned int ui32Length;
		char *pID;

		SetupPaint(pNode, VG_FILL_PATH);
		SetupPaint(pNode, VG_STROKE_PATH);

		pID = pNode->GetPathID();

		if(!pNode->GetPath() && pID)
		{
			if(*pID == '#')
			{
				++pID; // Skip the #
				ui32Length = (unsigned int) strlen(pID);

				CPVRTPath *pPath = (CPVRTPath*) FindAsset(pID, ui32Length);

				if(pPath)
					pNode->SetPath(pPath);
				else
					pNode->SetDef((CPVRTSVGNode *) FindNode(m_pRoot, pID, ui32Length));
			}
		}

		if(bDoChildren)
		{
			CPVRTSVGNode *pChild = pNode->GetFirstChild();

			while(pChild)
			{
				SetupReferences(pChild, true);
				pChild = (CPVRTSVGNode*) pChild->GetNext();
			}
		}
	}
}

/*!***************************************************************************
 @Function		AddAsset
 @Input			pAsset The asset to add
 @Returns		True if the asset was successfully added
 @Description	Adds an asset to the list
*****************************************************************************/
bool CPVRTSVGObject::AddAsset(CPVRTSVGBase* pAsset)
{
	if(pAsset)
	{
		if(m_pFirstAsset)
			m_pLastAsset->SetNext(pAsset);
		else
			m_pFirstAsset = pAsset;

		m_pLastAsset = pAsset;
		return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		FindNode
 @Input			pRoot	The root of the list to search
 @Input			pID		The ID to search for
 @Input			ui32Size The size of the ID
 @Returns		false if unsuccessful or a pointer to the node
 @Description	Finds a node in a particular list
*****************************************************************************/
CPVRTSVGNode* CPVRTSVGObject::FindNode(CPVRTSVGNode* pRoot, const char * pID, unsigned int ui32Size)
{
	if(pID && pRoot)
	{
		if(pRoot->GetID() && strncmp(pRoot->GetID(), pID, ui32Size) == 0)
			return pRoot;

		CPVRTSVGNode *pNode = FindNode((CPVRTSVGNode *) pRoot->GetNext(), pID, ui32Size);

		if(pNode)
			return pNode;

		CPVRTSVGNode *pChild = pRoot->GetFirstChild();

		while(pChild)
		{
			pNode = FindNode(pChild, pID, ui32Size);

			if(pNode)
				return pNode;

			pChild = (CPVRTSVGNode *) pChild->GetNext();
		}
	}

	return false;
}

/*!***************************************************************************
 @Function		FindAsset
 @Input			pID		Asset to find
 @Input			ui32Size Length of ID
 @Returns		false if unsuccessful or a point to the asset
 @Description	Finds an asset
*****************************************************************************/
CPVRTSVGBase* CPVRTSVGObject::FindAsset(const char * pID, unsigned int ui32Size)
{
	if(pID)
	{
		CPVRTSVGBase *pAsset = m_pFirstAsset;

		while(pAsset)
		{
			if(pAsset->GetID() && strncmp(pAsset->GetID(), pID, ui32Size) == 0)
				return pAsset;

			pAsset = pAsset->GetNext();
		}
	}

	return false;
}

/*!***************************************************************************
 @Function		SetRoot
 @Input			pRoot The root node
 @Returns		true if successful
 @Description	Sets the root node of the SVG Object
*****************************************************************************/
bool CPVRTSVGObject::SetRoot(CPVRTSVGNode* pRoot)
{
	if(pRoot)
	{
		if(m_pRoot)
			delete m_pRoot;

		m_pRoot = pRoot;
		return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		GetRoot
 @Returns		The root node
 @Description	Returns the root node
*****************************************************************************/
CPVRTSVGNode* CPVRTSVGObject::GetRoot()
{
	return m_pRoot;
}

/*!***************************************************************************
 @Function	Destroy
 @Description Frees all the memory of the SVG object
*****************************************************************************/
void CPVRTSVGObject::Destroy()
{
	CPVRTSVGBase *pBase = m_pRoot;
	CPVRTSVGBase *pNext;

	while(pBase)
	{
		pNext = pBase->GetNext();
		delete pBase;

		pBase = pNext;
	}

	pBase = m_pFirstAsset;

	while(pBase)
	{
		pNext = pBase->GetNext();
		delete pBase;

		pBase = pNext;
	}

	m_pRoot = 0;
	m_pFirstAsset = 0;
	m_pLastAsset  = 0;
}

/*!***************************************************************************
 @Function		SetTransformation
 @Input			pMatrix	The transformation
 @Description	Sets the transformation for the whole scene
*****************************************************************************/
void CPVRTSVGObject::SetTransformation(PVRTMat3 *pMatrix)
{
	m_UserTransform = *pMatrix;
}

/*!***************************************************************************
 @Function		Draw
 @Returns		True on success
 @Description	Draws the SVG Object
*****************************************************************************/
bool CPVRTSVGObject::Draw()
{
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

	vgLoadMatrix(m_InitialTransform.f);
	vgMultMatrix(m_UserTransform.f);

	vgTranslate(0.0f, (float) m_ui32Height);
	vgScale(1.0f, -1.0f);

	vgMultMatrix(m_ViewBoxTransform.f);

	Draw(m_pRoot);
	return true;
}

/*!***************************************************************************
 @Function		Draw
 @Input			pNode The node to draw
 @Returns		True on success
 @Description	Draws a particular SVG Node
*****************************************************************************/
bool CPVRTSVGObject::Draw(CPVRTSVGNode *pNode)
{
	if(pNode)
	{
		PVRTMat3 Matrix;

		vgGetMatrix(&Matrix.f[0]);

		if(pNode->GetRender())
			pNode->Draw();
		else
			pNode->ApplyTransform();

		if(pNode->GetRenderChildren())
		{
			CPVRTSVGNode *pChild = pNode->GetFirstChild();

			while(pChild)
			{
				Draw(pChild);
				pChild = (CPVRTSVGNode *) pChild->GetNext();
			}
		}

		vgLoadMatrix(Matrix.f);
	}

	return true;
}

/*!***************************************************************************
 @Function		SetScreenDimensions
 @Input			ui32Width	Width of the screen
 @Input			ui32Height	Height of the screen
 @Description	Sets the screen width and height for the SVG object
*****************************************************************************/
void CPVRTSVGObject::SetScreenDimensions(unsigned int ui32Width, unsigned int ui32Height)
{
	m_ui32Width = ui32Width;
	m_ui32Height = ui32Height;
}

/*****************************************************************************
 End of file (PVRTSVGObject.cpp)
*****************************************************************************/
