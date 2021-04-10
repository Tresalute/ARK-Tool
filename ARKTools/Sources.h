#pragma once
#include <winioctl.h>

#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_IN_DIRECT,FILE_ANY_ACCESS)
#define MAKE_LONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 16))
#define MAKE_ULONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 24))


// ͨѶ��
enum CTLCODE
{
	CTL_Test		= MYCTLCODE(0),		// ���Ժ�
	CTL_EnumDrive	= MYCTLCODE(1),		// ��������
	CTL_EnumPEM		= MYCTLCODE(2),		// ��������
	CTL_EnumFile	= MYCTLCODE(3),		// �����ļ�
	CTL_EnumReg		= MYCTLCODE(4),		// ����ע���
	CTL_EnumIDT		= MYCTLCODE(5),		// ����IDT
	CTL_EnumGDT		= MYCTLCODE(6),		// ����GDT
	CTL_EnumSSDT	= MYCTLCODE(7),		// ����SSDT

	CTL_HideDri		= MYCTLCODE(11),	// ��������
	CTL_HideProcess = MYCTLCODE(12),	// ���ؽ���

	CTL_SYSHOOK		= MYCTLCODE(21),	// Sysentry-HOOK
};

// ���������ļ�����·��
typedef struct _UDRIVE
{
	CHAR* SysName;
	CHAR* SysPath;
}uDrive, *puDrive;

// ͨ�Žṹ��

typedef struct _COMMITINFO
{
	int bNum;// �ڼ��η���
	int bufferSize;
	char buffer[1];
}CommintInfo, *pCommintInfo;

// �������ݽṹ��
typedef struct _ENUMDRIVE
{
	ULONG DllBase;
	ULONG SizeOfImage;
	char buffer[1]; 
}EnumDrive,*pEnumDrive;


// ͨ����Ϣ�ṹ 
typedef struct _INTERRACTINFO
{
	CTLCODE Type;				// ��Ϣ����
	int Status;					// ״̬ ��Ҫ�����ں˷��� �ж�ִ��״̬
	WCHAR Buffer[0x20];			// ���ڴ����ַ�����
	UINT32 Number;					// ���ڴ������� ֮������
}INTERRACTINFO,*PINTERRACTINFO;
