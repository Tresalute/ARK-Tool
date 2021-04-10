#pragma once
#include <winioctl.h>

#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_BUFFERED,FILE_ANY_ACCESS)

// 通讯码
enum CTLCODE
{
	CTL_Test = MYCTLCODE(0),
	CTL_EnumDrive = MYCTLCODE(1),
};

// 保存驱动文件名和路径
typedef struct _UDRIVE
{
	CHAR* SysName;
	CHAR* SysPath;
}uDrive, *puDrive;

// 驱动结构体
typedef struct _ENUMDRIVE
{
	int DllBase;
	int SizeOfImage;
	char FullDllName[256] = { 0 };
}EnumDrive, *pEnumDrive;

class COMMIT
{
public:
	// 对 DeviceIoControl 的包装


};

