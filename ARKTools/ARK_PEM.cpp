// ARK_PEM.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_PEM.h"
#include "afxdialogex.h"

/*=================================自定义头=================================*/
#include "Sources.h"

/*=================================自定义消息=================================*/
#define UM_PEM		WM_USER+200

/*=================================自定义结构体=================================*/
typedef struct _PEMINFO
{
	ULONG pid;
	ULONG parentPid;
	ULONG numOfThe;
	ULONG eprocess;
	CHAR ImageName[15];
	CHAR cPath[1];

}PEBInfo, *pPEBInfo;

// ARK_PEM 对话框

IMPLEMENT_DYNAMIC(ARK_PEM, CDialogEx)

ARK_PEM::ARK_PEM(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumPEM, pParent)
{

}

ARK_PEM::~ARK_PEM()
{
}

void ARK_PEM::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PEM_LIST, m_PEM_List);
}




BEGIN_MESSAGE_MAP(ARK_PEM, CDialogEx)
	ON_MESSAGE(UM_PEM, &ARK_PEM::OnUmPem)
	ON_COMMAND(ID_HIDEPROCESS, &ARK_PEM::OnHideProcess)
	ON_NOTIFY(NM_RCLICK, IDC_PEM_LIST, &ARK_PEM::OnNMRClickPemList)
	ON_COMMAND(ID_KILLPROCESS, &ARK_PEM::OnKillprocess)
	ON_COMMAND(ID_PROCPROCESS, &ARK_PEM::OnProtectProcess)
END_MESSAGE_MAP()


// ARK_PEM 消息处理程序


BOOL ARK_PEM::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
		// ListInitial
	m_PEM_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | WS_VSCROLL);
	m_PEM_List.EnsureVisible(m_PEM_List.GetItemCount() - 1, FALSE);

	const WCHAR* wTtile[] = {
			L"进程名称",
			L"进程ID",
			L"EPROCESS",
			L"父进程PID",
			L"进程路径",
	};
	m_PEM_List.InsertColumn(0, wTtile[0], LVCFMT_CENTER, 200);
	m_PEM_List.InsertColumn(1, wTtile[1], LVCFMT_CENTER, 100);
	m_PEM_List.InsertColumn(2, wTtile[2], LVCFMT_CENTER, 100);
	m_PEM_List.InsertColumn(3, wTtile[3], LVCFMT_CENTER, 100);
	m_PEM_List.InsertColumn(4, wTtile[4], LVCFMT_CENTER, 200);


	// 绑定菜单
	m_Menu.LoadMenu(IDR_MENU2);
	SetMenu(&m_Menu);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


// 沟通内核 遍历进程 - 具体实现
VOID ARK_PEM::Kernel_EnumPEM()
{
	HANDLE hDev = CreateFileW(L"\\\\.\\mydevicesym1",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDev == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode;
		CString csErroCode;
		ErrorCode = GetLastError();
		csErroCode.Format(_T("\n文件读取失败 错误码:%d\n"), ErrorCode);
		OutputDebugStringW(csErroCode.GetString());
		return;
	}

	//=================================第一次发送=================================//
	/*发送头获取数据大小*/
	CommintInfo BufferToUser_Fir = { 0, 10, 0 };
	DWORD dwSizeOfGetBuffer_Fir = sizeof(BufferToUser_Fir);
	CommintInfo BufferToKernel_Fir = { 0, 10, 0 };		/*输出缓冲区*/
	DWORD dwSizeOfPutBuffer_Fir = sizeof(BufferToUser_Fir);	/*输出缓冲区的字节数*/

	DWORD dwRetSize_Fir;
	DeviceIoControl(hDev, CTL_EnumPEM,
		&BufferToUser_Fir, dwSizeOfGetBuffer_Fir,
		&BufferToKernel_Fir, dwSizeOfPutBuffer_Fir,
		&dwRetSize_Fir, NULL);

	//=================================第二次发送=================================//
	/*用获取的数据大小申请缓冲区，存储数据*/
	DWORD dwSize = BufferToKernel_Fir.bufferSize + 8;
	//CHAR* buffer_Get = new CHAR[dwSize];
	//memset(buffer_Get, 0, dwSize);

	CommintInfo buffer_Get ;
	pCommintInfo BuufferToUser_Sec = (pCommintInfo)&buffer_Get;
	DWORD dwSizeOfGetBuffer_Sec = dwSize;
	BuufferToUser_Sec->bNum = 1;// 第二次发送

	CHAR* buffer_Put = new CHAR[dwSize];
	memset(buffer_Put, 0, dwSize);
	pCommintInfo BuufferKernel_Sec = (pCommintInfo)buffer_Put;
	DWORD dwSizeOfPutBuffer_Sec = dwSize;
	BuufferKernel_Sec->bNum = 1;

	DWORD dwRetSize_Sec;
	DeviceIoControl(hDev, CTL_EnumPEM,
		BuufferToUser_Sec, sizeof(buffer_Get),
		BuufferKernel_Sec, dwSizeOfPutBuffer_Sec,
		&dwRetSize_Sec, NULL);

	/*================================= 添加至列表控件中=================================*/
	/*对接受到的数据进行解析*/
	/*
		typedef struct _PEMINFO
	{
		ULONG pid;
		ULONG parentPid;
		ULONG numOfThe;
		WCHAR ImageName[1];
		CHAR cPath[1];

	}PEBInfo,*PEBInfo;	
	*/
	m_PEM_List.DeleteAllItems();

	pPEBInfo InfoBuffer = (pPEBInfo)((char*)BuufferKernel_Sec + 8);
	DWORD dwIndex = 0;
	DWORD dwDataSize = 0;
	while (InfoBuffer->pid != -1)
	{
		CString pid;					//	进程ID
		CString parentPid;				//	父进程ID
		CString numOfThe;				//	线程数量
		CString eprocess;				//	EPROCESS
		CString ImageName;				//	进程名
		CString wcPath;					//	进程路径
		CA2W wImageName(InfoBuffer->ImageName);
		CA2W wPath(InfoBuffer->cPath);
		// 添加行
		m_PEM_List.InsertItem(dwIndex, L"");
		m_PEM_List.UpdateWindow();

		// 格式化字符
		pid.Format(_T("%08X"), InfoBuffer->pid);
		parentPid.Format(_T("%08X"), InfoBuffer->parentPid);
		eprocess.Format(_T("%X"), InfoBuffer->eprocess);
		ImageName.Format(_T("%s"), wImageName.m_szBuffer);
		wcPath.Format(_T("%s"), wPath.m_szBuffer);

		// 插入数据
		m_PEM_List.SetItemText(dwIndex, 0, ImageName);
		m_PEM_List.SetItemText(dwIndex, 1, pid);
		m_PEM_List.SetItemText(dwIndex, 2, eprocess);
		m_PEM_List.SetItemText(dwIndex, 3, parentPid);
		m_PEM_List.SetItemText(dwIndex, 4, wcPath);

		// 计算 名称+路径 大小
		//int Lenth_Path = wcPath.GetLength();
		int Lenth_Path = MyGetLenth(InfoBuffer->cPath);
		//int Lenth_Path = sizeof(wcPath.GetBuffer());


		// 指向下一个数据块
		InfoBuffer = (pPEBInfo)((char*)InfoBuffer + sizeof(PEBInfo)+ Lenth_Path);

		++dwIndex;
	}
	CloseHandle(hDev);
	return VOID();
}

// 沟通内核 遍历进程 - 消息触发
afx_msg LRESULT ARK_PEM::OnUmPem(WPARAM wParam, LPARAM lParam)
{
	Kernel_EnumPEM();
	return 0;
}

// 弹出菜单
void ARK_PEM::OnNMRClickPemList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CMenu* pcMenu = m_Menu.GetSubMenu(NULL);

	CPoint cPoint;

	GetCursorPos(&cPoint);

	pcMenu->TrackPopupMenu(0, cPoint.x, cPoint.y, this);
}

// 隐藏进程 - 消息触发
void ARK_PEM::OnHideProcess()
{
	// TODO: 在此添加命令处理程序代码
	HideProcess();
}

// 隐藏进程 - 具体实现
VOID ARK_PEM::HideProcess()
{
	HANDLE hDev = CreateFileW(L"\\\\.\\mydevicesym1",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDev == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode;
		CString csErroCode;
		ErrorCode = GetLastError();
		csErroCode.Format(_T("\n文件读取失败 错误码:%d\n"), ErrorCode);
		OutputDebugStringW(csErroCode.GetString());
		return;
	}

	// 获取选中的行 
	int index = (int)m_PEM_List.GetFirstSelectedItemPosition();
	if (index == 0) return;
	--index;

	// 获取作为标识的信息 BASE
	CString DllName = m_PEM_List.GetItemText(index, 1);
	UINT32 pid =  wcstol(DllName.GetBuffer(), NULL, 16);

	// 通用的消息传输结构体， 可以传输字符或数字
	INTERRACTINFO RecInfo = { CTL_HideProcess,0,NULL,0 };

	RecInfo.Number = pid;
	INT SizeS = sizeof(INTERRACTINFO);

	DWORD dwRetSize_Sec;
	BOOL bRec = DeviceIoControl(hDev, CTL_HideProcess,
		0, 0,
		&RecInfo, SizeS,
		&dwRetSize_Sec, NULL);

	if (RecInfo.Status)
		MessageBoxW(L"隐藏进程成功", L"提示", MB_OK);
	else MessageBoxW(L"隐藏进程失败", L"提示", MB_OK);

	CloseHandle(hDev);
	return VOID();
}

// 结束进程 - 消息触发
void ARK_PEM::OnKillprocess()
{
	// TODO: 在此添加命令处理程序代码
	KillProcess();
}

// 结束进程 - 具体实现
VOID ARK_PEM::KillProcess()
{

	int index = (int)m_PEM_List.GetFirstSelectedItemPosition();
	if (index == 0) return;
	--index;
	CString strPid = m_PEM_List.GetItemText(index, 1);
	DWORD dwPid = wcstol(strPid.GetBuffer(), NULL, 16);

	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);

	if (hProcess == 0)
	{
		MessageBox(L"打开进程失败");
		CloseHandle(hProcess);
		return;
	}
	else
	{
		BOOL REC = TerminateProcess(hProcess, 4);
		CloseHandle(hProcess);
		if (REC)
		{
			MessageBox(L"进程结束成功", 0, MB_OK);
		}
		else
		{
			MessageBox(L"进程结束失败", 0, MB_OK);
		}
	}

	return VOID();
}

// 保护进程 - 消息触发
void ARK_PEM::OnProtectProcess()
{
	// TODO: 在此添加命令处理程序代码
	ProtectProcess();
}

// 保护进程 - 具体实现
VOID ARK_PEM::ProtectProcess()
{
	/* 通过SystemEntry HOOK  保护*/
	/* 具体实现是获取PID 发送到内核层进行保护*/

	HANDLE hDev = CreateFileW(L"\\\\.\\mydevicesym1",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDev == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode;
		CString csErroCode;
		ErrorCode = GetLastError();
		csErroCode.Format(_T("\n文件读取失败 错误码:%d\n"), ErrorCode);
		OutputDebugStringW(csErroCode.GetString());
		return;
	}

	// 获取选中的行 
	int index = (int)m_PEM_List.GetFirstSelectedItemPosition();
	if (index == 0) return;
	--index;

	// 获取作为标识的信息 BASE
	CString DllName = m_PEM_List.GetItemText(index, 1);
	UINT32 pid = wcstol(DllName.GetBuffer(), NULL, 16);

	// 通用的消息传输结构体， 可以传输字符或数字
	INTERRACTINFO RecInfo = { CTL_SYSHOOK,0,NULL,0 };

	RecInfo.Number = pid;
	INT SizeS = sizeof(INTERRACTINFO);

	DWORD dwRetSize_Sec;
	BOOL bRec = DeviceIoControl(hDev, CTL_SYSHOOK,
		0, 0,
		&RecInfo, SizeS,
		&dwRetSize_Sec, NULL);

	if (RecInfo.Status)
		MessageBoxW(L"进程保护成功", L"提示", MB_OK);
	else MessageBoxW(L"进程保护失败", L"提示", MB_OK);

	CloseHandle(hDev);
	return VOID();
}


/*==========================工具函数==========================*/
INT MyGetLenth(char* buffer)
{
	int i = 0;
	char* p = buffer;
	while (*p != NULL)
	{
		++i;
		++p;
	}
	return i;
}






