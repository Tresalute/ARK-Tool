#pragma once
#include <winioctl.h>

#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_BUFFERED,FILE_ANY_ACCESS)

// ͨѶ��
enum CTLCODE
{
	CTL_Test = MYCTLCODE(0),
	CTL_EnumDrive = MYCTLCODE(1),
};

// ���������ļ�����·��
typedef struct _UDRIVE
{
	CHAR* SysName;
	CHAR* SysPath;
}uDrive, *puDrive;

// �����ṹ��
typedef struct _ENUMDRIVE
{
	int DllBase;
	int SizeOfImage;
	char FullDllName[256] = { 0 };
}EnumDrive, *pEnumDrive;

class COMMIT
{
public:
	// �� DeviceIoControl �İ�װ


};

