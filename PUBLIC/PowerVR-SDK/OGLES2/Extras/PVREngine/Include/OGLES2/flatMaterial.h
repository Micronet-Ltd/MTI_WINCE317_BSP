/*!***************************************************************************
 @File			OGLES2\flatMaterial.h

 @Brief			Default ambient/diffuse PFX

 @Author        PowerVR

 @Date			11/10/2007

 @Copyright		Copyright 2007 by Imagination Technologies Limited.
				All rights reserved. No part of this software, either
				material or conceptual may be copied or distributed,
				transmitted, transcribed, stored in a retrieval system
				or translated into any human or computer language in any
				form by any means, electronic, mechanical, manual or
				other-wise, or disclosed to third parties without the
				express written permission of Imagination Technologies
				Limited, Unit 8, HomePark Industrial Estate,
				King's Langley, Hertfordshire, WD4 8LZ, U.K.

 @Platform		ANSI compatible

 @Description	A default PFX to be used with the PVREngine when no other
 				PFX is specified.

 $Revision: 1.7 $
*****************************************************************************/

#ifndef _FLAT_MATERIAL_H_
#define _FLAT_MATERIAL_H_

const char flatMaterial[] =
"[HEADER]\n"
"	VERSION		01.00.00.00\n"
"	DESCRIPTION Flat material\n"
"	COPYRIGHT	Img Tec\n"
"[/HEADER]\n"

"[EFFECT]\n"
"	NAME 	Flat\n"

	// GLOBALS UNIFORMS
"	UNIFORM myWVPMatrix 	WORLDVIEWPROJECTION\n"

	// ATTRIBUTES
"	ATTRIBUTE 	myVertex	POSITION\n"

"	VERTEXSHADER MyVertexShader\n"
"	FRAGMENTSHADER MyFragmentShader\n"

"[/EFFECT]\n"

"[VERTEXSHADER]\n"
"	NAME 		MyVertexShader\n"

	// LOAD GLSL AS CODE
"	[GLSL_CODE]\n"
"	attribute highp vec3	myVertex;\n"
"	uniform highp mat4	myWVPMatrix;\n"

"		void main(void)\n"
"		{\n"
"			gl_Position = myWVPMatrix * vec4(myVertex,1.0);\n"
"		}\n"
"	[/GLSL_CODE]\n"
"[/VERTEXSHADER]\n"

"[FRAGMENTSHADER]\n"
"	NAME 		MyFragmentShader\n"

	// LOAD GLSL AS CODE
"	[GLSL_CODE]\n"

"		void main (void)\n"
"		{\n"
"			gl_FragColor =  vec4(1.0,0.0,0.0,1.0);\n"
"		}\n"
"	[/GLSL_CODE]\n"
"[/FRAGMENTSHADER]\n";

#endif // _FLAT_MATERIAL_H_
