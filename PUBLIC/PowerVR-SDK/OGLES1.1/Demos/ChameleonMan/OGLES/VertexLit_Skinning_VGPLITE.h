/******************************************************************************

 @File         VertexLit_Skinning_VGPLITE.h

 @Title        ChameleonMan

 @Copyright    Copyright (C) 2008 - 2008 by Imagination Technologies Limited.

 @Platform     Independant

 @Description  Requires the PVRShell. Requires VGP support.

******************************************************************************/
const unsigned int vgp_VertexLit_Skinning_VGPLITE[] = {

0x20504756, // magic vgp id
0xffff0101, // vgp version
0x0000002d, // code size (in instructions)
0x00000300, // clipping planes

0x00000200, // Section 0
0x0f002b03, // Section 1
0x00002c2c, // Section 2
0xf0000000, // Section 3
0xf0000000, // Section 4
0xf0000000, // Section 5
0xf0000000, // Section 6
0xf0000000, // Section 7
0xf0000000, // Section 8
0xf0000000, // Section 9
0xf0000000, // Section 10
0xf0000000, // Section 11
0xf0000000, // Section 12
0xf0000000, // Section 13
0xf0000000, // Section 14
0xf0000000, // Section 15

0x0000c601, 0x07c00001,   // (S00)  mov od0,v1.zyxw
0x0000e401, 0x0fc00002,   // (S00)  mov ot0,v2
0x0000e401, 0x03c00000,   // (S00)  mov opos,v0

0x00000012, 0x03c00005,   // (S01)  amv a0,v5.xxxx
0x0000e401, 0x0cc00002,   // (S01)  mov ot0.xy,v2
0x0000e401, 0x74c00002,   // (S01)  mov r13.xy,v2
0x0000e404, 0x4fc18045,   // (S01)  mul r3,c[a0.x+5],v4.xxxx
0x0000e404, 0x53c18046,   // (S01)  mul r4,c[a0.x+6],v4.xxxx
0x0000e404, 0x57c18047,   // (S01)  mul r5,c[a0.x+7],v4.xxxx
0x00005512, 0x03c00005,   // (S01)  amv a0,v5.yyyy
0x0000e401, 0x13c2000d,   // (S01)  mov ot1,r13
0x00000000, 0x00000000,   // (S01)  nop 
0x00000000, 0x00000000,   // (S01)  nop 
0x00000000, 0x00000000,   // (S01)  nop 
0xe455e403, 0x4fe18345,   // (S01)  mad r3,c[a0.x+5],v4.yyyy,r3
0xe455e403, 0x53e18446,   // (S01)  mad r4,c[a0.x+6],v4.yyyy,r4
0xe455e403, 0x57e18547,   // (S01)  mad r5,c[a0.x+7],v4.yyyy,r5
0x0000aa12, 0x03c00005,   // (S01)  amv a0,v5.zzzz
0x00000000, 0x00000000,   // (S01)  nop 
0x00000000, 0x00000000,   // (S01)  nop 
0x00000000, 0x00000000,   // (S01)  nop 
0xe4aae403, 0x4fe18345,   // (S01)  mad r3,c[a0.x+5],v4.zzzz,r3
0xe4aae403, 0x53e18446,   // (S01)  mad r4,c[a0.x+6],v4.zzzz,r4
0xe4aae403, 0x57e18547,   // (S01)  mad r5,c[a0.x+7],v4.zzzz,r5
0x00e4e408, 0x64420003,   // (S01)  dp4 r9.x,r3,v0
0x00e4e408, 0x64820004,   // (S01)  dp4 r9.y,r4,v0
0x00e4e408, 0x65020005,   // (S01)  dp4 r9.z,r5,v0
0x00e4e407, 0x58420033,   // (S01)  dp3 r6.x,r3,v3
0x00e4e407, 0x58820034,   // (S01)  dp3 r6.y,r4,v3
0x00e4e407, 0x59020035,   // (S01)  dp3 r6.z,r5,v3
0x0000e401, 0x66012006,   // (S01)  mov r9.w,c38
0x00e4e442, 0x6bc92095,   // (S01)  add r10,c37,-r9
0x00e4e408, 0x48490091,   // (S01)  dp4 r2.x,c1,r9
0x00e4e408, 0x48890092,   // (S01)  dp4 r2.y,c2,r9
0x00e4e408, 0x49090093,   // (S01)  dp4 r2.z,c3,r9
0x00e4e408, 0x4a090094,   // (S01)  dp4 r2.w,c4,r9
0x00e4e407, 0x6e0a00aa,   // (S01)  dp3 r11.w,r10,r10
0x0000e401, 0x03c20002,   // (S01)  mov opos,r2
0x00e4e408, 0x14492027,   // (S01)  dp4 oc2.x,c39,r2
0x0000ff06, 0x6e02000b,   // (S01)  rsq r11.w,r11.wwww
0x00e4e408, 0x14892028,   // (S01)  dp4 oc2.y,c40,r2
0x00e4ff04, 0x69ca00ab,   // (S01)  mul r10.xyz,r11.wwww,r10
0x00c6c607, 0x07ca00a6,   // (S01)  dp3 od0,r6.zyxw,r10.zyxw
0x0000ff01, 0x06012006,   // (S01)  mov od0.w,c38.wwww

0x0000e401, 0x0fc2000d,   // (S02)  mov ot0,r13
0x00000231,// Code Flags : 561
0x00010001,// Code Compatibility Version : 65537
0x00000001, // Section 0 Flags : 1
0x00000002, // Section 1 Flags : 2
0x00000006, // Section 2 Flags : 6
0x00000000, // Section 3 Flags : 0
0x00000000, // Section 4 Flags : 0
0x00000000, // Section 5 Flags : 0
0x00000000, // Section 6 Flags : 0
0x00000000, // Section 7 Flags : 0
0x00000000, // Section 8 Flags : 0
0x00000000, // Section 9 Flags : 0
0x00000000, // Section 10 Flags : 0
0x00000000, // Section 11 Flags : 0
0x00000000, // Section 12 Flags : 0
0x00000000, // Section 13 Flags : 0
0x00000000, // Section 14 Flags : 0
0x00000000, // Section 15 Flags : 0
0x00000043,// Num Bindings : 67
0x00000003, // Binding type : 3
0x00000000, // Constant number : 0
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000003, // Section Flags: 3
0x00000005, // Binding type : 5
0x00000001, // Constant number : 1
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000001, // Section Flags: 1
0x00000007, // Binding type : 7
0x00000002, // Constant number : 2
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000003, // Section Flags: 3
0x00000004, // Binding type : 4
0x00000003, // Constant number : 3
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000002, // Binding type : 2
0x00000004, // Constant number : 4
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000002, // Binding type : 2
0x00000005, // Constant number : 5
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003f, // Binding type : 63
0x00000000, // Constant number : 0
0x3f800000, // x : 1
0xbf800000, // y : -1
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x0000002d, // Binding type : 45
0x00000001, // Constant number : 1
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000002e, // Binding type : 46
0x00000002, // Constant number : 2
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000002f, // Binding type : 47
0x00000003, // Constant number : 3
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000030, // Binding type : 48
0x00000004, // Constant number : 4
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000005, // Constant number : 5
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x00000006, // Constant number : 6
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x00000007, // Constant number : 7
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000008, // Constant number : 8
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000009, // Constant number : 9
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x0000000a, // Constant number : 10
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x0000000b, // Constant number : 11
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x0000000c, // Constant number : 12
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x0000000d, // Constant number : 13
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x0000000e, // Constant number : 14
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x0000000f, // Constant number : 15
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000010, // Constant number : 16
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000011, // Constant number : 17
0x40400000, // x : 3
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x00000012, // Constant number : 18
0x40400000, // x : 3
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x00000013, // Constant number : 19
0x40400000, // x : 3
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000014, // Constant number : 20
0x40400000, // x : 3
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000015, // Constant number : 21
0x40800000, // x : 4
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x00000016, // Constant number : 22
0x40800000, // x : 4
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x00000017, // Constant number : 23
0x40800000, // x : 4
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000018, // Constant number : 24
0x40800000, // x : 4
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000019, // Constant number : 25
0x40a00000, // x : 5
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x0000001a, // Constant number : 26
0x40a00000, // x : 5
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x0000001b, // Constant number : 27
0x40a00000, // x : 5
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x0000001c, // Constant number : 28
0x40a00000, // x : 5
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x0000001d, // Constant number : 29
0x40c00000, // x : 6
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x0000001e, // Constant number : 30
0x40c00000, // x : 6
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x0000001f, // Constant number : 31
0x40c00000, // x : 6
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000020, // Constant number : 32
0x40c00000, // x : 6
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000039, // Binding type : 57
0x00000021, // Constant number : 33
0x40e00000, // x : 7
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003a, // Binding type : 58
0x00000022, // Constant number : 34
0x40e00000, // x : 7
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003b, // Binding type : 59
0x00000023, // Constant number : 35
0x40e00000, // x : 7
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003c, // Binding type : 60
0x00000024, // Constant number : 36
0x40e00000, // x : 7
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003d, // Binding type : 61
0x00000025, // Constant number : 37
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x0000003f, // Binding type : 63
0x00000026, // Constant number : 38
0x3f000000, // x : 0.5
0x3f000000, // y : 0.5
0x3f800000, // z : 1
0x3f800000, // w : 1
0x00000002, // Section Flags: 2
0x00000021, // Binding type : 33
0x00000027, // Constant number : 39
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000022, // Binding type : 34
0x00000028, // Constant number : 40
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000000, // Constant number : 0
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x00000001, // Binding type : 1
0x00000001, // Constant number : 1
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x00000001, // Binding type : 1
0x00000002, // Constant number : 2
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000003, // Constant number : 3
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000004, // Constant number : 4
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000005, // Constant number : 5
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000006, // Constant number : 6
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x00000007, // Constant number : 7
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x00000001, // Binding type : 1
0x00000008, // Constant number : 8
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x00000001, // Binding type : 1
0x00000009, // Constant number : 9
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x0000000a, // Constant number : 10
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x0000000b, // Constant number : 11
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000001, // Binding type : 1
0x0000000c, // Constant number : 12
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000000, // Section Flags: 0
0x00000001, // Binding type : 1
0x0000000d, // Constant number : 13
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000006, // Section Flags: 6
0x00000041, // Binding type : 65
0x00000000, // Constant number : 0
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000042, // Binding type : 66
0x00000000, // Constant number : 0
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000003, // Section Flags: 3
0x00000043, // Binding type : 67
0x00000001, // Constant number : 1
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000003, // Section Flags: 3
0x00000045, // Binding type : 69
0x00000002, // Constant number : 2
0x00000000, // x : 0
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000007, // Section Flags: 7
0x00000045, // Binding type : 69
0x00000003, // Constant number : 3
0x3f800000, // x : 1
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0x00000047, // Binding type : 71
0x00000004, // Constant number : 4
0x40000000, // x : 2
0x00000000, // y : 0
0x00000000, // z : 0
0x00000000, // w : 0
0x00000002, // Section Flags: 2
0};
