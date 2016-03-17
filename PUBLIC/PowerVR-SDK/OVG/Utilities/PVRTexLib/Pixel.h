/******************************************************************************
* Name			: Pixel.h
* Author		: PowerVR
*
* Created		: April 2006
*
*
* Copyright	: 2005 by Imagination Technologies Limited. All rights reserved.
*				: No part of this software, either material or conceptual
*				: may be copied or distributed, transmitted, transcribed,
*				: stored in a retrieval system or translated into any
*				: human or computer language in any form by any means,
*				: electronic, mechanical, manual or other-wise, or
*				: disclosed to third parties without the express written
*				: permission of VideoLogic Limited, Unit 8, HomePark
*				: Industrial Estate, King's Langley, Hertfordshire,
*				: WD4 8LZ, nU.K.
* 
* Description	: Pixel structs.
*
* Platform		: ANSI
******************************************************************************/

#ifndef PIXEL_H
#define PIXEL_H

#include <assert.h>

namespace pvrtexlib
{
	// note order of enums reflecta order of members of Pixel struct
	enum eColourChannel
	{
		ePIXEL_Red=0,
		ePIXEL_Green,
		ePIXEL_Blue,
		ePIXEL_Alpha,
		ePIXEL_None
	};

	// Pixel structures for standard pixel types
	template<typename channelType> struct Pixel
	{
		typedef channelType chanType;
		channelType Red, Green, Blue, Alpha;

		Pixel(){}
		Pixel(unsigned int u32Colour);

		Pixel(channelType nRed, channelType nGreen, channelType nBlue, channelType nAlpha)
			:Red(nRed),Green(nGreen),Blue(nBlue),Alpha(nAlpha){}

		unsigned int	toUnsignedInt();
		inline channelType		getMaxChannelValue() const;
		inline int				getSizeInBytes() const;

		channelType& operator[](const unsigned int channel)
		{
			assert(channel<4);
			return ((channelType*)(&Red))[channel];
		}

		Pixel<channelType> operator+(const Pixel& b)
		{
			return Pixel<channelType>(Red+b.Red,Green+b.Green,Blue+b.Blue,Alpha+b.Alpha);
		}

		Pixel<channelType> operator*(const Pixel<channelType>& b)
		{
			return Pixel<channelType>(Red*b.Red,Green*b.Green,Blue*b.Blue,Alpha*b.Alpha);
		}

		friend inline	Pixel<channelType> operator*(const float f,const Pixel<channelType>& b)
		{
			return Pixel<channelType>((channelType)(f*(float)b.Red),
				(channelType)(f*(float)b.Green),
				(channelType)(f*(float)b.Blue),
				(channelType)(f*(float)b.Alpha));
		}
	};

	template<typename channelType>
	inline int Pixel<channelType>::getSizeInBytes() const
	{
		return sizeof(channelType);
	}


	// specialisations for uint8
	template<>
	inline Pixel<uint8>::Pixel(const unsigned int u32Colour)
	{
		Alpha = uint8(u32Colour>>24);
		Blue = uint8((u32Colour>>16)&0xff);
		Green = uint8((u32Colour>>8)&0xff);
		Red = uint8(u32Colour&0xff);
	}

	template<>
	inline unsigned int	Pixel<uint8>::toUnsignedInt()
	{
		return ((unsigned int)(Alpha)<<24)
			|((unsigned int) (Blue)<<16)
			|((unsigned int) (Green)<<8)
			|(unsigned int) (Red);
	}

	template<>
	inline uint8		Pixel<uint8>::getMaxChannelValue() const
	{
		return 0xFF;
	}


	// specialisations for uint16
	template<>
	inline uint16		Pixel<uint16>::getMaxChannelValue() const
	{
		return 0xFFFF;
	}

	// specialisations for uint32
	template<>
	inline uint32		Pixel<uint32>::getMaxChannelValue() const
	{
		return 0xFFFFFFFF;
	}


	// specialisations for float
	template<>
	inline float32		Pixel<float32>::getMaxChannelValue() const
	{
		return 1.0f;
	}



	// convenience struct for PixelFormats
	template<typename channelType> struct PixelRGB
	{
		channelType Red;
		channelType Green;
		channelType Blue;
	};

	// convenience struct for PixelFormats
	template<typename channelType> struct PixelRG
	{
		channelType Red;
		channelType Green;
	};

}
#endif // PIXEL_H

/*****************************************************************************
End of file (Pixel.h)
*****************************************************************************/
