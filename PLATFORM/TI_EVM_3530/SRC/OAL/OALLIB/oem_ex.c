/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module oem_mem.c
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   30-Nov-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

//================================================================================
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
#include <bsp.h>
#include <bsp_version.h>
#include <bldver.h>
#include <oalex.h>
#include <oal_memory.h>

#if 0
// ##### Michael. 6-Dec-2009
// Convertion table for IQR to Virtual Installable IRQ number
// Need for solving problem of IRQ num restriction (up to 256)
// in Winodws functions 'NKCallIntChain' and 'LoadIntChainHandler'

BOOL OALgetVIIRQ( UINT32 irqNum, BYTE *pVIIRQ );

const BYTE g_IRQ2VIIRQ[OMAP_IRQ_MAXIMUM] =
{
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 IRQ_GPTIMER3, //  the same num, using table is not mandatory for these timers,  39  // GPTimer module 3
 IRQ_GPTIMER4, //                   40  // GPTimer module 4
 IRQ_GPTIMER5, //                   41  // GPTimer module 5
 IRQ_GPTIMER6, //                   42  // GPTimer module 6
 IRQ_GPTIMER7, //                   43  // GPTimer module 7
 IRQ_GPTIMER8, //                   44  // GPTimer module 8
 IRQ_GPTIMER9, //                   45  // GPTimer module 9
 IRQ_GPTIMER10, //                  46  // GPTimer module 10
 IRQ_GPTIMER11, //                  47  // GPTimer module 11
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 64, //  IRQ_GPIO_0,  //128         // GPIO1 bit 0
 65, //  IRQ_GPIO_1,  //129         // GPIO1 bit 1
 66, //  IRQ_GPIO_2,  //130         // GPIO1 bit 2
 67, //  IRQ_GPIO_3,  //131         // GPIO1 bit 3
 68, //  IRQ_GPIO_4,  //132         // GPIO1 bit 4
 69, //  IRQ_GPIO_5,  //133         // GPIO1 bit 5
 70, //  IRQ_GPIO_6,  //134         // GPIO1 bit 6
 71, //  IRQ_GPIO_7,  //135         // GPIO1 bit 7
 72, //  IRQ_GPIO_8,  //136         // GPIO1 bit 8
 73, //  IRQ_GPIO_9,  //137         // GPIO1 bit 9
 74, //  IRQ_GPIO_10,  //138         // GPIO1 bit 10
 75, //  IRQ_GPIO_11,  //139         // GPIO1 bit 11
 76, //  IRQ_GPIO_12,  //140         // GPIO1 bit 12
 77, //  IRQ_GPIO_13,  //141         // GPIO1 bit 13
 78, //  IRQ_GPIO_14,  //142         // GPIO1 bit 14
 79, //  IRQ_GPIO_15,  //143         // GPIO1 bit 15
 80, //  IRQ_GPIO_16,  //144         // GPIO1 bit 16
 81, //  IRQ_GPIO_17,  //145         // GPIO1 bit 17
 82, //  IRQ_GPIO_18,  //146         // GPIO1 bit 18
 83, //  IRQ_GPIO_19,  //147         // GPIO1 bit 19
 84, //  IRQ_GPIO_20,  //148         // GPIO1 bit 20
 85, //  IRQ_GPIO_21,  //149         // GPIO1 bit 21
 86, //  IRQ_GPIO_22,  //150         // GPIO1 bit 22
 87, //  IRQ_GPIO_23,  //151         // GPIO1 bit 23
 88, //  IRQ_GPIO_24,  //152         // GPIO1 bit 24
 89, //  IRQ_GPIO_25,  //153         // GPIO1 bit 25
 90, //  IRQ_GPIO_26,  //154         // GPIO1 bit 26
 91, //  IRQ_GPIO_27,  //155         // GPIO1 bit 27
 92, //  IRQ_GPIO_28,  //156         // GPIO1 bit 28
 93, //  IRQ_GPIO_29,  //157         // GPIO1 bit 29
 94, //  IRQ_GPIO_30,  //158         // GPIO1 bit 30
 95, //  IRQ_GPIO_31,  //159         // GPIO1 bit 31
 96, //  IRQ_GPIO_32,  //160         // GPIO2 bit 0
 97, //  IRQ_GPIO_33,  //161         // GPIO2 bit 1
 98, //  IRQ_GPIO_34,  //162         // GPIO2 bit 2
 99, //  IRQ_GPIO_35,  //163         // GPIO2 bit 3
 100, //  IRQ_GPIO_36,  //164         // GPIO2 bit 4
 101, //  IRQ_GPIO_37,  //165         // GPIO2 bit 5
 102, //  IRQ_GPIO_38,  //166         // GPIO2 bit 6
 103, //  IRQ_GPIO_39,  //167         // GPIO2 bit 7
 104, //  IRQ_GPIO_40,  //168         // GPIO2 bit 8
 105, //  IRQ_GPIO_41,  //169         // GPIO2 bit 9
 106, //  IRQ_GPIO_42,  //170         // GPIO2 bit 10
 107, //  IRQ_GPIO_43,  //171         // GPIO2 bit 11
 108, //  IRQ_GPIO_44,  //172         // GPIO2 bit 12
 109, //  IRQ_GPIO_45,  //173         // GPIO2 bit 13
 110, //  IRQ_GPIO_46,  //174         // GPIO2 bit 14
 111, //  IRQ_GPIO_47,  //175         // GPIO2 bit 15
 112, //  IRQ_GPIO_48,  //176         // GPIO2 bit 16
 113, //  IRQ_GPIO_49,  //177         // GPIO2 bit 17
 114, //  IRQ_GPIO_50,  //178         // GPIO2 bit 18
 115, //  IRQ_GPIO_51,  //179         // GPIO2 bit 19
 116, //  IRQ_GPIO_52,  //180         // GPIO2 bit 20
 117, //  IRQ_GPIO_53,  //181         // GPIO2 bit 21
 118, //  IRQ_GPIO_54,  //182         // GPIO2 bit 22
 119, //  IRQ_GPIO_55,  //183         // GPIO2 bit 23
 120, //  IRQ_GPIO_56,  //184         // GPIO2 bit 24
 121, //  IRQ_GPIO_57,  //185         // GPIO2 bit 25
 122, //  IRQ_GPIO_58,  //186         // GPIO2 bit 26
 123, //  IRQ_GPIO_59,  //187         // GPIO2 bit 27
 124, //  IRQ_GPIO_60,  //188         // GPIO2 bit 28
 125, //  IRQ_GPIO_61,  //189         // GPIO2 bit 29
 126, //  IRQ_GPIO_62,  //190         // GPIO2 bit 30
 127, //  IRQ_GPIO_63,  //191         // GPIO2 bit 31
 128, //  IRQ_GPIO_64,  //192         // GPIO3 bit 0
 129, //  IRQ_GPIO_65,  //193         // GPIO3 bit 1
 130, //  IRQ_GPIO_66,  //194         // GPIO3 bit 2
 131, //  IRQ_GPIO_67,  //195         // GPIO3 bit 3
 132, //  IRQ_GPIO_68,  //196         // GPIO3 bit 4
 133, //  IRQ_GPIO_69,  //197         // GPIO3 bit 5
 134, //  IRQ_GPIO_70,  //198         // GPIO3 bit 6
 135, //  IRQ_GPIO_71,  //199         // GPIO3 bit 7
 136, //  IRQ_GPIO_72,  //200         // GPIO3 bit 8
 137, //  IRQ_GPIO_73,  //201         // GPIO3 bit 9
 138, //  IRQ_GPIO_74,  //202         // GPIO3 bit 10
 139, //  IRQ_GPIO_75,  //203         // GPIO3 bit 11
 140, //  IRQ_GPIO_76,  //204         // GPIO3 bit 12
 141, //  IRQ_GPIO_77,  //205         // GPIO3 bit 13
 142, //  IRQ_GPIO_78,  //206         // GPIO3 bit 14
 143, //  IRQ_GPIO_79,  //207         // GPIO3 bit 15
 144, //  IRQ_GPIO_80,  //208         // GPIO3 bit 16
 145, //  IRQ_GPIO_81,  //209         // GPIO3 bit 17
 146, //  IRQ_GPIO_82,  //210         // GPIO3 bit 18
 147, //  IRQ_GPIO_83,  //211         // GPIO3 bit 19
 148, //  IRQ_GPIO_84,  //212         // GPIO3 bit 20
 149, //  IRQ_GPIO_85,  //213         // GPIO3 bit 21
 150, //  IRQ_GPIO_86,  //214         // GPIO3 bit 22
 151, //  IRQ_GPIO_87,  //215         // GPIO3 bit 23
 152, //  IRQ_GPIO_88,  //216         // GPIO3 bit 24
 153, //  IRQ_GPIO_89,  //217         // GPIO3 bit 25
 154, //  IRQ_GPIO_90,  //218         // GPIO3 bit 26
 155, //  IRQ_GPIO_91,  //219         // GPIO3 bit 27
 156, //  IRQ_GPIO_92,  //220         // GPIO3 bit 28
 157, //  IRQ_GPIO_93,  //221         // GPIO3 bit 29
 158, //  IRQ_GPIO_94,  //222         // GPIO3 bit 30
 159, //  IRQ_GPIO_95,  //223         // GPIO3 bit 31
 160, //  IRQ_GPIO_96,  //224         // GPIO4 bit 0
 161, //  IRQ_GPIO_97,  //225         // GPIO4 bit 1
 162, //  IRQ_GPIO_98,  //226         // GPIO4 bit 2
 163, //  IRQ_GPIO_99,  //227         // GPIO4 bit 3
 164, //  IRQ_GPIO_100,  //228         // GPIO4 bit 4
 165, //  IRQ_GPIO_101,  //229         // GPIO4 bit 5
 166, //  IRQ_GPIO_102,  //230         // GPIO4 bit 6
 167, //  IRQ_GPIO_103,  //231         // GPIO4 bit 7
 168, //  IRQ_GPIO_104,  //232         // GPIO4 bit 8
 169, //  IRQ_GPIO_105,  //233         // GPIO4 bit 9
 170, //  IRQ_GPIO_106,  //234         // GPIO4 bit 10
 171, //  IRQ_GPIO_107,  //235         // GPIO4 bit 11
 172, //  IRQ_GPIO_108,  //236         // GPIO4 bit 12
 173, //  IRQ_GPIO_109,  //237         // GPIO4 bit 13
 174, //  IRQ_GPIO_110,  //238         // GPIO4 bit 14
 175, //  IRQ_GPIO_111,  //239         // GPIO4 bit 15
 176, //  IRQ_GPIO_112,  //240         // GPIO4 bit 16
 177, //  IRQ_GPIO_113,  //241         // GPIO4 bit 17
 178, //  IRQ_GPIO_114,  //242         // GPIO4 bit 18
 179, //  IRQ_GPIO_115,  //243         // GPIO4 bit 19
 180, //  IRQ_GPIO_116,  //244         // GPIO4 bit 20
 181, //  IRQ_GPIO_117,  //245         // GPIO4 bit 21
 182, //  IRQ_GPIO_118,  //246         // GPIO4 bit 22
 183, //  IRQ_GPIO_119,  //247         // GPIO4 bit 23
 184, //  IRQ_GPIO_120,  //248         // GPIO4 bit 24
 185, //  IRQ_GPIO_121,  //249         // GPIO4 bit 25
 186, //  IRQ_GPIO_122,  //250         // GPIO4 bit 26
 187, //  IRQ_GPIO_123,  //251         // GPIO4 bit 27
 188, //  IRQ_GPIO_124,  //252         // GPIO4 bit 28
 189, //  IRQ_GPIO_125,  //253         // GPIO4 bit 29
 190, //  IRQ_GPIO_126,  //254         // GPIO4 bit 30
 191, //  IRQ_GPIO_127,  //255         // GPIO4 bit 21
 192, //  IRQ_GPIO_128,  //256         // GPIO4 bit 0
 193, //  IRQ_GPIO_129,  //257         // GPIO5 bit 1
 194, //  IRQ_GPIO_130,  //258         // GPIO5 bit 2
 195, //  IRQ_GPIO_131,  //259         // GPIO5 bit 3
 196, //  IRQ_GPIO_132,  //260         // GPIO5 bit 4
 197, //  IRQ_GPIO_133,  //261         // GPIO5 bit 5
 198, //  IRQ_GPIO_134,  //262         // GPIO5 bit 6
 199, //  IRQ_GPIO_135,  //263         // GPIO5 bit 7
 200, //  IRQ_GPIO_136,  //264         // GPIO5 bit 8
 201, //  IRQ_GPIO_137,  //265         // GPIO5 bit 9
 202, //  IRQ_GPIO_138,  //266         // GPIO5 bit 10
 203, //  IRQ_GPIO_139,  //267         // GPIO5 bit 11
 204, //  IRQ_GPIO_140,  //268         // GPIO5 bit 12
 205, //  IRQ_GPIO_141,  //269         // GPIO5 bit 13
 206, //  IRQ_GPIO_142,  //270         // GPIO5 bit 14
 207, //  IRQ_GPIO_143,  //271         // GPIO5 bit 15
 208, //  IRQ_GPIO_144,  //272         // GPIO5 bit 16
 209, //  IRQ_GPIO_145,  //273         // GPIO5 bit 17
 210, //  IRQ_GPIO_146,  //274         // GPIO5 bit 18
 211, //  IRQ_GPIO_147,  //275         // GPIO5 bit 19
 212, //  IRQ_GPIO_148,  //276         // GPIO5 bit 20
 213, //  IRQ_GPIO_149,  //277         // GPIO5 bit 21
 214, //  IRQ_GPIO_150,  //278         // GPIO5 bit 22
 215, //  IRQ_GPIO_151,  //279         // GPIO5 bit 23
 216, //  IRQ_GPIO_152,  //280         // GPIO5 bit 24
 217, //  IRQ_GPIO_153,  //281         // GPIO5 bit 25
 218, //  IRQ_GPIO_154,  //282         // GPIO5 bit 26
 219, //  IRQ_GPIO_155,  //283         // GPIO5 bit 27
 220, //  IRQ_GPIO_156,  //284         // GPIO5 bit 28
 221, //  IRQ_GPIO_157,  //285         // GPIO5 bit 29
 222, //  IRQ_GPIO_158,  //286         // GPIO5 bit 30
 223, //  IRQ_GPIO_159,  //287         // GPIO5 bit 31
 224, //  IRQ_GPIO_160,  //288         // GPIO4 bit 0
 225, //  IRQ_GPIO_161,  //289         // GPIO5 bit 1
 226, //  IRQ_GPIO_162,  //290         // GPIO5 bit 2
 227, //  IRQ_GPIO_163,  //291         // GPIO5 bit 3
 228, //  IRQ_GPIO_164,  //292         // GPIO5 bit 4
 229, //  IRQ_GPIO_165,  //293         // GPIO5 bit 5
 230, //  IRQ_GPIO_166,  //294         // GPIO5 bit 6
 231, //  IRQ_GPIO_167,  //295         // GPIO5 bit 7
 232, //  IRQ_GPIO_168,  //296         // GPIO5 bit 8
 233, //  IRQ_GPIO_169,  //297         // GPIO5 bit 9
 234, //  IRQ_GPIO_170,  //298         // GPIO5 bit 10
 235, //  IRQ_GPIO_171,  //299         // GPIO5 bit 11
 236, //  IRQ_GPIO_172,  //300         // GPIO5 bit 12
 237, //  IRQ_GPIO_173,  //301         // GPIO5 bit 13
 238, //  IRQ_GPIO_174,  //302         // GPIO5 bit 14
 239, //  IRQ_GPIO_175,  //303         // GPIO5 bit 15
 240, //  IRQ_GPIO_176,  //304         // GPIO5 bit 16
 241, //  IRQ_GPIO_177,  //305         // GPIO5 bit 17
 242, //  IRQ_GPIO_178,  //306         // GPIO5 bit 18
 243, //  IRQ_GPIO_179,  //307         // GPIO5 bit 19
 244, //  IRQ_GPIO_180,  //308         // GPIO5 bit 20
 245, //  IRQ_GPIO_181,  //309         // GPIO5 bit 21
 246, //  IRQ_GPIO_182,  //310         // GPIO5 bit 22
 247, //  IRQ_GPIO_183,  //311         // GPIO5 bit 23
 248, //  IRQ_GPIO_184,  //312         // GPIO5 bit 24
 249, //  IRQ_GPIO_185,  //313         // GPIO5 bit 25
 250, //  IRQ_GPIO_186,  //314         // GPIO5 bit 26
 251, //  IRQ_GPIO_187,  //315         // GPIO5 bit 27
 252, //  IRQ_GPIO_188,  //316         // GPIO5 bit 28
 253, //  IRQ_GPIO_189,  //317         // GPIO5 bit 29
 254, //  IRQ_GPIO_190,  //318         // GPIO5 bit 30
 255, //  IRQ_GPIO_191,  //319         // GPIO5 bit 31
};



//------------------------------------------------------------------------------
//
//  Function:  OALgetVIIRQ
//
//  This function get virtual IRQ for Installable Interrupt function
//  using only: LoadIntChainHandler and NKCallIntChain
//
BOOL OALgetVIIRQ( UINT32 irqNum, BYTE *pVIIRQ )
{
	if( pVIIRQ == NULL )
      return(FALSE);

	if( irqNum >= OMAP_IRQ_MAXIMUM )
     return(FALSE);

	// g_IRQ2VIIRQ
    *pVIIRQ = g_IRQ2VIIRQ[irqNum];

	return(TRUE);
}

BOOL 
OALIoCtlOALgetVIIRQ(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
	BYTE vIIRQ;

    OALMSG( 1 /*OAL_IOCTL&&OAL_FUNC*/, (L"+OALIoCtlOALgetVIIRQ\r\n"));
    
    if (pInBuffer == NULL || inSize != sizeof(UINT32))
        {
        goto cleanUp;
        }

	if (pOutBuffer == NULL || outSize < sizeof(BYTE)) goto cleanUp;

    // update return information
    //
    if(pOutSize != NULL) 
	  *(UINT32*)pOutSize = sizeof(BYTE);

    if( !(OALgetVIIRQ( *(UINT32 *)pInBuffer, &vIIRQ )) )
	  goto cleanUp;

	*(BYTE *)pOutBuffer = vIIRQ;

	OALMSG(1 /*OAL_INTR&&OAL_FUNC*/, (L"-OALIoCtlOALgetVIIRQ (pInBuffer = %d)\r\n", *(UINT32 *)pInBuffer));
	rc = TRUE;

cleanUp:
    OALMSG( 1 /*OAL_INTR&&OAL_FUNC*/, (L"-OALIoCtlOALgetVIIRQ(rc = %d)\r\n", rc));
    return rc;


}
#endif /* 0 */


//-----------------------------------------------------------------------------
//  ##### Added by Michael, 1-Dec-2009
//  Function:  OALPAtoVA
//
//  Allow call Allow call OALPAtoVA from driver from driver
//

BOOL 
OALIoCtlOALPAtoVA(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    IOCTL_HAL_OALPATOVA_IN *pIn;

    OALMSG( OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlOALPAtoVA\r\n"));
    
    if (pInBuffer == NULL || inSize != sizeof(IOCTL_HAL_OALPATOVA_IN))
        {
        goto cleanUp;
        }

	if (pOutBuffer == NULL || outSize < sizeof(VOID*)) goto cleanUp;

    // update return information
    //
    if(pOutSize != NULL) 
	  *(UINT32*)pOutSize = sizeof(LPVOID);

    // update info and call appropriate routine
    pIn = (IOCTL_HAL_OALPATOVA_IN *)pInBuffer;
	//OALMSG(1 /*OAL_INTR&&OAL_FUNC*/, (L"-OALIoCtlOALPAtoVA (pa = 0x%X, cahced = %d)\r\n", pIn->pa, pIn->cached));
	*(LPVOID *)pOutBuffer = (LPVOID)OALPAtoVA(pIn->pa, pIn->cached);
	// For test puposes:
	//*(LPVOID *)pOutBuffer = (LPVOID)0xAAAA8888;

	//OALMSG(1 /*OAL_INTR&&OAL_FUNC*/, (L"-OALIoCtlOALPAtoVA (pOutBuffer = 0x%X)\r\n", pOutBuffer));
	rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIoCtlOALPAtoVA(rc = %d)\r\n", rc));
    return rc;

}
