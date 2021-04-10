#pragma once
#include <ntddk.h>

#define NAME_DEVICE L"\\Device\\mydevicesym1"
#define NAME_SYMBOL L"\\DosDevices\\mydevicesym1"

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#define MAKE_LONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 16))

/*SSDT֧��*/
#define SYSCALL_INDEX(_funcation) (ULONG)*(PULONG)((PUCHAR)_funcation+1)
#define _GET_SSDT_ADDR(_funcation) (PLONG)&g_pMappedSystemCallTable[SYSCALL_INDEX(_funcation)]
#define GET_SSDT_ADDR(_Index_) (PLONG)&g_pMappedSystemCallTable[_Index_]


/*======================================ȫ�ֱ�������======================================*/
PVOID g_oldKiFastCallEntry;		// ����ԭ����KiFastCallEntry
int g_Pid;						// ��������PID

/*======================================��������======================================*/
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT *pDevice, IRP *pIrp);
NTSTATUS OnCreate(DEVICE_OBJECT *DeviceObject, IRP *Irp);
// 0�Ų��Ժ���
NTSTATUS TestFuncation(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 1��->��������
NTSTATUS Nt_EnumDriver(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 2��->���̡��̡߳�ģ�����
NTSTATUS Nt_EnumPEM(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 3��->�ļ�����
NTSTATUS Nt_EnumFile(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 4��->ע������
NTSTATUS Nt_EnumReg(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 5��->IDT����
NTSTATUS Nt_EnumIDT(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 6��->GDT����
NTSTATUS Nt_EnumGDT(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 7��->SSDT����
NTSTATUS Nt_EnumSSDT(DEVICE_OBJECT *pDevice, IRP *pIrp);


// 11��->��������
NTSTATUS Nt_HideDri(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 12��->���ؽ���
NTSTATUS Nt_HideProcess(DEVICE_OBJECT *pDevice, IRP *pIrp);


// 21��->SystemEntry Hook ��ȡ��Ҫ������ ID 
NTSTATUS Nt_SysEntryHook(DEVICE_OBJECT *pDevice, IRP *pIrp);


/*======================================���ߺ�������======================================*/

/* 2��->���ߺ���*/
/* ͨ��EPROCESS ��ȡ����·��*/

void DisplayPathByEP(PEPROCESS pep, CHAR* cpath);

/* 21��->���ߺ���*/
/* ��HOOK��  KiFastCAllEntry  */
//void _declspec(naked)MyKiFastCallEntry();

/* HOOK KiFastCallEntry*/
int SetKiFastCallEntryAddr(PVOID pAddress);

/* ��ȡ KiFastCallEntry ��ַ*/
PVOID GetKiFastCallEntryAddr();

/*======================================ͨѶ�붨���Լ�����======================================*/
#define MYCTLCODE( code ) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_IN_DIRECT, FILE_ANY_ACCESS)
typedef enum _CTLCODE
{
	CTL_Test		= MYCTLCODE(0),			// ���Ժ�
	CTL_EnumDrive	= MYCTLCODE(1),			// ��������
	CTL_EnumPEM		= MYCTLCODE(2),			// ��������
	CTL_EnumFile	= MYCTLCODE(3),			// �����ļ�
	CTL_EnumReg		= MYCTLCODE(4),			// ����ע���
	CTL_EnumIDT		= MYCTLCODE(5),			// ����IDT
	CTL_EnumGDT		= MYCTLCODE(6),			// ����GDT
	CTL_EnumSSDT	= MYCTLCODE(7),			// ����SSDT

	CTL_HideDri		= MYCTLCODE(11),		// ��������
	CTL_HideProcess = MYCTLCODE(12),		// ���ؽ���

	CTL_SYSHOOK		= MYCTLCODE(21),		// Sysentry-HOOK

}CTLCODE;




/*======================================�ṹ��======================================*/
typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;    //˫������
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		}s1;
	}u1;
	union {
		struct {
			ULONG TimeDateStamp;
		}s2;
		struct {
			PVOID LoadedImports;
		}s3;
	}u2;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


/*======================================�������ṹ======================================*/

typedef struct _IDT_INFO
{
	UINT16 uIdtLimit;			// IDT��Χ
	UINT16 uLowIdtBase;			// IDT�͵�ַ��Χ
	UINT16 uHighIdtBase;		// IDT�ߵ�ַ
}IDT_INFO,*PIDT_INFO;

typedef struct _IDT_ENTRY
{
	UINT16	uOffsetLow;			// �������׵�ַ
	UINT16	uSelector;			// ��ѡ����
	UINT8	uReserved;			// ����
	UINT8	GateType : 4;		// �ж�����
	UINT8	StorageSegment:1;	// Ϊ0���ж���
	UINT8	DPL : 2;			// ��Ȩ����
	UINT8	Present : 1;		// δʹ�ÿ���Ϊ0
	UINT8	uOffsetHigh;		// �������ߵ�ַƫ��
}IDT_ENTRY,*PIDT_ENTRY;

/*======================================��������ṹ======================================*/

typedef struct _GDT_INFO
{
	UINT16 uGDTLimit;		// GDT��Χ
	UINT16 uLowGDTBase;		// GDT�ͻ�ַ
	UINT16 uHighGDTBase;	// GDT�߻�ַ
}GDT_INFO,*PGDT_INFO;


/*======================================�ߺ�����ṹ======================================*/
#pragma pack(1)
typedef	struct _ServiceDesriptEntry
{
	ULONG* ServiceTableBase;
	ULONG* ServiceCounterTabBase;
	ULONG* NumberOfService;
	UCHAR* ParamTableBase;
}SSDTEntry,*PSSDTEntry;

typedef  struct  _KSERVICE_TABLE_DESCRIPTOR
{
	SSDTEntry   ntoskrnl;// ntoskrnl.exe�ķ���������SSDT
	SSDTEntry   win32k; // win32k.sys�ķ�����(GDI32.dll/User32.dll ���ں�֧��)����ShadowSSDT
	SSDTEntry   notUsed1; // ��ʹ��
	SSDTEntry   notUsed2; // ��ʹ��
}KSTD, *PKSD;

#pragma pack()
/*======================================�����ɷ��ṹ======================================*/
// �ɷ������ṹ
typedef struct DISPATCHFUNCATION
{
	CTLCODE code;
	NTSTATUS(*callback)(DEVICE_OBJECT *DeviceObject, IRP *Irp);
}DispatchFuncation;

// ͨ�Žṹ
typedef struct _COMMITINFO
{
	int bNum;// �ڼ��η���
	int bufferSize;
	char buffer[1];
}CommintInfo, *pCommintInfo;

// 1��->�����ṹ��
typedef struct _ENUMDRIVE
{
	WCHAR DllName[0x30];
	ULONG DllBase;			// ���ػ�ַ
	ULONG SizeOfImage;		// ӳ���С
	WCHAR buffer[1];			// ·��
}EnumDrive, *pEnumDrive;

// 2��->�߳̽ṹ��
typedef struct _PEMINFO
{
	ULONG pid;				// ����PID		-
	ULONG parentPid;		// ������PID	- EPRO + 0x140
	ULONG numOfThe;			// �߳�����		- 0x198
	ULONG eprocess;			// EPROCESS��ַ	-	
	UCHAR ImageName[15];	// ��������		- 16c
	CHAR cPath[1];		// ����·��		- 1ec
}PEBInfo, *pPEBInfo;

// 5��->IDT�ṹ��
typedef struct _IDTINFO
{
	ULONG uAddrOffset;
	ULONG uNum;
	ULONG uSelector;
	ULONG uType;
	ULONG uLevel;
}IDTInfo, *pIDTInfo;;

// 6��->GDT�ṹ��
//typedef struct _GDTINFO
//{
//	ULONG uSelector;		// ��ѡ����
//	ULONG uBase;			// ��ַ
//	ULONG uLimit;			// ����
//	ULONG uPG;				// ����
//	ULONG uDPL;				// ����Ȩ��
//	ULONG uType;			// ����
//}GDTInfo, *pGDTInfo;

typedef struct _GDT
{
	UINT64 Limit0_15 : 16;
	UINT64 Base0_23 : 24;
	UINT64 Type : 4;
	UINT64 S : 1;
	UINT64 DPL : 2;
	UINT64 P : 1;
	UINT64 Limit16_19 : 4;
	UINT64 AVL : 1;
	UINT64 O : 1;
	UINT64 D_B : 1;
	UINT64 G : 1;
	UINT64 Base24_31 : 8;
}GDT, *PGDT;


// 7��->SSDT�ṹ��
typedef struct _SSDTINFO
{
	LONG Num;				// ���ú�
	INT Addr;				// ��ַ
}SSDTINFO,*PSSDTINFO;

typedef struct _SSDTINFO_
{
	CHAR Name;				// ������
	LONG Num;				// ���ú�
	INT Addr;				// ��ַ
}SSDTINFO_, *PSSDTINFO_;

/*======================================��Ϣ����======================================*/

// ͨ����Ϣ�ṹ interactive
typedef struct _INTERRACTINFO
{
	CTLCODE Type;				// ��Ϣ����
	int Status;					// ״̬ ��Ҫ�����ں˷��� �ж�ִ��״̬
	WCHAR Buffer[0x20];			// ���ڴ����ַ�����
	UINT32 Number;					// ���ڴ���ID ֮������
}INTERRACTINFO, *PINTERRACTINFO;
