
#include "ARKDriverFile.h"

/*=================================声明函数原型=================================*/
CHAR* PsGetProcessImageFileName(IN PEPROCESS Process);

/* 21号->SystemEntry Hook */
/* 被HOOK的  KiFastCAllEntry  */
void _declspec(naked)MyKiFastCallEntry()
{
	__asm
	{
		;// 检查调用号
		cmp eax, 0xBE;
		jne _DONE;		// 调用号不是0xBE,执行第四部

		;// 检查进程ID是不是要保护的进程ID
		push eax;		// 备份寄存器

		;// 获取参数(进程ID)
		mov eax, [edx + 0x14];	// eax保存的是PCLIENT_ID
		mov eax, [eax];			// eax保存的是PCLIENT_ID->UniqueProcess

		;// 判断是不是要保护的进程ID
		cmp eax, [g_Pid];
		pop eax;		// 恢复寄存器
		jne _DONE;		// 不是要保护的进程就跳转

		;// 是的话就该调用参数， 让后续函数调用失败
		mov[edx + 0xC], 0;		// 将范文权限设置为0

	_DONE:
		;// 调用原来的KiFastCallEntry
		jmp g_oldKiFastCallEntry;
	}
}


DispatchFuncation g_funcation[] = {
	{CTL_Test		,TestFuncation},		// 测试
	{CTL_EnumDrive	,Nt_EnumDriver},		// 遍历驱动
	{CTL_EnumPEM	,Nt_EnumPEM},			// 进程
	{CTL_EnumFile	,Nt_EnumFile},			// 文件
	{CTL_EnumReg	,Nt_EnumReg},			// 注册表
	{CTL_EnumIDT	,Nt_EnumIDT},			// IDT
	{CTL_EnumGDT	,Nt_EnumGDT},			// GDT
	{CTL_EnumSSDT	,Nt_EnumSSDT},			// SSDT

	{CTL_HideDri	,Nt_HideDri},			// 隐藏驱动
	{CTL_HideProcess,Nt_HideProcess},		// 隐藏进程

	{CTL_SYSHOOK	,Nt_SysEntryHook},		// SystemEntry Hook
};

DRIVER_OBJECT* g_pDriver = NULL;

NTSTATUS DriverUnLoad(DRIVER_OBJECT* pDriver)
{
	pDriver;

	// 还原HOOK
	SetKiFastCallEntryAddr(g_oldKiFastCallEntry);

	KdPrint(("File_驱动卸载\n"));
	IoDeleteDevice(pDriver->DeviceObject);
	UNICODE_STRING symName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoDeleteSymbolicLink(&symName);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(DRIVER_OBJECT* pDriver, UNICODE_STRING* regpath)
{
	KdPrint(("File驱动加载\n"));
	regpath;
	pDriver->DriverUnload = &DriverUnLoad;
	g_pDriver = pDriver;
	NTSTATUS status = STATUS_SUCCESS;
	// 创建一个设备对象
	UNICODE_STRING uDeviceName = RTL_CONSTANT_STRING(NAME_DEVICE);
	DEVICE_OBJECT* pDevice = NULL;
	status = IoCreateDevice(
		pDriver,			// 驱动对象
		0,
		&uDeviceName,		// 设备名
		FILE_DEVICE_UNKNOWN,
		0, 
		0, 
		&pDevice);			// 设备对象
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备对象失败 错误码：%d", status));
		return status;
	}
	pDevice->Flags |= DO_DIRECT_IO;

	// 绑定符号
	UNICODE_STRING uName_SyMBol = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoCreateSymbolicLink(&uName_SyMBol, &uDeviceName);

	// 派遣绑定
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceIoControl;
	pDriver->MajorFunction[IRP_MJ_CREATE] = OnCreate;

	/*=======================HOOK KiFastCallEntry=======================*/
	// 获取 KiFastCallEntry 地址
	g_oldKiFastCallEntry = GetKiFastCallEntryAddr();

	if (g_oldKiFastCallEntry == NULL)
	{
		return STATUS_SUCCESS;
	}

	// 将自身的编写 MyKiFastCallEntry 替换 原本的KiFastCallEntry
	SetKiFastCallEntryAddr((PVOID)MyKiFastCallEntry);


	return STATUS_SUCCESS;
}

/*======================================空处理函数======================================*/

/*不做任何处理*/
/*防止返回错误*/
NTSTATUS OnCreate(DEVICE_OBJECT *DeviceObject,IRP *Irp)
{
	DeviceObject;
	Irp;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

/*======================================消息派遣函数======================================*/
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	char* pBuff = NULL;
	if (pIrp->MdlAddress != NULL) {
		// 需要使用一个函数, 让MDL对象映射出一个新的系统领空的虚拟地址.
		// MmGetSystemAddressForMdlSafe - 获取系统MDL对象映射出来的新的虚拟地址.
		// 这个虚拟地址可以直接修改到用户层的虚拟地址空间.
		pBuff = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, 0);
	}
	else if (pIrp->AssociatedIrp.SystemBuffer != NULL) {
		pBuff = pIrp->AssociatedIrp.SystemBuffer;
	}
	else if (pIrp->UserBuffer != NULL) {
		pBuff = pIrp->UserBuffer;
	}
	else {
		// 没有缓冲区.
		pBuff = NULL;
	}


	IO_STACK_LOCATION* pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	// 得到消息码
	LONG uCtrlCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;
	
	for (int i = 0; i< _countof(g_funcation);++i)
	{
		if (uCtrlCode == g_funcation[i].code)
		{	// 消息码匹配则调用对应的方法
			g_funcation[i].callback(pDevice, pIrp);
			break;
		}
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

/*======================================消息码处理函数======================================*/

// 0号测试函数
NTSTATUS TestFuncation(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	if (buffer == NULL)
	{
		KdPrint(("无接受数据\n"));
		return STATUS_SUCCESS;
	}
	pDevice;
	pIrp->MdlAddress;
	KdPrint((buffer));
	RtlCopyMemory(buffer, "[0环]这是来自零环的测试字符\n", 0x100);

	pIrp->IoStatus.Information = 0x100;
	return STATUS_SUCCESS;
}

// 1号遍历驱动函数
NTSTATUS Nt_EnumDriver(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================通用处理头=================================*/
	/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;
	/*=================================函数实现主体=================================*/

	LDR_DATA_TABLE_ENTRY* pLdr = (LDR_DATA_TABLE_ENTRY*)g_pDriver->DriverSection;	// 设备双向链表
	LDR_DATA_TABLE_ENTRY* pBegin = pLdr;
	EnumDrive* BufferFromKernel = (EnumDrive*)(buffer+8);				// 来自用户层的缓冲区
	int sizeOfData = 0;
	__try {
		do
		{
			//KdPrint(("%08X | %06X | %wZ\n",
			//	pLdr->DllBase,
			//	pLdr->SizeOfImage,
			//	&pLdr->FullDllName));
			if (Button_Copy)
			{
				// 第二次接受消息 开始拷贝数据
				EnumDrive* Tmp = (EnumDrive*)((char*)BufferFromKernel + sizeOfData);
			
				RtlCopyMemory(Tmp->DllName,						// 设备名称
					pLdr->BaseDllName.Buffer, pLdr->BaseDllName.Length);
				Tmp->DllBase = (ULONG)pLdr->DllBase;			// 加载基址
				Tmp->SizeOfImage = (ULONG)pLdr->SizeOfImage;	// 映像大小
				RtlCopyMemory(Tmp->buffer,						// 设备路径 
					pLdr->FullDllName.Buffer, pLdr->FullDllName.Length);
			}
			// +2 是因为这不是以空字符结尾且是宽字符
			sizeOfData += (sizeof(EnumDrive)+
				pLdr->FullDllName.Length);									// 文件总大小
			pLdr = (LDR_DATA_TABLE_ENTRY*)(pLdr->InLoadOrderLinks.Flink);
		} while (pBegin != pLdr);
		//添加一个结构体的空内存 用于结束判断；
		{
			if (Button_Copy)
			{
				EnumDrive* Tmp = (EnumDrive*)((char*)BufferFromKernel + sizeOfData);
				Tmp->DllBase = 0;
				Tmp->SizeOfImage = 0;
			}
			/*空字符 + 头大小*/
			sizeOfData += (sizeof(EnumDrive) + 8);
			UserInfo->bufferSize = sizeOfData;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		KdPrint(("出现异常:%08x", GetExceptionCode()));
	}

	/*=================================通用处理尾=================================*/
	// 如果是首次接受，则只返回一个头大小
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// 如果是次次接受，则只返回数据大小
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}

	return STATUS_SUCCESS;
}

// 2号->进程、线程、模块遍历
NTSTATUS Nt_EnumPEM(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================通用处理头=================================*/
	/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================函数实现主体=================================*/
	// 去掉数据头 获取数据缓冲区
	PEBInfo * BufferFromKernel = (PEBInfo*)(buffer + 8);
	int sizeOfData = 0;

	// 1. 获取当前进程对象
	ULONG_PTR proc = (ULONG_PTR)PsGetCurrentProcess();
	// 2. 获取进程对象内的当前活动进程链表
	LIST_ENTRY* pProcList = (LIST_ENTRY*)(proc + 0xB8);
	LIST_ENTRY* listBegin = pProcList;
	// 开始遍历
	__try 
	{
		do {
			CHAR cpath[255] = { 0 };
			proc = (ULONG_PTR)((CHAR*)pProcList - 0xB8);
			PEPROCESS perp = (PEPROCESS)proc;
			perp;
			// 获取进程ID,进程路径,EPROCESS.
			ULONG pid = (ULONG)PsGetProcessId((PEPROCESS)proc); //*(ULONG*)(proc + 0xB4);															
			// 剔除坏链 跳到下一个链表
			{
				if (pid > 0x10000000)
				{
					pProcList = pProcList->Flink;
					if (pProcList != listBegin)
					{
						continue;
					}
					else
					{
						break;
					}					
				}
			}												// 进程名称
			//CHAR* procName = PsGetProcessImageFileName((PEPROCESS)proc);//(CHAR*)proc + 0x1ec;
			UCHAR* procName = (UCHAR*)((char*)proc + 0x16c);
	
			// 获取进程路径	
			DisplayPathByEP((PEPROCESS)proc, cpath);
			// 父进程ID
			ULONG parentPid = (ULONG)((char)proc + 0x140);
			
		/*	KdPrint(("进程名：%s | PID:%d\n", procName, pid));
			KdPrint(("父进程PID=%d | EPROCES=%08X\n", parentPid, &proc));
			KdPrint(("进程路径=%s \n", cpath));
*/
			// 满足条件 打开开关进行复制
			if (Button_Copy)
			{
				PEBInfo* Tmp = (PEBInfo*)((char*)BufferFromKernel + sizeOfData);
				RtlCopyMemory(Tmp->ImageName, procName, 15); // 拷贝进程名
				Tmp->pid = pid;
				Tmp->parentPid = parentPid;
				Tmp->eprocess = proc;
				Tmp->cPath;
				RtlCopyMemory(Tmp->cPath, cpath,strlen(cpath)); // 拷贝进程路径
			}

			/*  大小 = 头大小 + 路径大小 */
			/* -1 是因为路劲开始定义的是一个字节*/
			sizeOfData += sizeof(PEBInfo) + strlen(cpath);
			// 得到进程链表的下一个.
			pProcList = pProcList->Flink ;

		} while (pProcList != listBegin);

		// 数据大小
		if (Button_Copy)
		{
			PEBInfo* Tmp = (PEBInfo*)((char*)BufferFromKernel + sizeOfData);
			Tmp->ImageName;
			Tmp->pid = (ULONG)-1;
			Tmp->eprocess = 0;
			Tmp->numOfThe = 0;
			Tmp->cPath;
		}
		UserInfo->bufferSize = sizeOfData + sizeof(PEBInfo);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("出现异常:%08x\n", GetExceptionCode()));
	}

	/*=================================通用处理尾=================================*/
	// 如果是首次接受，则只返回一个头大小
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// 如果是次次接受，则只返回数据大小
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}
	
	return STATUS_SUCCESS;
}

// 3号->文件遍历
NTSTATUS Nt_EnumFile(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================通用处理头=================================*/
	/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;




	/*=================================通用处理尾=================================*/
// 如果是首次接受，则只返回一个头大小
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// 如果是次次接受，则只返回数据大小
	else
	{
		pIrp->IoStatus.Information = 0;//此处填写数据总大小
	}
	return STATUS_SUCCESS;
}

// 4号->注册表遍历
NTSTATUS Nt_EnumReg(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;


	return STATUS_SUCCESS;
}

// 5号->IDT遍历
NTSTATUS Nt_EnumIDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{

	/*=================================通用处理头=================================*/
	/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================函数实现主体=================================*/
	// 去掉数据头 获取数据缓冲区
	IDTInfo * BufferFromKernel = (IDTInfo*)(buffer + 8);
	int sizeOfData = 0;
	
	IDT_INFO _SIDT = { 0,0,0 };
	PIDT_ENTRY pIDTEntry = NULL;

	// 获取IDT表地址;
	__asm sidt _SIDT;

	// 获取IDT表数组地址
	pIDTEntry = (PIDT_ENTRY)MAKE_LONG(_SIDT.uLowIdtBase, _SIDT.uHighIdtBase);

	// 获取IDT信息
	__try
	{
		for (ULONG i = 0; i < 0x100; i++)
		{

			//KdPrint(("-------中断描述符表-------"));

			// 中断地址
			ULONG Idt_address = MAKE_LONG(pIDTEntry[i].uOffsetLow, pIDTEntry[i].uOffsetHigh);
			//KdPrint(("addr : 0x%08x\n", Idt_address));

			//// 中断号
			//KdPrint(("中断号：【%d】\n", i));

			//// 段选择子
			//KdPrint(("selector : %d\n", pIDTEntry[i].uSelector));

			//// 类型
			//KdPrint(("GateType : %d\n", pIDTEntry[i].GateType));

			//// 特权级别
			//KdPrint(("DPL: %d\n"), pIDTEntry[i].DPL);


			if (Button_Copy)
			{
				BufferFromKernel[i].uAddrOffset = Idt_address;
				BufferFromKernel[i].uNum = i;
				BufferFromKernel[i].uSelector = pIDTEntry[i].uSelector;
				BufferFromKernel[i].uType = pIDTEntry[i].GateType;
				BufferFromKernel[i].uLevel = pIDTEntry[i].DPL;
			}
			sizeOfData += sizeof(IDTInfo);
		}
		/*添加一个空结构体用以结尾*/
		if (Button_Copy)
		{
			IDTInfo* Tmp = (IDTInfo*)((char*)BufferFromKernel + sizeOfData);
			Tmp->uAddrOffset = 0;
			Tmp->uLevel = 0;
			Tmp->uNum = 0;
			Tmp->uSelector = 0;
			Tmp->uType = 0;
		}
		UserInfo->bufferSize = sizeOfData + sizeof(IDTInfo);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
	

	/*=================================通用处理尾=================================*/
	// 如果是首次接受，则只返回一个头大小
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// 如果是次次接受，则只返回数据大小
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}

	return STATUS_SUCCESS;
}

// 6号->GDT遍历
NTSTATUS Nt_EnumGDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================通用处理头=================================*/
	/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================函数实现主体=================================*/
	// 去掉数据头 获取数据缓冲区
	GDT * BufferFromKernel = (GDT*)(buffer + 8);
	int sizeOfData = 0;

	GDT_INFO;
	GDT_INFO _SGDT = { 0,0,0 };
	PGDT pGDTEntry = NULL;

	// 获取GDT表地址;
	__asm sgdt _SGDT;

	// 获取GDT表数组地址
	pGDTEntry = (PGDT)MAKE_LONG(_SGDT.uLowGDTBase, _SGDT.uHighGDTBase);

	// 获取GDT信息
	// 19.12.25
	__try
	{
		for (ULONG i = 0; i < 0x100; i++)
		{

			//KdPrint(("-------GDT描述符表-------"));

			// 基址
			ULONG GDT_address = MAKE_LONG(pGDTEntry[i].Base0_23, pGDTEntry[i].Base24_31);
			//KdPrint(("address : %d | ", GDT_address));
			GDT_address;
			// 界限
			ULONG GDT_limit = MAKE_LONG(pGDTEntry[i].Limit0_15, pGDTEntry[i].Limit16_19);
			//KdPrint(("GateType : %d\n", GDT_limit));
			GDT_limit;
			// 段粒度
			//if (pGDTEntry[i].G == 0)
			//	KdPrint(("PG:Byte | "));
			//else KdPrint(("PG:Page\n"));

			// 段特权等级
			//KdPrint(("DPL: %d| ", pGDTEntry[i].DPL));

			// 类型
			//KdPrint(("DPL: %d\n", pGDTEntry[i].Type));


			if (Button_Copy)
			{
				BufferFromKernel[i] = pGDTEntry[i];
			}
			sizeOfData += sizeof(GDT);
		}
		/*添加一个空结构体用以结尾*/
		if (Button_Copy)
		{
			PGDT* Tmp = (PGDT*)((char*)BufferFromKernel + sizeOfData);
			*Tmp = 0;
		}
		UserInfo->bufferSize = sizeOfData + sizeof(GDT);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		KdPrint(("Error!\n"));
	}


	return STATUS_SUCCESS;
}

// 7号->SSDT遍历
NTSTATUS Nt_EnumSSDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================通用处理头=================================*/
		/*获取缓冲区 | 添加开关 | 判定接受次数*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// 是否拷贝数据的开关
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver 空指针\n"));
		return STATUS_SUCCESS;
	}
	// 判断是第几次接受指令
	//	第一次接收只计算数据大小，并返回数据大小，用于用户层申请对应大小的空间。
	//	第二次接受将返回数据
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================函数实现主体=================================*/
	// 去掉数据头 获取数据缓冲区
	SSDTINFO * BufferFromKernel = (SSDTINFO*)(buffer + 8);
	int sizeOfData = 0;
	// 获取SSDT

	PETHREAD pCurThread = PsGetCurrentThread();

	KSTD * pSSDT = (KSTD*)
		(*(ULONG*)((ULONG_PTR)pCurThread + 0xBC));
	__try
	{
		ULONG Num = (ULONG)pSSDT->ntoskrnl.NumberOfService;
		for (ULONG i = 0; i < Num; i++)
		{
			// 调用号
			i;
			// 函数地址
			INT Addr = pSSDT->ntoskrnl.ServiceTableBase[i];
			Addr;
			if (Button_Copy)
			{
				BufferFromKernel[i].Num = i;
				BufferFromKernel[i].Addr = pSSDT->ntoskrnl.ServiceTableBase[i];
			}		
			
			// 计算数据的大小
			sizeOfData += sizeof(SSDTINFO);
		}
		/*添加一个空结构体用以结尾*/
		if (Button_Copy)
		{
			SSDTINFO* Tmp = (SSDTINFO*)((char*)BufferFromKernel + sizeOfData);
			Tmp->Num = -1;
			Tmp->Addr = 0;
		}

		UserInfo->bufferSize = sizeOfData + sizeof(SSDTINFO);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{KdPrint(("Error!\n"));}




	return STATUS_SUCCESS;
}

// 11号->驱动隐藏
NTSTATUS Nt_HideDri(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}

	// 获取LDR链
	LDR_DATA_TABLE_ENTRY* pLdr = (LDR_DATA_TABLE_ENTRY*)g_pDriver->DriverSection;	// 设备双向链表
	LDR_DATA_TABLE_ENTRY* pBegin = pLdr;

	// 获取缓冲区
	INTERRACTINFO* InterractInfo = (INTERRACTINFO*)buffer;

	// 开始遍历 寻找 隐藏
	do
	{
		if (pBegin->FullDllName.Buffer != 0)
		{
			if (wcscmp(pBegin->BaseDllName.Buffer, InterractInfo->Buffer) == 0)
			{			

				KdPrint(("驱动 %ws 隐藏成功!\n", pBegin->BaseDllName.Buffer));

				*((ULONG*)pBegin->InLoadOrderLinks.Blink) =
					(ULONG)pBegin->InLoadOrderLinks.Flink;
				pBegin->InLoadOrderLinks.Flink->Blink =
					pBegin->InLoadOrderLinks.Blink;

					pBegin->InLoadOrderLinks.Flink = 
					(LIST_ENTRY*)&(pBegin->InLoadOrderLinks.Flink);
					pBegin->InLoadOrderLinks.Blink =
						(LIST_ENTRY*)&(pBegin->InLoadOrderLinks.Flink);

					// 返回正确状态
					InterractInfo->Status = 1;

					return STATUS_SUCCESS;
			}
		}
		// 下一个链表
		pBegin = (PLDR_DATA_TABLE_ENTRY)pBegin->InLoadOrderLinks.Flink;
	} while (pBegin != pLdr);
	// 返回错误状态
	InterractInfo->Status = 0;

	return STATUS_SUCCESS;
}

// 12号->隐藏进程
NTSTATUS Nt_HideProcess(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	INTERRACTINFO* InterractInfo = (INTERRACTINFO*)buffer;
	// 获取EPROCESS
	ULONG_PTR proc = (ULONG_PTR)PsGetCurrentProcess();
	// 2. 获取进程对象内的当前活动进程链表
	LIST_ENTRY* pProcList = (LIST_ENTRY*)((ULONG)proc + 0xB8);
	LIST_ENTRY* listBegin = pProcList;

	// 开始遍历比较
	__try
	{
		do 
		{
			proc = (ULONG_PTR)((CHAR*)listBegin - 0xB8);
			// 获取进程 ID 与传入的作比较 
			ULONG pid = (ULONG)PsGetProcessId((PEPROCESS)proc);
			if (pid == InterractInfo->Number)
			{
				// 改变前后
				*((ULONG*)listBegin->Blink )=
					(ULONG)listBegin->Flink;
				listBegin->Flink->Blink =
					(LIST_ENTRY*)listBegin->Blink;

				// 改变自己
				listBegin->Flink =
					(LIST_ENTRY*)&(listBegin->Flink);
				listBegin->Blink =
					(LIST_ENTRY*)&(listBegin->Flink);


				InterractInfo->Status = 1;
				return STATUS_SUCCESS;
			}

			listBegin = listBegin->Flink;
		} while (listBegin != pProcList);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Error!\n"));
	}

	InterractInfo->Status = 0;
	
	return STATUS_SUCCESS;
}

// 21号->SystemEntry Hook 获取需要保护的 ID 
NTSTATUS Nt_SysEntryHook(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	INTERRACTINFO* InterractInfo = (INTERRACTINFO*)buffer;

	g_Pid = (int)InterractInfo->Number;

	InterractInfo->Status = 1;

	return STATUS_SUCCESS;
}

/*=================================对各派遣函数的支持=================================*/

/* 1号->通过EPROCESS 获取进程路径*/
void DisplayPathByEP(PEPROCESS pep, CHAR* path)
{
	if (*(PULONG)((ULONG)pep + 0x0b4) == 0x4)
	{
		KdPrint(("Systemn"));
		return;
	}
	//获取 _SE_AUDIT_PROCESS_CREATION_INFO
	ULONG SEAuditValue = *(PULONG)((ULONG)pep + 0x1ec);
	//获取_OBJECT_NAME_INFORMATION指针
	PULONG pNameInfo = (PULONG)SEAuditValue;
	PUNICODE_STRING uPath = (PUNICODE_STRING)(PVOID)pNameInfo;
	//UNICODE_STRING uTmp = RTL_CONSTANT_STRING(uPath->Buffer);
	
	ANSI_STRING aPath = { 0 };
	RtlUnicodeStringToAnsiString(&aPath, uPath, TRUE);
	RtlCopyMemory(path,aPath.Buffer,aPath.Length);

	RtlFreeAnsiString(&aPath);
}

/* 21号->SystemEntry Hook */
/* HOOK KiFastCallEntry*/
int SetKiFastCallEntryAddr(PVOID pAddress)
{
	__asm push ecx;
	__asm push edx;				// 保存寄存器
	__asm xor edx, edx;		
	__asm mov ecx, 0x176;
	__asm mov eax, pAddress;	
	__asm wrmsr;				// 将地址写入0x176号寄存器
	__asm pop edx;
	__asm pop ecx;				// 恢复寄存器
	return TRUE;
}

/* 获取 KiFastCallEntry 地址*/
PVOID GetKiFastCallEntryAddr()
{
	PVOID oldKiFastCallEntry;	// 保存原始的 KiFastCallEntry
	__asm
	{
		push ecx;				// 保存寄存器
		mov ecx, 0x176;
		rdmsr;				// 将ECX指定的MSR加载到 EDX:EAX
		mov oldKiFastCallEntry, eax;
		pop ecx;
	}
	return oldKiFastCallEntry;
}



