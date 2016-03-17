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
  jurisdictions.  Patents may be pending.

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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    Provides defines for the product name, version, copyright, ...

    !!!! DO NOT MODIFY THE "SIBLD" LINES !!!!
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlver.h $
    Revision 1.928.1.59  2012/04/25 04:37:04Z  qa
    AUTOPROJ Build 1190BH
    Revision 1.928.1.58  2012/04/18 01:52:40Z  qa
    AUTOPROJ Build 1190BG
    Revision 1.928.1.57  2012/04/17 21:56:27Z  garyp
    Updated for the pending release.
    Revision 1.928.1.56  2012/04/14 04:44:09Z  qa
    AUTOPROJ Build 1190BE
    Revision 1.928.1.55  2012/04/13 04:26:32Z  qa
    AUTOPROJ Build 1190BD
    Revision 1.928.1.54  2012/04/12 04:13:24Z  qa
    AUTOPROJ Build 1190BC
    Revision 1.928.1.53  2012/04/07 05:21:17Z  qa
    AUTOPROJ Build 1190BB
    Revision 1.928.1.52  2012/04/05 04:43:43Z  qa
    AUTOPROJ Build 1190BA
    Revision 1.928.1.51  2012/03/23 04:11:20Z  qa
    AUTOPROJ Build 1190AZ
    Revision 1.928.1.50  2012/02/10 19:30:25Z  qa
    AUTOPROJ Build 1190AY
    Revision 1.928.1.49  2011/12/15 22:55:45Z  qa
    AUTOPROJ Build 1190AX
    Revision 1.928.1.48  2011/12/15 05:21:25Z  qa
    AUTOPROJ Build 1190AW
    Revision 1.928.1.47  2011/12/06 05:46:22Z  qa
    AUTOPROJ Build 1190AV
    Revision 1.928.1.46  2011/12/02 05:26:19Z  qa
    AUTOPROJ Build 1190AT
    Revision 1.928.1.45  2011/12/01 05:13:23Z  qa
    AUTOPROJ Build 1190AS
    Revision 1.928.1.44  2011/11/23 05:21:23Z  qa
    AUTOPROJ Build 1190AR
    Revision 1.928.1.43  2011/11/18 07:36:21Z  qa
    AUTOPROJ Build 1190AQ
    Revision 1.928.1.42  2011/10/26 00:56:15Z  qa
    AUTOPROJ Build 1190AP
    Revision 1.928.1.41  2011/10/26 00:48:25Z  garyp
    Removed the BETA flag.
    Revision 1.928.1.40  2011/06/16 04:41:24Z  qa
    AUTOPROJ Build 1190AN
    Revision 1.928.1.39  2011/06/15 04:41:19Z  qa
    AUTOPROJ Build 1190AM
    Revision 1.928.1.38  2011/06/03 09:51:28Z  qa
    AUTOPROJ Build 1190AL
    Revision 1.928.1.37  2011/05/24 18:55:26Z  qa
    AUTOPROJ Build 1190AK
    Revision 1.928.1.36  2011/05/24 18:50:20Z  garyp
    Updated to v3.1.2 BETA.
    Revision 1.928.1.35  2011/05/20 04:13:24Z  qa
    AUTOPROJ Build 1190AJ
    Revision 1.928.1.34  2011/05/05 04:13:11Z  qa
    AUTOPROJ Build 1190AH
    Revision 1.928.1.33  2011/04/29 04:11:15Z  qa
    AUTOPROJ Build 1190AG
    Revision 1.928.1.32  2011/04/28 04:36:13Z  qa
    AUTOPROJ Build 1190AF
    Revision 1.928.1.31  2011/04/24 04:17:01Z  qa
    AUTOPROJ Build 1190AE
    Revision 1.928.1.30  2011/04/23 01:40:35Z  qa
    AUTOPROJ Build 1190AD
    Revision 1.928.1.29  2011/04/19 04:13:15Z  qa
    AUTOPROJ Build 1190AC
    Revision 1.928.1.28  2011/04/18 04:16:20Z  qa
    AUTOPROJ Build 1190AB
    Revision 1.928.1.27  2011/04/16 04:19:15Z  qa
    AUTOPROJ Build 1190AA
    Revision 1.928.1.26  2011/04/15 02:51:13Z  qa
    AUTOPROJ Build 1190Z
    Revision 1.928.1.25  2011/03/30 04:16:10Z  qa
    AUTOPROJ Build 1190Y
    Revision 1.928.1.24  2011/03/20 23:21:50Z  qa
    AUTOPROJ Build 1190X
    Revision 1.928.1.23  2011/03/16 04:11:20Z  qa
    AUTOPROJ Build 1190W
    Revision 1.928.1.22  2011/03/03 05:11:25Z  qa
    AUTOPROJ Build 1190V
    Revision 1.928.1.21  2011/02/15 05:11:29Z  qa
    AUTOPROJ Build 1190T
    Revision 1.928.1.20  2011/02/10 04:03:37Z  qa
    AUTOPROJ Build 1190S
    Revision 1.928.1.19  2011/02/06 05:28:10Z  qa
    AUTOPROJ Build 1190R
    Revision 1.928.1.18  2011/02/04 05:13:03Z  qa
    AUTOPROJ Build 1190Q
    Revision 1.928.1.17  2011/01/23 05:22:31Z  qa
    AUTOPROJ Build 1190P
    Revision 1.928.1.16  2011/01/19 00:13:19Z  qa
    AUTOPROJ Build 1190N
    Revision 1.928.1.15  2011/01/18 23:06:00Z  garyp
    Updated for the pending release.
    Revision 1.928.1.14  2011/01/15 05:21:06Z  qa
    AUTOPROJ Build 1190M
    Revision 1.928.1.13  2011/01/13 05:34:04Z  qa
    AUTOPROJ Build 1190L
    Revision 1.928.1.12  2011/01/10 05:39:53Z  qa
    AUTOPROJ Build 1190K
    Revision 1.928.1.11  2011/01/05 05:18:56Z  qa
    AUTOPROJ Build 1190J
    Revision 1.928.1.10  2011/01/02 05:16:25Z  qa
    AUTOPROJ Build 1190H
    Revision 1.928.1.9  2011/01/01 19:50:47Z  garyp
    Changed to v3.1.1 BETA.  Updated the copyright date.
    Revision 1.928.1.8  2010/12/26 05:16:29Z  qa
    AUTOPROJ Build 1190G
    Revision 1.928.1.7  2010/12/25 05:16:03Z  qa
    AUTOPROJ Build 1190F
    Revision 1.928.1.6  2010/12/22 03:51:47Z  qa
    AUTOPROJ Build 1190E
    Revision 1.928.1.5  2010/12/21 05:38:34Z  qa
    AUTOPROJ Build 1190D
    Revision 1.928.1.4  2010/12/17 05:12:46Z  qa
    AUTOPROJ Build 1190C
    Revision 1.928.1.3  2010/12/14 17:51:11Z  qa
    AUTOPROJ Build 1190B
    Revision 1.928.1.2  2010/12/14 17:23:27Z  qa
    Updated for the new v3.1 branch.
    Revision 1.928  2010/12/14 05:03:36Z  qa
    AUTOPROJ Build 1190
    Revision 1.927  2010/12/11 20:16:03Z  qa
    AUTOPROJ Build 1189
    Revision 1.926  2010/12/11 03:49:06Z  qa
    AUTOPROJ Build 1188
    Revision 1.925  2010/12/10 05:07:54Z  qa
    AUTOPROJ Build 1187
    Revision 1.924  2010/12/09 05:06:35Z  qa
    AUTOPROJ Build 1186
    Revision 1.923  2010/12/08 02:21:38Z  qa
    AUTOPROJ Build 1185
    Revision 1.922  2010/12/08 01:15:47Z  qa
    AUTOPROJ Build 1184
    Revision 1.921  2010/12/03 02:06:23Z  qa
    AUTOPROJ Build 1183
    Revision 1.920  2010/12/02 05:06:36Z  qa
    AUTOPROJ Build 1182
    Revision 1.919  2010/12/01 05:03:37Z  qa
    AUTOPROJ Build 1181
    Revision 1.918  2010/11/28 05:07:01Z  qa
    AUTOPROJ Build 1180
    Revision 1.917  2010/11/20 05:03:55Z  qa
    AUTOPROJ Build 1179
    Revision 1.916  2010/11/17 05:04:05Z  qa
    AUTOPROJ Build 1178
    Revision 1.915  2010/11/13 02:45:48Z  qa
    AUTOPROJ Build 1177
    Revision 1.914  2010/11/12 02:26:08Z  qa
    AUTOPROJ Build 1176
    Revision 1.913  2010/11/05 04:06:37Z  qa
    AUTOPROJ Build 1175
    Revision 1.912  2010/11/03 02:54:53Z  qa
    AUTOPROJ Build 1174
    Revision 1.911  2010/11/01 16:36:48Z  qa
    AUTOPROJ Build 1173
    Revision 1.910  2010/11/01 16:34:34Z  garyp
    Changed to Alpha-3.
    Revision 1.909  2010/11/01 06:57:57Z  qa
    AUTOPROJ Build 1172
    Revision 1.908  2010/11/01 04:36:56Z  qa
    AUTOPROJ Build 1171
    Revision 1.907  2010/10/23 02:24:46Z  qa
    AUTOPROJ Build 1170
    Revision 1.906  2010/10/22 04:03:45Z  qa
    AUTOPROJ Build 1169
    Revision 1.905  2010/10/21 20:33:44Z  qa
    AUTOPROJ Build 1168
    Revision 1.904  2010/10/21 04:06:36Z  qa
    AUTOPROJ Build 1167
    Revision 1.903  2010/10/19 22:42:43Z  qa
    AUTOPROJ Build 1166
    Revision 1.902  2010/10/18 04:21:36Z  qa
    AUTOPROJ Build 1165
    Revision 1.901  2010/10/17 22:51:53Z  qa
    AUTOPROJ Build 1164
    Revision 1.900  2010/10/17 04:07:11Z  qa
    AUTOPROJ Build 1163
    Revision 1.899  2010/10/15 04:03:36Z  qa
    AUTOPROJ Build 1162
    Revision 1.898  2010/10/13 04:06:45Z  qa
    AUTOPROJ Build 1161
    Revision 1.897  2010/10/12 18:30:46Z  garyp
    Changed to Alpha-2.
    Revision 1.896  2010/10/12 04:03:46Z  qa
    AUTOPROJ Build 1160
    Revision 1.895  2010/10/10 04:07:14Z  qa
    AUTOPROJ Build 1159
    Revision 1.894  2010/10/09 04:06:37Z  qa
    AUTOPROJ Build 1158
    Revision 1.893  2010/10/08 04:03:36Z  qa
    AUTOPROJ Build 1157
    Revision 1.892  2010/10/07 04:06:36Z  qa
    AUTOPROJ Build 1156
    Revision 1.891  2010/10/06 04:06:45Z  qa
    AUTOPROJ Build 1155
    Revision 1.890  2010/10/04 18:04:48Z  qa
    AUTOPROJ Build 1154
    Revision 1.889  2010/10/04 01:01:03Z  qa
    AUTOPROJ Build 1153
    Revision 1.888  2010/10/01 20:39:37Z  qa
    AUTOPROJ Build 1152
    Revision 1.887  2010/09/29 00:54:40Z  qa
    AUTOPROJ Build 1151
    Revision 1.886  2010/09/25 04:05:55Z  qa
    AUTOPROJ Build 1150
    Revision 1.885  2010/09/23 20:51:39Z  qa
    AUTOPROJ Build 1149
    Revision 1.884  2010/09/23 09:09:43Z  qa
    AUTOPROJ Build 1148
    Revision 1.883  2010/09/20 05:56:06Z  qa
    AUTOPROJ Build 1147
    Revision 1.882  2010/09/19 04:07:11Z  qa
    AUTOPROJ Build 1146
    Revision 1.881  2010/09/18 04:06:44Z  qa
    AUTOPROJ Build 1145
    Revision 1.880  2010/09/17 04:06:46Z  qa
    AUTOPROJ Build 1144
    Revision 1.879  2010/09/15 04:03:43Z  qa
    AUTOPROJ Build 1143
    Revision 1.878  2010/09/14 04:06:43Z  qa
    AUTOPROJ Build 1142
    Revision 1.877  2010/09/11 01:12:47Z  qa
    AUTOPROJ Build 1141
    Revision 1.876  2010/09/10 22:20:59Z  qa
    AUTOPROJ Build 1140
    Revision 1.875  2010/09/09 04:03:43Z  qa
    AUTOPROJ Build 1139
    Revision 1.874  2010/09/08 19:42:34Z  qa
    AUTOPROJ Build 1138
    Revision 1.873  2010/09/08 18:50:36Z  qa
    AUTOPROJ Build 1137
    Revision 1.872  2010/09/08 02:27:34Z  qa
    AUTOPROJ Build 1136
    Revision 1.871  2010/09/08 01:54:58Z  qa
    AUTOPROJ Build 1135
    Revision 1.870  2010/09/03 04:06:49Z  qa
    AUTOPROJ Build 1134
    Revision 1.869  2010/09/01 01:24:48Z  qa
    AUTOPROJ Build 1133
    Revision 1.868  2010/08/31 18:53:41Z  garyp
    Updated to Alpha-1.
    Revision 1.867  2010/08/31 04:06:43Z  qa
    AUTOPROJ Build 1132
    Revision 1.866  2010/08/30 00:54:49Z  qa
    AUTOPROJ Build 1131
    Revision 1.865  2010/08/28 18:10:51Z  qa
    AUTOPROJ Build 1130
    Revision 1.864  2010/08/28 04:20:55Z  qa
    AUTOPROJ Build 1129
    Revision 1.863  2010/08/25 04:20:57Z  qa
    AUTOPROJ Build 1128
    Revision 1.862  2010/08/24 04:15:53Z  qa
    AUTOPROJ Build 1127
    Revision 1.861  2010/08/12 04:06:33Z  qa
    AUTOPROJ Build 1126
    Revision 1.860  2010/08/07 04:03:32Z  qa
    AUTOPROJ Build 1125
    Revision 1.859  2010/08/04 18:46:03Z  qa
    AUTOPROJ Build 1124
    Revision 1.858  2010/08/04 02:09:52Z  qa
    AUTOPROJ Build 1123
    Revision 1.857  2010/08/03 22:48:22Z  garyp
    Updated to define PRODUCTBASENAME and PRODUCTPREFIX.
    Revision 1.856  2010/08/01 05:05:27Z  qa
    AUTOPROJ Build 1122
    Revision 1.855  2010/07/31 23:12:50Z  qa
    AUTOPROJ Build 1121
    Revision 1.854  2010/07/29 01:59:59Z  qa
    AUTOPROJ Build 1120
    Revision 1.853  2010/07/28 04:06:46Z  qa
    AUTOPROJ Build 1119
    Revision 1.852  2010/07/22 04:06:42Z  qa
    AUTOPROJ Build 1118
    Revision 1.851  2010/07/18 04:03:42Z  qa
    AUTOPROJ Build 1117
    Revision 1.850  2010/07/16 04:06:30Z  qa
    AUTOPROJ Build 1116
    Revision 1.849  2010/07/15 04:06:31Z  qa
    AUTOPROJ Build 1115
    Revision 1.848  2010/07/14 04:06:43Z  qa
    AUTOPROJ Build 1114
    Revision 1.847  2010/07/02 04:16:42Z  qa
    AUTOPROJ Build 1113
    Revision 1.846  2010/06/29 04:09:38Z  autotest
    AUTOPROJ Build 1112
    Revision 1.845  2010/06/26 04:15:59Z  qa
    AUTOPROJ Build 1111
    Revision 1.844  2010/06/25 04:06:36Z  autotest
    AUTOPROJ Build 1110
    Revision 1.843  2010/06/23 04:06:37Z  autotest
    AUTOPROJ Build 1109
    Revision 1.842  2010/06/22 04:03:42Z  qa
    AUTOPROJ Build 1108
    Revision 1.841  2010/06/19 04:45:42Z  autotest
    AUTOPROJ Build 1107
    Revision 1.840  2010/06/18 04:06:41Z  qa
    AUTOPROJ Build 1106
    Revision 1.839  2010/06/17 04:06:42Z  qa
    AUTOPROJ Build 1105
    Revision 1.838  2010/06/15 04:03:42Z  qa
    AUTOPROJ Build 1104
    Revision 1.837  2010/06/14 04:39:35Z  autotest
    AUTOPROJ Build 1103
    Revision 1.836  2010/06/13 07:10:56Z  qa
    AUTOPROJ Build 1102
    Revision 1.835  2010/06/13 04:03:41Z  qa
    AUTOPROJ Build 1101
    Revision 1.834  2010/06/12 05:03:06Z  qa
    AUTOPROJ Build 1100
    Revision 1.833  2010/06/10 04:03:41Z  qa
    AUTOPROJ Build 1099
    Revision 1.832  2010/06/09 05:39:46Z  qa
    AUTOPROJ Build 1098
    Revision 1.831  2010/06/08 21:09:45Z  qa
    AUTOPROJ Build 1097
    Revision 1.830  2010/06/08 19:36:52Z  qa
    AUTOPROJ Build 1096
    Revision 1.829  2010/06/08 04:10:58Z  qa
    AUTOPROJ Build 1095
    Revision 1.828  2010/06/02 16:39:50Z  autotest
    AUTOPROJ Build 1094
    Revision 1.827  2010/06/02 15:26:43Z  qa
    AUTOPROJ Build 1093
    Revision 1.826  2010/06/02 04:36:00Z  qa
    AUTOPROJ Build 1092
    Revision 1.825  2010/05/24 04:05:55Z  qa
    AUTOPROJ Build 1091
    Revision 1.824  2010/05/23 04:03:41Z  autotest
    AUTOPROJ Build 1090
    Revision 1.823  2010/05/20 23:36:14Z  qa
    AUTOPROJ Build 1089
    Revision 1.822  2010/05/20 04:06:02Z  qa
    AUTOPROJ Build 1088
    Revision 1.821  2010/05/19 16:54:52Z  qa
    AUTOPROJ Build 1087
    Revision 1.820  2010/05/18 22:09:57Z  qa
    AUTOPROJ Build 1086
    Revision 1.819  2010/05/16 16:58:04Z  qa
    AUTOPROJ Build 1085
    Revision 1.818  2010/05/15 17:41:50Z  autotest
    AUTOPROJ Build 1084
    Revision 1.817  2010/05/15 06:39:30Z  qa
    AUTOPROJ Build 1083
    Revision 1.816  2010/05/12 04:06:58Z  qa
    AUTOPROJ Build 1082
    Revision 1.815  2010/05/11 04:39:55Z  qa
    AUTOPROJ Build 1081
    Revision 1.814  2010/05/10 03:58:33Z  qa
    AUTOPROJ Build 1080
    Revision 1.813  2010/05/09 18:10:01Z  qa
    AUTOPROJ Build 1079
    Revision 1.812  2010/05/09 16:55:59Z  qa
    AUTOPROJ Build 1078
    Revision 1.811  2010/05/09 04:07:02Z  qa
    AUTOPROJ Build 1077
    Revision 1.810  2010/05/08 22:17:17Z  qa
    AUTOPROJ Build 1076
    Revision 1.809  2010/05/08 20:18:23Z  qa
    AUTOPROJ Build 1075
    Revision 1.808  2010/05/08 18:29:30Z  qa
    AUTOPROJ Build 1074
    Revision 1.807  2010/05/08 16:18:01Z  qa
    AUTOPROJ Build 1073
    Revision 1.806  2010/05/07 23:15:54Z  qa
    AUTOPROJ Build 1072
    Revision 1.805  2010/05/07 21:49:13Z  qa
    AUTOPROJ Build 1071
    Revision 1.804  2010/05/07 18:34:58Z  qa
    AUTOPROJ Build 1070
    Revision 1.803  2010/05/06 04:06:42Z  garyp
    AUTOPROJ Build 1069
    Revision 1.802  2010/05/05 01:23:51Z  qa
    AUTOPROJ Build 1068
    Revision 1.801  2010/05/04 04:09:57Z  qa
    AUTOPROJ Build 1067
    Revision 1.800  2010/05/03 02:17:50Z  garyp
    AUTOPROJ Build 1066
    Revision 1.799  2010/05/02 21:23:04Z  qa
    AUTOPROJ Build 1065
    Revision 1.798  2010/05/02 17:01:08Z  qa
    AUTOPROJ Build 1064
    Revision 1.797  2010/05/01 21:28:25Z  qa
    AUTOPROJ Build 1063
    Revision 1.796  2010/05/01 20:31:36Z  qa
    AUTOPROJ Build 1062
    Revision 1.795  2010/05/01 02:58:42Z  qa
    AUTOPROJ Build 1061
    Revision 1.794  2010/04/29 04:00:57Z  qa
    AUTOPROJ Build 1060
    Revision 1.793  2010/04/28 04:05:58Z  qa
    AUTOPROJ Build 1059
    Revision 1.792  2010/04/23 04:05:55Z  qa
    AUTOPROJ Build 1058
    Revision 1.791  2010/04/22 04:00:47Z  qa
    AUTOPROJ Build 1057
    Revision 1.790  2010/04/19 01:30:43Z  qa
    AUTOPROJ Build 1056
    Revision 1.789  2010/04/18 21:46:51Z  qa
    AUTOPROJ Build 1055
    Revision 1.788  2010/04/12 05:58:18Z  qa
    AUTOPROJ Build 1054
    Revision 1.787  2010/04/12 04:00:50Z  qa
    AUTOPROJ Build 1053
    Revision 1.786  2010/03/26 18:30:48Z  qa
    AUTOPROJ Build 1052
    Revision 1.785  2010/03/23 04:01:01Z  qa
    AUTOPROJ Build 1051
    Revision 1.784  2010/03/19 04:01:00Z  qa
    AUTOPROJ Build 1050
    Revision 1.783  2010/03/18 04:05:55Z  qa
    AUTOPROJ Build 1049
    Revision 1.782  2010/03/11 07:00:54Z  qa
    AUTOPROJ Build 1048
    Revision 1.781  2010/03/10 07:05:52Z  qa
    AUTOPROJ Build 1047
    Revision 1.780  2010/03/09 07:05:51Z  qa
    AUTOPROJ Build 1046
    Revision 1.779  2010/03/06 07:05:54Z  qa
    AUTOPROJ Build 1045
    Revision 1.778  2010/03/04 05:20:58Z  qa
    AUTOPROJ Build 1044
    Revision 1.777  2010/02/27 05:10:56Z  qa
    AUTOPROJ Build 1043
    Revision 1.776  2010/02/26 05:25:54Z  qa
    AUTOPROJ Build 1042
    Revision 1.775  2010/02/26 01:49:49Z  qa
    AUTOPROJ Build 1041
    Revision 1.774  2010/02/25 05:00:54Z  qa
    AUTOPROJ Build 1040
    Revision 1.773  2010/02/24 05:10:52Z  qa
    AUTOPROJ Build 1039
    Revision 1.772  2010/02/23 06:30:55Z  qa
    AUTOPROJ Build 1038
    Revision 1.771  2010/02/20 05:01:04Z  qa
    AUTOPROJ Build 1037
    Revision 1.770  2010/02/19 21:09:07Z  garyp
    Condensed some of the revision history.
    Revision 1.769  2010/02/19 05:00:57Z  qa
    AUTOPROJ Build 1036
    Revision 1.768  2010/02/18 05:01:02Z  qa
    AUTOPROJ Build 1035
    Revision 1.767  2010/02/16 05:10:59Z  qa
    AUTOPROJ Build 1034
    Revision 1.766  2010/02/14 21:14:13Z  garyp
    AUTOPROJ Build 1033
    Revision 1.765  2010/02/14 20:17:40Z  qa
    AUTOPROJ Build 1032
    Revision 1.764  2010/02/14 01:51:03Z  qa
    AUTOPROJ Build 1031
    Revision 1.763  2010/02/13 20:45:58Z  qa
    AUTOPROJ Build 1030
    Revision 1.762  2010/02/13 05:01:12Z  qa
    AUTOPROJ Build 1029
    Revision 1.761  2010/02/12 05:01:06Z  qa
    AUTOPROJ Build 1028
    Revision 1.760  2010/02/11 05:01:07Z  qa
    AUTOPROJ Build 1027
    Revision 1.759  2010/02/05 03:18:30Z  qa
    AUTOPROJ Build 1026
    Revision 1.758  2010/02/03 00:13:05Z  qa
    AUTOPROJ Build 1025
    Revision 1.757  2010/01/27 05:01:04Z  qa
    AUTOPROJ Build 1024
    Revision 1.756  2010/01/24 00:16:05Z  qa
    AUTOPROJ Build 1023
    Revision 1.755  2010/01/23 16:48:53Z  qa
    AUTOPROJ Build 1022
    Revision 1.754  2010/01/13 05:05:58Z  qa
    AUTOPROJ Build 1021
    Revision 1.753  2010/01/12 05:01:07Z  qa
    AUTOPROJ Build 1020
    Revision 1.752  2010/01/11 05:01:00Z  qa
    AUTOPROJ Build 1019
    Revision 1.751  2010/01/10 05:01:12Z  qa
    AUTOPROJ Build 1018
    Revision 1.750  2010/01/09 05:01:08Z  qa
    AUTOPROJ Build 1017
    Revision 1.749  2010/01/08 02:06:06Z  qa
    AUTOPROJ Build 1016
    Revision 1.748  2010/01/08 01:22:17Z  garyp
    AUTOPROJ Build 1015
    Revision 1.747  2010/01/08 00:23:25Z  garyp
    AUTOPROJ Build 1014
    Revision 1.746  2010/01/07 02:50:20Z  qa
    AUTOPROJ Build 1013
    Revision 1.745  2010/01/06 05:01:13Z  qa
    AUTOPROJ Build 1012
    Revision 1.744  2010/01/05 22:53:07Z  garyp
    Updated the copyright date.
    Revision 1.743  2010/01/05 17:01:00Z  qa
    AUTOPROJ Build 1011
    Revision 1.742  2010/01/04 22:40:03Z  qa
    AUTOPROJ Build 1010
    Revision 1.741  2009/12/24 05:00:58Z  qa
    AUTOPROJ Build 1009
    Revision 1.740  2009/12/23 18:30:52Z  qa
    AUTOPROJ Build 1008
    Revision 1.739  2009/12/22 02:01:57Z  qa
    AUTOPROJ Build 1007
    Revision 1.738  2009/12/18 18:50:46Z  qa
    AUTOPROJ Build 1006
    Revision 1.737  2009/12/14 05:00:55Z  qa
    AUTOPROJ Build 1005
    Revision 1.736  2009/12/13 09:05:46Z  qa
    AUTOPROJ Build 1004
    Revision 1.735  2009/12/13 02:50:55Z  qa
    AUTOPROJ Build 1003
    Revision 1.734  2009/12/12 19:26:01Z  qa
    AUTOPROJ Build 1002
    Revision 1.733  2009/12/05 05:00:56Z  qa
    AUTOPROJ Build 1001
    Revision 1.732  2009/12/03 05:01:03Z  qa
    AUTOPROJ Build 1000
    Revision 1.731  2009/12/02 05:01:22Z  qa
    AUTOPROJ Build 999
    Revision 1.730  2009/12/01 05:01:04Z  qa
    AUTOPROJ Build 998
    Revision 1.729  2009/11/25 23:46:02Z  qa
    AUTOPROJ Build 997
    Revision 1.728  2009/11/24 05:01:02Z  qa
    AUTOPROJ Build 996
    Revision 1.727  2009/11/20 05:01:03Z  qa
    AUTOPROJ Build 995
    Revision 1.726  2009/11/18 05:01:02Z  qa
    AUTOPROJ Build 994
    Revision 1.725  2009/11/13 05:00:53Z  qa
    AUTOPROJ Build 993
    Revision 1.724  2009/11/12 05:01:26Z  qa
    AUTOPROJ Build 992
    Revision 1.723  2009/11/11 17:55:55Z  qa
    AUTOPROJ Build 991
    Revision 1.722  2009/11/10 05:11:01Z  qa
    AUTOPROJ Build 990
    Revision 1.721  2009/11/08 05:01:09Z  qa
    AUTOPROJ Build 989
    Revision 1.720  2009/11/06 05:01:02Z  qa
    AUTOPROJ Build 988
    Revision 1.719  2009/11/05 05:00:48Z  qa
    AUTOPROJ Build 987
    Revision 1.718  2009/11/04 05:00:58Z  qa
    AUTOPROJ Build 986
    Revision 1.717  2009/11/03 05:10:52Z  qa
    AUTOPROJ Build 985
    Revision 1.716  2009/10/31 04:01:05Z  qa
    AUTOPROJ Build 984
    Revision 1.715  2009/10/29 04:01:01Z  qa
    AUTOPROJ Build 983
    Revision 1.714  2009/10/21 04:01:13Z  qa
    AUTOPROJ Build 982
    Revision 1.713  2009/10/18 01:16:01Z  qa
    AUTOPROJ Build 981
    Revision 1.712  2009/10/17 04:01:08Z  qa
    AUTOPROJ Build 980
    Revision 1.711  2009/10/15 00:12:49Z  qa
    AUTOPROJ Build 979
    Revision 1.710  2009/10/14 04:10:55Z  qa
    AUTOPROJ Build 978
    Revision 1.709  2009/10/10 00:20:51Z  qa
    AUTOPROJ Build 977
    Revision 1.708  2009/10/09 18:36:01Z  qa
    AUTOPROJ Build 976
    Revision 1.707  2009/10/09 02:26:05Z  qa
    AUTOPROJ Build 975
    Revision 1.706  2009/10/07 04:01:21Z  qa
    AUTOPROJ Build 974
    Revision 1.705  2009/10/03 04:01:17Z  qa
    AUTOPROJ Build 973
    Revision 1.704  2009/10/03 01:16:27Z  garyp
    Updated to v3.1.
    Revision 1.703  2009/10/02 04:01:21Z  qa
    AUTOPROJ Build 972
    Revision 1.702  2009/10/01 04:01:10Z  qa
    AUTOPROJ Build 971
    Revision 1.701  2009/09/30 04:10:52Z  qa
    AUTOPROJ Build 970
    Revision 1.700  2009/09/26 04:01:14Z  qa
    AUTOPROJ Build 969
    Revision 1.699  2009/09/24 04:10:51Z  qa
    AUTOPROJ Build 968
    Revision 1.698  2009/09/22 22:55:52Z  qa
    AUTOPROJ Build 967
    Revision 1.697  2009/09/21 04:01:29Z  qa
    AUTOPROJ Build 966
    Revision 1.696  2009/09/18 04:11:02Z  qa
    AUTOPROJ Build 965
    Revision 1.695  2009/09/17 02:22:55Z  qa
    AUTOPROJ Build 964
    Revision 1.694  2009/09/17 00:23:18Z  qa
    AUTOPROJ Build 963
    Revision 1.693  2009/09/16 23:40:54Z  qa
    AUTOPROJ Build 962
    Revision 1.692  2009/09/16 22:25:52Z  qa
    AUTOPROJ Build 961
    Revision 1.691  2009/09/16 03:32:22Z  qa
    AUTOPROJ Build 960
    Revision 1.690  2009/09/15 04:01:21Z  qa
    AUTOPROJ Build 959
    Revision 1.689  2009/09/14 04:10:53Z  qa
    AUTOPROJ Build 958
    Revision 1.688  2009/09/11 04:50:47Z  qa
    AUTOPROJ Build 957
    Revision 1.687  2009/09/10 21:47:00Z  qa
    AUTOPROJ Build 956
    Revision 1.686  2009/09/09 04:01:21Z  qa
    AUTOPROJ Build 955
    Revision 1.685  2009/09/04 04:01:21Z  qa
    AUTOPROJ Build 954
    Revision 1.684  2009/09/03 04:01:18Z  qa
    AUTOPROJ Build 953
    Revision 1.683  2009/09/02 21:40:45Z  qa
    AUTOPROJ Build 952
    Revision 1.682  2009/08/29 04:01:33Z  qa
    AUTOPROJ Build 951
    Revision 1.681  2009/08/27 23:25:57Z  qa
    AUTOPROJ Build 950
    Revision 1.680  2009/08/26 22:54:38Z  qa
    AUTOPROJ Build 949
    Revision 1.679  2009/08/26 22:05:14Z  qa
    AUTOPROJ Build 948
    Revision 1.678  2009/08/26 21:05:48Z  qa
    AUTOPROJ Build 947
    Revision 1.677  2009/08/26 04:10:48Z  qa
    AUTOPROJ Build 946
    Revision 1.676  2009/08/22 04:01:00Z  qa
    AUTOPROJ Build 945
    Revision 1.675  2009/08/21 04:00:54Z  qa
    AUTOPROJ Build 944
    Revision 1.674  2009/08/15 04:00:52Z  qa
    AUTOPROJ Build 943
    Revision 1.673  2009/08/08 04:00:54Z  qa
    AUTOPROJ Build 942
    Revision 1.672  2009/08/07 04:10:57Z  qa
    AUTOPROJ Build 941
    Revision 1.671  2009/08/03 15:45:49Z  qa
    AUTOPROJ Build 940
    Revision 1.670  2009/07/28 03:20:48Z  qa
    AUTOPROJ Build 939
    Revision 1.669  2009/07/17 05:20:52Z  qa
    AUTOPROJ Build 938
    Revision 1.668  2009/07/15 18:00:49Z  qa
    AUTOPROJ Build 937
    Revision 1.667  2009/07/15 16:30:48Z  qa
    AUTOPROJ Build 936
    Revision 1.666  2009/07/15 01:30:45Z  qa
    AUTOPROJ Build 935
    Revision 1.665  2009/07/08 04:10:48Z  qa
    AUTOPROJ Build 934
    Revision 1.664  2009/07/02 23:27:15Z  qa
    AUTOPROJ Build 933
    Revision 1.663  2009/07/02 19:45:57Z  qa
    AUTOPROJ Build 932
    Revision 1.662  2009/07/01 23:20:57Z  qa
    AUTOPROJ Build 931
    Revision 1.661  2009/06/29 02:10:56Z  qa
    AUTOPROJ Build 930
    Revision 1.660  2009/06/28 04:08:19Z  qa
    AUTOPROJ Build 929
    Revision 1.659  2009/06/26 04:01:00Z  qa
    AUTOPROJ Build 928
    Revision 1.658  2009/06/24 04:01:11Z  qa
    AUTOPROJ Build 927
    Revision 1.657  2009/06/19 04:01:13Z  qa
    AUTOPROJ Build 926
    Revision 1.656  2009/06/16 04:01:09Z  qa
    AUTOPROJ Build 925
    Revision 1.655  2009/06/14 04:01:05Z  qa
    AUTOPROJ Build 924
    Revision 1.654  2009/06/12 02:46:02Z  qa
    AUTOPROJ Build 923
    Revision 1.653  2009/05/31 04:01:02Z  qa
    AUTOPROJ Build 922
    Revision 1.652  2009/05/30 04:01:06Z  qa
    AUTOPROJ Build 921
    Revision 1.651  2009/05/28 01:21:02Z  qa
    AUTOPROJ Build 920
    Revision 1.650  2009/05/27 02:39:04Z  qa
    AUTOPROJ Build 919
    Revision 1.649  2009/05/23 15:41:41Z  qa
    AUTOPROJ Build 918
    Revision 1.648  2009/05/23 04:01:04Z  qa
    AUTOPROJ Build 917
    Revision 1.647  2009/05/22 03:35:42Z  qa
    AUTOPROJ Build 916
    Revision 1.646  2009/05/21 05:03:09Z  qa
    AUTOPROJ Build 915
    Revision 1.645  2009/05/20 02:20:59Z  qa
    AUTOPROJ Build 914
    Revision 1.644  2009/05/19 21:34:03Z  garyp
    Extended "VERSIONVAL" to use the full four bytes for the version number
    and no longer tack the "Alpha" or "Beta" onto the end.
    Revision 1.643  2009/05/19 05:40:59Z  qa
    AUTOPROJ Build 913
    Revision 1.642  2009/05/18 05:48:25Z  qa
    AUTOPROJ Build 912
    Revision 1.641  2009/05/17 17:55:07Z  qa
    AUTOPROJ Build 911
    Revision 1.640  2009/05/16 04:20:10Z  qa
    AUTOPROJ Build 910
    Revision 1.639  2009/05/16 02:32:34Z  qa
    AUTOPROJ Build 909
    Revision 1.638  2009/05/15 10:02:00Z  qa
    AUTOPROJ Build 908
    Revision 1.637  2009/05/15 04:01:25Z  qa
    AUTOPROJ Build 907
    Revision 1.636  2009/05/13 03:25:54Z  qa
    AUTOPROJ Build 906
    Revision 1.635  2009/05/13 02:29:48Z  qa
    AUTOPROJ Build 905
    Revision 1.634  2009/05/12 16:09:48Z  qa
    AUTOPROJ Build 904
    Revision 1.633  2009/05/11 18:40:37Z  qa
    AUTOPROJ Build 903
    Revision 1.632  2009/05/09 19:40:56Z  qa
    AUTOPROJ Build 902
    Revision 1.631  2009/05/09 02:50:57Z  qa
    AUTOPROJ Build 901
    Revision 1.630  2009/05/08 22:11:03Z  qa
    AUTOPROJ Build 900
    Revision 1.629  2009/05/08 21:45:58Z  qa
    AUTOPROJ Build 899
    Revision 1.628  2009/05/08 19:15:52Z  qa
    AUTOPROJ Build 898
    Revision 1.627  2009/05/08 15:04:52Z  qa
    AUTOPROJ Build 897
    Revision 1.626  2009/05/08 04:01:01Z  qa
    AUTOPROJ Build 896
    Revision 1.625  2009/05/03 05:50:57Z  qa
    AUTOPROJ Build 895
    Revision 1.624  2009/05/02 08:28:43Z  qa
    AUTOPROJ Build 894
    Revision 1.623  2009/04/30 04:00:58Z  qa
    AUTOPROJ Build 893
    Revision 1.622  2009/04/28 04:10:59Z  qa
    AUTOPROJ Build 892
    Revision 1.621  2009/04/26 04:01:27Z  qa
    AUTOPROJ Build 891
    Revision 1.620  2009/04/25 06:37:10Z  qa
    AUTOPROJ Build 890
    Revision 1.619  2009/04/21 04:00:52Z  qa
    AUTOPROJ Build 889
    Revision 1.618  2009/04/19 07:50:52Z  qa
    AUTOPROJ Build 888
    Revision 1.617  2009/04/17 20:32:26Z  qa
    AUTOPROJ Build 887
    Revision 1.616  2009/04/16 21:35:53Z  qa
    AUTOPROJ Build 886
    Revision 1.615  2009/04/16 19:20:54Z  qa
    AUTOPROJ Build 885
    Revision 1.614  2009/04/16 09:50:20Z  garyp
    Updated to v2.8 and turned the ALPHA flag back on.
    Revision 1.613  2009/04/16 02:47:01Z  qa
    AUTOPROJ Build 884
    Revision 1.612  2009/04/16 02:30:26Z  qa
    AUTOPROJ Build 883
    Revision 1.611  2009/04/15 00:15:51Z  qa
    AUTOPROJ Build 882
    Revision 1.610  2009/04/11 04:01:01Z  qa
    AUTOPROJ Build 881
    Revision 1.609  2009/04/10 02:35:53Z  qa
    AUTOPROJ Build 880
    Revision 1.608  2009/04/09 04:01:04Z  qa
    AUTOPROJ Build 879
    Revision 1.607  2009/04/09 02:25:58Z  qa
    AUTOPROJ Build 878
    Revision 1.606  2009/04/08 23:59:15Z  garyp
    Changed the develoment state from ALPHA to release.
    Revision 1.605  2009/04/08 15:40:59Z  qa
    AUTOPROJ Build 877
    Revision 1.604  2009/04/03 13:11:35Z  qa
    AUTOPROJ Build 876
    Revision 1.603  2009/04/02 19:30:31Z  qa
    AUTOPROJ Build 875
    Revision 1.602  2009/04/01 04:00:54Z  qa
    AUTOPROJ Build 874
    Revision 1.601  2009/03/31 04:10:49Z  qa
    AUTOPROJ Build 873
    Revision 1.600  2009/03/30 08:15:59Z  qa
    AUTOPROJ Build 872
    Revision 1.599  2009/03/29 20:55:15Z  qa
    AUTOPROJ Build 871
    Revision 1.598  2009/03/27 23:49:49Z  qa
    AUTOPROJ Build 870
    Revision 1.597  2009/03/27 22:14:17Z  qa
    AUTOPROJ Build 869
    Revision 1.596  2009/03/27 09:40:51Z  qa
    AUTOPROJ Build 868
    Revision 1.595  2009/03/26 01:22:36Z  qa
    AUTOPROJ Build 867
    Revision 1.594  2009/03/25 17:34:36Z  qa
    AUTOPROJ Build 866
    Revision 1.593  2009/03/25 13:28:38Z  qa
    AUTOPROJ Build 865
    Revision 1.592  2009/03/25 09:16:04Z  qa
    AUTOPROJ Build 864
    Revision 1.591  2009/03/24 23:39:01Z  qa
    AUTOPROJ Build 863
    Revision 1.590  2009/03/23 00:25:48Z  qa
    AUTOPROJ Build 862
    Revision 1.589  2009/03/21 04:10:44Z  qa
    AUTOPROJ Build 861
    Revision 1.588  2009/03/19 20:53:18Z  qa
    AUTOPROJ Build 860
    Revision 1.587  2009/03/19 04:10:45Z  qa
    AUTOPROJ Build 859
    Revision 1.586  2009/03/13 15:21:51Z  qa
    AUTOPROJ Build 858
    Revision 1.585  2009/03/13 14:04:24Z  qa
    AUTOPROJ Build 857
    Revision 1.584  2009/03/13 06:19:22Z  qa
    AUTOPROJ Build 856
    Revision 1.583  2009/03/13 03:30:46Z  qa
    AUTOPROJ Build 855
    Revision 1.582  2009/03/13 02:00:49Z  qa
    AUTOPROJ Build 854
    Revision 1.581  2009/03/13 01:25:09Z  qa
    AUTOPROJ Build 853
    Revision 1.580  2009/03/11 04:00:45Z  qa
    AUTOPROJ Build 852
    Revision 1.579  2009/03/10 04:00:46Z  qa
    AUTOPROJ Build 851
    Revision 1.578  2009/03/08 17:01:52Z  qa
    AUTOPROJ Build 850
    Revision 1.577  2009/03/07 05:01:41Z  qa
    AUTOPROJ Build 849
    Revision 1.576  2009/03/06 01:51:24Z  qa
    AUTOPROJ Build 848
    Revision 1.575  2009/03/05 18:06:41Z  qa
    AUTOPROJ Build 847
    Revision 1.574  2009/03/05 05:01:37Z  qa
    AUTOPROJ Build 846
    Revision 1.573  2009/03/04 05:01:22Z  qa
    AUTOPROJ Build 845
    Revision 1.572  2009/03/03 17:36:25Z  qa
    AUTOPROJ Build 844
    Revision 1.571  2009/03/03 05:01:45Z  qa
    AUTOPROJ Build 843
    Revision 1.570  2009/02/27 05:01:40Z  qa
    AUTOPROJ Build 842
    Revision 1.569  2009/02/25 05:01:29Z  qa
    AUTOPROJ Build 841
    Revision 1.568  2009/02/24 05:01:21Z  qa
    AUTOPROJ Build 840
    Revision 1.567  2009/02/21 05:01:37Z  qa
    AUTOPROJ Build 839
    Revision 1.566  2009/02/19 05:01:34Z  qa
    AUTOPROJ Build 838
    Revision 1.565  2009/02/17 05:01:28Z  qa
    AUTOPROJ Build 837
    Revision 1.564  2009/02/14 05:01:29Z  qa
    AUTOPROJ Build 836
    Revision 1.563  2009/02/13 20:51:29Z  qa
    AUTOPROJ Build 835
    Revision 1.562  2009/02/13 05:01:23Z  qa
    AUTOPROJ Build 834
    Revision 1.561  2009/02/12 05:01:23Z  qa
    AUTOPROJ Build 833
    Revision 1.560  2009/02/11 05:01:23Z  qa
    AUTOPROJ Build 832
    Revision 1.559  2009/02/10 05:01:22Z  qa
    AUTOPROJ Build 831
    Revision 1.558  2009/02/09 08:16:48Z  qa
    AUTOPROJ Build 830
    Revision 1.557  2009/02/09 06:27:02Z  qa
    AUTOPROJ Build 829
    Revision 1.556  2009/02/08 05:14:51Z  qa
    AUTOPROJ Build 828
    Revision 1.555  2009/02/07 00:30:13Z  garyp
    Always build the signon strings with the compile date and time, regardless
    whether the code is built in DEBUG or RELEASE mode.  Updated the copyright
    date to 2009.
    Revision 1.554  2009/02/06 05:01:45Z  qa
    AUTOPROJ Build 827
    Revision 1.553  2009/02/04 05:01:18Z  qa
    AUTOPROJ Build 826
    Revision 1.552  2009/01/28 16:26:16Z  qa
    AUTOPROJ Build 825
    Revision 1.551  2009/01/28 05:01:37Z  qa
    AUTOPROJ Build 824
    Revision 1.550  2009/01/24 05:01:22Z  qa
    AUTOPROJ Build 823
    Revision 1.549  2009/01/23 05:01:19Z  qa
    AUTOPROJ Build 822
    Revision 1.548  2009/01/22 22:05:35Z  qa
    AUTOPROJ Build 821
    Revision 1.547  2009/01/22 05:01:26Z  qa
    AUTOPROJ Build 820
    Revision 1.546  2009/01/21 03:06:07Z  qa
    AUTOPROJ Build 819
    Revision 1.545  2009/01/20 01:41:16Z  qa
    AUTOPROJ Build 818
    Revision 1.544  2009/01/19 21:06:22Z  qa
    AUTOPROJ Build 817
    Revision 1.543  2009/01/17 08:01:15Z  qa
    AUTOPROJ Build 816
    Revision 1.542  2009/01/16 11:01:21Z  qa
    AUTOPROJ Build 815
    Revision 1.541  2009/01/15 05:01:22Z  qa
    AUTOPROJ Build 814
    Revision 1.540  2009/01/13 05:01:29Z  qa
    AUTOPROJ Build 813
    Revision 1.539  2009/01/12 05:01:08Z  qa
    AUTOPROJ Build 812
    Revision 1.538  2009/01/11 05:01:12Z  qa
    AUTOPROJ Build 811
    Revision 1.537  2009/01/09 05:01:13Z  qa
    AUTOPROJ Build 810
    Revision 1.536  2009/01/08 05:01:05Z  qa
    AUTOPROJ Build 809
    Revision 1.535  2009/01/06 05:01:20Z  qa
    AUTOPROJ Build 808
    Revision 1.534  2009/01/05 05:01:15Z  qa
    AUTOPROJ Build 807
    Revision 1.533  2009/01/03 05:08:27Z  qa
    AUTOPROJ Build 806
    < Entries for 2008 builds 715 to 805 have been removed >
    < Entries for 2007 builds 505 to 714 have been removed >
    < Entries for 2006 builds 305 to 504 have been removed >
    Revision 1.2  2005/12/13 19:49:29  Rickc
    Set build number to 300 for new source tree.
    Revision 1.1  2005/12/05 20:17:16  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLVER_H_INCLUDED
#define DLVER_H_INCLUDED


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
#define DCLBUILDNUM     "1190BH"

/*  $SIBLDDATE"" */
#define DCLBUILDDATE    "04/24/2012"

/*  $SIBLDVER""         Standard forms: "v2.4", "v2.4.1a", "v2.x.x", or "Codename" */
#define DCLVERSIONSTR   "v3.1.2"


/*--- End of stuff used by AutoProj ---*/


    /*--------------------------------------------------------------*\
     *  NOTE! The DCLVERSIONSTR string above is manually changed    *
     *        when moving to a new version.  DO NOT forget to also  *
     *        change the VERSIONVAL value below, to match.          *
     *                                                              *
     *        DO NOT forget to also change the DL_VERSION and       *
     *        DL_VERSIONVAL settings in version.bat.                *
    \*--------------------------------------------------------------*/


/*  Version number in hex.  The low byte is either zero, or the lowercase
    hex representation of the alphabetic revision character -- 'a' == 0x61.
*/
#define VERSIONVAL      0x03010200UL    /* 3.1.2    */

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

#define PRODUCTBASENAME "Common Libraries"
#define PRODUCTPREFIX   "DL"
#define PRODUCTVERSION  DCLVERSIONSTR "-" DCLBUILDNUM ALPHABETA
#define PRODUCTNAME     "Datalight "PRODUCTBASENAME" "DCLVERSIONSTR" Build "DCLBUILDNUM ALPHABETA PRODDEBUGMSG
#define PRODUCTLEGAL    "Copyright (c) 1993-2012 Datalight, Inc.  All Rights Reserved Worldwide."
#define PRODUCTEDITION  DCL_OSNAME" Edition"PRODCPUMSG" -- Compiled "__DATE__" at "__TIME__


#endif  /* DLVER_H_INCLUDED */

