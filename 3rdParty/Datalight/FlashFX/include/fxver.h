/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions. 

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    Provides defines for the product name, version, copyright, ...

    !!!! DO NOT MODIFY THE "SIBLD" LINES !!!!
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxver.h $
    Revision 1.968.1.101  2012/04/24 18:19:51Z  qa
    AUTOPROJ Build 2128DE
    Revision 1.968.1.100  2012/04/24 01:43:17Z  qa
    AUTOPROJ Build 2128DD
    Revision 1.968.1.99  2012/04/23 16:37:56Z  qa
    AUTOPROJ Build 2128DC
    Revision 1.968.1.98  2012/04/20 04:28:06Z  qa
    AUTOPROJ Build 2128DB
    Revision 1.968.1.97  2012/04/18 02:34:41Z  qa
    AUTOPROJ Build 2128DA
    Revision 1.968.1.96  2012/04/17 21:58:16Z  garyp
    Updated to v2.1.1 for the pending release.
    Revision 1.968.1.95  2012/04/14 05:25:57Z  qa
    AUTOPROJ Build 2128CZ
    Revision 1.968.1.94  2012/04/13 05:11:58Z  qa
    AUTOPROJ Build 2128CY
    Revision 1.968.1.93  2012/04/12 04:42:14Z  qa
    AUTOPROJ Build 2128CX
    Revision 1.968.1.92  2012/04/07 19:56:41Z  qa
    AUTOPROJ Build 2128CW
    Revision 1.968.1.91  2012/04/05 09:06:07Z  qa
    AUTOPROJ Build 2128CV
    Revision 1.968.1.90  2012/03/23 04:41:53Z  qa
    AUTOPROJ Build 2128CT
    Revision 1.968.1.89  2012/03/07 05:22:36Z  qa
    AUTOPROJ Build 2128CS
    Revision 1.968.1.88  2012/03/06 05:56:05Z  qa
    AUTOPROJ Build 2128CR
    Revision 1.968.1.87  2012/02/10 20:55:54Z  qa
    AUTOPROJ Build 2128CQ
    Revision 1.968.1.86  2011/12/31 05:21:27Z  qa
    AUTOPROJ Build 2128CP
    Revision 1.968.1.85  2011/12/22 11:05:47Z  qa
    AUTOPROJ Build 2128CN
    Revision 1.968.1.84  2011/12/21 05:21:29Z  qa
    AUTOPROJ Build 2128CM
    Revision 1.968.1.83  2011/12/15 23:19:59Z  qa
    AUTOPROJ Build 2128CL
    Revision 1.968.1.82  2011/12/15 05:58:43Z  qa
    AUTOPROJ Build 2128CK
    Revision 1.968.1.81  2011/12/15 02:10:34Z  garyp
    Updated to v2.1 for the pending release.
    Revision 1.968.1.80  2011/12/14 05:26:45Z  qa
    AUTOPROJ Build 2128CJ
    Revision 1.968.1.79  2011/12/13 08:03:07Z  qa
    AUTOPROJ Build 2128CH
    Revision 1.968.1.78  2011/12/10 10:33:10Z  qa
    AUTOPROJ Build 2128CG
    Revision 1.968.1.77  2011/12/09 06:07:44Z  qa
    AUTOPROJ Build 2128CF
    Revision 1.968.1.76  2011/12/06 23:39:21Z  qa
    AUTOPROJ Build 2128CD
    Revision 1.968.1.75  2011/12/05 05:43:10Z  qa
    AUTOPROJ Build 2128CC
    Revision 1.968.1.74  2011/12/02 09:09:38Z  qa
    AUTOPROJ Build 2128CB
    Revision 1.968.1.73  2011/12/01 05:42:44Z  qa
    AUTOPROJ Build 2128CA
    Revision 1.968.1.72  2011/11/30 05:15:06Z  qa
    AUTOPROJ Build 2128BZ
    Revision 1.968.1.71  2011/11/24 06:12:14Z  qa
    AUTOPROJ Build 2128BY
    Revision 1.968.1.70  2011/11/23 06:05:21Z  qa
    AUTOPROJ Build 2128BX
    Revision 1.968.1.69  2011/11/22 16:58:42Z  qa
    AUTOPROJ Build 2128BW
    Revision 1.968.1.68  2011/11/18 18:27:44Z  qa
    AUTOPROJ Build 2128BV
    Revision 1.968.1.67  2011/11/09 19:23:48Z  qa
    AUTOPROJ Build 2128BT
    Revision 1.968.1.66  2011/11/09 18:59:12Z  garyp
    Removed the BETA flag for the pending release.
    Revision 1.968.1.65  2011/10/12 00:34:12Z  qa
    AUTOPROJ Build 2128BR
    Revision 1.968.1.64  2011/10/11 19:59:12Z  qa
    AUTOPROJ Build 2128BQ
    Revision 1.968.1.63  2011/10/11 19:52:25Z  billr
    Updated for the v2.0.2 Beta-2 release.
    Revision 1.968.1.62  2011/06/28 04:26:26Z  qa
    AUTOPROJ Build 2128BP
    Revision 1.968.1.61  2011/06/23 04:26:19Z  qa
    AUTOPROJ Build 2128BN
    Revision 1.968.1.60  2011/06/17 06:05:27Z  qa
    AUTOPROJ Build 2128BL
    Revision 1.968.1.59  2011/06/15 05:08:07Z  qa
    AUTOPROJ Build 2128BK
    Revision 1.968.1.58  2011/06/11 04:19:52Z  qa
    AUTOPROJ Build 2128BH
    Revision 1.968.1.57  2011/06/03 12:00:22Z  qa
    AUTOPROJ Build 2128BG
    Revision 1.968.1.56  2011/05/25 00:35:21Z  qa
    AUTOPROJ Build 2128BE
    Revision 1.968.1.55  2011/05/24 22:20:34Z  garyp
    Updated for the v2.0.2 Beta release.
    Revision 1.968.1.54  2011/05/20 04:48:25Z  qa
    AUTOPROJ Build 2128BD
    Revision 1.968.1.53  2011/05/05 04:34:59Z  qa
    AUTOPROJ Build 2128BC
    Revision 1.968.1.52  2011/04/29 04:35:02Z  qa
    AUTOPROJ Build 2128BB
    Revision 1.968.1.51  2011/04/28 05:03:54Z  qa
    AUTOPROJ Build 2128BA
    Revision 1.968.1.50  2011/04/27 04:14:11Z  qa
    AUTOPROJ Build 2128AZ
    Revision 1.968.1.49  2011/04/24 04:43:32Z  qa
    AUTOPROJ Build 2128AY
    Revision 1.968.1.48  2011/04/23 04:18:03Z  qa
    AUTOPROJ Build 2128AX
    Revision 1.968.1.47  2011/04/19 04:28:58Z  qa
    AUTOPROJ Build 2128AW
    Revision 1.968.1.46  2011/04/18 04:42:11Z  qa
    AUTOPROJ Build 2128AV
    Revision 1.968.1.45  2011/04/16 04:41:16Z  qa
    AUTOPROJ Build 2128AT
    Revision 1.968.1.44  2011/04/15 06:57:40Z  qa
    AUTOPROJ Build 2128AS
    Revision 1.968.1.43  2011/04/13 04:14:46Z  qa
    AUTOPROJ Build 2128AR
    Revision 1.968.1.42  2011/04/12 04:26:00Z  qa
    AUTOPROJ Build 2128AQ
    Revision 1.968.1.41  2011/04/08 04:21:00Z  qa
    AUTOPROJ Build 2128AP
    Revision 1.968.1.40  2011/03/30 05:07:21Z  qa
    AUTOPROJ Build 2128AN
    Revision 1.968.1.39  2011/03/24 04:26:15Z  qa
    AUTOPROJ Build 2128AM
    Revision 1.968.1.38  2011/03/21 01:33:44Z  qa
    AUTOPROJ Build 2128AL
    Revision 1.968.1.37  2011/03/16 05:39:38Z  qa
    AUTOPROJ Build 2128AK
    Revision 1.968.1.36  2011/03/03 05:50:32Z  qa
    AUTOPROJ Build 2128AJ
    Revision 1.968.1.35  2011/03/02 05:38:52Z  qa
    AUTOPROJ Build 2128AH
    Revision 1.968.1.34  2011/03/01 06:12:15Z  qa
    AUTOPROJ Build 2128AG
    Revision 1.968.1.33  2011/02/17 05:14:57Z  qa
    AUTOPROJ Build 2128AF
    Revision 1.968.1.32  2011/02/15 05:59:10Z  qa
    AUTOPROJ Build 2128AE
    Revision 1.968.1.31  2011/02/12 05:27:00Z  qa
    AUTOPROJ Build 2128AD
    Revision 1.968.1.30  2011/02/10 04:32:42Z  qa
    AUTOPROJ Build 2128AC
    Revision 1.968.1.29  2011/02/10 01:04:46Z  garyp
    Updated to v2.0.1 and turned off the BETA flag.
    Revision 1.968.1.28  2011/02/09 06:07:56Z  qa
    AUTOPROJ Build 2128AB
    Revision 1.968.1.27  2011/02/07 05:26:54Z  qa
    AUTOPROJ Build 2128AA
    Revision 1.968.1.26  2011/02/04 05:48:17Z  qa
    AUTOPROJ Build 2128Z
    Revision 1.968.1.25  2011/01/23 06:03:27Z  qa
    AUTOPROJ Build 2128Y
    Revision 1.968.1.24  2011/01/19 05:41:34Z  qa
    AUTOPROJ Build 2128X
    Revision 1.968.1.23  2011/01/15 05:59:53Z  qa
    AUTOPROJ Build 2128W
    Revision 1.968.1.22  2011/01/13 06:13:10Z  qa
    AUTOPROJ Build 2128V
    Revision 1.968.1.21  2011/01/10 06:10:49Z  qa
    AUTOPROJ Build 2128T
    Revision 1.968.1.20  2011/01/05 05:53:45Z  qa
    AUTOPROJ Build 2128S
    Revision 1.968.1.19  2011/01/03 05:42:04Z  qa
    AUTOPROJ Build 2128R
    Revision 1.968.1.18  2011/01/01 19:58:59Z  garyp
    Changed to v2.0.x BETA.  Updated the copyright date.
    Revision 1.968.1.17  2010/12/26 05:46:44Z  qa
    AUTOPROJ Build 2128Q
    Revision 1.968.1.16  2010/12/25 08:31:47Z  qa
    AUTOPROJ Build 2128P
    Revision 1.968.1.15  2010/12/22 21:22:34Z  qa
    AUTOPROJ Build 2128N
    Revision 1.968.1.14  2010/12/22 13:22:38Z  qa
    AUTOPROJ Build 2128M
    Revision 1.968.1.13  2010/12/22 09:28:35Z  qa
    AUTOPROJ Build 2128L
    Revision 1.968.1.12  2010/12/22 05:19:35Z  qa
    AUTOPROJ Build 2128K
    Revision 1.968.1.11  2010/12/21 06:11:19Z  qa
    AUTOPROJ Build 2128J
    Revision 1.968.1.10  2010/12/21 01:07:30Z  garyp
    Removed the Beta flag.
    Revision 1.968.1.9  2010/12/20 05:19:55Z  qa
    AUTOPROJ Build 2128H
    Revision 1.968.1.8  2010/12/19 05:20:09Z  qa
    AUTOPROJ Build 2128G
    Revision 1.968.1.7  2010/12/18 05:28:27Z  qa
    AUTOPROJ Build 2128F
    Revision 1.968.1.6  2010/12/17 05:40:32Z  qa
    AUTOPROJ Build 2128E
    Revision 1.968.1.5  2010/12/16 05:49:27Z  qa
    AUTOPROJ Build 2128D
    Revision 1.968.1.4  2010/12/15 12:40:34Z  qa
    AUTOPROJ Build 2128C
    Revision 1.968.1.3  2010/12/14 22:34:18Z  qa
    AUTOPROJ Build 2128B
    Revision 1.968.1.2  2010/12/14 17:40:53Z  qa
    Updated for the new v2.0 branch.
    Revision 1.968  2010/12/14 05:58:07Z  qa
    AUTOPROJ Build 2128
    Revision 1.967  2010/12/13 01:11:27Z  qa
    AUTOPROJ Build 2127
    Revision 1.966  2010/12/10 05:43:08Z  qa
    AUTOPROJ Build 2126
    Revision 1.965  2010/12/09 05:43:08Z  qa
    AUTOPROJ Build 2125
    Revision 1.964  2010/12/04 05:16:28Z  qa
    AUTOPROJ Build 2124
    Revision 1.963  2010/12/03 05:19:08Z  qa
    AUTOPROJ Build 2123
    Revision 1.962  2010/12/02 05:40:12Z  qa
    AUTOPROJ Build 2122
    Revision 1.961  2010/12/01 05:40:23Z  qa
    AUTOPROJ Build 2121
    Revision 1.960  2010/11/28 05:40:18Z  qa
    AUTOPROJ Build 2120
    Revision 1.959  2010/11/24 05:19:28Z  qa
    AUTOPROJ Build 2119
    Revision 1.958  2010/11/20 05:40:52Z  qa
    AUTOPROJ Build 2118
    Revision 1.957  2010/11/18 05:19:44Z  qa
    AUTOPROJ Build 2117
    Revision 1.956  2010/11/17 05:40:56Z  qa
    AUTOPROJ Build 2116
    Revision 1.955  2010/11/14 04:10:30Z  qa
    AUTOPROJ Build 2115
    Revision 1.954  2010/11/12 05:19:27Z  qa
    AUTOPROJ Build 2114
    Revision 1.953  2010/11/05 04:40:30Z  qa
    AUTOPROJ Build 2113
    Revision 1.952  2010/11/03 05:10:17Z  qa
    AUTOPROJ Build 2112
    Revision 1.951  2010/11/01 17:28:29Z  qa
    AUTOPROJ Build 2111
    Revision 1.950  2010/11/01 13:43:31Z  garyp
    Updated to Alpha-3.
    Revision 1.949  2010/10/29 04:13:14Z  qa
    AUTOPROJ Build 2110
    Revision 1.948  2010/10/26 04:19:17Z  qa
    AUTOPROJ Build 2109
    Revision 1.947  2010/10/23 04:16:29Z  qa
    AUTOPROJ Build 2108
    Revision 1.946  2010/10/22 04:40:28Z  qa
    AUTOPROJ Build 2107
    Revision 1.945  2010/10/21 21:12:24Z  qa
    AUTOPROJ Build 2106
    Revision 1.944  2010/10/21 04:40:54Z  qa
    AUTOPROJ Build 2105
    Revision 1.943  2010/10/17 04:40:37Z  qa
    AUTOPROJ Build 2104
    Revision 1.942  2010/10/15 04:40:12Z  qa
    AUTOPROJ Build 2103
    Revision 1.941  2010/10/13 04:40:29Z  qa
    AUTOPROJ Build 2102
    Revision 1.940  2010/10/12 18:34:31Z  garyp
    Changed to Alpha-2.
    Revision 1.939  2010/10/12 04:40:27Z  qa
    AUTOPROJ Build 2101
    Revision 1.938  2010/10/10 04:40:15Z  qa
    AUTOPROJ Build 2100
    Revision 1.937  2010/10/09 04:40:12Z  qa
    AUTOPROJ Build 2099
    Revision 1.936  2010/10/08 04:40:10Z  qa
    AUTOPROJ Build 2098
    Revision 1.935  2010/10/07 04:40:11Z  qa
    AUTOPROJ Build 2097
    Revision 1.934  2010/10/06 15:10:12Z  qa
    AUTOPROJ Build 2096
    Revision 1.933  2010/10/06 05:55:25Z  qa
    AUTOPROJ Build 2095
    Revision 1.932  2010/10/06 05:48:15Z  garyp
    Reverted the build number to accommodate an interrupted build.
    Revision 1.931  2010/10/05 20:01:10Z  qa
    AUTOPROJ Build 2095
    Revision 1.930  2010/10/05 06:28:45Z  qa
    AUTOPROJ Build 2094
    Revision 1.929  2010/10/05 06:17:04Z  garyp
    Try again.
    Revision 1.928  2010/10/05 02:24:04Z  qa
    AUTOPROJ Build 2094
    Revision 1.927  2010/10/05 02:14:54Z  garyp
    Reverted to build number 2093 because there was no 2094 checkpoint
    due to network problems (again).
    Revision 1.926  2010/10/04 18:47:52Z  qa
    AUTOPROJ Build 2094
    Revision 1.925  2010/10/04 18:40:36Z  garyp
    Reverted to build number 2093 because there was no 2094 checkpoint
    due to network problems.
    Revision 1.924  2010/10/04 04:25:31Z  qa
    AUTOPROJ Build 2094
    Revision 1.923  2010/10/02 04:19:29Z  qa
    AUTOPROJ Build 2093
    Revision 1.922  2010/09/30 04:16:11Z  qa
    AUTOPROJ Build 2092
    Revision 1.921  2010/09/29 01:22:32Z  qa
    AUTOPROJ Build 2091
    Revision 1.920  2010/09/28 04:13:30Z  qa
    AUTOPROJ Build 2090
    Revision 1.919  2010/09/25 04:55:32Z  qa
    AUTOPROJ Build 2089
    Revision 1.918  2010/09/24 04:16:34Z  qa
    AUTOPROJ Build 2088
    Revision 1.917  2010/09/23 09:40:24Z  qa
    AUTOPROJ Build 2087
    Revision 1.916  2010/09/22 04:16:14Z  qa
    AUTOPROJ Build 2086
    Revision 1.915  2010/09/21 04:13:22Z  qa
    AUTOPROJ Build 2085
    Revision 1.914  2010/09/20 06:40:27Z  qa
    AUTOPROJ Build 2084
    Revision 1.913  2010/09/19 04:40:56Z  qa
    AUTOPROJ Build 2083
    Revision 1.912  2010/09/18 04:40:27Z  qa
    AUTOPROJ Build 2082
    Revision 1.911  2010/09/17 04:43:39Z  qa
    AUTOPROJ Build 2081
    Revision 1.910  2010/09/15 04:40:32Z  qa
    AUTOPROJ Build 2080
    Revision 1.909  2010/09/14 04:40:33Z  qa
    AUTOPROJ Build 2079
    Revision 1.908  2010/09/11 04:34:31Z  qa
    AUTOPROJ Build 2078
    Revision 1.907  2010/09/10 04:22:27Z  qa
    AUTOPROJ Build 2077
    Revision 1.906  2010/09/09 04:34:31Z  qa
    AUTOPROJ Build 2076
    Revision 1.905  2010/09/08 20:13:05Z  qa
    AUTOPROJ Build 2075
    Revision 1.904  2010/09/03 04:40:29Z  qa
    AUTOPROJ Build 2074
    Revision 1.903  2010/09/01 04:16:33Z  qa
    AUTOPROJ Build 2073
    Revision 1.902  2010/08/31 04:40:30Z  qa
    AUTOPROJ Build 2072
    Revision 1.901  2010/08/30 04:19:31Z  qa
    AUTOPROJ Build 2071
    Revision 1.900  2010/08/29 04:14:00Z  qa
    AUTOPROJ Build 2070
    Revision 1.899  2010/08/28 07:36:45Z  qa
    AUTOPROJ Build 2069
    Revision 1.898  2010/08/25 10:22:15Z  qa
    AUTOPROJ Build 2068
    Revision 1.897  2010/08/24 08:57:13Z  qa
    AUTOPROJ Build 2067
    Revision 1.896  2010/08/14 04:25:20Z  qa
    AUTOPROJ Build 2066
    Revision 1.895  2010/08/13 04:19:21Z  qa
    AUTOPROJ Build 2065
    Revision 1.894  2010/08/12 04:55:20Z  qa
    AUTOPROJ Build 2064
    Revision 1.893  2010/08/11 04:25:20Z  qa
    AUTOPROJ Build 2063
    Revision 1.892  2010/08/10 04:16:04Z  qa
    AUTOPROJ Build 2062
    Revision 1.891  2010/08/07 04:40:20Z  qa
    AUTOPROJ Build 2061
    Revision 1.890  2010/08/05 04:28:02Z  qa
    AUTOPROJ Build 2060
    Revision 1.889  2010/08/04 02:43:30Z  qa
    AUTOPROJ Build 2059
    Revision 1.888  2010/08/02 04:34:29Z  qa
    AUTOPROJ Build 2058
    Revision 1.887  2010/08/01 04:19:32Z  qa
    AUTOPROJ Build 2057
    Revision 1.886  2010/07/31 22:42:10Z  garyp
    Added the product name symbols PRODUCTBASENAME and PRODUCTPREFIX.
    Changed the product name to FlashFX Tera.
    Revision 1.885  2010/07/31 19:30:02Z  qa
    AUTOPROJ Build 2056
    Revision 1.884  2010/07/29 16:55:30Z  qa
    AUTOPROJ Build 2055
    Revision 1.883  2010/07/22 04:46:02Z  qa
    AUTOPROJ Build 2054
    Revision 1.882  2010/07/18 04:49:13Z  qa
    AUTOPROJ Build 2053
    Revision 1.881  2010/07/17 04:25:22Z  qa
    AUTOPROJ Build 2052
    Revision 1.880  2010/07/16 04:40:01Z  qa
    AUTOPROJ Build 2051
    Revision 1.879  2010/07/15 04:40:02Z  qa
    AUTOPROJ Build 2050
    Revision 1.878  2010/07/14 04:55:02Z  qa
    AUTOPROJ Build 2049
    Revision 1.877  2010/07/13 04:28:03Z  qa
    AUTOPROJ Build 2048
    Revision 1.876  2010/07/09 23:19:22Z  qa
    AUTOPROJ Build 2047
    Revision 1.875  2010/07/08 04:40:19Z  qa
    AUTOPROJ Build 2046
    Revision 1.874  2010/07/06 04:16:31Z  qa
    AUTOPROJ Build 2045
    Revision 1.873  2010/07/03 04:38:58Z  qa
    AUTOPROJ Build 2044
    Revision 1.872  2010/07/02 05:31:08Z  autotest
    AUTOPROJ Build 2043
    Revision 1.871  2010/07/01 04:40:09Z  autotest
    AUTOPROJ Build 2042
    Revision 1.870  2010/06/29 05:34:08Z  autotest
    AUTOPROJ Build 2041
    Revision 1.869  2010/06/26 05:16:08Z  autotest
    AUTOPROJ Build 2040
    Revision 1.868  2010/06/25 09:17:32Z  qa
    AUTOPROJ Build 2039
    Revision 1.867  2010/06/23 07:28:08Z  autotest
    AUTOPROJ Build 2038
    Revision 1.866  2010/06/22 04:40:11Z  autotest
    AUTOPROJ Build 2037
    Revision 1.865  2010/06/21 04:16:11Z  autotest
    AUTOPROJ Build 2036
    Revision 1.864  2010/06/20 02:07:22Z  qa
    AUTOPROJ Build 2035
    Revision 1.863  2010/06/17 04:55:20Z  qa
    AUTOPROJ Build 2034
    Revision 1.862  2010/06/15 04:37:23Z  qa
    AUTOPROJ Build 2033
    Revision 1.861  2010/06/14 05:16:07Z  autotest
    AUTOPROJ Build 2032
    Revision 1.860  2010/06/13 04:40:18Z  qa
    AUTOPROJ Build 2031
    Revision 1.859  2010/06/12 07:46:19Z  qa
    AUTOPROJ Build 2030
    Revision 1.858  2010/06/10 04:52:19Z  qa
    AUTOPROJ Build 2029
    Revision 1.857  2010/06/09 06:07:12Z  qa
    AUTOPROJ Build 2028
    Revision 1.856  2010/06/08 21:36:57Z  qa
    AUTOPROJ Build 2027
    Revision 1.855  2010/06/08 20:46:36Z  qa
    AUTOPROJ Build 2026
    Revision 1.854  2010/06/08 20:04:41Z  qa
    AUTOPROJ Build 2025
    Revision 1.853  2010/06/08 09:46:53Z  qa
    AUTOPROJ Build 2024
    Revision 1.852  2010/06/02 17:10:36Z  autotest
    AUTOPROJ Build 2023
    Revision 1.851  2010/05/29 01:33:26Z  qa
    AUTOPROJ Build 2022
    Revision 1.850  2010/05/24 05:10:06Z  autotest
    AUTOPROJ Build 2021
    Revision 1.849  2010/05/23 04:43:16Z  qa
    AUTOPROJ Build 2020
    Revision 1.848  2010/05/22 04:31:32Z  autotest
    AUTOPROJ Build 2019
    Revision 1.847  2010/05/21 04:46:28Z  autotest
    AUTOPROJ Build 2018
    Revision 1.846  2010/05/20 05:13:28Z  autotest
    AUTOPROJ Build 2017
    Revision 1.845  2010/05/19 04:10:38Z  qa
    AUTOPROJ Build 2016
    Revision 1.844  2010/05/17 04:19:20Z  qa
    AUTOPROJ Build 2015
    Revision 1.843  2010/05/15 23:39:29Z  autotest
    AUTOPROJ Build 2014
    Revision 1.842  2010/05/15 09:43:44Z  qa
    AUTOPROJ Build 2013
    Revision 1.841  2010/05/12 05:13:18Z  qa
    AUTOPROJ Build 2012
    Revision 1.840  2010/05/11 07:58:36Z  qa
    AUTOPROJ Build 2011
    Revision 1.839  2010/05/10 05:19:37Z  qa
    AUTOPROJ Build 2010
    Revision 1.838  2010/05/09 20:08:13Z  qa
    AUTOPROJ Build 2009
    Revision 1.837  2010/05/09 07:16:37Z  qa
    AUTOPROJ Build 2008
    Revision 1.836  2010/05/08 04:19:20Z  qa
    AUTOPROJ Build 2007
    Revision 1.835  2010/05/06 04:25:17Z  garyp
    AUTOPROJ Build 2006
    Revision 1.834  2010/05/05 02:10:05Z  garyp
    AUTOPROJ Build 2005
    Revision 1.833  2010/05/04 09:26:35Z  qa
    AUTOPROJ Build 2004
    Revision 1.832  2010/05/03 02:37:16Z  garyp
    AUTOPROJ Build 2003
    Revision 1.831  2010/05/01 23:05:18Z  garyp
    AUTOPROJ Build 2002
    Revision 1.830  2010/05/01 06:17:32Z  qa
    AUTOPROJ Build 2001
    Revision 1.829  2010/04/29 04:52:28Z  qa
    AUTOPROJ Build 2000
    Revision 1.828  2010/04/28 08:17:28Z  qa
    AUTOPROJ Build 1999
    Revision 1.827  2010/04/23 05:11:52Z  qa
    AUTOPROJ Build 1998
    Revision 1.826  2010/04/22 04:51:46Z  qa
    AUTOPROJ Build 1997
    Revision 1.825  2010/04/20 04:46:54Z  qa
    AUTOPROJ Build 1996
    Revision 1.824  2010/04/19 04:32:57Z  qa
    AUTOPROJ Build 1995
    Revision 1.823  2010/04/16 07:12:04Z  qa
    AUTOPROJ Build 1994
    Revision 1.822  2010/04/14 07:42:01Z  qa
    AUTOPROJ Build 1993
    Revision 1.821  2010/04/12 10:26:39Z  qa
    AUTOPROJ Build 1992
    Revision 1.820  2010/04/03 07:06:53Z  qa
    AUTOPROJ Build 1991
    Revision 1.819  2010/03/27 07:27:22Z  qa
    AUTOPROJ Build 1990
    Revision 1.818  2010/03/23 07:37:14Z  qa
    AUTOPROJ Build 1989
    Revision 1.817  2010/03/19 07:27:21Z  qa
    AUTOPROJ Build 1988
    Revision 1.816  2010/03/18 04:57:14Z  qa
    AUTOPROJ Build 1987
    Revision 1.815  2010/03/16 06:22:20Z  qa
    AUTOPROJ Build 1986
    Revision 1.814  2010/03/12 10:07:17Z  qa
    AUTOPROJ Build 1985
    Revision 1.813  2010/03/11 10:12:20Z  qa
    AUTOPROJ Build 1984
    Revision 1.812  2010/03/10 10:22:17Z  qa
    AUTOPROJ Build 1983
    Revision 1.811  2010/03/09 10:17:10Z  qa
    AUTOPROJ Build 1982
    Revision 1.810  2010/03/06 07:37:13Z  qa
    AUTOPROJ Build 1981
    Revision 1.809  2010/03/04 09:17:22Z  qa
    AUTOPROJ Build 1980
    Revision 1.808  2010/03/03 09:07:24Z  qa
    AUTOPROJ Build 1979
    Revision 1.807  2010/03/02 08:52:28Z  qa
    AUTOPROJ Build 1978
    Revision 1.806  2010/02/28 08:52:23Z  qa
    AUTOPROJ Build 1977
    Revision 1.805  2010/02/27 05:47:20Z  qa
    AUTOPROJ Build 1976
    Revision 1.804  2010/02/26 09:32:20Z  qa
    AUTOPROJ Build 1975
    Revision 1.803  2010/02/26 02:01:45Z  garyp
    Condensed some of the revision history.
    Revision 1.802  2010/02/25 09:32:21Z  qa
    AUTOPROJ Build 1974
    Revision 1.801  2010/02/24 06:11:55Z  qa
    AUTOPROJ Build 1973
    Revision 1.800  2010/02/23 07:32:23Z  qa
    AUTOPROJ Build 1972
    Revision 1.799  2010/02/20 09:17:07Z  qa
    AUTOPROJ Build 1971
    Revision 1.798  2010/02/19 08:01:43Z  qa
    AUTOPROJ Build 1970
    Revision 1.797  2010/02/18 09:22:09Z  qa
    AUTOPROJ Build 1969
    Revision 1.796  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.795  2010/02/16 09:32:25Z  qa
    AUTOPROJ Build 1968
    Revision 1.794  2010/02/15 05:52:21Z  qa
    AUTOPROJ Build 1967
    Revision 1.793  2010/02/14 08:57:30Z  qa
    AUTOPROJ Build 1966
    Revision 1.792  2010/02/13 21:47:20Z  qa
    AUTOPROJ Build 1965
    Revision 1.791  2010/02/11 09:32:10Z  qa
    AUTOPROJ Build 1964
    Revision 1.790  2010/02/09 09:07:11Z  qa
    AUTOPROJ Build 1963
    Revision 1.789  2010/02/05 09:48:30Z  qa
    AUTOPROJ Build 1962
    Revision 1.788  2010/02/03 09:02:05Z  qa
    AUTOPROJ Build 1961
    Revision 1.787  2010/02/01 05:51:58Z  qa
    AUTOPROJ Build 1960
    Revision 1.786  2010/01/31 08:57:05Z  qa
    AUTOPROJ Build 1959
    Revision 1.785  2010/01/27 06:06:59Z  qa
    AUTOPROJ Build 1958
    Revision 1.784  2010/01/23 07:56:57Z  qa
    AUTOPROJ Build 1957
    Revision 1.783  2010/01/13 06:07:02Z  qa
    AUTOPROJ Build 1956
    Revision 1.782  2010/01/12 08:56:57Z  qa
    AUTOPROJ Build 1955
    Revision 1.781  2010/01/11 06:22:03Z  qa
    AUTOPROJ Build 1954
    Revision 1.780  2010/01/10 09:02:16Z  qa
    AUTOPROJ Build 1953
    Revision 1.779  2010/01/09 08:46:53Z  qa
    AUTOPROJ Build 1952
    Revision 1.778  2010/01/08 02:43:59Z  garyp
    AUTOPROJ Build 1951
    Revision 1.777  2010/01/07 06:06:43Z  qa
    AUTOPROJ Build 1950
    Revision 1.776  2010/01/06 08:46:50Z  qa
    AUTOPROJ Build 1949
    Revision 1.775  2010/01/05 22:53:05Z  garyp
    Changed to v2.0.  Updated the copyright date.
    Revision 1.774  2010/01/05 01:40:21Z  qa
    AUTOPROJ Build 1948
    Revision 1.773  2009/12/28 22:49:47Z  qa
    AUTOPROJ Build 1947
    Revision 1.772  2009/12/23 17:12:20Z  qa
    AUTOPROJ Build 1946
    Revision 1.771  2009/12/20 05:56:59Z  qa
    AUTOPROJ Build 1945
    Revision 1.770  2009/12/15 08:16:48Z  qa
    AUTOPROJ Build 1944
    Revision 1.769  2009/12/13 09:26:40Z  qa
    AUTOPROJ Build 1943
    Revision 1.768  2009/12/05 06:06:57Z  qa
    AUTOPROJ Build 1942
    Revision 1.767  2009/12/04 02:22:29Z  qa
    AUTOPROJ Build 1941
    Revision 1.766  2009/12/03 06:07:00Z  qa
    AUTOPROJ Build 1940
    Revision 1.765  2009/12/02 06:07:16Z  qa
    AUTOPROJ Build 1939
    Revision 1.764  2009/12/01 08:56:56Z  qa
    AUTOPROJ Build 1938
    Revision 1.763  2009/11/27 18:18:10Z  qa
    AUTOPROJ Build 1937
    Revision 1.762  2009/11/24 06:06:59Z  qa
    AUTOPROJ Build 1936
    Revision 1.761  2009/11/20 06:06:51Z  qa
    AUTOPROJ Build 1935
    Revision 1.760  2009/11/18 17:47:28Z  qa
    AUTOPROJ Build 1934
    Revision 1.759  2009/11/13 06:06:49Z  qa
    AUTOPROJ Build 1933
    Revision 1.758  2009/11/12 09:01:59Z  qa
    AUTOPROJ Build 1932
    Revision 1.757  2009/11/11 18:21:56Z  qa
    AUTOPROJ Build 1931
    Revision 1.756  2009/11/10 06:11:51Z  qa
    AUTOPROJ Build 1930
    Revision 1.755  2009/11/08 06:06:53Z  qa
    AUTOPROJ Build 1929
    Revision 1.754  2009/11/06 08:56:45Z  qa
    AUTOPROJ Build 1928
    Revision 1.753  2009/11/05 06:06:42Z  qa
    AUTOPROJ Build 1927
    Revision 1.752  2009/11/04 06:06:46Z  qa
    AUTOPROJ Build 1926
    Revision 1.751  2009/11/03 06:11:52Z  qa
    AUTOPROJ Build 1925
    Revision 1.750  2009/11/01 07:26:48Z  qa
    AUTOPROJ Build 1924
    Revision 1.749  2009/10/31 05:06:40Z  qa
    AUTOPROJ Build 1923
    Revision 1.748  2009/10/30 04:51:41Z  qa
    AUTOPROJ Build 1922
    Revision 1.747  2009/10/29 05:06:43Z  qa
    AUTOPROJ Build 1921
    Revision 1.746  2009/10/22 07:26:38Z  qa
    AUTOPROJ Build 1920
    Revision 1.745  2009/10/21 05:06:41Z  qa
    AUTOPROJ Build 1919
    Revision 1.744  2009/10/18 04:51:44Z  qa
    AUTOPROJ Build 1918
    Revision 1.743  2009/10/17 05:06:36Z  qa
    AUTOPROJ Build 1917
    Revision 1.742  2009/10/15 05:28:11Z  qa
    AUTOPROJ Build 1916
    Revision 1.741  2009/10/14 18:06:38Z  qa
    AUTOPROJ Build 1915
    Revision 1.740  2009/10/13 07:27:36Z  qa
    AUTOPROJ Build 1914
    Revision 1.739  2009/10/12 07:11:44Z  qa
    AUTOPROJ Build 1913
    Revision 1.738  2009/10/10 05:06:44Z  qa
    AUTOPROJ Build 1912
    Revision 1.737  2009/10/09 18:54:34Z  qa
    AUTOPROJ Build 1911
    Revision 1.736  2009/10/08 05:06:46Z  qa
    AUTOPROJ Build 1910
    Revision 1.735  2009/10/07 05:06:46Z  qa
    AUTOPROJ Build 1909
    Revision 1.734  2009/10/03 05:06:46Z  qa
    AUTOPROJ Build 1908
    Revision 1.733  2009/10/02 05:06:43Z  qa
    AUTOPROJ Build 1907
    Revision 1.732  2009/09/30 05:11:41Z  qa
    AUTOPROJ Build 1906
    Revision 1.731  2009/09/29 05:01:49Z  qa
    AUTOPROJ Build 1905
    Revision 1.730  2009/09/26 05:06:46Z  qa
    AUTOPROJ Build 1904
    Revision 1.729  2009/09/25 04:56:41Z  qa
    AUTOPROJ Build 1903
    Revision 1.728  2009/09/24 05:11:44Z  qa
    AUTOPROJ Build 1902
    Revision 1.727  2009/09/23 04:51:37Z  qa
    AUTOPROJ Build 1901
    Revision 1.726  2009/09/21 05:06:39Z  qa
    AUTOPROJ Build 1900
    Revision 1.725  2009/09/19 07:22:34Z  qa
    AUTOPROJ Build 1899
    Revision 1.724  2009/09/18 07:47:48Z  qa
    AUTOPROJ Build 1898
    Revision 1.723  2009/09/10 16:17:04Z  qa
    AUTOPROJ Build 1897
    Revision 1.722  2009/09/09 05:12:09Z  qa
    AUTOPROJ Build 1896
    Revision 1.721  2009/09/04 05:12:05Z  qa
    AUTOPROJ Build 1895
    Revision 1.720  2009/09/03 05:12:06Z  qa
    AUTOPROJ Build 1894
    Revision 1.719  2009/09/01 04:52:04Z  qa
    AUTOPROJ Build 1893
    Revision 1.718  2009/08/29 05:07:16Z  qa
    AUTOPROJ Build 1892
    Revision 1.717  2009/08/27 04:51:56Z  qa
    AUTOPROJ Build 1891
    Revision 1.716  2009/08/26 08:02:06Z  qa
    AUTOPROJ Build 1890
    Revision 1.715  2009/08/22 08:07:10Z  qa
    AUTOPROJ Build 1889
    Revision 1.714  2009/08/21 05:12:07Z  qa
    AUTOPROJ Build 1888
    Revision 1.713  2009/08/15 05:12:03Z  qa
    AUTOPROJ Build 1887
    Revision 1.712  2009/08/08 05:06:57Z  qa
    AUTOPROJ Build 1886
    Revision 1.711  2009/08/07 05:12:13Z  qa
    AUTOPROJ Build 1885
    Revision 1.710  2009/08/06 05:17:06Z  qa
    AUTOPROJ Build 1884
    Revision 1.709  2009/08/05 03:31:45Z  qa
    AUTOPROJ Build 1883
    Revision 1.708  2009/06/26 05:16:57Z  qa
    AUTOPROJ Build 1882
    Revision 1.707  2009/06/24 07:47:19Z  qa
    AUTOPROJ Build 1881
    Revision 1.706  2009/06/19 07:47:21Z  qa
    AUTOPROJ Build 1880
    Revision 1.705  2009/06/16 05:02:06Z  qa
    AUTOPROJ Build 1879
    Revision 1.704  2009/05/30 07:02:12Z  qa
    AUTOPROJ Build 1878
    Revision 1.703  2009/05/29 04:52:13Z  qa
    AUTOPROJ Build 1877
    Revision 1.702  2009/05/28 04:57:06Z  qa
    AUTOPROJ Build 1876
    Revision 1.701  2009/05/27 04:56:48Z  qa
    AUTOPROJ Build 1875
    Revision 1.700  2009/05/24 04:52:06Z  qa
    AUTOPROJ Build 1874
    Revision 1.699  2009/05/23 07:37:17Z  qa
    AUTOPROJ Build 1873
    Revision 1.698  2009/05/22 04:02:13Z  qa
    AUTOPROJ Build 1872
    Revision 1.697  2009/05/21 05:22:08Z  qa
    AUTOPROJ Build 1871
    Revision 1.696  2009/05/19 22:23:24Z  garyp
    Extended "VERSIONVAL" to use the full four bytes for the version number
    and no longer tack the "Alpha" or "Beta" onto the end.
    Revision 1.695  2009/05/19 06:07:16Z  qa
    AUTOPROJ Build 1870
    Revision 1.694  2009/05/18 06:17:12Z  qa
    AUTOPROJ Build 1869
    Revision 1.693  2009/05/15 05:07:16Z  qa
    AUTOPROJ Build 1868
    Revision 1.692  2009/05/14 04:52:22Z  qa
    AUTOPROJ Build 1867
    Revision 1.691  2009/05/13 07:42:24Z  qa
    AUTOPROJ Build 1866
    Revision 1.690  2009/05/10 04:52:23Z  qa
    AUTOPROJ Build 1865
    Revision 1.689  2009/05/09 10:56:59Z  qa
    AUTOPROJ Build 1864
    Revision 1.688  2009/05/08 16:47:04Z  qa
    AUTOPROJ Build 1863
    Revision 1.687  2009/05/08 05:02:01Z  qa
    AUTOPROJ Build 1862
    Revision 1.686  2009/05/04 02:55:43Z  qa
    AUTOPROJ Build 1861
    Revision 1.685  2009/05/02 09:17:03Z  qa
    AUTOPROJ Build 1860
    Revision 1.684  2009/04/30 05:06:58Z  qa
    AUTOPROJ Build 1859
    Revision 1.683  2009/04/28 05:07:11Z  qa
    AUTOPROJ Build 1858
    Revision 1.682  2009/04/26 05:02:23Z  qa
    AUTOPROJ Build 1857
    Revision 1.681  2009/04/25 15:38:47Z  qa
    AUTOPROJ Build 1856
    Revision 1.680  2009/04/23 07:17:32Z  qa
    AUTOPROJ Build 1855
    Revision 1.679  2009/04/22 14:02:30Z  qa
    AUTOPROJ Build 1854
    Revision 1.678  2009/04/21 05:01:54Z  qa
    AUTOPROJ Build 1853
    Revision 1.677  2009/04/20 09:57:10Z  qa
    AUTOPROJ Build 1852
    Revision 1.676  2009/04/19 08:37:16Z  qa
    AUTOPROJ Build 1851
    Revision 1.675  2009/04/16 20:46:21Z  garyp
    Updated to v1.2 and turned the ALPHA flag on.
    Revision 1.674  2009/04/16 05:36:57Z  qa
    AUTOPROJ Build 1850
    Revision 1.673  2009/04/15 16:21:18Z  qa
    AUTOPROJ Build 1849
    Revision 1.672  2009/04/15 00:03:33Z  garyp
    Reverted the previous checkin.
    Revision 1.671  2009/04/14 20:57:06Z  thomd
    Updated to new Product Name
    Revision 1.670  2009/04/14 06:21:55Z  qa
    AUTOPROJ Build 1848
    Revision 1.669  2009/04/11 04:52:07Z  qa
    AUTOPROJ Build 1847
    Revision 1.668  2009/04/10 04:42:20Z  qa
    AUTOPROJ Build 1846
    Revision 1.667  2009/04/09 04:52:13Z  qa
    AUTOPROJ Build 1845
    Revision 1.666  2009/04/08 23:57:51Z  garyp
    Changed to v1.1.  Changed the development state from ALPHA to RELEASE.
    Revision 1.665  2009/04/08 16:06:59Z  qa
    AUTOPROJ Build 1844
    Revision 1.664  2009/04/07 04:43:11Z  qa
    AUTOPROJ Build 1843
    Revision 1.663  2009/04/06 14:47:12Z  qa
    AUTOPROJ Build 1842
    Revision 1.662  2009/04/03 13:32:46Z  qa
    AUTOPROJ Build 1841
    Revision 1.661  2009/04/02 19:48:25Z  qa
    AUTOPROJ Build 1840
    Revision 1.660  2009/04/02 04:47:28Z  qa
    AUTOPROJ Build 1839
    Revision 1.659  2009/04/01 04:52:55Z  qa
    AUTOPROJ Build 1838
    Revision 1.658  2009/03/31 19:56:23Z  qa
    AUTOPROJ Build 1837
    Revision 1.657  2009/03/31 15:41:33Z  qa
    AUTOPROJ Build 1836
    Revision 1.656  2009/03/31 05:02:23Z  qa
    AUTOPROJ Build 1835
    Revision 1.655  2009/03/30 08:57:52Z  qa
    AUTOPROJ Build 1834
    Revision 1.654  2009/03/28 22:28:11Z  qa
    AUTOPROJ Build 1833
    Revision 1.653  2009/03/27 17:51:41Z  qa
    AUTOPROJ Build 1832
    Revision 1.652  2009/03/26 17:27:47Z  qa
    AUTOPROJ Build 1831
    Revision 1.651  2009/03/25 09:57:57Z  qa
    AUTOPROJ Build 1830
    Revision 1.650  2009/03/25 01:42:17Z  qa
    AUTOPROJ Build 1829
    Revision 1.649  2009/03/23 21:37:10Z  qa
    AUTOPROJ Build 1828
    Revision 1.648  2009/03/23 01:22:14Z  qa
    AUTOPROJ Build 1827
    Revision 1.647  2009/03/22 04:47:11Z  qa
    AUTOPROJ Build 1826
    Revision 1.646  2009/03/21 05:37:12Z  qa
    AUTOPROJ Build 1825
    Revision 1.645  2009/03/20 04:47:12Z  qa
    AUTOPROJ Build 1824
    Revision 1.644  2009/03/19 21:52:12Z  qa
    AUTOPROJ Build 1823
    Revision 1.643  2009/03/14 05:22:52Z  qa
    AUTOPROJ Build 1822
    Revision 1.642  2009/03/13 17:22:04Z  qa
    AUTOPROJ Build 1821
    Revision 1.641  2009/03/13 07:52:34Z  qa
    AUTOPROJ Build 1820
    Revision 1.640  2009/03/11 04:52:00Z  qa
    AUTOPROJ Build 1819
    Revision 1.639  2009/03/10 04:52:28Z  qa
    AUTOPROJ Build 1818
    Revision 1.638  2009/03/09 19:54:20Z  qa
    AUTOPROJ Build 1817
    Revision 1.637  2009/03/07 17:12:11Z  qa
    AUTOPROJ Build 1816
    Revision 1.636  2009/03/07 09:08:50Z  qa
    AUTOPROJ Build 1815
    Revision 1.635  2009/03/07 00:43:58Z  qa
    AUTOPROJ Build 1814
    Revision 1.634  2009/03/06 05:43:41Z  qa
    AUTOPROJ Build 1813
    Revision 1.633  2009/03/05 18:49:32Z  qa
    AUTOPROJ Build 1812
    Revision 1.632  2009/03/04 05:59:01Z  qa
    AUTOPROJ Build 1811
    Revision 1.631  2009/03/03 05:59:03Z  qa
    AUTOPROJ Build 1810
    Revision 1.630  2009/03/01 05:43:58Z  qa
    AUTOPROJ Build 1809
    Revision 1.629  2009/02/28 05:44:13Z  qa
    AUTOPROJ Build 1808
    Revision 1.628  2009/02/26 05:43:26Z  qa
    AUTOPROJ Build 1807
    Revision 1.627  2009/02/25 05:58:38Z  qa
    AUTOPROJ Build 1806
    Revision 1.626  2009/02/24 05:58:24Z  qa
    AUTOPROJ Build 1805
    Revision 1.625  2009/02/23 23:53:47Z  qa
    AUTOPROJ Build 1804
    Revision 1.624  2009/02/17 05:58:40Z  qa
    AUTOPROJ Build 1803
    Revision 1.623  2009/02/14 06:14:08Z  qa
    AUTOPROJ Build 1802
    Revision 1.622  2009/02/13 06:13:41Z  qa
    AUTOPROJ Build 1801
    Revision 1.621  2009/02/12 05:59:00Z  qa
    AUTOPROJ Build 1800
    Revision 1.620  2009/02/11 05:58:51Z  qa
    AUTOPROJ Build 1799
    Revision 1.619  2009/02/10 06:13:35Z  qa
    AUTOPROJ Build 1798
    Revision 1.618  2009/02/09 09:48:45Z  qa
    AUTOPROJ Build 1797
    Revision 1.617  2009/02/07 02:50:17Z  garyp
    Display the compile date and time in the signon, regardless whether built
    in DEBUG or RELEASE mode.  Updated the copyright date.
    Revision 1.616  2009/02/05 05:58:37Z  qa
    AUTOPROJ Build 1796
    Revision 1.615  2009/02/04 05:48:14Z  qa
    AUTOPROJ Build 1795
    Revision 1.614  2009/02/03 16:36:25Z  qa
    AUTOPROJ Build 1794
    Revision 1.613  2009/01/31 05:52:39Z  qa
    AUTOPROJ Build 1793
    Revision 1.612  2009/01/30 18:53:06Z  qa
    AUTOPROJ Build 1792
    Revision 1.611  2009/01/29 05:43:47Z  qa
    AUTOPROJ Build 1791
    Revision 1.610  2009/01/28 16:38:46Z  qa
    AUTOPROJ Build 1790
    Revision 1.609  2009/01/22 09:18:38Z  qa
    AUTOPROJ Build 1789
    Revision 1.608  2009/01/21 03:43:41Z  qa
    AUTOPROJ Build 1788
    Revision 1.607  2009/01/18 05:38:46Z  qa
    AUTOPROJ Build 1787
    Revision 1.606  2009/01/17 08:13:43Z  qa
    AUTOPROJ Build 1786
    Revision 1.605  2009/01/05 05:48:18Z  qa
    AUTOPROJ Build 1785
    Revision 1.604  2009/01/03 06:53:14Z  qa
    AUTOPROJ Build 1784
    < Entries for 2008 builds 1636 to 1783 have been removed >
    < Entries for 2007 builds 1393 to 1635 have been removed >
    < Entries for 2006 builds 1201 to 1392 have been removed >
    Revision 1.2  2005/12/13 22:27:16  Rickc
    Increased build num to 1200 for new project tree.
    Revision 1.1  2005/12/05 20:17:14  Pauli
    Initial revision
    Revision 1.2  2005/12/05 20:17:14Z  Garyp
    Updated build and version numbers for 4GR v1.0 release.
    Revision 1.1  2005/10/05 05:07:48Z  Garyp
    Initial revision
    < Entries for 2005 builds 860 to 923 have been removed >
    < Entries for 2004 builds 761 to 859 have been removed >
    Revision 1.180  2004/03/19 02:51:56  garys
    version 6.20 OSE release
    Revision 1.179  2004/03/18 23:04:56Z  autoproj
    AUTOPROJ Build 760
    Revision 1.178  2004/03/16 16:41:21  garys
    Updated FFX version from 6.13 to 6.20 after merge from FlashFXMT
    Updated copyright year to 2004
    Removed autoproj entries in revision history through build 759
    Revision 1.171  2003/12/19 05:05:12  garyp
    Changed version to 6.13.
    Revision 1.140  2003/09/11 22:11:06  garyp
    Removed the "beta" designation.
    Revision 1.129  2003/08/07 00:26:58  garyp
    Changed to v6.12 beta.
    Revision 1.126  2003/07/24 21:17:16  garys
    updated version number and date for 6.11.710 engr release
    Revision 1.114  2003/07/02 01:13:46  garyp
    Modified DEBUG builds to say "Debug Version" in the sign-on.
    Revision 1.106  2003/06/11 02:03:42  garyp
    Removed the "Beta" designation.
    Revision 1.76  2003/04/22 00:42:12  garys
    restored autoproj build 664 to make the build numbers consistent
    Revision 1.75  2003/04/16 19:57:39  billr
    Merge from VBF4 branch.
    Revision 1.73  2003/04/15 20:55:16Z  garyp
    Updated to include CPUNUM.H.
    Revision 1.72  2003/04/15 20:13:18Z  garyp
    Changed to v6.10 and turned on the "Beta" flag.
    Revision 1.66  2003/03/27 22:16:58  garyp
    Changed to version 6.03.  Updated the standardized product name
    and version strings.
    Revision 1.44.1.2  2003/03/14 22:31:39Z  billr
    C++ style comments are not allowed in ISO C89. Convert them to C style.
    Revision 1.53  2003/02/13 19:35:42  garyp
    Updated to version 6.02
    Revision 1.41  2003/01/14 19:52:18  garys
    updated for v6.01 engineering release
    Revision 1.15  2002/11/15 05:41:00  garyp
    Removed the "Beta" designation.
    Revision 1.13  2002/11/14 05:55:14  garyp
    Eliminated the "other patents pending" message.
    Revision 1.10  2002/11/12 04:46:36  dennis
    Added copyright and usage restrictions header to several source files.
    Revision 1.2  2002/10/18 19:12:52  garyp
    Updated the version number to 6.00, and reset the build number to
    600.  Set the ALPHABETA value to "Beta".
    10/11/02 gjs Updated for 5.11 release
    08/26/02 gp  Cleaned up the version strings.
    02/11/02 gp  Eliminated the use of the CPU types in the version string.
    02/07/02 DE  Updated copyright notice for 2002.
    02/07/02 de  Updated version to 5.10
    08/08/01 gjs Changed version from 4.08 to 5.00 for release.
    07/18/01 de  Added BBM patent number to signon string.
    04/16/01 mjm Corrected VERSIONNAME FlashFX case.
                 Corrected/added comments for #ENDIFs.
    03/21/01 mjm Changed version to 4.08 for release.
    03/08/01 DE  Updated patent and copyright strings.
    12/05/00 DE  Changed versioin to 4.07 for release.
    08/04/00 HDS Changed version from 4.05 to 4.06 for general release.
    04/03/00 gjs Changed version from 4.04 to 4.05 for vxworks eng rel
                 Changed copyright string from "93-99" to "1993-2002"
    12/15/99 gjs Changed version from 4.03 to 4.04 for general release
    04/05/99 gjs Changed TVC tracking number to MKS build numbers
    03/05/99 gjs Added ARM720 and SA1100 CE CPU's
    02/09/99 HDS Changed sign on message to limit line size to 20 characters.
    01/06/99 PKG Made patent note plural, updated copyright date
    12/16/98 DM  Fixed filename in heading
    12/16/98 DM  Added WINNT_TARGET and FXVMAPI defines for versions option
    09/30/98 PKG Added default VERSIONSTR if not target platform or CPU is
                 set by MACHINE.H.  Also added more WinCE targets.
    12/31/96 PKG Created
---------------------------------------------------------------------------*/

#ifndef FXVER_H_INCLUDED
#define FXVER_H_INCLUDED


    /*--------------------------------------------------------------*\
     *  The following sequence of lines is used in the automated    *
     *  build process to manage the versions, build numbers, and    *
     *  the build date.  The line following each of the commented   *
     *  keyword lines ($SIBLD...) contains a value enclosed in the  *
     *  delimeters that immediately follow the keyword.  Do not     *
     *  manually modify these lines unless you know what you are    *
     *  doing.                                                      *
    \*--------------------------------------------------------------*/

/*  $SIBLDNUM"" */
#define FFXBUILDNUM     "2128DF"

/*  $SIBLDDATE"" */
#define FFXBUILDDATE    "04/24/2012"

/*  $SIBLDVER""         Standard forms: "v2.4", "v2.4.1", "v2.x.x", or "Codename" */
#define FFXVERSIONSTR   "v2.1.1"


/*--- End of stuff used by AutoProj ---*/


    /*--------------------------------------------------------------*\
     *  NOTE! The FFXVERSIONSTR string above is manually changed    *
     *        when moving to a new version.  DO NOT forget to also  *
     *        change the VERSIONVAL value below, to match.          *
     *                                                              *
     *        DO NOT forget to also change the FX_VERSION and       *
     *        FX_VERSIONVAL settings in version.bat.                *
    \*--------------------------------------------------------------*/

/*  Version number in hex.  The low byte is either zero, or the lowercase
    hex representation of the alphabetic revision character -- 'a' == 0x61.
*/
#define VERSIONVAL      0x02010100UL    /* 2.1.1    */

#if 0
  #if 0
    #define ALPHABETA   " (Alpha)"
  #else
    #define ALPHABETA   " (Beta)"
  #endif
#else
  #define ALPHABETA     ""
#endif


    /*--------------------------------------------------------------*\
     *                                                              *
     *    Stuff below this point only changes when product name,    *
     *    copyright date, or patent numbers have changed.           *
     *                                                              *
    \*--------------------------------------------------------------*/

#if D_DEBUG
  #define PRODDEBUGMSG  " (DEBUG version)"
#else
  #define PRODDEBUGMSG  ""
#endif

#if defined(D_CPUTYPE)
  #define PRODCPUMSG    " for "DCLSTRINGIZE(D_CPUTYPE)
#else
  #define PRODCPUMSG    ""
#endif


    /*--------------------------------------------------------------*\
     * NOTE! The text in the strings below may be automatically     *
     *       changed by the kit building process (see bldcomp.bat), *
     *       as well as code in winhelpabout.c, and modifying these *
     *       strings without making the appropriate changes else-   *
     *       where may break things.                                *
    \*--------------------------------------------------------------*/

#define PRODUCTBASENAME "FlashFX Tera"
#define PRODUCTPREFIX   "FX"
#define PRODUCTVERSION  FFXVERSIONSTR "-" FFXBUILDNUM ALPHABETA
#define PRODUCTNAME     "Datalight "PRODUCTBASENAME" "FFXVERSIONSTR" Build "FFXBUILDNUM ALPHABETA PRODDEBUGMSG
#define PRODUCTLEGAL    "Copyright (c) 1993-2012 Datalight, Inc.  All Rights Reserved Worldwide."
#define PRODUCTPATENT   "Patents:  US#5860082, US#6260156."
#define PRODUCTEDITION  DCL_OSNAME" Edition"PRODCPUMSG" -- Compiled "__DATE__" at "__TIME__


#endif  /* FXVER_H_INCLUDED */

