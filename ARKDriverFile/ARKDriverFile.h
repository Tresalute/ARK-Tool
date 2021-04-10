#pragma once
#include <ntddk.h>

#define NAME_DEVICE L"\\Device\\mydevicesym1"
#define NAME_SYMBOL L"\\DosDevices\\mydevicesym1"

#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#define MAKE_LONG(a, b) ((LONG)(((UINT16)(a)) | ((UINT32)((UINT16)(b))) << 16))

/*SSDT支持*/
#define SYSCALL_INDEX(_funcation) (ULONG)*(PULONG)((PUCHAR)_funcation+1)
#define _GET_SSDT_ADDR(_funcation) (PLONG)&g_pMappedSystemCallTable[SYSCALL_INDEX(_funcation)]
#define GET_SSDT_ADDR(_Index_) (PLONG)&g_pMappedSystemCallTable[_Index_]


/*======================================全局变量声明======================================*/
PVOID g_oldKiFastCallEntry;		// 保存原本的KiFastCallEntry
int g_Pid;						// 被保护的PID

/*======================================函数申明======================================*/
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT *pDevice, IRP *pIrp);
NTSTATUS OnCreate(DEVICE_OBJECT *DeviceObject, IRP *Irp);
// 0号测试函数
NTSTATUS TestFuncation(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 1号->驱动遍历
NTSTATUS Nt_EnumDriver(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 2号->进程、线程、模块遍历
NTSTATUS Nt_EnumPEM(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 3号->文件遍历
NTSTATUS Nt_EnumFile(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 4号->注册表遍历
NTSTATUS Nt_EnumReg(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 5号->IDT遍历
NTSTATUS Nt_EnumIDT(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 6号->GDT遍历
NTSTATUS Nt_EnumGDT(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 7号->SSDT遍历
NTSTATUS Nt_EnumSSDT(DEVICE_OBJECT *pDevice, IRP *pIrp);


// 11号->驱动隐藏
NTSTATUS Nt_HideDri(DEVICE_OBJECT *pDevice, IRP *pIrp);
// 12号->隐藏进程
NTSTATUS Nt_HideProcess(DEVICE_OBJECT *pDevice, IRP *pIrp);


// 21号->SystemEntry Hook 获取需要保护的 ID 
NTSTATUS Nt_SysEntryHook(DEVICE_OBJECT *pDevice, IRP *pIrp);


/*======================================工具函数申明======================================*/

/* 2号->工具函数*/
/* 通过EPROCESS 获取进程路径*/

void DisplayPathByEP(PEPROCESS pep, CHAR* cpath);

/* 21号->工具函数*/
/* 被HOOK的  KiFastCAllEntry  */
//void _declspec(naked)MyKiFastCallEntry();

/* HOOK KiFastCallEntry*/
int SetKiFastCallEntryAddr(PVOID pAddress);

/* 获取 KiFastCallEntry 地址*/
PVOID GetKiFastCallEntryAddr();

/*======================================通讯码定义以及声明======================================*/
#define MYCTLCODE( code ) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code), METHOD_IN_DIRECT, FILE_ANY_ACCESS)
typedef enum _CTLCODE
{
	CTL_Test		= MYCTLCODE(0),			// 测试号
	CTL_EnumDrive	= MYCTLCODE(1),			// 遍历驱动
	CTL_EnumPEM		= MYCTLCODE(2),			// 遍历进程
	CTL_EnumFile	= MYCTLCODE(3),			// 遍历文件
	CTL_EnumReg		= MYCTLCODE(4),			// 遍历注册表
	CTL_EnumIDT		= MYCTLCODE(5),			// 遍历IDT
	CTL_EnumGDT		= MYCTLCODE(6),			// 遍历GDT
	CTL_EnumSSDT	= MYCTLCODE(7),			// 遍历SSDT

	CTL_HideDri		= MYCTLCODE(11),		// 隐藏驱动
	CTL_HideProcess = MYCTLCODE(12),		// 隐藏进程

	CTL_SYSHOOK		= MYCTLCODE(21),		// Sysentry-HOOK

}CTLCODE;




/*======================================结构体======================================*/
typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;    //双向链表
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


/*======================================五号所需结构======================================*/

typedef struct _IDT_INFO
{
	UINT16 uIdtLimit;			// IDT范围
	UINT16 uLowIdtBase;			// IDT低地址范围
	UINT16 uHighIdtBase;		// IDT高地址
}IDT_INFO,*PIDT_INFO;

typedef struct _IDT_ENTRY
{
	UINT16	uOffsetLow;			// 处理程序底地址
	UINT16	uSelector;			// 段选择子
	UINT8	uReserved;			// 保留
	UINT8	GateType : 4;		// 中断类型
	UINT8	StorageSegment:1;	// 为0是中断门
	UINT8	DPL : 2;			// 特权级别
	UINT8	Present : 1;		// 未使用可置为0
	UINT8	uOffsetHigh;		// 处理程序高地址偏移
}IDT_ENTRY,*PIDT_ENTRY;

/*======================================六号所需结构======================================*/

typedef struct _GDT_INFO
{
	UINT16 uGDTLimit;		// GDT范围
	UINT16 uLowGDTBase;		// GDT低基址
	UINT16 uHighGDTBase;	// GDT高基址
}GDT_INFO,*PGDT_INFO;


/*======================================七号所需结构======================================*/
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
	SSDTEntry   ntoskrnl;// ntoskrnl.exe的服务函数，即SSDT
	SSDTEntry   win32k; // win32k.sys的服务函数(GDI32.dll/User32.dll 的内核支持)，即ShadowSSDT
	SSDTEntry   notUsed1; // 不使用
	SSDTEntry   notUsed2; // 不使用
}KSTD, *PKSD;

#pragma pack()
/*======================================所有派发结构======================================*/
// 派发函数结构
typedef struct DISPATCHFUNCATION
{
	CTLCODE code;
	NTSTATUS(*callback)(DEVICE_OBJECT *DeviceObject, IRP *Irp);
}DispatchFuncation;

// 通信结构
typedef struct _COMMITINFO
{
	int bNum;// 第几次访问
	int bufferSize;
	char buffer[1];
}CommintInfo, *pCommintInfo;

// 1号->驱动结构体
typedef struct _ENUMDRIVE
{
	WCHAR DllName[0x30];
	ULONG DllBase;			// 加载基址
	ULONG SizeOfImage;		// 映像大小
	WCHAR buffer[1];			// 路径
}EnumDrive, *pEnumDrive;

// 2号->线程结构体
typedef struct _PEMINFO
{
	ULONG pid;				// 进程PID		-
	ULONG parentPid;		// 父进程PID	- EPRO + 0x140
	ULONG numOfThe;			// 线程数量		- 0x198
	ULONG eprocess;			// EPROCESS地址	-	
	UCHAR ImageName[15];	// 进程名称		- 16c
	CHAR cPath[1];		// 进程路径		- 1ec
}PEBInfo, *pPEBInfo;

// 5号->IDT结构体
typedef struct _IDTINFO
{
	ULONG uAddrOffset;
	ULONG uNum;
	ULONG uSelector;
	ULONG uType;
	ULONG uLevel;
}IDTInfo, *pIDTInfo;;

// 6号->GDT结构体
//typedef struct _GDTINFO
//{
//	ULONG uSelector;		// 段选择子
//	ULONG uBase;			// 基址
//	ULONG uLimit;			// 界限
//	ULONG uPG;				// 粒度
//	ULONG uDPL;				// 段特权级
//	ULONG uType;			// 类型
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


// 7号->SSDT结构体
typedef struct _SSDTINFO
{
	LONG Num;				// 调用号
	INT Addr;				// 地址
}SSDTINFO,*PSSDTINFO;

typedef struct _SSDTINFO_
{
	CHAR Name;				// 函数名
	LONG Num;				// 调用号
	INT Addr;				// 地址
}SSDTINFO_, *PSSDTINFO_;

/*======================================消息交互======================================*/

// 通用消息结构 interactive
typedef struct _INTERRACTINFO
{
	CTLCODE Type;				// 消息类型
	int Status;					// 状态 主要用于内核返回 判断执行状态
	WCHAR Buffer[0x20];			// 用于传输字符数据
	UINT32 Number;					// 用于传输ID 之类数据
}INTERRACTINFO, *PINTERRACTINFO;
