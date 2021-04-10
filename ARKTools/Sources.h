#pragma once
#include <winioctl.h>

#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_IN_DIRECT,FILE_ANY_ACCESS)
#define MAKE_LONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 16))
#define MAKE_ULONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 24))


// 通讯码
enum CTLCODE
{
	CTL_Test		= MYCTLCODE(0),		// 测试号
	CTL_EnumDrive	= MYCTLCODE(1),		// 遍历驱动
	CTL_EnumPEM		= MYCTLCODE(2),		// 遍历进程
	CTL_EnumFile	= MYCTLCODE(3),		// 遍历文件
	CTL_EnumReg		= MYCTLCODE(4),		// 遍历注册表
	CTL_EnumIDT		= MYCTLCODE(5),		// 遍历IDT
	CTL_EnumGDT		= MYCTLCODE(6),		// 遍历GDT
	CTL_EnumSSDT	= MYCTLCODE(7),		// 遍历SSDT

	CTL_HideDri		= MYCTLCODE(11),	// 隐藏驱动
	CTL_HideProcess = MYCTLCODE(12),	// 隐藏进程

	CTL_SYSHOOK		= MYCTLCODE(21),	// Sysentry-HOOK
};

// 保存驱动文件名和路径
typedef struct _UDRIVE
{
	CHAR* SysName;
	CHAR* SysPath;
}uDrive, *puDrive;

// 通信结构体

typedef struct _COMMITINFO
{
	int bNum;// 第几次访问
	int bufferSize;
	char buffer[1];
}CommintInfo, *pCommintInfo;

// 驱动数据结构体
typedef struct _ENUMDRIVE
{
	ULONG DllBase;
	ULONG SizeOfImage;
	char buffer[1]; 
}EnumDrive,*pEnumDrive;


// 通用消息结构 
typedef struct _INTERRACTINFO
{
	CTLCODE Type;				// 消息类型
	int Status;					// 状态 主要用于内核返回 判断执行状态
	WCHAR Buffer[0x20];			// 用于传输字符数据
	UINT32 Number;					// 用于传输数字 之类数据
}INTERRACTINFO,*PINTERRACTINFO;
