/******************************************************************************

 @File         PVRTSVGParser.cpp

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  A simple XML parser for parsing XML files

******************************************************************************/
#include "PVRTSVGParser.h"
#include "../PVRTResourceFile.h"
#include "../PVRTError.h"

/*!****************************************************************************
 @Class       CPVRTSVGAttribute
 @Brief       Stores the xml attribute's value and name
******************************************************************************/
/*!***************************************************************************
 @Function		CPVRTSVGAttribute
 @Description	Constructor
*****************************************************************************/
CPVRTSVGAttribute::CPVRTSVGAttribute(void)
{
	m_pNext = 0;
	m_pszName = 0;
	m_pszValueString = 0;
	m_pValueFloatArray = 0;
	m_pValueStringArray = 0;
	m_i32Count = 0;
}

/*!***************************************************************************
 @Function		~CPVRTSVGAttribute
 @Description	Destructor
*****************************************************************************/
CPVRTSVGAttribute::~CPVRTSVGAttribute(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function			Destroy
 @Description		Tidys up
*****************************************************************************/
void CPVRTSVGAttribute::Destroy()
{
	m_pNext = 0;

	delete[] m_pszName;
	m_pszName = 0;

	DestroyValues();
}

/*!***************************************************************************
 @Function			DestroyValues
 @Description		Destroys the arrays containing all the values
*****************************************************************************/
void CPVRTSVGAttribute::DestroyValues()
{
	delete[] m_pszValueString;
	m_pszValueString = 0;

	if(m_pValueStringArray)
	{
		for(int i = 0; i < m_i32Count; ++i)
		{
			delete[] m_pValueStringArray[i];
		}

		delete[] m_pValueStringArray;
		m_pValueStringArray = 0;
	}

	delete[] m_pValueFloatArray;

	m_pValueFloatArray = 0;
	m_i32Count = 0;
}

/*!***************************************************************************
 @Function			SetName
 @Returns			returns true if successful
 @Input				pStart
 @Input				pEnd
 @Description		This will set the attributes name to the string between
					pStart and pEnd. If pEnd is NULL then it is assumed that
					pStart is NULL terminated.
*****************************************************************************/
bool CPVRTSVGAttribute::SetName(const char *pStart, const char *pEnd)
{
	return Set(&m_pszName, pStart, pEnd);
}

/*!***************************************************************************
 @Function			SetValue
 @Returns			returns true if successful
 @Input				pStart
 @Input				pEnd
 @Description		This will set the attributes vale to the string between
					pStart and pEnd. If pEnd is NULL then it is assumed that
					pStart is NULL terminated.
*****************************************************************************/
bool CPVRTSVGAttribute::SetValue(const char *pStart, const char *pEnd)
{
	return Set(&m_pszValueString, pStart, pEnd);
}

/*!***************************************************************************
 @Function		Set
 @Modified		pParam The output string
 @Input			pStart The start of a string
 @Input			pEnd	The end of a string. If 0 then it is assumed the string is null terminated.
 @Returns 		True on success
 @Description	Sets pParam to the string defined between pStart and pEnd
*****************************************************************************/
bool CPVRTSVGAttribute::Set(char** pParam, const char * const pStart, const char * const pEnd)
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

		if(*pParam)
		{
			memcpy(*pParam, pStart, ui32Length);
			(*pParam)[ui32Length] = '\0';
		}else
			return false;
	}

	return true;
}

/*!***************************************************************************
 @Function			GetNext
 @Returns			returns m_pNext
 @Description		returns the next attribute in the linked list
*****************************************************************************/
CPVRTSVGAttribute* CPVRTSVGAttribute::GetNext()
{
	return m_pNext;
}

/*!***************************************************************************
 @Function			GetName
 @Returns			returns the name of the attribute
 @Description		returns the name of the attribute
*****************************************************************************/
char* CPVRTSVGAttribute::GetName()
{
	return m_pszName;
}

/*!***************************************************************************
 @Function			GetValue
 @Returns			returns the value of the attribute
 @Description		returns the value of the attribute
*****************************************************************************/
char* CPVRTSVGAttribute::GetValue()
{
	return m_pszValueString;
}

/*!***************************************************************************
 @Function			GetValueAsStringArray
 @Returns			Returns the value as an array of strings
 @Modified			i32Count Returns the number of values in the array
 @Description		Returns the value as an array of strings. The array is
					generated by splitting the value up by spaces and commas.
*****************************************************************************/
char** CPVRTSVGAttribute::GetValueAsStringArray(int &i32Count)
{
	if(m_pValueStringArray || !m_pszValueString || m_pszValueString[0] == '\0')
	{
		i32Count = m_i32Count;
		return m_pValueStringArray;
	}

	char* pP = m_pszValueString;
	int i32IsSpacePrev = 0;
	int i32IsSpace     = 0;

	while(*pP != '\0')
	{
		i32IsSpace = isspace((unsigned char) *pP) || *pP == ',';

		if(i32IsSpace && !i32IsSpacePrev)
			++m_i32Count;

		i32IsSpacePrev = i32IsSpace;
		++pP;
	}

	if(!i32IsSpacePrev)
		++m_i32Count;

	_ASSERT(m_i32Count);

	m_pValueStringArray = new char*[m_i32Count];

	if(!m_pValueStringArray)
	{
		m_i32Count = 0;
		return 0;
	}

	pP = m_pszValueString;

	// Remove any white space and commas from the front
	while(isspace((unsigned char) *pP) || *pP == ',')
		++pP;

	char *pNo;
	char pOrigP;
	unsigned int ui32Length;
	i32Count = 0;

	while(*pP != '\0' && i32Count < m_i32Count)
	{
		// Skip any white space and commas
		while(isspace((unsigned char) *pP) || *pP == ',')
			++pP;

		pNo = pP;

		// Now get to the end of the float
		while(!isspace((unsigned char) *pP) && *pP != '\0' && *pP != ',')
			++pP;

		pOrigP = *pP;
		*pP = '\0';

		ui32Length = (unsigned int) strlen(pNo);

		m_pValueStringArray[i32Count] = new char[ui32Length + 1];

		memcpy(m_pValueStringArray[i32Count], pNo, ui32Length);

		m_pValueStringArray[i32Count][ui32Length] = '\0';

		*pP = pOrigP;

		++i32Count;

		if(pP != '\0')
			++pP;
	}

	return m_pValueStringArray;
}

/*!***************************************************************************
 @Function		GetValueAsFloat
 @Returns		The attributes value as a float
 @Description	Returns the attributes value as a float
*****************************************************************************/
float CPVRTSVGAttribute::GetValueAsFloat()
{
	if(!m_pszValueString || m_pszValueString[0] == '\0')
		return 0.0f;

	return (float) atof(m_pszValueString);
}

/*!***************************************************************************
 @Function			GetValueAsFloatArray
 @Returns			Returns the value as an array of floats
 @Modified			i32Count Returns the number of values in the array
 @Description		Returns the value as an array of floats. The array is
					generated by splitting the value up by spaces and commas.
*****************************************************************************/
float* CPVRTSVGAttribute::GetValueAsFloatArray(int &i32Count)
{
	if(m_pValueFloatArray || !m_pszValueString || m_pszValueString[0] == '\0')
	{
		i32Count = m_i32Count;
		return m_pValueFloatArray;
	}

	char* pP = m_pszValueString;
	int i32IsSpacePrev = 0;
	int i32IsSpace     = 0;

	while(*pP != '\0')
	{
		i32IsSpace = isspace((unsigned char) *pP) || *pP == ',';

		if(i32IsSpace && !i32IsSpacePrev)
			++m_i32Count;

		i32IsSpacePrev = i32IsSpace;
		++pP;
	}

	if(!i32IsSpacePrev)
		++m_i32Count;

	_ASSERT(m_i32Count);

	m_pValueFloatArray = new float[m_i32Count];

	if(!m_pValueFloatArray)
	{
		m_i32Count = 0;
		return 0;
	}

	pP = m_pszValueString;

	// Remove any white space from the front
	while(isspace((unsigned char) *pP))
		++pP;

	char *pNo;
	char pOrigP;
	i32Count = 0;

	while(*pP != '\0' && i32Count < m_i32Count)
	{
		// Skip any white space and commas
		while(isspace((unsigned char) *pP) || *pP == ',')
			++pP;

		pNo = pP;

		// Now get to the end of the float
		while(!isspace((unsigned char) *pP) && *pP != '\0' && *pP != ',')
			++pP;

		pOrigP = *pP;
		*pP = '\0';

		m_pValueFloatArray[i32Count] = (float) atof(pNo);

		*pP = pOrigP;

		++i32Count;

		if(pP != '\0')
			++pP;
	}

	return m_pValueFloatArray;
}

/*!***************************************************************************
 @Function			SetNext
 @Input				pAttribute An attribute
 @Description		Sets m_pNext to pAttribute
*****************************************************************************/
void CPVRTSVGAttribute::SetNext(CPVRTSVGAttribute* pAttribute)
{
	m_pNext = pAttribute;
}

/*!****************************************************************************
 @Class       CPVRTSVGElement
 @Brief       Represents the XML Elements
******************************************************************************/
/*!***************************************************************************
 @Function		CPVRTSVGElement
 @Description	Constructor
*****************************************************************************/
CPVRTSVGElement::CPVRTSVGElement(void)
{
	m_pNext = 0;

	m_pszName = 0;
	m_pszData = 0;
	m_pszNameSpace = 0;

	m_pFirstChild = 0;
	m_pLastChild = 0;

	m_pFirstAttribute = 0;
	m_pLastAttribute = 0;

	m_bIsClosed = false;
	m_ui32NoOfChildren = 0;
	m_bWantsChildren = false;
}

/*!***************************************************************************
 @Function		~CPVRTSVGElement
 @Description	Destructor
*****************************************************************************/
CPVRTSVGElement::~CPVRTSVGElement(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function		Destroy
 @Description	Tidys up
*****************************************************************************/
void CPVRTSVGElement::Destroy()
{
	// Destroy all the children
	CPVRTSVGElement *pElement;
	CPVRTSVGAttribute *pAttribute;

	pElement = m_pFirstChild;

	while(m_pFirstChild)
	{
		m_pFirstChild = m_pFirstChild->GetNext();

		pElement->Destroy();
		delete pElement;

		pElement = m_pFirstChild;
	}

	m_pFirstChild = 0;
	m_pLastChild  = 0;

	// Destroy all the attributes
	pAttribute = m_pFirstAttribute;

	while(m_pFirstAttribute)
	{
		m_pFirstAttribute = m_pFirstAttribute->GetNext();

		pAttribute->Destroy();
		delete pAttribute;

		pAttribute = m_pFirstAttribute;
	}

	m_pFirstAttribute = 0;
	m_pLastAttribute  = 0;

	DELETE_ARRAY(m_pszName);
	DELETE_ARRAY(m_pszData);
	DELETE_ARRAY(m_pszNameSpace);

	m_bIsClosed = false;
	m_ui32NoOfChildren = 0;
}

/*!***************************************************************************
 @Function			SetName
 @Returns			returns true if successful
 @Input				pStart
 @Input				pEnd
 @Description		This will set the elements name to the string between
					pStart and pEnd. If pEnd is NULL then it is assumed that
					pStart is NULL terminated.
*****************************************************************************/
bool CPVRTSVGElement::SetName(const char *pStart, const char *pEnd)
{
	return Set(&m_pszName, pStart, pEnd);
}

/*!***************************************************************************
 @Function			SetNameSpace
 @Returns			returns true if successful
 @Input				pStart
 @Input				pEnd
 @Description		This will set the elements namespace to the string between
					pStart and pEnd. If pEnd is NULL then it is assumed that
					pStart is NULL terminated.
*****************************************************************************/
bool CPVRTSVGElement::SetNameSpace(const char * const pStart, const char * const pEnd)
{
	return Set(&m_pszNameSpace, pStart, pEnd);
}

/*!***************************************************************************
 @Function			GetNameSpace
 @Returns			returns the elements namespace
 @Description		returns the elements namespace
*****************************************************************************/
char* CPVRTSVGElement::GetNameSpace()
{
	return m_pszNameSpace;
}

/*!***************************************************************************
 @Function		Set
 @Modified		pParam The output string
 @Input			pStart The start of a string
 @Input			pEnd	The end of a string. If 0 then it is assumed the string is null terminated.
 @Returns 		True on success
 @Description	Sets pParam to the string defined between pStart and pEnd
*****************************************************************************/
bool CPVRTSVGElement::Set(char** pParam, const char * const pStart, const char * const pEnd)
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

/*!***************************************************************************
 @Function			GetName
 @Returns			Returns the elements name
 @Description		Returns the elements name
*****************************************************************************/
char* CPVRTSVGElement::GetName()
{
	return m_pszName;
}

/*!***************************************************************************
 @Function			SetData
 @Returns			returns true if successful
 @Input				pStart
 @Input				pEnd
 @Description		This will set the elements data to the string between
					pStart and pEnd. If pEnd is NULL then it is assumed that
					pStart is NULL terminated.
*****************************************************************************/
bool CPVRTSVGElement::SetData(const char *pStart, const char *pEnd)
{
	return Set(&m_pszData, pStart, pEnd);
}

/*!***************************************************************************
 @Function			GetData
 @Returns			returns the elements data
 @Description		returns the elements data
*****************************************************************************/
char* CPVRTSVGElement::GetData()
{
	return m_pszData;
}

/*!***************************************************************************
 @Function			AddChild
 @Returns			returns true if successful
 @Input				pElement
 @Description		Adds pElement as a child to the element
*****************************************************************************/
bool CPVRTSVGElement::AddChild(CPVRTSVGElement *pElement)
{
	if(!pElement)
		return false;

	if(!m_pFirstChild)
		m_pFirstChild = pElement;
	else
		m_pLastChild->SetNext(pElement);

	++m_ui32NoOfChildren;
	m_pLastChild = pElement;

	return true;
}

/*!***************************************************************************
 @Function			RemoveChild
 @Returns			returns true if successful
 @Input				pElement
 @Description		Removes pElement from the current element
*****************************************************************************/
bool CPVRTSVGElement::RemoveChild(CPVRTSVGElement *pElement)
{
	// If the child is of this element then remove it
	CPVRTSVGElement *pTmp = m_pFirstChild;
	CPVRTSVGElement *pPrev = 0;

	while(pTmp)
	{
		if(pTmp == pElement)
		{
			if(pPrev)
			{
				pPrev->SetNext(pElement->GetNext());

				if(pElement == m_pLastChild)
					m_pLastChild = pPrev;
			}
			else // Must be first element
				m_pFirstChild = m_pLastChild = 0;

			pElement->Destroy();
			delete pElement;
			--m_ui32NoOfChildren;

			return true;
		}

		pPrev = pTmp;
		pTmp = pTmp->GetNext();
	}

	return false;
}

/*!***************************************************************************
 @Function			AddAttribute
 @Returns			returns true if successful
 @Input				pAttribute
 @Description		Adds pAttribute to this element
*****************************************************************************/
bool CPVRTSVGElement::AddAttribute(CPVRTSVGAttribute *pAttribute)
{
	if(!pAttribute)
		return false;

	if(!m_pFirstAttribute)
		m_pFirstAttribute = m_pLastAttribute = pAttribute;
	else
	{
		m_pLastAttribute->SetNext(pAttribute);
		m_pLastAttribute = pAttribute;
	}

	return true;
}

/*!***************************************************************************
 @Function			GetNext
 @Returns			returns m_pNext
 @Description		returns the next element in the list
*****************************************************************************/
CPVRTSVGElement* CPVRTSVGElement::GetNext()
{
	return m_pNext;
}

/*!***************************************************************************
 @Function			GetFirstChild
 @Returns			returns the first child of this element
 @Description		returns the first child of this element
*****************************************************************************/
CPVRTSVGElement* CPVRTSVGElement::GetFirstChild()
{
	return m_pFirstChild;
}

/*!***************************************************************************
 @Function			SetNext
 @Input				pElement
 @Description		sets m_pNext to pElement
*****************************************************************************/
void CPVRTSVGElement::SetNext(CPVRTSVGElement* pElement)
{
	m_pNext = pElement;
}

/*!***************************************************************************
 @Function			GetFirstAttribute
 @Returns			Returns the first attribute
 @Description		Returns the first attribute
*****************************************************************************/
CPVRTSVGAttribute* CPVRTSVGElement::GetFirstAttribute()
{
	return m_pFirstAttribute;
}


/*!****************************************************************************
 @Class      CPVRTSVGParser
 @Brief      The main parser that goes through the file creating the XML tree
******************************************************************************/
/*!***************************************************************************
 @Function		CPVRTSVGParser
 @Description	Constructor
*****************************************************************************/
CPVRTSVGParser::CPVRTSVGParser(void)
{
	m_pObject = 0;
	m_pDefaultFillPaint = 0;
}

/*!***************************************************************************
 @Function		~CPVRTSVGParser
 @Description	Destructor
*****************************************************************************/
CPVRTSVGParser::~CPVRTSVGParser(void)
{
	Destroy();
}

/*!***************************************************************************
 @Function			Destroy
 @Description		Tidys up
*****************************************************************************/
void CPVRTSVGParser::Destroy()
{
	m_pObject = 0;
	m_pDefaultFillPaint = 0;
}

/*!***************************************************************************
 @Function			Load
 @Input				pFileName	The filename of the SVG file to load
 @Modified			pObject		The SVG Object to load the data into
 @Input				ui32Width	Screen width
 @Input				ui32Height	Screen height
 @Returns			Returns true if successful
 @Description		Opens up and parses the file pFileName.
*****************************************************************************/
bool CPVRTSVGParser::Load(const char * pFileName, CPVRTSVGObject *pObject, unsigned int ui32Width, unsigned int ui32Height)
{
	if(!pFileName || !pObject)
		return false;

	m_pObject = pObject;

	CPVRTResourceFile SVGFile(pFileName);

	if(!SVGFile.IsOpen())
	{
		PVRTErrorOutputDebug("CPVRTSVGParser::Load - Unable to open file\n");
		return 0;
	}

	m_pObject->SetScreenDimensions(ui32Width, ui32Height);

	bool bResult = CreateTree((char*) SVGFile.DataPtr());

	SVGFile.Close();

	m_pObject->SetupReferences();
	m_pObject = 0;
	return bResult;
}

/*!***************************************************************************
 @Function			SkipElement
 @Returns			Returns the remainder of the XML file if successful
 @Input				pXMLData - the entire XML data
 @Description		Skips an XML element in the file
*****************************************************************************/
char* CPVRTSVGParser::SkipElement(char* pXMLData)
{
	bool bComment = false;

	if(pXMLData[0] == '!' && pXMLData[1] == '-' && pXMLData[2] == '-')
		bComment = true;

	while(pXMLData && *pXMLData != '\0' && *pXMLData != '>')
	{
		if(!bComment && *pXMLData == '<') // An element within an element, therefore skip it
		{
			++pXMLData;
			pXMLData = SkipElement(pXMLData);
		}

		++pXMLData;
	}

	if(*pXMLData == '\0')
	{
		PVRTERROR_OUTPUT_DEBUG("CPVRTSVGParser::SkipElement - Unexpected end of file\n");
		return 0;
	}

	// Skip the '>'
	++pXMLData;
	return pXMLData;
}

/*!***************************************************************************
 @Function			Process
 @Input				pXMLData The XML data
 @Input				pParent  The parent to add any new elements to
 @Input				pParentNode The parent node to add any new nodes to
 @Returns			Returns the remainder of the XML file if successful
 @Description		Processes the data in pXMLData to determine what needs to
					be done.
*****************************************************************************/
char* CPVRTSVGParser::Process(char* pXMLData, CPVRTSVGElement *pParent, CPVRTSVGNode *pParentNode)
{
	if(!pXMLData)
		return 0;

	if(*pXMLData == '\0')
		return pXMLData;

	//Remove white space
	while(isspace((unsigned char) *pXMLData))
		++pXMLData;

	if(*pXMLData == '<')
	{
		//Skip '<' Tag
		++pXMLData;

		switch(*pXMLData)
		{
			case '?':
			case '!':
				if(!strncmp(pXMLData, "![CDATA[", 8))
					pXMLData = AddCDATA(pXMLData, pParent);
				else
					pXMLData = SkipElement(pXMLData);
				break;
			default:
				pXMLData = AddElement(pXMLData, pParent, pParentNode);
		}
	}
	else
	{
		if(pParent)
		{
			char *pStart = pXMLData - 1;

			while(*pXMLData != '<')
			{
				if(*pXMLData == '\0')
				{
					PVRTERROR_OUTPUT_DEBUG("PVRTSVGParser: Unexpected End Of File\n");
					return 0;
				}

				++pXMLData;
			}

			pParent->SetData(pStart, pXMLData);

			pXMLData = Process(pXMLData, pParent); // Process the end tag
		}
		else
		{
			PVRTERROR_OUTPUT_DEBUG("PVRTSVGParser: Parse error, orphaned data outside of root element\n");
			return 0;
		}
	}

	return pXMLData;
}

/*!***************************************************************************
 @Function			AddCDATA
 @Input				pXMLData - the entire XML data
 @Input				pParent  - the parent to add the data to
 @Returns			Returns the remainder of the XML file if successful
 @Description		Adds the contents of the CData block to pParent
*****************************************************************************/
char* CPVRTSVGParser::AddCDATA(char* pXMLData, CPVRTSVGElement *pParent)
{
	pXMLData += 8; // To skip ![CDATA[

	char* pStart = pXMLData;

	while(pXMLData && pXMLData != '\0')
	{
		if(pXMLData == '\0')
		{
			PVRTERROR_OUTPUT_DEBUG("CPVRTSVGParser::AddCDATA - Parse Error: End of file unexpected\n");
			return 0;
		}

		if(pXMLData[0] == ']' && pXMLData[1] == ']')
		{
			pXMLData += 2; // to skip ]]

			if(*pXMLData == '>') // We've found the end of the CData block
			{
				++pXMLData; // To skip '>'

				pParent->SetData(pStart, pXMLData - 3);
				return pXMLData;
			}
		}

		++pXMLData;
	}

	return pXMLData;
}

/*!***************************************************************************
 @Function			AddElement
 @Input				pXMLData the entire XML data
 @Input				pParent	 The element to add the new element to
 @Input				pParentNode The node to add the any new nodes to
 @Returns			Returns the remainder of the XML file if successful
 @Description		Adds an element as a child to an already existing element
*****************************************************************************/
char* CPVRTSVGParser::AddElement(char* pXMLData, CPVRTSVGElement *pParent, CPVRTSVGNode *pParentNode)
{
	if(*pXMLData == '/') // It's a close tag
	{
		while(*pXMLData != '\0' && *pXMLData != '>')
			++pXMLData;

		++pXMLData; // Skip the >
		return pXMLData;
	}

	CPVRTSVGElement *pElement = new CPVRTSVGElement();

	if(!pElement)
		return false;

	char* pStartOfElement = pXMLData;

	// get element name
	while( !isspace((unsigned char) *pXMLData) && *pXMLData != '>')
	{
		if(*pXMLData == ':') // We have a namespace
		{
			pElement->SetNameSpace(pStartOfElement, pXMLData);
			pStartOfElement = pXMLData + 1;
		}

		++pXMLData;
	}

	if(*pXMLData == '\0')
	{
		PVRTERROR_OUTPUT_DEBUG("CPVRTSVGParser::AddElement - Unexpected end of file\n");
		delete pElement;
		return 0;
	}

	pElement->SetName(pStartOfElement, pXMLData);

	if(pParent && pParent->WantsChildren())
 		pParent->AddChild(pElement);

	CPVRTSVGNode* pChildNode = 0;

	// Add Attributes
	pXMLData = AddAttributes(pElement, pXMLData);

	// Do the SVG to OpenVG conversion here
	if(IsDummy(pElement))
		pChildNode = CreateDummy(pElement, pParentNode);
	else if(IsShape(pElement))
		pChildNode = Create(pElement, pParentNode);
	else if(IsGradient(pElement))
	{
		pChildNode = pParentNode;
		CreateGradient(pElement, &pXMLData);
	}
	else
	{
		// Unknown element, skip it and all its children if we need to
		if(!pElement->IsClosed())
		{
			while(*pXMLData != '\0')
			{
				if(pXMLData[0] == '<' && pXMLData[1] == '/') // A close tag
				{
					pXMLData += 2; // skip </

					pStartOfElement = pXMLData;

					while(*pXMLData != '\0' && *pXMLData != '>')
					{
						if(*pXMLData == ':')
							pStartOfElement = pXMLData + 1;

						++pXMLData;
					}

					if(*pXMLData == '\0')
					{
						delete pElement;
						return 0;
					}

					if(strncmp(pElement->GetName(), pStartOfElement, pXMLData - pStartOfElement) == 0) // Is it the one we want
					{
						++pXMLData;
						delete pElement;
						return pXMLData;
					}
				}

				++pXMLData;
			}

			delete pElement;
			return 0;
		}
	}

	// Add the children if this element isn't closed
	if(!pElement->IsClosed())
		pXMLData = AddChildren(pElement, pChildNode, pXMLData);

	if(!pParent || !pParent->WantsChildren())
		delete pElement;

	return pXMLData;
}

/*!***************************************************************************
 @Function		AddChildren
 @Input			pParent The parent SVG element to add the children to
 @Input			pParentNode The parent SVG node to add the children to
 @Input			pXMLData the xml data
 @Returns		An updated pointer to the XMLData
 @Description	Partially parses the XMLData and adds the children to the parents
*****************************************************************************/
char* CPVRTSVGParser::AddChildren(CPVRTSVGElement *pParent, CPVRTSVGNode* pParentNode, char* pXMLData)
{
	if(pXMLData)
	{
		// Skip the '>'
		if(*pXMLData == '>')
			++pXMLData;

		// Remove any white space
		while(isspace((unsigned char) *pXMLData))
			++pXMLData;

		// A Close element so don't treat it as a child
		if(pXMLData[0] == '<' && pXMLData[1] == '/')
		{
			// Skip the close tag
			while(*pXMLData != '>')
				++pXMLData;

			if(*pXMLData == '>')
				++pXMLData;

			return pXMLData;
		}

		// Add children
		while(pXMLData && *pXMLData != '\0' && pXMLData[1] != '/')
		{
			pXMLData = Process(pXMLData, pParent, pParentNode);

			// Remove any white space
			while(pXMLData && *pXMLData != '\0' && isspace((unsigned char) *pXMLData))
				++pXMLData;
		}

		if(pXMLData && *pXMLData != '\0' && pXMLData[1] == '/')
		{
			// Skip the close tag
			while(*pXMLData != '>')
				++pXMLData;

			if(*pXMLData == '>')
				++pXMLData;
		}
	}

	return pXMLData;
}

/*!***************************************************************************
 @Function			AddAttribute
 @Input				pElement The element to add the attribute to
 @Input				pXMLData The XML data
 @Returns			Returns the remainder of the XML file if successful
 @Description		Adds an attribute to pElement
*****************************************************************************/
char* CPVRTSVGParser::AddAttributes(CPVRTSVGElement *pElement, char *pXMLData)
{
	while(pXMLData) // xml_data is set to 0 on parse error
	{
		while(isspace((unsigned char) *pXMLData))
			++pXMLData;

		if(*pXMLData == '/' && pXMLData[1] == '>')
		{
			// empty tag in form <tag/>
			// step to after tag and exit element

			pXMLData += 2;
			pElement->SetClosed(true);
			return pXMLData;
		}
		else
		{
			if(*pXMLData != '>')
			{
				pXMLData = AddAttribute(pElement, pXMLData);
			}
			else
			{
				// end of element open tag skip over closing '>' and continue
				++pXMLData;
				return pXMLData;
			}
		}
	}

	return pXMLData;
}

/*!***************************************************************************
 @Function		IsGradient
 @Input			pElement An SVG element
 @Returns		True if pElement represents a gradient
 @Description	Returns true if pElement represents a gradient
*****************************************************************************/
bool CPVRTSVGParser::IsGradient(CPVRTSVGElement *pElement)
{
	char * pName = pElement->GetName();

	if(pName)
	{
		if(_stricmp(pName, "lineargradient") == 0)
			return true;
		else if(_stricmp(pName, "radialgradient") == 0)
			return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		IsDummy
 @Input			pElement An SVG element
 @Returns		True if pElement represents a dummy element
 @Description	Returns true if pElement represents a dummy element
*****************************************************************************/
bool CPVRTSVGParser::IsDummy(CPVRTSVGElement *pElement)
{
	char * pName = pElement->GetName();

	if(pName)
	{
		if(_stricmp(pName, "svg") == 0)
			return true;
		else if(_stricmp(pName, "defs") == 0)
			return true;
		else if(_stricmp(pName, "switch") == 0) // The features of switch are not supported
			return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		IsShape
 @Input			pElement An SVG element
 @Returns		True if pElement represents a shape
 @Description	Returns true if pElement represents a shape
*****************************************************************************/
bool CPVRTSVGParser::IsShape(CPVRTSVGElement *pElement)
{
	char* pName = pElement->GetName();

	if(pName)
	{
		if(_stricmp(pName, "circle") == 0)
			return true;
		else if(_stricmp(pName, "ellipse") == 0)
			return true;
		else if(_stricmp(pName, "line") == 0)
			return true;
		else if(_stricmp(pName, "path") == 0)
			return true;
		else if(_stricmp(pName, "polygon") == 0)
			return true;
		else if(_stricmp(pName, "polyline") == 0)
			return true;
		else if(_stricmp(pName, "rect") == 0)
			return true;
		else if(_stricmp(pName, "use") == 0)
			return true;
		else if(_stricmp(pName, "g") == 0)
			return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		CreateGradient
 @Input			pElement An SVG element
 @Input			pXMLData The XML data
 @Returns		true on success
 @Description	Creates a gradient
*****************************************************************************/
bool CPVRTSVGParser::CreateGradient(CPVRTSVGElement *pElement, char **pXMLData)
{
	if(!pElement)
		return 0;

	char *pName = pElement->GetName();

	if(pName)
	{
		if(!*pXMLData)
			return 0;

		CPVRTPaint *pGradient = new CPVRTPaint();

		if(pGradient)
		{
			CPVRTSVGAttribute* pAttrib = pElement->GetFirstAttribute();

			for(int i = 0; i < 5; ++i)
				pGradient->m_bPointSet[i] = false;

			// Set default %
			if(_stricmp(pName, "lineargradient") == 0)
			{
				pGradient->m_afPoints[2] = 1.0f;
				pGradient->m_bPointSet[4] = true;
			}
			else
				pGradient->m_afPoints[0] = pGradient->m_afPoints[1] = pGradient->m_afPoints[4] = 0.05f;

			VGColorRampSpreadMode vgSpreadMode = VG_COLOR_RAMP_SPREAD_PAD;

			while(pAttrib)
			{
				if(_stricmp(pAttrib->GetName(), "x1") == 0 || _stricmp(pAttrib->GetName(), "cx") == 0)
				{
					pGradient->m_bPointSet[0] = true;
					pGradient->m_afPoints[0] = pAttrib->GetValueAsFloat();
				}
				else if(_stricmp(pAttrib->GetName(), "y1") == 0 || _stricmp(pAttrib->GetName(), "cy") == 0)
				{
					pGradient->m_bPointSet[1] = true;
					pGradient->m_afPoints[1] = pAttrib->GetValueAsFloat();
				}
				else if(_stricmp(pAttrib->GetName(), "x2") == 0 || _stricmp(pAttrib->GetName(), "fx") == 0)
				{
					pGradient->m_bPointSet[2] = true;
					pGradient->m_afPoints[2] = pAttrib->GetValueAsFloat();
				}
				else if(_stricmp(pAttrib->GetName(), "y2") == 0 || _stricmp(pAttrib->GetName(), "fY") == 0)
				{
					pGradient->m_bPointSet[3] = true;
					pGradient->m_afPoints[3] = pAttrib->GetValueAsFloat();
				}
				else if(_stricmp(pAttrib->GetName(), "r") == 0)
				{
					pGradient->m_bPointSet[4] = true;
					pGradient->m_afPoints[4] = pAttrib->GetValueAsFloat();
				}
				else if(_stricmp(pAttrib->GetName(), "id") == 0)
					pGradient->SetID(pAttrib->GetValue());
				else if(_stricmp(pAttrib->GetName(), "gradienttransform") == 0)
					ConvertTransform(pGradient->GetTransform(), pAttrib->GetValue());
				else if(_stricmp(pAttrib->GetName(), "gradientunits") == 0)
				{
					if(_stricmp(pAttrib->GetValue(), "userspaceonuse") == 0)
						pGradient->m_eGradientUnits = eUserSpaceOnUse;
					else if(_stricmp(pAttrib->GetValue(), "userspace") == 0)
						pGradient->m_eGradientUnits = eUserSpace;
					else if(_stricmp(pAttrib->GetValue(), "objectboundingbox") == 0)
						pGradient->m_eGradientUnits = eObjectBoundingBox;
					else
						_ASSERT(false);
				}
				else if(_stricmp(pAttrib->GetName(), "href") == 0)
					pGradient->SetHref(pAttrib->GetValue());
				else if(_stricmp(pAttrib->GetName(), "spreadmethod") == 0)
				{
					if(_stricmp(pAttrib->GetValue(), "pad") == 0)
						vgSpreadMode = VG_COLOR_RAMP_SPREAD_PAD;
					else if(_stricmp(pAttrib->GetValue(), "reflect") == 0)
						vgSpreadMode = VG_COLOR_RAMP_SPREAD_REFLECT;
					else if(_stricmp(pAttrib->GetValue(), "repeat") == 0)
						vgSpreadMode = VG_COLOR_RAMP_SPREAD_REPEAT;
				}
				//else
				//	_ASSERT(false);

				pAttrib = pAttrib->GetNext();
			}

			pGradient->m_vgPaint = vgCreatePaint();

			if(_stricmp(pName, "lineargradient") == 0)
			{
				vgSetParameteri(pGradient->m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
				pGradient->SetType(eAssetLinearPaint);
				vgSetParameterfv(pGradient->m_vgPaint, VG_PAINT_LINEAR_GRADIENT, 4, pGradient->m_afPoints);
			}
			else
			{
				if(!pGradient->m_bPointSet[2] && pGradient->m_bPointSet[0])
				{
					pGradient->m_afPoints[2] = pGradient->m_afPoints[0];
					pGradient->m_bPointSet[2] = true;
				}

				if(!pGradient->m_bPointSet[3] && pGradient->m_bPointSet[1])
				{
					pGradient->m_afPoints[3] = pGradient->m_afPoints[1];
					pGradient->m_bPointSet[3] = true;
				}

				vgSetParameteri(pGradient->m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
				pGradient->SetType(eAssetRadialPaint);
				vgSetParameterfv(pGradient->m_vgPaint, VG_PAINT_RADIAL_GRADIENT, 5, pGradient->m_afPoints);
			}

			vgSetParameteri(pGradient->m_vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, vgSpreadMode);

			m_pObject->AddAsset((CPVRTSVGBase*) pGradient);

			// Do Stops
			if(!pElement->IsClosed())
			{
				// This element wants children so don't delete them after they have been created
				pElement->SetWantsChildren(true);

				// Add any children to pElement, they should all be stops
				*pXMLData = AddChildren(pElement, 0, *pXMLData);

				unsigned int ui32ChildCount = pElement->GetNoOfChildren();

				if(ui32ChildCount)
				{
					float *pfStops = new float[5 * ui32ChildCount];

					if(pfStops)
					{
						float fColour[4];
						int i32Count = 0;

						CPVRTSVGElement *pChild = pElement->GetFirstChild();

						while(pChild)
						{
							if(_stricmp(pChild->GetName(), "stop") == 0)
							{
								pAttrib = pChild->GetFirstAttribute();

								if(i32Count == 0)
									pfStops[5 * i32Count] = 0.0f;
								else
									pfStops[5 * i32Count] = pfStops[5 * (i32Count - 1)];

								pfStops[5 * i32Count + 1] = 0.0f;
								pfStops[5 * i32Count + 2] = 0.0f;
								pfStops[5 * i32Count + 3] = 0.0f;
								pfStops[5 * i32Count + 4] = 1.0f;

								while(pAttrib)
								{
									if(_stricmp(pAttrib->GetName(), "offset") == 0)
									{
										pfStops[5 * i32Count] = pAttrib->GetValueAsFloat();

										if(strstr(pAttrib->GetValue(), "%"))
											pfStops[5 * i32Count] *= 0.01f;
									}
									else if(_stricmp(pAttrib->GetName(), "stop-color") == 0)
									{
										if(ExtractColor(pAttrib->GetValue(), &fColour[0]))
										{
											pfStops[5 * i32Count + 1] = fColour[0];
											pfStops[5 * i32Count + 2] = fColour[1];
											pfStops[5 * i32Count + 3] = fColour[2];
										}
									}
									else if(_stricmp(pAttrib->GetName(), "stop-opacity") == 0)
										pfStops[5 * i32Count + 4] = pAttrib->GetValueAsFloat();
									else if(_stricmp(pAttrib->GetName(), "id") == 0)
									{} // Ignore
									else
										_ASSERT(false);


									pAttrib = pAttrib->GetNext();
								}

								++i32Count;
							}

							pChild = pChild->GetNext();
						}

						vgSetParameterfv(pGradient->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, 5 * i32Count, pfStops);
						vgSetParameteri(pGradient->m_vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);

						delete[] pfStops;
						pfStops = 0;
					}
				}
			}

			pElement->SetClosed(true); // We've added the children of this element so lets sat it is closed
			return true;
		}
	}

	return false;
}

/*!***************************************************************************
 @Function		CreateDummy
 @Input			pElement An SVG element
 @Input			pParent The new node's parent
 @Returns		A new SVG node
 @Description	Creates a dummy node from an SVG element
*****************************************************************************/
CPVRTSVGNode* CPVRTSVGParser::CreateDummy(CPVRTSVGElement *pElement, CPVRTSVGNode *pParent)
{
	if(!pElement)
		return 0;

	char *pName = pElement->GetName();

	if(pName)
	{
		CPVRTSVGNode *pNode = new CPVRTSVGNode();

		if(!pNode)
			return 0;

		if(_stricmp(pName, "defs") == 0) // Don't render defines
		{
			pNode->SetRender(false);
			pNode->SetRenderChildren(false);
		}

		float *pWidth = 0;
		float *pHeight= 0;
		int i32Count = 0;
		float *pViewBox = 0;

		//Inherit any "values" from the parent
		if(pParent)
		{
			float *pValue;

			pValue = pParent->GetValue(eFillOpacity);

			if(pValue)
				pNode->SetValue(eFillOpacity, *pValue);

			pValue = pParent->GetValue(eOpacity);

			if(pValue)
				pNode->SetValue(eOpacity, *pValue);
		}

		CPVRTSVGAttribute* pAttrib = pElement->GetFirstAttribute();

		while(pAttrib)
		{
			if(_stricmp(pAttrib->GetName(), "fill-opacity") == 0)
				pNode->SetValue(eFillOpacity, pAttrib->GetValueAsFloat());
			else if(_stricmp(pAttrib->GetName(), "stroke-opacity") == 0)
				pNode->SetValue(eStrokeOpacity, pAttrib->GetValueAsFloat());
			else if(_stricmp(pAttrib->GetName(), "width") == 0)
				pWidth = pAttrib->GetValueAsFloatArray(i32Count);
			else if(_stricmp(pAttrib->GetName(), "height") == 0)
				pHeight = pAttrib->GetValueAsFloatArray(i32Count);
			else if(_stricmp(pAttrib->GetName(), "viewbox") == 0)
				pViewBox = pAttrib->GetValueAsFloatArray(i32Count);
			else if(_stricmp(pAttrib->GetName(), "id") == 0)
				pNode->SetID(pAttrib->GetName());
			else if(_stricmp(pAttrib->GetName(), "baseprofile") == 0)
			{}
			else if(_stricmp(pAttrib->GetName(), "version") == 0)
			{}
			else if(_stricmp(pAttrib->GetName(), "xmlns") == 0)
			{}
			else if(_stricmp(pAttrib->GetName(), "xlink") == 0)
			{}
			//else
			//	_ASSERT(false);

			pAttrib = pAttrib->GetNext();
		}

		if(pParent)
			pParent->AddChild(pNode);
		else
		{
			m_pObject->SetRoot(pNode);

			if(pViewBox)
				m_pObject->SetViewBox(pViewBox[0], pViewBox[1], pViewBox[2], pViewBox[3]);
			else if(pWidth && pHeight)
				m_pObject->SetViewBox(0.0f, 0.0f, *pWidth, *pHeight);
			else if(pWidth)
				m_pObject->SetViewBox(0.0f, 0.0f, *pWidth, *pWidth);
			else if(pHeight)
				m_pObject->SetViewBox(0.0f, 0.0f, *pHeight, *pHeight);
		}

		return pNode;
	}

	return 0;
}

/*!***************************************************************************
 @Function		Create
 @Input			pElement An SVG element
 @Input			pParent The new node's parent
 @Returns		A new SVG node
 @Description	Creates an SVG node from an SVG element
*****************************************************************************/
CPVRTSVGNode* CPVRTSVGParser::Create(CPVRTSVGElement *pElement, CPVRTSVGNode *pParent)
{
	if(!pElement)
		return pParent;

	if(!pParent) // If pParent is false then something is wrong
		return 0;

	char* pName = pElement->GetName();

	CPVRTSVGNode *pNode;

	if(pName)
	{
		pNode = new CPVRTSVGNode();

		if(!pNode)
			return pParent;

		float fX = 0.0f;
		float fY = 0.0f;
		float fWidth = 0.0f;
		float fHeight= 0.0f;
		float fCX = 0.0f;
		float fCY = 0.0f;
		float fRX = 0.0f;
		float fRY = 0.0f;

		float *pPoints = 0;
		int ui32PointsNo = 0;

		float fFillOpacity = -1.0f;
		float fOpacity = -1.0f;
		char *pFill = 0;
		char *pStroke = 0;
		char *pD = 0;
		char *pTransform = 0;

		CPVRTStroke *pPathStroke = 0;
		bool bInheritedStroke = false;

		//Inherit any "values" from the parent
		float *pValue;

		pValue = pParent->GetValue(eFillOpacity);

		if(pValue)
			fFillOpacity = *pValue;

		pValue = pParent->GetValue(eOpacity);

		if(pValue)
			fOpacity = *pValue;

		if(pParent->GetStroke())
		{
			pPathStroke = pParent->GetStroke();
			bInheritedStroke = true;
		}

		CPVRTSVGAttribute* pAttrib = pElement->GetFirstAttribute();

		while(pAttrib)
		{
		// Shapes
			if(_stricmp(pAttrib->GetName(), "x") == 0)
				fX = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "y") == 0)
				fY = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "cx") == 0)
				fCX = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "cy") == 0)
				fCY = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "width") == 0)
				fWidth = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "height") == 0)
				fHeight = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "rx") == 0)
				fRX = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "ry") == 0)
				fRY = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "r") == 0)
			{
				fRX = pAttrib->GetValueAsFloat();
				fRY = pAttrib->GetValueAsFloat();
			}
			else if(_stricmp(pAttrib->GetName(), "x1") == 0)
				fX = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "y1") == 0)
				fY = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "x2") == 0)
				fRX = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "y2") == 0)
				fRY = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "points") == 0)
			{
				pPoints = pAttrib->GetValueAsFloatArray(ui32PointsNo);
				ui32PointsNo = (int) (ui32PointsNo * 0.5f);
			}
			else if(_stricmp(pAttrib->GetName(), "d") == 0)
				pD = pAttrib->GetValue();
		// Fills
			else if(_stricmp(pAttrib->GetName(), "fill") == 0)
				pFill = pAttrib->GetValue();
			else if(_stricmp(pAttrib->GetName(), "fill-opacity") == 0)
				fFillOpacity = pAttrib->GetValueAsFloat();
			else if(_stricmp(pAttrib->GetName(), "fill-rule") == 0)
			{
				if (!strcmp(pAttrib->GetValue(),"evenodd"))
					pNode->SetFillRule(VG_EVEN_ODD);
				else if (!strcmp(pAttrib->GetValue(),"nonzero"))
					pNode->SetFillRule(VG_NON_ZERO);
			}
		// Strokes
			else if(_stricmp(pAttrib->GetName(), "stroke") == 0)
				pStroke = pAttrib->GetValue();
			else if(strstr(pAttrib->GetName(), "stroke"))
			{
				if(!pPathStroke || bInheritedStroke)
				{
					pPathStroke = new CPVRTStroke();

					if(bInheritedStroke)
					{
						*pPathStroke = *pParent->GetStroke();
						bInheritedStroke = false;
					}

					m_pObject->AddAsset((CPVRTSVGBase*) pPathStroke);
				}

				if(pPathStroke)
				{
					if(_stricmp(pAttrib->GetName(), "stroke-width") == 0)
						pPathStroke->m_fWidth = pAttrib->GetValueAsFloat();
					else if(_stricmp(pAttrib->GetName(), "stroke-linecap") == 0)
					{
						if (!strcmp(pAttrib->GetValue(),"butt"))
							pPathStroke->m_i32LineCap = (int)VG_CAP_BUTT;
						else if (!strcmp(pAttrib->GetValue(),"round"))
							pPathStroke->m_i32LineCap = (int)VG_CAP_ROUND;
						else if (!strcmp(pAttrib->GetValue(),"square"))
							pPathStroke->m_i32LineCap = (int)VG_CAP_SQUARE;
					}
					else if(_stricmp(pAttrib->GetName(), "stroke-opacity") == 0)
					{
						pPathStroke->m_fOpacity = pAttrib->GetValueAsFloat();
						pNode->SetValue(eStrokeOpacity, pPathStroke->m_fOpacity);
					}
					else if(_stricmp(pAttrib->GetName(), "stroke-dasharray") == 0)
					{
						delete[] pPathStroke->m_pafDashArray;
						pPathStroke->m_pafDashArray = 0;
						pPathStroke->m_i32DashSize  = 0;

						if(strcmp(pAttrib->GetValue(), "none") != 0)
						{
							float *pValue = pAttrib->GetValueAsFloatArray(pPathStroke->m_i32DashSize);

							if(pValue && pPathStroke->m_i32DashSize > 0)
							{
								pPathStroke->m_pafDashArray = new float[pPathStroke->m_i32DashSize];

								if(pPathStroke->m_pafDashArray)
									memcpy(pPathStroke->m_pafDashArray, pValue, pPathStroke->m_i32DashSize * sizeof(float));
							}
						}
					}
					else if(_stricmp(pAttrib->GetName(), "stroke-dashoffset") == 0)
						pPathStroke->m_fDashOffset = pAttrib->GetValueAsFloat();
					else if(_stricmp(pAttrib->GetName(), "stroke-linejoin") == 0)
					{
						if (!strcmp(pAttrib->GetValue(),"miter"))
							pPathStroke->m_i32LineJoin = (int)VG_JOIN_MITER;
						else if (!strcmp(pAttrib->GetValue(),"round"))
							pPathStroke->m_i32LineJoin = (int)VG_JOIN_ROUND;
						else if (!strcmp(pAttrib->GetValue(),"bevel"))
							pPathStroke->m_i32LineJoin = (int)VG_JOIN_BEVEL;
					}
					else if(_stricmp(pAttrib->GetName(), "stroke-miterlimit") == 0)
						pPathStroke->m_fMiterLimit = pAttrib->GetValueAsFloat();
				}
			}
			else if(_stricmp(pAttrib->GetName(), "id") == 0)
				pNode->SetID(pAttrib->GetValue());
			else if(_stricmp(pAttrib->GetName(), "href") == 0)
				pNode->SetPathID(pAttrib->GetValue());
			else if(_stricmp(pAttrib->GetName(), "transform") == 0)
				pTransform = pAttrib->GetValue();
			else if(_stricmp(pAttrib->GetName(), "opacity") == 0)
				fOpacity = pAttrib->GetValueAsFloat();

			pAttrib = pAttrib->GetNext();
		}

		ConvertTransform(pNode->GetTransform(), pTransform);

		// Not quite the correct way to handle the "opacity" value
		if(fOpacity != -1)
		{
			if(fFillOpacity != -1)
				fFillOpacity *= fOpacity;
			else
				fFillOpacity = fOpacity;

			if(pPathStroke)
			{
				pPathStroke->m_fOpacity *= fOpacity;
				pNode->SetValue(eStrokeOpacity, pPathStroke->m_fOpacity);
			}

			pNode->SetValue(eOpacity, fOpacity);
		}

		if(fFillOpacity != -1)
			pNode->SetValue(eFillOpacity, fFillOpacity);

		CPVRTPaint *pPaint = 0;
		CPVRTPaint *pStrokePaint = 0;
		CPVRTPath *pPath = 0;

		pPaint = CreatePaint(pNode, pFill, fFillOpacity, VG_FILL_PATH, pParent);
		pStrokePaint = CreatePaint(pNode, pStroke, pPathStroke ? pPathStroke->m_fOpacity : -1.0f, VG_STROKE_PATH, pParent);

		if(_stricmp(pName, "g") ==0)
			pNode->SetRender(false);
		else if(_stricmp(pName, "use") != 0)
		{
			pPath = new CPVRTPath();

			if(!pPath)
				return pParent;

			// Create the Path
			pPath->m_vgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD,VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, (unsigned int)VG_PATH_CAPABILITY_ALL);

			if(_stricmp(pName, "circle") == 0 || _stricmp(pName, "ellipse") == 0)
				vguEllipse(pPath->m_vgPath,fCX, fCY, 2.0f * fRX, 2.0f * fRY);
			else if(_stricmp(pName, "line") == 0)
				vguLine(pPath->m_vgPath, fX, fY, fRX, fRY);
			else if(_stricmp(pName, "path") == 0)
				CreateVGPath(pPath, pD);
			else if(_stricmp(pName, "polygon") == 0)
				vguPolygon(pPath->m_vgPath, pPoints, ui32PointsNo, (VGboolean) true);
			else if(_stricmp(pName, "polyline") == 0)
				vguPolygon(pPath->m_vgPath, pPoints, ui32PointsNo, (VGboolean) false);
			else if(_stricmp(pName, "rect") == 0)
				vguRoundRect(pPath->m_vgPath,fX, fY, fWidth,fHeight, 2 * fRX,2 * fRY);

			m_pObject->AddAsset((CPVRTSVGBase*) pPath);
		}

		pNode->SetPath(pPath);
		pNode->SetPaint(VG_FILL_PATH, pPaint);
		pNode->SetPaint(VG_STROKE_PATH, pStrokePaint);
		pNode->SetStroke(pPathStroke);

		pParent->AddChild(pNode);
		return pNode;
	}

	return pParent;
}

/*!***************************************************************************
 @Function		CreatePaint
 @Input 		pNode	The node the paint belongs to
 @Input 		pValue	The SVG value
 @Input 		fOpacity The opacity of the paint
 @Input 		iType	The paint mode
 @Input 		pParent The node's parent incase it has to inherit a paint
 @Returns 		A new paint if one was created
 @Description	Creates a paint for pNode
*****************************************************************************/
CPVRTPaint* CPVRTSVGParser::CreatePaint(CPVRTSVGNode *pNode, char *pValue, float fOpacity, VGPaintMode iType, CPVRTSVGNode *pParent)
{
	if(!pNode)
		return false;

	CPVRTPaint* pPaint = 0;
	float fColour[4];

	if(pValue)
	{
		if(!strstr(pValue, "url("))
		{
			if(_stricmp(pValue, "none") == 0)
			{
				pNode->SetNone(iType, true);
			}
			else if(ExtractColor(pValue, &fColour[0]))
			{
				pPaint = new CPVRTPaint();

				if(fOpacity != -1)
					fColour[3] = fOpacity;
			}

			if(pPaint)
			{
				pPaint->m_vgPaint = vgCreatePaint();
				vgSetParameteri(pPaint->m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
				vgSetParameterfv(pPaint->m_vgPaint, VG_PAINT_COLOR, 4, &fColour[0]);

				m_pObject->AddAsset((CPVRTSVGBase*) pPaint);
			}
		}
		else
		{
			pNode->SetPaintID(iType, pValue);
			m_pObject->SetupReferences(pNode);

			pPaint = pNode->GetPaint(iType);

			if(pPaint && pPaint->IsInit())
				pPaint = CopyPaint(pNode->GetPaint(iType), fOpacity);
		}
	}
	else if(!pNode->GetPathID())
	{
		if(pParent && pParent->GetPaint(iType))
		{
			m_pObject->SetupReferences(pParent);

			pPaint = pParent->GetPaint(iType);

			if(pPaint && pPaint->IsInit())
				pPaint = CopyPaint(pPaint, fOpacity);
		}
		else if(iType == VG_FILL_PATH) // use default fill paint
		{
			if(!m_pDefaultFillPaint)
			{
				m_pDefaultFillPaint = new CPVRTPaint();

				if(m_pDefaultFillPaint && ExtractColor("black", &fColour[0]))
				{
					m_pDefaultFillPaint->m_vgPaint = vgCreatePaint();
					vgSetParameteri(m_pDefaultFillPaint->m_vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
					vgSetParameterfv(m_pDefaultFillPaint->m_vgPaint, VG_PAINT_COLOR, 4, fColour);
					m_pObject->AddAsset((CPVRTSVGBase*) m_pDefaultFillPaint);
				}
			}

			pPaint = CopyPaint(m_pDefaultFillPaint, fOpacity);
		}
	}

	return pPaint;
}

/*!***************************************************************************
 @Function		CopyPaint
 @Input			pPaint The paint to copy
 @Input			fOpacity An opacity value to apply to the paint
 @Returns		A new paint
 @Description	Copies a paint and applies and alters its transparancy
*****************************************************************************/
CPVRTPaint* CPVRTSVGParser::CopyPaint(CPVRTPaint *pPaint, float fOpacity)
{
	if(!pPaint)
		return 0;

	if(fOpacity == -1.0f || fOpacity == 1.0f)
		return pPaint;

	if(fOpacity < 0.0f)
		fOpacity = 0.0f;

	if(fOpacity > 1.0f)
		fOpacity = 1.0f;

	CPVRTPaint *pPaintNew = new CPVRTPaint();

	if(pPaintNew)
	{
		*pPaintNew = *pPaint;

		float fValues[100];
		int i32Value = 0;

		switch(vgGetParameteri(pPaint->m_vgPaint, VG_PAINT_TYPE))
		{
			case VG_PAINT_TYPE_COLOR:
				vgGetParameterfv(pPaint->m_vgPaint, VG_PAINT_COLOR, 4, fValues);

				fValues[3] *= fOpacity;

				vgSetParameterfv(pPaintNew->m_vgPaint, VG_PAINT_COLOR, 4, fValues);
			break;
			case VG_PAINT_TYPE_LINEAR_GRADIENT:
			case VG_PAINT_TYPE_RADIAL_GRADIENT:
				i32Value = vgGetParameterVectorSize(pPaint->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS);

				if(i32Value < 100)
				{
					vgGetParameterfv(pPaint->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);

					for(int i = 0; i < (i32Value / 5); ++i)
					{
						fValues[5 * i + 4] *= fOpacity;
					}

					vgSetParameterfv(pPaintNew->m_vgPaint, VG_PAINT_COLOR_RAMP_STOPS, i32Value, fValues);
				}
			break;
		}

		m_pObject->AddAsset((CPVRTSVGBase*) pPaintNew);
	}

	return pPaintNew;
}

#define TRANSFORMATION_MATRIX    "matrix"
#define TRANSFORMATION_TRANSLATE "translate"
#define TRANSFORMATION_SCALE     "scale"
#define TRANSFORMATION_ROTATE    "rotate"
#define TRANSFORMATION_SKEWX     "skewX"
#define TRANSFORMATION_SKEWY     "skewY"

/*!***************************************************************************
 @Function		ConvertTransform
 @Modified		pMatrix The matrix to set to the transformation
 @Input			pTransform The SVG transformation
 @Returns		True if successful
 @Description	Converts an SVG transformation to a PVRTMat3
*****************************************************************************/
bool CPVRTSVGParser::ConvertTransform(PVRTMat3* pMatrix, char* pTransform)
{
	int i32Count;
	*pMatrix = PVRTMat3::Identity();

	if(!pTransform)
		return true;

	PVRTMat3 fMatTmp;

	while(pTransform[0] != '\0')
	{
		if (isspace(pTransform[0]) || pTransform[0]==',')
		{
			++pTransform;
			continue;
		}

		// 'matrix' transformation
		if (!strncmp(pTransform,TRANSFORMATION_MATRIX,sizeof(TRANSFORMATION_MATRIX) - 1))
		{
			fMatTmp = PVRTMat3::Identity();

			if(sscanf(pTransform, "matrix(%f%*c%f%*c%f%*c%f%*c%f%*c%f)",
					&fMatTmp.f[0], &fMatTmp.f[1], &fMatTmp.f[3], &fMatTmp.f[4], &fMatTmp.f[6], &fMatTmp.f[7]))
			{
 				*pMatrix *= fMatTmp;
			}

			//bypass this transformation
			while(*pTransform != ')')
				++pTransform;

			++pTransform;
		}
		// 'translate' transformation
		else if (!strncmp(pTransform,TRANSFORMATION_TRANSLATE,sizeof(TRANSFORMATION_TRANSLATE) - 1))
		{
			fMatTmp = PVRTMat3::Identity();

			if(sscanf(pTransform, "translate(%f%*c%f)", &fMatTmp.f[6],&fMatTmp.f[7]))
			{
				*pMatrix *= fMatTmp;
			}

			//bypass this transformation
			while(*pTransform != ')')
				++pTransform;

			++pTransform;
		}
		// 'scale' transformation
		else if (!strncmp(pTransform,TRANSFORMATION_SCALE,sizeof(TRANSFORMATION_SCALE) - 1))
		{
			fMatTmp = PVRTMat3::Identity();

			float fSx, fSy;

			i32Count = sscanf(pTransform, "scale(%f%*c%f)", &fSx, &fSy);

			if(i32Count == 1)
			{
				fMatTmp = PVRTMat3::Scale(fSx, fSx, 1.0f);
				*pMatrix *= fMatTmp;
			}
			else if(i32Count == 2)
			{
				fMatTmp = PVRTMat3::Scale(fSx, fSy, 1.0f);
				*pMatrix *= fMatTmp;
			}

			//bypass this transformation
			while(*pTransform != ')')
				++pTransform;

			++pTransform;
		}
		// 'rotate' transformation
		else if (!strncmp(pTransform,TRANSFORMATION_ROTATE,sizeof(TRANSFORMATION_ROTATE) - 1))
		{
			fMatTmp = PVRTMat3::Identity();

			float fAngle = 0.0f;
			float fCX = 0.0f;
			float fCY = 0.0f;

			i32Count = sscanf(pTransform, "rotate(%f%*c%f%*c%f)", &fAngle, &fCX, &fCY);

			// Convert to radians
			fAngle *= (PVRT_PI / 180.0f);

			if(i32Count == 1)
			{
				fMatTmp.RotationZ(fAngle);
				*pMatrix *= fMatTmp;
			}
			else if(i32Count == 3)
			{
				fMatTmp  = PVRTMat3::Translation(fCX, fCY);
				*pMatrix *= fMatTmp;

				fMatTmp = PVRTMat3::Identity();
				fMatTmp.RotationZ(fAngle);
				*pMatrix *= fMatTmp;

				fMatTmp = PVRTMat3::Translation(-fCX, -fCY);
				*pMatrix *= fMatTmp;
			}

			//bypass this transformation
			while(*pTransform != ')')
				++pTransform;

			++pTransform;
		}
		// 'skewX' transformation
		else if (!strncmp(pTransform,TRANSFORMATION_SKEWX,sizeof(TRANSFORMATION_SKEWX) - 1))
		{
			float fSkewX = 0.0f;

			i32Count = sscanf(pTransform, "skewX(%f)", &fSkewX);

			if(i32Count == 1)
			{
				fMatTmp = PVRTMat3::Identity();
				fMatTmp.f[3] = (float) tan(fSkewX);
				*pMatrix *= fMatTmp;
			}
		}
		// 'skewY' transformation
		else if (!strncmp(pTransform,TRANSFORMATION_SKEWY,sizeof(TRANSFORMATION_SKEWY) - 1))
		{
			float fSkewY = 0.0f;

			i32Count = sscanf(pTransform, "skewY(%f)", &fSkewY);

			if(i32Count == 1)
			{
				fMatTmp = PVRTMat3::Identity();
				fMatTmp.f[1] = (float) tan(fSkewY);
				*pMatrix *= fMatTmp;
			}
		}
	}

	return true;
}

/*!***************************************************************************
 @Function			IsCommand
 @Input				pc	Input char
 @Modified			i32Points The number of points
 @Returns			True if successful
 @Description		Tells you whether a char is an SVG path command or not. If
 					it is then it also sets i32Points to the number of associated
 					points.
*****************************************************************************/
bool CPVRTSVGParser::IsCommand(const char *pc, int &i32Points)
{
	if(!pc)
		return false;

	i32Points = 0;

	switch(*pc)
	{
		case 'Z': // CLOSE_PATH
		case 'z':
			return true;
		case 'H': // HLINE_TO
		case 'h': // HLINE_TO
		case 'V': // VLINE_TO
		case 'v': // VLINE_TO
			i32Points = 1;
			return true;
		case 'M': // MOVE_TO
		case 'm': // MOVE_TO
		case 'T': // SQUAD_TO
		case 't': // SQUAD_TO
		case 'L': // LINE_TO
		case 'l': // LINE_TO
			i32Points = 2;
			return true;
		case 'S': // SCUBIC_TO
		case 's': // SCUBIC_TO
		case 'Q': // QUAD_TO
		case 'q': // QUAD_TO
			i32Points = 4;
			return true;
		case 'C': // CUBIC_TO
		case 'c': // CUBIC_TO
			i32Points = 6;
			return true;
		case 'A': // VG_SCWARC_TO
		case 'a': // VG_SCWARC_TO
			i32Points = 5;
			return true;
	}

	return false;
}

/*!***************************************************************************
 @Function	GetCommand
 @Input		pc	The SVG path command
 @Modified	i32Command	The output for the openvg command
 @Modified	i32Points	The output for the number of points
 @Returns	True on success
 @Description Takes an SVG path command and returns the openvg version and the
 			  number of associated points.
*****************************************************************************/
bool CPVRTSVGParser::GetCommand(const char *pc, int &i32Command, int &i32Points)
{
	if(!pc)
		return false;

	i32Points = 0;

	switch(*pc)
	{
		case 'Z': // CLOSE_PATH
		case 'z':
			i32Points = 0;
			i32Command = VG_CLOSE_PATH;
			return true;
		case 'H': // HLINE_TO
			i32Points = 1;
			i32Command = VG_HLINE_TO_ABS;
			return true;
		case 'h': // HLINE_TO
			i32Points = 1;
			i32Command = VG_HLINE_TO_REL;
			return true;
		case 'V': // VLINE_TO
			i32Points = 1;
			i32Command = VG_VLINE_TO_ABS;
			return true;
		case 'v': // VLINE_TO
			i32Points = 1;
			i32Command = VG_VLINE_TO_REL;
			return true;
		case 'M': // MOVE_TO
			i32Points = 2;
			i32Command = VG_MOVE_TO_ABS;
			return true;
		case 'm': // MOVE_TO
			i32Points = 2;
			i32Command = VG_MOVE_TO_REL;
			return true;
		case 'T': // SQUAD_TO
			i32Points = 2;
			i32Command = VG_SQUAD_TO_ABS;
			return true;
		case 't': // SQUAD_TO
			i32Points = 2;
			i32Command = VG_SQUAD_TO_REL;
			return true;
		case 'L': // LINE_TO
			i32Points = 2;
			i32Command = VG_LINE_TO_ABS;
			return true;
		case 'l': // LINE_TO
			i32Points = 2;
			i32Command = VG_LINE_TO_REL;
			return true;
		case 'S': // SCUBIC_TO
			i32Points = 4;
			i32Command = VG_SCUBIC_TO_ABS;
			return true;
		case 's': // SCUBIC_TO
			i32Points = 4;
			i32Command = VG_SCUBIC_TO_REL;
			return true;
		case 'Q': // QUAD_TO
			i32Points = 4;
			i32Command = VG_QUAD_TO_ABS;
			return true;
		case 'q': // QUAD_TO
			i32Points = 4;
			i32Command = VG_QUAD_TO_REL;
			return true;
		case 'C': // CUBIC_TO
			i32Points = 6;
			i32Command = VG_CUBIC_TO_ABS;
			return true;
		case 'c': // CUBIC_TO
			i32Points = 6;
			i32Command = VG_CUBIC_TO_REL;
			return true;
		case 'A': // VG_SCWARC_TO
			i32Points = 7;
			i32Command = VG_SCWARC_TO_ABS;
			return true;
		case 'a': // VG_SCWARC_TO
			i32Points = 7;
			i32Command = VG_SCWARC_TO_REL;
			return true;
	}

	return false;
}

/*!***************************************************************************
 @Function		GetPointsAndCommands
 @Input			pData	Input data
 @Modified 		pCommands	An array for the commands
 @Modified 		pPoints		An array for the points
 @Modified 		i32CommandSize	The size of the commands array
 @Modified 		i32PointsSize	The size of the points array
 @Input 		bPass1			If true then don't populate the arrays instead just get the sizes
 @Returns		True on success
 @Description	Parses pData to produce a list of points and commands for path creation
*****************************************************************************/
bool CPVRTSVGParser::GetPointsAndCommands(const char* pData, char* pCommands, float* pPoints, int &i32CommandSize, int &i32PointsSize, bool bPass1)
{
	if(!pData)
		return false;

	if(!bPass1 && (!pCommands || !pPoints || !i32CommandSize || !i32PointsSize))
		return false;

	int i32PrevPoints = 0;
	int i32Points = 0;
	float fTemp[7];
	int iCommand;
	int iPrevCommand = 0;
	int i32Index = 0;
	int i32CommandNo = 0;
	int i;
	bool bLoop = false;

	// Pre-process the data to find out how much space we need to calculate
	while(*pData != '\0')
	{
		if(GetCommand(pData, iCommand, i32Points))
		{
			iPrevCommand  = iCommand;
			i32PrevPoints = i32Points;

			if(!bPass1)
			{
				if(i32CommandNo >= i32CommandSize)
					return false;
				else
					pCommands[i32CommandNo] = (char) iCommand;
			}

			++i32CommandNo;
			++pData;
		}
		else
		{
			do
			{
				bLoop = false;


				while(*pData != '\0' && (*pData == ' ' || *pData == '\n' || *pData == '\r' || *pData == '\t' || *pData == ','))
					++pData;

				if(*pData == '\0')
				{
					if(bPass1)
					{
						i32CommandSize = i32CommandNo;
						i32PointsSize  = i32Index;
					}

					return true;
				}

				//First parse all the params for the current command
				for(i = 0; i < i32PrevPoints; ++i)
				{
					if(!ExtractFloat(&pData, &fTemp[i]))
						return false;

					// Skip the seperators
					while(*pData != '\0' && (*pData == ' ' || *pData == '\n' || *pData == '\r' || *pData == '\t' || *pData == ','))
						++pData;

					if(i < i32PrevPoints - 1 && *pData == '\0')
						return false;

					if(i == i32PrevPoints - 1 && IsCommand(pData, i32Points))
						bLoop = false;
					else if(i == i32PrevPoints - 1 && *pData != '\0')
					{
						if(!bPass1)
						{
							if(i32CommandNo >= i32CommandSize)
								return false;
							else
								pCommands[i32CommandNo] = (char) iPrevCommand;
						}

						++i32CommandNo;
						bLoop = true;
					}
				}

				if(!bPass1)
				{
					// Special case for elliptical arc
					if(pCommands[i32CommandNo - 1] == VG_SCWARC_TO_ABS || pCommands[i32CommandNo - 1] == VG_SCWARC_TO_REL)
					{
						pCommands[i32CommandNo - 1] ^= VG_SCWARC_TO;

						int iFlag = 0;;

						if(fabs(fTemp[3]) < g_fEpsilon) // Small Ellipse
							iFlag |=1;

						if(fabs(fTemp[4]) < g_fEpsilon) // Clock Wise
							iFlag |=2;

						switch(iFlag)
						{
							case 0:	 pCommands[i32CommandNo - 1] |= VG_LCCWARC_TO; break;
							case 1:	 pCommands[i32CommandNo - 1] |= VG_SCCWARC_TO; break;
							case 2:	 pCommands[i32CommandNo - 1] |= VG_LCWARC_TO; break;
							case 3:	 pCommands[i32CommandNo - 1] |= VG_SCWARC_TO; break;
						}

						fTemp[3] = fTemp[5];
						fTemp[4] = fTemp[6];

						for(i = 0; i < 5; ++i)
						{
							pPoints[i32Index++] = fTemp[i];
						}
					}
					else
					{
						for(i = 0; i < i32PrevPoints; ++i)
						{
							pPoints[i32Index++] = fTemp[i];
						}
					}
				}
				else
				{
					if(iPrevCommand == VG_SCWARC_TO_ABS || iPrevCommand == VG_SCWARC_TO_REL)
						i32Index += 5;
					else
						i32Index += i32PrevPoints;
				}
			}while(bLoop);
		}
	}

	if(bPass1)
	{
		i32CommandSize = i32CommandNo;
		i32PointsSize  = i32Index;
	}

	return true;
}

/*!***************************************************************************
 @Function		CreateVGPath
 @Modified		pPath The path class to initialise
 @Input			pSVGPathData The SVG path data
 @Returns		True on success
 @Description	Creates an OpenVG path from SVG data
*****************************************************************************/
bool CPVRTSVGParser::CreateVGPath(CPVRTPath* pPath, const char * pSVGPathData)
{
	if(!pPath || !pSVGPathData)
		return false;

	char * aCommand = 0;
	int iCountCommand = 0;

	float * afPoints = 0;
	int iCountPoint = 0;

	iCountPoint = 0;
	iCountCommand = 0;

	if(!GetPointsAndCommands(pSVGPathData, 0,0, iCountCommand, iCountPoint, true))
		return false;

	aCommand = new char[iCountCommand];
	afPoints = new float[iCountPoint];


	if(GetPointsAndCommands(pSVGPathData, aCommand, afPoints, iCountCommand, iCountPoint))
	{
		vgAppendPathData(pPath->m_vgPath, iCountCommand, (const VGubyte*) aCommand, afPoints);
	}

	delete[] aCommand;
	delete[] afPoints;

	return true;
}

/*!***************************************************************************
 @Function		ExtractFloat
 @Input			ppzString a point to the string
 @Modified		pfFloat the place where to put the float when extracted
 @Returns		True on success
 @Description	Extracts a float from a string
*****************************************************************************/
bool CPVRTSVGParser::ExtractFloat(const char ** ppzString,float *pfFloat)
{
	char * pzString;
	int iCountDot;
	char * pzEPos;
	bool bTest;

	pzString = (char*) *ppzString;
	iCountDot = 0;
	pzEPos = 0;

	bTest = (pzString != 0)? true:false;

	while( bTest )
	{
		switch( pzString[0] )
		{
		// A digit
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			pzString++;
			break;
		// End of the string
		case 0:
			bTest = false;
			break;
		// only if first character
		case '-':
			if ( (pzString == *ppzString) || (pzEPos == pzString-1) )
				pzString++;
			else
				bTest = false;
			break;
		// Only one . per float
		case '.':
			if (iCountDot == 0)
				pzString++;
			else
				bTest = false;
			iCountDot++;
			break;
		// Only one 'e'/'E' per float
		case 'e':
		case 'E':
			if (pzEPos == 0)
				pzEPos = pzString++;
			else
				bTest = false;
			break;
		// Other character => end of float
		default:
			bTest = false;
			break;
		}
	}

	if(pzString == *ppzString)
		return false;
	else
	{
		char cTmp = *pzString;
		*pzString = '\0';
		*pfFloat = (float)atof(*ppzString);
		*pzString = cTmp;

		*ppzString = pzString;
	}

	return true;
}

/*!***************************************************************************
 @Function	ExtractColor
 @Input		pzString	A string
 @Modified	pfFloat		A float array to place the RGBA colour
 @Returns 	True on success
 @Description Extracts the colour from the string and places it in pfFloat
*****************************************************************************/
bool CPVRTSVGParser::ExtractColor(char * pzString, float *pfFloat)
{
	// Check for #rgb or #rrggbb format
	if(pzString[0] == '#' )
	{
		++pzString;

		int iValue,iSize;

		iSize = (int) strlen(pzString);

		if (iSize == 3)
		{
			sscanf(pzString,"%x",&iValue);
			pfFloat[0] = (iValue >> 8 & 0xF) / 15.0f;
			pfFloat[1] = (iValue >> 4 & 0xF) / 15.0f;
			pfFloat[2] = (iValue >> 0 & 0xF) / 15.0f;
			pfFloat[3] = 1.0f;
		}
		else if (iSize == 6)
		{
			sscanf(pzString,"%x",&iValue);
			pfFloat[0] = (iValue >> 16 & 0xFF) / 255.0f;
			pfFloat[1] = (iValue >> 8  & 0xFF) / 255.0f;
			pfFloat[2] = (iValue >> 0  & 0xFF) / 255.0f;
			pfFloat[3] = 1.0f;
		}

		return true;
	}
	else if (!strncmp(pzString,"rgb(",3)) // Check for rgb() functional format
	{
		sscanf(pzString, "rgb(%f,%f,%f)", &pfFloat[0], &pfFloat[1], &pfFloat[2]);
		pfFloat[0] /= 255.0f;
		pfFloat[1] /= 255.0f;
		pfFloat[2] /= 255.0f;
		pfFloat[3] = 1.0f;
		return true;
	}
	else // Check for the color keyword format
	{
		for(int i = 0; i < g_ColourNumber; ++i)
		{
			if(_stricmp(g_psSVGColours[i].pName, pzString) == 0)
			{
				pfFloat[0] = g_psSVGColours[i].r / 255.0f;
				pfFloat[1] = g_psSVGColours[i].g / 255.0f;
				pfFloat[2] = g_psSVGColours[i].b / 255.0f;
				pfFloat[3] = 1.0f;
				return true;
			}
		}
	}

	PVRTERROR_OUTPUT_DEBUG("WARNING: Colour format not recognised");
	return false;
}

/*!***************************************************************************
 @Function			IsColorKeyWord
 @Input				pzString
 @Modified			pfFloat the colour to return
 @Input				pzColorKeyWord - the colour keyword to match with pzString
 @Input				r - The red value for the colour keyword
 @Input				g - The blue value for the colour keyword
 @Input				b - the blue value for the colour keyword
 @Returns			returns true if successful
 @Description		Matches pzString with pzColorKeyWord and it there is a
					match then it sets pfFloat to the r,g,b values.
*****************************************************************************/
bool CPVRTSVGParser::IsColorKeyWord(char * pzString, float *pfFloat, char * pzColorKeyWord,int r,int g,int b)
{
	if(!strcmp(pzString,pzColorKeyWord))
	{
		pfFloat[0] = r / 255.0f;
		pfFloat[1] = g / 255.0f;
		pfFloat[2] = b / 255.0f;
		pfFloat[3] = 1.0f;
		return true;
	}
	else
		return false;
}

/*!***************************************************************************
 @Function			AddAttribute
 @Input				pElement The element to add the attribute to
 @Input				pXMLData The XML data
 @Input				bStyle Is the attribute a style
 @Returns			Returns the remainder of the XML file if successful
 @Description		Adds an attribute to pElement
*****************************************************************************/
char* CPVRTSVGParser::AddAttribute(CPVRTSVGElement *pElement, char* pXMLData, bool bStyle)
{
	if(!pElement)
		return false;

	// Remove any white space
	while(isspace((unsigned char) *pXMLData))
		++pXMLData;

	char* pStart = pXMLData;

	// get attribute name
	char cSeperator;

	if(!bStyle)
		cSeperator = '=';
	else
		cSeperator = ':';

	while(!isspace((unsigned char) *pXMLData) && *pXMLData != cSeperator && *pXMLData != '\0')
	{
		if(bStyle && *pXMLData == '\"') //We've reached the close quote
		{
			++pXMLData; // Skip close quote
			return pXMLData;
		}
		else if(!bStyle && *pXMLData == ':') // A namespace has been found, ignore it
			pStart = pXMLData + 1;

		++pXMLData;
	}

	if(*pXMLData == '\0')
	{
		_ASSERT(false);
		printf("PVRTSVGParser: Unexpected end of data stream in element name");
		return 0;
	}

	if(isspace((unsigned char) *pXMLData))
	{
		_ASSERT(false);
		printf("PVRTSVGParser: Unexpected space after attribute name");
		return 0;
	}

	CPVRTSVGAttribute* pAttrib = new CPVRTSVGAttribute();

	if(!pAttrib)
		return false;

	pAttrib->SetName(pStart, pXMLData);

	++pXMLData; // skip '=' / ':'

	if(!bStyle)
	{
		// get the attribute data (enclosed in "")
		if(*pXMLData != '\"')
		{
			PVRTERROR_OUTPUT_DEBUG("PVRTSVGParser: Expected '\"' after '=' in attribute");
			return 0;
		}

		// Skip quotes
		++pXMLData;
	}

	if(_stricmp(pAttrib->GetName(), "style") == 0)
	{
		delete pAttrib;
		pXMLData = AddAttribute(pElement, pXMLData, true);
	}
	else
	{
		pStart = pXMLData;

		if(bStyle)
		{
			while(*pXMLData != '\"' && *pXMLData != ';')
				++pXMLData;
		}
		else
		{
			while(*pXMLData != '\"')
				++pXMLData;
		}

		pAttrib->SetValue(pStart, pXMLData);
		pElement->AddAttribute(pAttrib);

		if(bStyle && *pXMLData == ';') // Are there more attributes to go
		{
			++pXMLData; // Skip the ;
			pXMLData = AddAttribute(pElement, pXMLData, bStyle);
		}
		else
			++pXMLData; // move one beyond the final " / ;
	}

	return pXMLData;
}

/*!***************************************************************************
 @Function			CreateTree
 @Input				pXMLData - the entire XML data
 @Returns			Returns true if successful
 @Description		Creates a tree from the XML data stored in pXMLData
*****************************************************************************/
bool CPVRTSVGParser::CreateTree(char* pXMLData)
{
	while(pXMLData)	 // xml_data will be set to a NULL pointer on parser error
	{
		while(isspace((unsigned char) *pXMLData))
			++pXMLData;

		if(*pXMLData == '\0')
			return true;

		pXMLData = Process(pXMLData);
	}

	if(!pXMLData)
		return false;

	return true;
}

/*!***************************************************************************
 @Function			GetRoot
 @Returns			Returns the root element of the XML tree
 @Description		Returns the root element of the XML tree
*****************************************************************************/
CPVRTSVGElement* CPVRTSVGParser::GetRoot()
{
	return m_pRootElement;
}

/******************************************************************************
 End of file PVRTSVGParser.cpp
******************************************************************************/
