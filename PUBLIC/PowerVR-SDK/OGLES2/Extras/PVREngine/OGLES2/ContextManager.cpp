/*!***************************************************************************
 @File			ContextManager.cpp

 @Brief			Manages contexts so that extensions can be accessed.

 @Author        PowerVR

 @Date			August 1999

 @Copyright		Copyright 2003-2004 by Imagination Technologies Limited.
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

 @Description	Manages contexts so that extensions can be accessed. TODO: make work with multiple contexts

 $Revision: 1.6 $
*****************************************************************************/

#include "ContextManager.h"

namespace pvrengine
{



/****************************************************************************
** Functions
****************************************************************************/

	/******************************************************************************/

	ContextManager::ContextManager():
		m_daContext(dynamicArray<SPVRTContext>(1)),	// unlikely to use more than one context
			m_i32CurrentContext(0)
	{
	}

	/******************************************************************************/

	CPVRTgles2Ext* ContextManager::getExtensions()
	{
		return NULL;	// no extensions in OGLES2
	}

	/******************************************************************************/

	SPVRTContext* ContextManager::getCurrentContext()
	{
		if(m_i32CurrentContext>=0)
			return &m_daContext[m_i32CurrentContext];
		else
			return NULL;
	}

	/******************************************************************************/

	void ContextManager::initContext()
	{
		m_i32CurrentContext = 0;
	}

	/******************************************************************************/

	int ContextManager::addNewContext()
	{
		m_daContext.expandToSize(m_daContext.getSize()+1);
		return m_daContext.getSize();
	}

	/******************************************************************************/

	void ContextManager::setCurrentContext(const unsigned int i32Context)
	{
		if(i32Context>0 && i32Context<m_daContext.getSize())
		{
			m_i32CurrentContext = i32Context;
		}
	}

}

/*****************************************************************************
 End of file (ContextManager.cpp)
*****************************************************************************/
