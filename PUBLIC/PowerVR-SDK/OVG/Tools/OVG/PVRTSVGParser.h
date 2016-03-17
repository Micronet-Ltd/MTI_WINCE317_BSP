/******************************************************************************

 @File         PVRTSVGParser.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI Independant

 @Description  A simple SVG parser for parsing SVG files

******************************************************************************/
#ifndef __PVRTSVGParser_H__
#define __PVRTSVGParser_H__

#include "PVRTContext.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "PVRTSVGObject.h"

#include "vg/vgu.h"

#ifndef PVRT_PI
#define PVRT_PI (3.1415926535f)
#endif

/*!****************************************************************************
 @Struct      SSVGColours
 @Brief       A struct for pairing the SVG colour name with its RGB value
******************************************************************************/
struct SSVGColours
{
	const char	*pName;	// String containing semantic details
	int r;
	int g;
	int b;
};

const float g_fEpsilon = 1.1920929e-7f;

const int g_ColourNumber = 147;

const static SSVGColours g_psSVGColours[g_ColourNumber] =
{
	{"aliceblue" ,240, 248, 255 },
	{"antiquewhite" ,250, 235, 215 },
	{"aqua" , 0, 255, 255 },
	{"aquamarine" ,127, 255, 212 },
	{"azure" ,240, 255, 255 },
	{"beige" ,245, 245, 220 },
	{"bisque" ,255, 228, 196 },
	{"black" , 0, 0, 0 },
	{"blanchedalmond" ,255, 235, 205 },
	{"blue" , 0, 0, 255 },
	{"blueviolet" ,138, 43, 226 },
	{"brown" ,165, 42, 42 },
	{"burlywood" ,222, 184, 135 },
	{"cadetblue" , 95, 158, 160 },
	{"chartreuse" ,127, 255, 0 },
	{"chocolate" ,210, 105, 30 },
	{"coral" ,255, 127, 80 },
	{"cornflowerblue" ,100, 149, 237 },
	{"cornsilk" ,255, 248, 220 },
	{"crimson" ,220, 20, 60 },
	{"cyan" , 0, 255, 255 },
	{"darkblue" , 0, 0, 139 },
	{"darkcyan" , 0, 139, 139 },
	{"darkgoldenrod" ,184, 134, 11 },
	{"darkgray" ,169, 169, 169 },
	{"darkgreen" , 0, 100, 0 },
	{"darkgrey" ,169, 169, 169 },
	{"darkkhaki" ,189, 183, 107 },
	{"darkmagenta" ,139, 0, 139 },
	{"darkolivegreen" , 85, 107, 47 },
	{"darkorange" ,255, 140, 0 },
	{"darkorchid" ,153, 50, 204 },
	{"darkred" ,139, 0, 0 },
	{"darksalmon" ,233, 150, 122 },
	{"darkseagreen" ,143, 188, 143 },
	{"darkslateblue" , 72, 61, 139 },
	{"darkslategray" , 47, 79, 79 },
	{"darkslategrey" , 47, 79, 79 },
	{"darkturquoise" , 0, 206, 209 },
	{"darkviolet" ,148, 0, 211 },
	{"deeppink" ,255, 20, 147 },
	{"deepskyblue" , 0, 191, 255 },
	{"dimgray" ,105, 105, 105 },
	{"dimgrey" ,105, 105, 105 },
	{"dodgerblue" , 30, 144, 255 },
	{"firebrick" ,178, 34, 34 },
	{"floralwhite" ,255, 250, 240 },
	{"forestgreen" , 34, 139, 34 },
	{"fuchsia" ,255, 0, 255 },
	{"gainsboro" ,220, 220, 220 },
	{"ghostwhite" ,248, 248, 255 },
	{"gold" ,255, 215, 0 },
	{"goldenrod" ,218, 165, 32 },
	{"gray" ,128, 128, 128 },
	{"grey" ,128, 128, 128 },
	{"green" , 0, 128, 0 },
	{"greenyellow" ,173, 255, 47 },
	{"honeydew" ,240, 255, 240 },
	{"hotpink" ,255, 105, 180 },
	{"indianred" ,205, 92, 92 },
	{"indigo" , 75, 0, 130 },
	{"ivory" ,255, 255, 240 },
	{"khaki" ,240, 230, 140 },
	{"lavender" ,230, 230, 250 },
	{"lavenderblush" ,255, 240, 245 },
	{"lawngreen" ,124, 252, 0 },
	{"lemonchiffon" ,255, 250, 205 },
	{"lightblue" ,173, 216, 230 },
	{"lightcoral" ,240, 128, 128 },
	{"lightcyan" ,224, 255, 255 },
	{"lightgoldenrodyellow" ,250, 250, 210 },
	{"lightgray" ,211, 211, 211 },
	{"lightgreen" ,144, 238, 144 },
	{"lightgrey" ,211, 211, 211 },
	{"lightpink" ,255, 182, 193 },
	{"lightsalmon" ,255, 160, 122 },
	{"lightseagreen" , 32, 178, 170 },
	{"lightskyblue" ,135, 206, 250 },
	{"lightslategray" ,119, 136, 153 },
	{"lightslategrey" ,119, 136, 153 },
	{"lightsteelblue" ,176, 196, 222 },
	{"lightyellow" ,255, 255, 224 },
	{"lime" , 0, 255, 0 },
	{"limegreen" , 50, 205, 50 },
	{"linen" ,250, 240, 230 },
	{"magenta" ,255, 0, 255 },
	{"maroon" ,128, 0, 0 },
	{"mediumaquamarine" ,102, 205, 170 },
	{"mediumblue" , 0, 0, 205 },
	{"mediumorchid" ,186, 85, 211 },
	{"mediumpurple" ,147, 112, 219 },
	{"mediumseagreen" , 60, 179, 113 },
	{"mediumslateblue" ,123, 104, 238 },
	{"mediumspringgreen" , 0, 250, 154 },
	{"mediumturquoise" , 72, 209, 204 },
	{"mediumvioletred" ,199, 21, 133 },
	{"midnightblue" , 25, 25, 112 },
	{"mintcream" ,245, 255, 250 },
	{"mistyrose" ,255, 228, 225 },
	{"moccasin" ,255, 228, 181 },
	{"navajowhite" ,255, 222, 173 },
	{"navy" , 0, 0, 128 },
	{"oldlace" ,253, 245, 230 },
	{"olive" ,128, 128, 0 },
	{"olivedrab" ,107, 142, 35 },
	{"orange" ,255, 165, 0 },
	{"orangered" ,255, 69, 0 },
	{"orchid" ,218, 112, 214 },
	{"palegoldenrod" ,238, 232, 170 },
	{"palegreen" ,152, 251, 152 },
	{"paleturquoise" ,175, 238, 238 },
	{"palevioletred" ,219, 112, 147 },
	{"papayawhip" ,255, 239, 213 },
	{"peachpuff" ,255, 218, 185 },
	{"peru" ,205, 133, 63 },
	{"pink" ,255, 192, 203 },
	{"plum" ,221, 160, 221 },
	{"powderblue" ,176, 224, 230 },
	{"purple" ,128, 0, 128 },
	{"red" ,255, 0, 0 },
	{"rosybrown" ,188, 143, 143 },
	{"royalblue" , 65, 105, 225 },
	{"saddlebrown" ,139, 69, 19 },
	{"salmon" ,250, 128, 114 },
	{"sandybrown" ,244, 164, 96 },
	{"seagreen" , 46, 139, 87 },
	{"seashell" ,255, 245, 238 },
	{"sienna" ,160, 82, 45 },
	{"silver" ,192, 192, 192 },
	{"skyblue" ,135, 206, 235 },
	{"slateblue" ,106, 90, 205 },
	{"slategray" ,112, 128, 144 },
	{"slategrey" ,112, 128, 144 },
	{"snow" ,255, 250, 250 },
	{"springgreen" , 0, 255, 127 },
	{"steelblue" , 70, 130, 180 },
	{"tan" ,210, 180, 140 },
	{"teal" , 0, 128, 128 },
	{"thistle" ,216, 191, 216 },
	{"tomato" ,255, 99, 71 },
	{"turquoise" , 64, 224, 208 },
	{"violet" ,238, 130, 238 },
	{"wheat" ,245, 222, 179 },
	{"white" ,255, 255, 255 },
	{"whitesmoke" ,245, 245, 245 },
	{"yellow" ,255, 255, 0 },
	{"yellowgreen" ,154, 205, 50 }
};


/*!****************************************************************************
 @Class       CPVRTSVGAttribute
 @Brief       Stores the xml attribute's value and name
******************************************************************************/
class CPVRTSVGAttribute
{
public:
	/*!***************************************************************************
	 @Function		CPVRTSVGAttribute
	 @Description	Constructor
	*****************************************************************************/
	CPVRTSVGAttribute(void);

	/*!***************************************************************************
	 @Function		~CPVRTSVGAttribute
	 @Description	Destructor
	*****************************************************************************/
	~CPVRTSVGAttribute(void);

	/*!***************************************************************************
	 @Function			Destroy
	 @Description		Tidys up
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function			GetNext
	 @Returns			returns m_pNext
	 @Description		returns the next attribute in the linked list
	*****************************************************************************/
	CPVRTSVGAttribute *GetNext();

	/*!***************************************************************************
	 @Function			GetName
	 @Returns			returns the name of the attribute
	 @Description		returns the name of the attribute
	*****************************************************************************/
	char* GetName();

	/*!***************************************************************************
	 @Function			GetValue
	 @Returns			returns the value of the attribute
	 @Description		returns the value of the attribute
	*****************************************************************************/
	char* GetValue();

	/*!***************************************************************************
	 @Function			GetValueAsFloatArray
	 @Returns			Returns the value as an array of floats
	 @Modified			i32Count Returns the number of values in the array
	 @Description		Returns the value as an array of floats. The array is
						generated by splitting the value up by spaces and commas.
	*****************************************************************************/
	float* GetValueAsFloatArray(int &i32Count);

	/*!***************************************************************************
	 @Function			SetNext
	 @Input				pAttribute An attribute
	 @Description		Sets m_pNext to pAttribute
	*****************************************************************************/
	void SetNext(CPVRTSVGAttribute* pAttribute);

	/*!***************************************************************************
	 @Function			SetName
	 @Returns			returns true if successful
	 @Input				pStart
	 @Input				pEnd
	 @Description		This will set the attributes name to the string between
						pStart and pEnd. If pEnd is NULL then it is assumed that
						pStart is NULL terminated.
	*****************************************************************************/
	bool SetName(const char *pStart, const char *pEnd = 0);

	/*!***************************************************************************
	 @Function			SetValue
	 @Returns			returns true if successful
	 @Input				pStart
	 @Input				pEnd
	 @Description		This will set the attributes vale to the string between
						pStart and pEnd. If pEnd is NULL then it is assumed that
						pStart is NULL terminated.
	*****************************************************************************/
	bool SetValue(const char *pStart, const char *pEnd = 0);

	/*!***************************************************************************
	 @Function			GetValueAsStringArray
	 @Returns			Returns the value as an array of strings
	 @Modified			i32Count Returns the number of values in the array
	 @Description		Returns the value as an array of strings. The array is
						generated by splitting the value up by spaces and commas.
	*****************************************************************************/
	char** GetValueAsStringArray(int &i32Count);

	/*!***************************************************************************
	 @Function		GetValueAsFloat
	 @Returns		The attributes value as a float
	 @Description	Returns the attributes value as a float
	*****************************************************************************/
	float GetValueAsFloat();

protected:
	/*!***************************************************************************
	 @Function			DestroyValues
	 @Description		Destroys the arrays containing all the values
	*****************************************************************************/
	void DestroyValues();

	/*!***************************************************************************
	 @Function		Set
	 @Modified		pParam The output string
	 @Input			pStart The start of a string
	 @Input			pEnd	The end of a string. If 0 then it is assumed the string is null terminated.
	 @Returns 		True on success
	 @Description	Sets pParam to the string defined between pStart and pEnd
	*****************************************************************************/
	bool Set(char** pParam, const char * const pStart, const char * const pEnd);

protected:
	CPVRTSVGAttribute *m_pNext;
	char *m_pszName;
	char *m_pszValueString;
	float *m_pValueFloatArray;
	char **m_pValueStringArray;
	int m_i32Count;
};

/*!****************************************************************************
 @Class       CPVRTSVGElement
 @Brief       Represents the XML Elements
******************************************************************************/
class CPVRTSVGElement
{
public:
	/*!***************************************************************************
	 @Function		CPVRTSVGElement
	 @Description	Constructor
	*****************************************************************************/
	CPVRTSVGElement(void);

	/*!***************************************************************************
	 @Function		~CPVRTSVGElement
	 @Description	Destructor
	*****************************************************************************/
	~CPVRTSVGElement(void);

	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Tidys up
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function			SetName
	 @Returns			returns true if successful
	 @Input				pStart
	 @Input				pEnd
	 @Description		This will set the elements name to the string between
						pStart and pEnd. If pEnd is NULL then it is assumed that
						pStart is NULL terminated.
	*****************************************************************************/
	bool SetName(const char *pStart, const char *pEnd = 0);

	/*!***************************************************************************
	 @Function			SetNameSpace
	 @Returns			returns true if successful
	 @Input				pStart
	 @Input				pEnd
	 @Description		This will set the elements namespace to the string between
						pStart and pEnd. If pEnd is NULL then it is assumed that
						pStart is NULL terminated.
	*****************************************************************************/
	bool SetNameSpace(const char * const pStart, const char * const pEnd);

	/*!***************************************************************************
	 @Function			SetData
	 @Returns			returns true if successful
	 @Input				pStart
	 @Input				pEnd
	 @Description		This will set the elements data to the string between
						pStart and pEnd. If pEnd is NULL then it is assumed that
						pStart is NULL terminated.
	*****************************************************************************/
	bool SetData(const char *pStart, const char *pEnd = 0);

	/*!***************************************************************************
	 @Function			GetName
	 @Returns			Returns the elements name
	 @Description		Returns the elements name
	*****************************************************************************/
	char* GetName();

	/*!***************************************************************************
	 @Function			GetData
	 @Returns			returns the elements data
	 @Description		returns the elements data
	*****************************************************************************/
	char* GetData();

	/*!***************************************************************************
	 @Function			GetNameSpace
	 @Returns			returns the elements namespace
	 @Description		returns the elements namespace
	*****************************************************************************/
	char* GetNameSpace();

	/*!***************************************************************************
	 @Function			AddChild
	 @Returns			returns true if successful
	 @Input				pElement
	 @Description		Adds pElement as a child to the element
	*****************************************************************************/
	bool AddChild(CPVRTSVGElement *pElement);

	/*!***************************************************************************
	 @Function			RemoveChild
	 @Returns			returns true if successful
	 @Input				pElement
	 @Description		Removes pElement from the current element
	*****************************************************************************/
	bool RemoveChild(CPVRTSVGElement *pElement);

	/*!***************************************************************************
	 @Function			AddAttribute
	 @Returns			returns true if successful
	 @Input				pAttribute
	 @Description		Adds pAttribute to this element
	*****************************************************************************/
	bool AddAttribute(CPVRTSVGAttribute *pAttribute);

	/*!***************************************************************************
	 @Function			SetNext
	 @Input				pElement
	 @Description		sets m_pNext to pElement
	*****************************************************************************/
	void SetNext(CPVRTSVGElement* pElement);

	/*!***************************************************************************
	 @Function			GetNext
	 @Returns			returns m_pNext
	 @Description		returns the next element in the list
	*****************************************************************************/
	CPVRTSVGElement* GetNext();

	/*!***************************************************************************
	 @Function			GetFirstChild
	 @Returns			returns the first child of this element
	 @Description		returns the first child of this element
	*****************************************************************************/
	CPVRTSVGElement* GetFirstChild();

	/*!***************************************************************************
	 @Function			GetFirstAttribute
	 @Returns			Returns the first attribute
	 @Description		Returns the first attribute
	*****************************************************************************/
	CPVRTSVGAttribute* GetFirstAttribute();
	void SetClosed(bool bClosed) { m_bIsClosed = bClosed; }
	bool IsClosed() { return m_bIsClosed; }
	void SetWantsChildren(bool bWants) { m_bWantsChildren = bWants; }
	bool WantsChildren() { return m_bWantsChildren; }
	unsigned int GetNoOfChildren() { return m_ui32NoOfChildren; }

protected:
	/*!***************************************************************************
	 @Function		Set
	 @Modified		pParam The output string
	 @Input			pStart The start of a string
	 @Input			pEnd	The end of a string. If 0 then it is assumed the string is null terminated.
	 @Returns 		True on success
	 @Description	Sets pParam to the string defined between pStart and pEnd
	*****************************************************************************/
	bool Set(char** pParam, const char * const pStart, const char * const pEnd);

protected:
	char* m_pszName;
	char* m_pszData;
	char* m_pszNameSpace;

	CPVRTSVGElement* m_pFirstChild;
	CPVRTSVGElement* m_pLastChild;

	CPVRTSVGAttribute* m_pFirstAttribute;
	CPVRTSVGAttribute* m_pLastAttribute;

	CPVRTSVGElement* m_pNext;
	bool m_bIsClosed;
	bool m_bWantsChildren;
	unsigned int m_ui32NoOfChildren;
};

/*!****************************************************************************
 @Class      CPVRTSVGParser
 @Brief      The main parser that goes through the file creating the XML tree
******************************************************************************/
class CPVRTSVGParser
{
public:
	/*!***************************************************************************
	 @Function		CPVRTSVGParser
	 @Description	Constructor
	*****************************************************************************/
	CPVRTSVGParser(void);

	/*!***************************************************************************
	 @Function		~CPVRTSVGParser
	 @Description	Destructor
	*****************************************************************************/
	~CPVRTSVGParser(void);

	/*!***************************************************************************
	 @Function			Load
	 @Input				pFileName	The filename of the SVG file to load
	 @Modified			pObject		The SVG Object to load the data into
	 @Input				ui32Width	Screen width
	 @Input				ui32Height	Screen height
	 @Returns			Returns true if successful
	 @Description		Opens up and parses the file pFileName.
	*****************************************************************************/
	bool Load(const char * pFileName, CPVRTSVGObject *pObject, unsigned int ui32Width, unsigned int ui32Height);

	/*!***************************************************************************
	 @Function			Destroy
	 @Description		Tidys up
	*****************************************************************************/
	void Destroy();

protected:
	/*!***************************************************************************
	 @Function			GetRoot
	 @Returns			Returns the root element of the XML tree
	 @Description		Returns the root element of the XML tree
	*****************************************************************************/
	CPVRTSVGElement *GetRoot();

	/*!***************************************************************************
	 @Function			CreateTree
	 @Input				pXMLData - the entire XML data
	 @Returns			Returns true if successful
	 @Description		Creates a tree from the XML data stored in pXMLData
	*****************************************************************************/
	bool CreateTree(char* pXMLData);

	/*!***************************************************************************
	 @Function			Process
	 @Input				pXMLData The XML data
	 @Input				pParent  The parent to add any new elements to
	 @Input				pParentNode The parent node to add any new nodes to
	 @Returns			Returns the remainder of the XML file if successful
	 @Description		Processes the data in pXMLData to determine what needs to
						be done.
	*****************************************************************************/
	char* Process(char* pXMLData, CPVRTSVGElement *pParent = 0, CPVRTSVGNode *pParentNode = 0);

	/*!***************************************************************************
	 @Function			AddAttribute
	 @Input				pElement The element to add the attribute to
	 @Input				pXMLData The XML data
	 @Input				bStyle Is the attribute a style
	 @Returns			Returns the remainder of the XML file if successful
	 @Description		Adds an attribute to pElement
	*****************************************************************************/
	char* AddAttribute(CPVRTSVGElement *pElement, char* pXMLData, bool bStyle = false);

	/*!***************************************************************************
	 @Function		AddAttributes
	 @Input			pElement The SVG element to add the attribute to
	 @Input			pXMLData The XML data
	 @Returns		An updated pointer to the XMLData
	 @Description	Partially parses the XMLData and adds the attributes to pElement
	*****************************************************************************/
	char* AddAttributes(CPVRTSVGElement *pElement, char* pXMLData);

	/*!***************************************************************************
	 @Function			AddCDATA
	 @Input				pXMLData - the entire XML data
	 @Input				pParent  - the parent to add the data to
	 @Returns			Returns the remainder of the XML file if successful
	 @Description		Adds the contents of the CData block to pParent
	*****************************************************************************/
	char* AddCDATA(char* pXMLData, CPVRTSVGElement *pParent);

	/*!***************************************************************************
	 @Function			AddElement
	 @Input				pXMLData the entire XML data
	 @Input				pParent	 The element to add the new element to
	 @Input				pParentNode The node to add the any new nodes to
	 @Returns			Returns the remainder of the XML file if successful
	 @Description		Adds an element as a child to an already existing element
	*****************************************************************************/
	char* AddElement(char* pXMLData, CPVRTSVGElement *pParent, CPVRTSVGNode *pParentNode);

	/*!***************************************************************************
	 @Function		AddChildren
	 @Input			pParent The parent SVG element to add the children to
	 @Input			pParentNode The parent SVG node to add the children to
	 @Input			pXMLData the xml data
	 @Returns		An updated pointer to the XMLData
	 @Description	Partially parses the XMLData and adds the children to the parents
	*****************************************************************************/
	char* AddChildren(CPVRTSVGElement *pParent, CPVRTSVGNode* pParentNode, char* pXMLData);

	/*!***************************************************************************
	 @Function			SkipElement
	 @Returns			Returns the remainder of the XML file if successful
	 @Input				pXMLData - the entire XML data
	 @Description		Skips an XML element in the file
	*****************************************************************************/
	char* SkipElement(char* pXMLData);

	/*!***************************************************************************
	 @Function		Create
	 @Input			pElement An SVG element
	 @Input			pParent The new node's parent
	 @Returns		A new SVG node
	 @Description	Creates an SVG node from an SVG element
	*****************************************************************************/
	CPVRTSVGNode* Create(CPVRTSVGElement *pElement, CPVRTSVGNode *pParent);

	/*!***************************************************************************
	 @Function		CreateDummy
	 @Input			pElement An SVG element
	 @Input			pParent The new node's parent
	 @Returns		A new SVG node
	 @Description	Creates a dummy node from an SVG element
	*****************************************************************************/
	CPVRTSVGNode* CreateDummy(CPVRTSVGElement *pElement, CPVRTSVGNode *pParent);

	/*!***************************************************************************
	 @Function		CreateGradient
	 @Input			pElement An SVG element
	 @Input			pXMLData The XML data
	 @Returns		true on success
	 @Description	Creates a gradient
	*****************************************************************************/
	bool CreateGradient(CPVRTSVGElement *pElement, char **pXMLData);

	/*!***************************************************************************
	 @Function		CreateVGPath
	 @Modified		pPath The path class to initialise
	 @Input			pSVGPathData The SVG path data
	 @Returns		True on success
	 @Description	Creates an OpenVG path from SVG data
	*****************************************************************************/
	bool CreateVGPath(CPVRTPath* pPath, const char * pSVGPathData);

	/*!***************************************************************************
	 @Function		ExtractFloat
	 @Input			ppzString a point to the string
	 @Modified		pfFloat the place where to put the float when extracted
	 @Returns		True on success
	 @Description	Extracts a float from a string
	*****************************************************************************/
	bool ExtractFloat(const char ** ppzString,float *pfFloat);

	/*!***************************************************************************
	 @Function		IsShape
	 @Input			pElement An SVG element
	 @Returns		True if pElement represents a shape
	 @Description	Returns true if pElement represents a shape
	*****************************************************************************/
	bool IsShape(CPVRTSVGElement *pElement);

	/*!***************************************************************************
	 @Function		IsDummy
	 @Input			pElement An SVG element
	 @Returns		True if pElement represents a dummy element
	 @Description	Returns true if pElement represents a dummy element
	*****************************************************************************/
	bool IsDummy(CPVRTSVGElement *pElement);

	/*!***************************************************************************
	 @Function		IsGradient
	 @Input			pElement An SVG element
	 @Returns		True if pElement represents a gradient
	 @Description	Returns true if pElement represents a gradient
	*****************************************************************************/
	bool IsGradient(CPVRTSVGElement *pElement);

	/*!***************************************************************************
	 @Function	ExtractColor
	 @Input		pzString	A string
	 @Modified	pfFloat		A float array to place the RGBA colour
	 @Returns 	True on success
	 @Description Extracts the colour from the string and places it in pfFloat
	*****************************************************************************/
	bool ExtractColor(char * pzString, float *pfFloat);

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
	bool IsColorKeyWord(char * pzString, float *pfFloat, char * pzColorKeyWord,int r,int g,int b);

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
	bool GetPointsAndCommands(const char* pData, char* pCommands, float* pPoints, int &i32CommandSize, int &i32PointsSize, bool bPass1 = false);

	/*!***************************************************************************
	 @Function			IsCommand
	 @Input				pc	Input char
	 @Modified			i32Points The number of points
	 @Returns			True if successful
	 @Description		Tells you whether a char is an SVG path command or not. If
	 					it is then it also sets i32Points to the number of associated
	 					points.
	*****************************************************************************/
	bool IsCommand(const char *pc, int &i32Points);

	/*!***************************************************************************
	 @Function	GetCommand
	 @Input		pc	The SVG path command
	 @Modified	i32Command	The output for the openvg command
	 @Modified	i32Points	The output for the number of points
	 @Returns	True on success
	 @Description Takes an SVG path command and returns the openvg version and the
	 			  number of associated points.
	*****************************************************************************/
	bool GetCommand(const char *pc, int &i32Command, int &i32Points);

	/*!***************************************************************************
	 @Function		ConvertTransform
	 @Modified		pMatrix The matrix to set to the transformation
	 @Input			pTransform The SVG transformation
	 @Returns		True if successful
	 @Description	Converts an SVG transformation to a PVRTMat3
	*****************************************************************************/
	bool ConvertTransform(PVRTMat3* pMatrix, char* pTransform);

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
	CPVRTPaint* CreatePaint(CPVRTSVGNode *pNode, char *pValue, float fOpacity, VGPaintMode iType, CPVRTSVGNode *pParent);

	/*!***************************************************************************
	 @Function		CopyPaint
	 @Input			pPaint The paint to copy
	 @Input			fOpacity An opacity value to apply to the paint
	 @Returns		A new paint
	 @Description	Copies a paint and applies and alters its transparancy
	*****************************************************************************/
	CPVRTPaint* CopyPaint(CPVRTPaint *pPaint, float fOpacity = 1.0f);

protected:
	CPVRTSVGElement *m_pRootElement;
	CPVRTSVGObject *m_pObject;
	CPVRTPaint *m_pDefaultFillPaint;
};


#endif /* __PVRTSVGParser_H__ */
/******************************************************************************
 End of file PVRTSVGParser.h
******************************************************************************/
