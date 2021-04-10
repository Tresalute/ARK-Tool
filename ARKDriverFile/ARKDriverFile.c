
#include "ARKDriverFile.h"

/*=================================��������ԭ��=================================*/
CHAR* PsGetProcessImageFileName(IN PEPROCESS Process);

/* 21��->SystemEntry Hook */
/* ��HOOK��  KiFastCAllEntry  */
void _declspec(naked)MyKiFastCallEntry()
{
	__asm
	{
		;// �����ú�
		cmp eax, 0xBE;
		jne _DONE;		// ���úŲ���0xBE,ִ�е��Ĳ�

		;// ������ID�ǲ���Ҫ�����Ľ���ID
		push eax;		// ���ݼĴ���

		;// ��ȡ����(����ID)
		mov eax, [edx + 0x14];	// eax�������PCLIENT_ID
		mov eax, [eax];			// eax�������PCLIENT_ID->UniqueProcess

		;// �ж��ǲ���Ҫ�����Ľ���ID
		cmp eax, [g_Pid];
		pop eax;		// �ָ��Ĵ���
		jne _DONE;		// ����Ҫ�����Ľ��̾���ת

		;// �ǵĻ��͸õ��ò����� �ú�����������ʧ��
		mov[edx + 0xC], 0;		// ������Ȩ������Ϊ0

	_DONE:
		;// ����ԭ����KiFastCallEntry
		jmp g_oldKiFastCallEntry;
	}
}


DispatchFuncation g_funcation[] = {
	{CTL_Test		,TestFuncation},		// ����
	{CTL_EnumDrive	,Nt_EnumDriver},		// ��������
	{CTL_EnumPEM	,Nt_EnumPEM},			// ����
	{CTL_EnumFile	,Nt_EnumFile},			// �ļ�
	{CTL_EnumReg	,Nt_EnumReg},			// ע���
	{CTL_EnumIDT	,Nt_EnumIDT},			// IDT
	{CTL_EnumGDT	,Nt_EnumGDT},			// GDT
	{CTL_EnumSSDT	,Nt_EnumSSDT},			// SSDT

	{CTL_HideDri	,Nt_HideDri},			// ��������
	{CTL_HideProcess,Nt_HideProcess},		// ���ؽ���

	{CTL_SYSHOOK	,Nt_SysEntryHook},		// SystemEntry Hook
};

DRIVER_OBJECT* g_pDriver = NULL;

NTSTATUS DriverUnLoad(DRIVER_OBJECT* pDriver)
{
	pDriver;

	// ��ԭHOOK
	SetKiFastCallEntryAddr(g_oldKiFastCallEntry);

	KdPrint(("File_����ж��\n"));
	IoDeleteDevice(pDriver->DeviceObject);
	UNICODE_STRING symName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoDeleteSymbolicLink(&symName);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(DRIVER_OBJECT* pDriver, UNICODE_STRING* regpath)
{
	KdPrint(("File��������\n"));
	regpath;
	pDriver->DriverUnload = &DriverUnLoad;
	g_pDriver = pDriver;
	NTSTATUS status = STATUS_SUCCESS;
	// ����һ���豸����
	UNICODE_STRING uDeviceName = RTL_CONSTANT_STRING(NAME_DEVICE);
	DEVICE_OBJECT* pDevice = NULL;
	status = IoCreateDevice(
		pDriver,			// ��������
		0,
		&uDeviceName,		// �豸��
		FILE_DEVICE_UNKNOWN,
		0, 
		0, 
		&pDevice);			// �豸����
	if (!NT_SUCCESS(status))
	{
		KdPrint(("�����豸����ʧ�� �����룺%d", status));
		return status;
	}
	pDevice->Flags |= DO_DIRECT_IO;

	// �󶨷���
	UNICODE_STRING uName_SyMBol = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoCreateSymbolicLink(&uName_SyMBol, &uDeviceName);

	// ��ǲ��
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceIoControl;
	pDriver->MajorFunction[IRP_MJ_CREATE] = OnCreate;

	/*=======================HOOK KiFastCallEntry=======================*/
	// ��ȡ KiFastCallEntry ��ַ
	g_oldKiFastCallEntry = GetKiFastCallEntryAddr();

	if (g_oldKiFastCallEntry == NULL)
	{
		return STATUS_SUCCESS;
	}

	// ������ı�д MyKiFastCallEntry �滻 ԭ����KiFastCallEntry
	SetKiFastCallEntryAddr((PVOID)MyKiFastCallEntry);


	return STATUS_SUCCESS;
}

/*======================================�մ�����======================================*/

/*�����κδ���*/
/*��ֹ���ش���*/
NTSTATUS OnCreate(DEVICE_OBJECT *DeviceObject,IRP *Irp)
{
	DeviceObject;
	Irp;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

/*======================================��Ϣ��ǲ����======================================*/
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	char* pBuff = NULL;
	if (pIrp->MdlAddress != NULL) {
		// ��Ҫʹ��һ������, ��MDL����ӳ���һ���µ�ϵͳ��յ������ַ.
		// MmGetSystemAddressForMdlSafe - ��ȡϵͳMDL����ӳ��������µ������ַ.
		// ��������ַ����ֱ���޸ĵ��û���������ַ�ռ�.
		pBuff = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, 0);
	}
	else if (pIrp->AssociatedIrp.SystemBuffer != NULL) {
		pBuff = pIrp->AssociatedIrp.SystemBuffer;
	}
	else if (pIrp->UserBuffer != NULL) {
		pBuff = pIrp->UserBuffer;
	}
	else {
		// û�л�����.
		pBuff = NULL;
	}


	IO_STACK_LOCATION* pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	// �õ���Ϣ��
	LONG uCtrlCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;
	
	for (int i = 0; i< _countof(g_funcation);++i)
	{
		if (uCtrlCode == g_funcation[i].code)
		{	// ��Ϣ��ƥ������ö�Ӧ�ķ���
			g_funcation[i].callback(pDevice, pIrp);
			break;
		}
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

/*======================================��Ϣ�봦����======================================*/

// 0�Ų��Ժ���
NTSTATUS TestFuncation(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	if (buffer == NULL)
	{
		KdPrint(("�޽�������\n"));
		return STATUS_SUCCESS;
	}
	pDevice;
	pIrp->MdlAddress;
	KdPrint((buffer));
	RtlCopyMemory(buffer, "[0��]���������㻷�Ĳ����ַ�\n", 0x100);

	pIrp->IoStatus.Information = 0x100;
	return STATUS_SUCCESS;
}

// 1�ű�����������
NTSTATUS Nt_EnumDriver(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================ͨ�ô���ͷ=================================*/
	/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;
	/*=================================����ʵ������=================================*/

	LDR_DATA_TABLE_ENTRY* pLdr = (LDR_DATA_TABLE_ENTRY*)g_pDriver->DriverSection;	// �豸˫������
	LDR_DATA_TABLE_ENTRY* pBegin = pLdr;
	EnumDrive* BufferFromKernel = (EnumDrive*)(buffer+8);				// �����û���Ļ�����
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
				// �ڶ��ν�����Ϣ ��ʼ��������
				EnumDrive* Tmp = (EnumDrive*)((char*)BufferFromKernel + sizeOfData);
			
				RtlCopyMemory(Tmp->DllName,						// �豸����
					pLdr->BaseDllName.Buffer, pLdr->BaseDllName.Length);
				Tmp->DllBase = (ULONG)pLdr->DllBase;			// ���ػ�ַ
				Tmp->SizeOfImage = (ULONG)pLdr->SizeOfImage;	// ӳ���С
				RtlCopyMemory(Tmp->buffer,						// �豸·�� 
					pLdr->FullDllName.Buffer, pLdr->FullDllName.Length);
			}
			// +2 ����Ϊ�ⲻ���Կ��ַ���β���ǿ��ַ�
			sizeOfData += (sizeof(EnumDrive)+
				pLdr->FullDllName.Length);									// �ļ��ܴ�С
			pLdr = (LDR_DATA_TABLE_ENTRY*)(pLdr->InLoadOrderLinks.Flink);
		} while (pBegin != pLdr);
		//���һ���ṹ��Ŀ��ڴ� ���ڽ����жϣ�
		{
			if (Button_Copy)
			{
				EnumDrive* Tmp = (EnumDrive*)((char*)BufferFromKernel + sizeOfData);
				Tmp->DllBase = 0;
				Tmp->SizeOfImage = 0;
			}
			/*���ַ� + ͷ��С*/
			sizeOfData += (sizeof(EnumDrive) + 8);
			UserInfo->bufferSize = sizeOfData;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		KdPrint(("�����쳣:%08x", GetExceptionCode()));
	}

	/*=================================ͨ�ô���β=================================*/
	// ������״ν��ܣ���ֻ����һ��ͷ��С
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// ����Ǵδν��ܣ���ֻ�������ݴ�С
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}

	return STATUS_SUCCESS;
}

// 2��->���̡��̡߳�ģ�����
NTSTATUS Nt_EnumPEM(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================ͨ�ô���ͷ=================================*/
	/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================����ʵ������=================================*/
	// ȥ������ͷ ��ȡ���ݻ�����
	PEBInfo * BufferFromKernel = (PEBInfo*)(buffer + 8);
	int sizeOfData = 0;

	// 1. ��ȡ��ǰ���̶���
	ULONG_PTR proc = (ULONG_PTR)PsGetCurrentProcess();
	// 2. ��ȡ���̶����ڵĵ�ǰ���������
	LIST_ENTRY* pProcList = (LIST_ENTRY*)(proc + 0xB8);
	LIST_ENTRY* listBegin = pProcList;
	// ��ʼ����
	__try 
	{
		do {
			CHAR cpath[255] = { 0 };
			proc = (ULONG_PTR)((CHAR*)pProcList - 0xB8);
			PEPROCESS perp = (PEPROCESS)proc;
			perp;
			// ��ȡ����ID,����·��,EPROCESS.
			ULONG pid = (ULONG)PsGetProcessId((PEPROCESS)proc); //*(ULONG*)(proc + 0xB4);															
			// �޳����� ������һ������
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
			}												// ��������
			//CHAR* procName = PsGetProcessImageFileName((PEPROCESS)proc);//(CHAR*)proc + 0x1ec;
			UCHAR* procName = (UCHAR*)((char*)proc + 0x16c);
	
			// ��ȡ����·��	
			DisplayPathByEP((PEPROCESS)proc, cpath);
			// ������ID
			ULONG parentPid = (ULONG)((char)proc + 0x140);
			
		/*	KdPrint(("��������%s | PID:%d\n", procName, pid));
			KdPrint(("������PID=%d | EPROCES=%08X\n", parentPid, &proc));
			KdPrint(("����·��=%s \n", cpath));
*/
			// �������� �򿪿��ؽ��и���
			if (Button_Copy)
			{
				PEBInfo* Tmp = (PEBInfo*)((char*)BufferFromKernel + sizeOfData);
				RtlCopyMemory(Tmp->ImageName, procName, 15); // ����������
				Tmp->pid = pid;
				Tmp->parentPid = parentPid;
				Tmp->eprocess = proc;
				Tmp->cPath;
				RtlCopyMemory(Tmp->cPath, cpath,strlen(cpath)); // ��������·��
			}

			/*  ��С = ͷ��С + ·����С */
			/* -1 ����Ϊ·����ʼ�������һ���ֽ�*/
			sizeOfData += sizeof(PEBInfo) + strlen(cpath);
			// �õ������������һ��.
			pProcList = pProcList->Flink ;

		} while (pProcList != listBegin);

		// ���ݴ�С
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
		KdPrint(("�����쳣:%08x\n", GetExceptionCode()));
	}

	/*=================================ͨ�ô���β=================================*/
	// ������״ν��ܣ���ֻ����һ��ͷ��С
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// ����Ǵδν��ܣ���ֻ�������ݴ�С
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}
	
	return STATUS_SUCCESS;
}

// 3��->�ļ�����
NTSTATUS Nt_EnumFile(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================ͨ�ô���ͷ=================================*/
	/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;




	/*=================================ͨ�ô���β=================================*/
// ������״ν��ܣ���ֻ����һ��ͷ��С
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// ����Ǵδν��ܣ���ֻ�������ݴ�С
	else
	{
		pIrp->IoStatus.Information = 0;//�˴���д�����ܴ�С
	}
	return STATUS_SUCCESS;
}

// 4��->ע������
NTSTATUS Nt_EnumReg(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;


	return STATUS_SUCCESS;
}

// 5��->IDT����
NTSTATUS Nt_EnumIDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{

	/*=================================ͨ�ô���ͷ=================================*/
	/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================����ʵ������=================================*/
	// ȥ������ͷ ��ȡ���ݻ�����
	IDTInfo * BufferFromKernel = (IDTInfo*)(buffer + 8);
	int sizeOfData = 0;
	
	IDT_INFO _SIDT = { 0,0,0 };
	PIDT_ENTRY pIDTEntry = NULL;

	// ��ȡIDT���ַ;
	__asm sidt _SIDT;

	// ��ȡIDT�������ַ
	pIDTEntry = (PIDT_ENTRY)MAKE_LONG(_SIDT.uLowIdtBase, _SIDT.uHighIdtBase);

	// ��ȡIDT��Ϣ
	__try
	{
		for (ULONG i = 0; i < 0x100; i++)
		{

			//KdPrint(("-------�ж���������-------"));

			// �жϵ�ַ
			ULONG Idt_address = MAKE_LONG(pIDTEntry[i].uOffsetLow, pIDTEntry[i].uOffsetHigh);
			//KdPrint(("addr : 0x%08x\n", Idt_address));

			//// �жϺ�
			//KdPrint(("�жϺţ���%d��\n", i));

			//// ��ѡ����
			//KdPrint(("selector : %d\n", pIDTEntry[i].uSelector));

			//// ����
			//KdPrint(("GateType : %d\n", pIDTEntry[i].GateType));

			//// ��Ȩ����
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
		/*���һ���սṹ�����Խ�β*/
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
	

	/*=================================ͨ�ô���β=================================*/
	// ������״ν��ܣ���ֻ����һ��ͷ��С
	if (Button_Copy == 0)
	{
		pIrp->IoStatus.Information = 10;
	}
	// ����Ǵδν��ܣ���ֻ�������ݴ�С
	else
	{
		pIrp->IoStatus.Information = sizeOfData;
	}

	return STATUS_SUCCESS;
}

// 6��->GDT����
NTSTATUS Nt_EnumGDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================ͨ�ô���ͷ=================================*/
	/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================����ʵ������=================================*/
	// ȥ������ͷ ��ȡ���ݻ�����
	GDT * BufferFromKernel = (GDT*)(buffer + 8);
	int sizeOfData = 0;

	GDT_INFO;
	GDT_INFO _SGDT = { 0,0,0 };
	PGDT pGDTEntry = NULL;

	// ��ȡGDT���ַ;
	__asm sgdt _SGDT;

	// ��ȡGDT�������ַ
	pGDTEntry = (PGDT)MAKE_LONG(_SGDT.uLowGDTBase, _SGDT.uHighGDTBase);

	// ��ȡGDT��Ϣ
	// 19.12.25
	__try
	{
		for (ULONG i = 0; i < 0x100; i++)
		{

			//KdPrint(("-------GDT��������-------"));

			// ��ַ
			ULONG GDT_address = MAKE_LONG(pGDTEntry[i].Base0_23, pGDTEntry[i].Base24_31);
			//KdPrint(("address : %d | ", GDT_address));
			GDT_address;
			// ����
			ULONG GDT_limit = MAKE_LONG(pGDTEntry[i].Limit0_15, pGDTEntry[i].Limit16_19);
			//KdPrint(("GateType : %d\n", GDT_limit));
			GDT_limit;
			// ������
			//if (pGDTEntry[i].G == 0)
			//	KdPrint(("PG:Byte | "));
			//else KdPrint(("PG:Page\n"));

			// ����Ȩ�ȼ�
			//KdPrint(("DPL: %d| ", pGDTEntry[i].DPL));

			// ����
			//KdPrint(("DPL: %d\n", pGDTEntry[i].Type));


			if (Button_Copy)
			{
				BufferFromKernel[i] = pGDTEntry[i];
			}
			sizeOfData += sizeof(GDT);
		}
		/*���һ���սṹ�����Խ�β*/
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

// 7��->SSDT����
NTSTATUS Nt_EnumSSDT(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	/*=================================ͨ�ô���ͷ=================================*/
		/*��ȡ������ | ��ӿ��� | �ж����ܴ���*/
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	// �Ƿ񿽱����ݵĿ���
	int Button_Copy = 0;
	if (g_pDriver == NULL)
	{
		KdPrint(("g_pDriver ��ָ��\n"));
		return STATUS_SUCCESS;
	}
	// �ж��ǵڼ��ν���ָ��
	//	��һ�ν���ֻ�������ݴ�С�����������ݴ�С�������û��������Ӧ��С�Ŀռ䡣
	//	�ڶ��ν��ܽ���������
	pCommintInfo UserInfo = (pCommintInfo)buffer;
	if (UserInfo->bNum == 0)
		Button_Copy = 0;
	else
		Button_Copy = 1;

	/*=================================����ʵ������=================================*/
	// ȥ������ͷ ��ȡ���ݻ�����
	SSDTINFO * BufferFromKernel = (SSDTINFO*)(buffer + 8);
	int sizeOfData = 0;
	// ��ȡSSDT

	PETHREAD pCurThread = PsGetCurrentThread();

	KSTD * pSSDT = (KSTD*)
		(*(ULONG*)((ULONG_PTR)pCurThread + 0xBC));
	__try
	{
		ULONG Num = (ULONG)pSSDT->ntoskrnl.NumberOfService;
		for (ULONG i = 0; i < Num; i++)
		{
			// ���ú�
			i;
			// ������ַ
			INT Addr = pSSDT->ntoskrnl.ServiceTableBase[i];
			Addr;
			if (Button_Copy)
			{
				BufferFromKernel[i].Num = i;
				BufferFromKernel[i].Addr = pSSDT->ntoskrnl.ServiceTableBase[i];
			}		
			
			// �������ݵĴ�С
			sizeOfData += sizeof(SSDTINFO);
		}
		/*���һ���սṹ�����Խ�β*/
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

// 11��->��������
NTSTATUS Nt_HideDri(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}

	// ��ȡLDR��
	LDR_DATA_TABLE_ENTRY* pLdr = (LDR_DATA_TABLE_ENTRY*)g_pDriver->DriverSection;	// �豸˫������
	LDR_DATA_TABLE_ENTRY* pBegin = pLdr;

	// ��ȡ������
	INTERRACTINFO* InterractInfo = (INTERRACTINFO*)buffer;

	// ��ʼ���� Ѱ�� ����
	do
	{
		if (pBegin->FullDllName.Buffer != 0)
		{
			if (wcscmp(pBegin->BaseDllName.Buffer, InterractInfo->Buffer) == 0)
			{			

				KdPrint(("���� %ws ���سɹ�!\n", pBegin->BaseDllName.Buffer));

				*((ULONG*)pBegin->InLoadOrderLinks.Blink) =
					(ULONG)pBegin->InLoadOrderLinks.Flink;
				pBegin->InLoadOrderLinks.Flink->Blink =
					pBegin->InLoadOrderLinks.Blink;

					pBegin->InLoadOrderLinks.Flink = 
					(LIST_ENTRY*)&(pBegin->InLoadOrderLinks.Flink);
					pBegin->InLoadOrderLinks.Blink =
						(LIST_ENTRY*)&(pBegin->InLoadOrderLinks.Flink);

					// ������ȷ״̬
					InterractInfo->Status = 1;

					return STATUS_SUCCESS;
			}
		}
		// ��һ������
		pBegin = (PLDR_DATA_TABLE_ENTRY)pBegin->InLoadOrderLinks.Flink;
	} while (pBegin != pLdr);
	// ���ش���״̬
	InterractInfo->Status = 0;

	return STATUS_SUCCESS;
}

// 12��->���ؽ���
NTSTATUS Nt_HideProcess(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	pDevice;
	CHAR* buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if (buffer == NULL)
	{
		return STATUS_SUCCESS;
	}
	INTERRACTINFO* InterractInfo = (INTERRACTINFO*)buffer;
	// ��ȡEPROCESS
	ULONG_PTR proc = (ULONG_PTR)PsGetCurrentProcess();
	// 2. ��ȡ���̶����ڵĵ�ǰ���������
	LIST_ENTRY* pProcList = (LIST_ENTRY*)((ULONG)proc + 0xB8);
	LIST_ENTRY* listBegin = pProcList;

	// ��ʼ�����Ƚ�
	__try
	{
		do 
		{
			proc = (ULONG_PTR)((CHAR*)listBegin - 0xB8);
			// ��ȡ���� ID �봫������Ƚ� 
			ULONG pid = (ULONG)PsGetProcessId((PEPROCESS)proc);
			if (pid == InterractInfo->Number)
			{
				// �ı�ǰ��
				*((ULONG*)listBegin->Blink )=
					(ULONG)listBegin->Flink;
				listBegin->Flink->Blink =
					(LIST_ENTRY*)listBegin->Blink;

				// �ı��Լ�
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

// 21��->SystemEntry Hook ��ȡ��Ҫ������ ID 
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

/*=================================�Ը���ǲ������֧��=================================*/

/* 1��->ͨ��EPROCESS ��ȡ����·��*/
void DisplayPathByEP(PEPROCESS pep, CHAR* path)
{
	if (*(PULONG)((ULONG)pep + 0x0b4) == 0x4)
	{
		KdPrint(("Systemn"));
		return;
	}
	//��ȡ _SE_AUDIT_PROCESS_CREATION_INFO
	ULONG SEAuditValue = *(PULONG)((ULONG)pep + 0x1ec);
	//��ȡ_OBJECT_NAME_INFORMATIONָ��
	PULONG pNameInfo = (PULONG)SEAuditValue;
	PUNICODE_STRING uPath = (PUNICODE_STRING)(PVOID)pNameInfo;
	//UNICODE_STRING uTmp = RTL_CONSTANT_STRING(uPath->Buffer);
	
	ANSI_STRING aPath = { 0 };
	RtlUnicodeStringToAnsiString(&aPath, uPath, TRUE);
	RtlCopyMemory(path,aPath.Buffer,aPath.Length);

	RtlFreeAnsiString(&aPath);
}

/* 21��->SystemEntry Hook */
/* HOOK KiFastCallEntry*/
int SetKiFastCallEntryAddr(PVOID pAddress)
{
	__asm push ecx;
	__asm push edx;				// ����Ĵ���
	__asm xor edx, edx;		
	__asm mov ecx, 0x176;
	__asm mov eax, pAddress;	
	__asm wrmsr;				// ����ַд��0x176�żĴ���
	__asm pop edx;
	__asm pop ecx;				// �ָ��Ĵ���
	return TRUE;
}

/* ��ȡ KiFastCallEntry ��ַ*/
PVOID GetKiFastCallEntryAddr()
{
	PVOID oldKiFastCallEntry;	// ����ԭʼ�� KiFastCallEntry
	__asm
	{
		push ecx;				// ����Ĵ���
		mov ecx, 0x176;
		rdmsr;				// ��ECXָ����MSR���ص� EDX:EAX
		mov oldKiFastCallEntry, eax;
		pop ecx;
	}
	return oldKiFastCallEntry;
}



