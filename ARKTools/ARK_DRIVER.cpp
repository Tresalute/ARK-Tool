// ARK_DRIVER.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_DRIVER.h"
#include "afxdialogex.h"


/*=================================自定义头=================================*/
#include "Sources.h"

/*=================================自定消息=================================*/
#define UM_DRIVER	WM_USER+100

/*=================================自定义结构体=================================*/
typedef struct _DRIVERINFO
{	
	WCHAR DllName[0x30];
	ULONG DllBase;
	ULONG SizeOfImage;
	WCHAR path[1];
}DriverInfo,*pDriverInfo;

// ARK_DRIVER 对话框

IMPLEMENT_DYNAMIC(ARK_DRIVER, CDialogEx)

ARK_DRIVER::ARK_DRIVER(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumDrive, pParent)
{

}

ARK_DRIVER::~ARK_DRIVER()
{
}

void ARK_DRIVER::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DRIVER_LIST, m_Dr_List);
}



BEGIN_MESSAGE_MAP(ARK_DRIVER, CDialogEx)
	ON_WM_SIZE()
	ON_MESSAGE(UM_DRIVER, &ARK_DRIVER::OnUmDriver)
	ON_COMMAND(ID_HideDriver, &ARK_DRIVER::OnHidedriver)
	ON_NOTIFY(NM_RCLICK, IDC_DRIVER_LIST, &ARK_DRIVER::OnNMRClickDriverList)
END_MESSAGE_MAP()


// ARK_DRIVER 消息处理程序


BOOL ARK_DRIVER::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// 绑定菜单
	m_Menu.LoadMenu(IDR_MENU1);
	SetMenu(&m_Menu);

	// ListInitial
	m_Dr_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | WS_VSCROLL);
	m_Dr_List.EnsureVisible(m_Dr_List.GetItemCount() - 1, FALSE);

	const WCHAR* wTtile[] = {
			L"DllName",
			L"DllBase",
			L"SizeOfImage",
			L"FullDllName",
	};
	m_Dr_List.InsertColumn(0, wTtile[0], LVCFMT_CENTER, 100);
	m_Dr_List.InsertColumn(1, wTtile[1], LVCFMT_CENTER, 100);
	m_Dr_List.InsertColumn(2, wTtile[2], LVCFMT_CENTER, 100);
	m_Dr_List.InsertColumn(3, wTtile[3], LVCFMT_CENTER, 600);

	// 初始化数据
	Kernel_EnumDriver();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// 沟通内核 遍历驱动
VOID ARK_DRIVER::Kernel_EnumDriver()
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
	CommintInfo BufferToUser_Fir = { 0, 10, 0 };
	DWORD dwSizeOfGetBuffer_Fir = sizeof(BufferToUser_Fir);
	CommintInfo BufferToKernel_Fir = { 0, 10, 0 };		/*输出缓冲区*/
	DWORD dwSizeOfPutBuffer_Fir = sizeof(BufferToUser_Fir);	/*输出缓冲区的字节数*/

	DWORD dwRetSize_Fir;
	DeviceIoControl(hDev, CTL_EnumDrive,
		&BufferToUser_Fir, dwSizeOfGetBuffer_Fir,
		&BufferToKernel_Fir, dwSizeOfPutBuffer_Fir,
		&dwRetSize_Fir, NULL);

	//=================================第二次发送=================================//
	DWORD dwSize = BufferToKernel_Fir.bufferSize + 8;
	CHAR* buffer_Get = new CHAR[dwSize];
	memset(buffer_Get, 0, dwSize);
	pCommintInfo BuufferToUser_Sec = (pCommintInfo)buffer_Get;
	DWORD dwSizeOfGetBuffer_Sec = dwSize;
	BuufferToUser_Sec->bNum = 1;// 第二次发送

	CHAR* buffer_Put = new CHAR[dwSize];
	memset(buffer_Put, 0, dwSize);
	pCommintInfo BuufferKernel_Sec = (pCommintInfo)buffer_Put;
	DWORD dwSizeOfPutBuffer_Sec = dwSize;
	BuufferKernel_Sec->bNum = 1;

	DWORD dwRetSize_Sec;
	DeviceIoControl(hDev, CTL_EnumDrive,
		BuufferToUser_Sec, dwSizeOfGetBuffer_Sec,
		BuufferKernel_Sec, dwSizeOfPutBuffer_Sec,
		&dwRetSize_Sec, NULL);

	/*================================= 添加至列表控件中=================================*/
	m_Dr_List.DeleteAllItems();

	pDriverInfo InfoBuffer = (pDriverInfo)((char*)BuufferKernel_Sec + 8);
	DWORD dwIndex = 0;
	while (InfoBuffer->DllBase != 0)
	{

		CString BaseDllName;
		CString DllBase;
		CString SizeOfImage;
		CString path;

		m_Dr_List.InsertItem(dwIndex, L"");
		m_Dr_List.UpdateWindow();

		BaseDllName.Format(_T("%s"), InfoBuffer->DllName);
		DllBase.Format(_T("%08X"), InfoBuffer->DllBase);
		SizeOfImage.Format(_T("%08X"), InfoBuffer->SizeOfImage);
		path.Format(_T("%s"), InfoBuffer->path);

		if (BaseDllName.GetLength()== 0)
		{
			InfoBuffer = (pDriverInfo)((char*)InfoBuffer + sizeof(DriverInfo));
			continue;
		}


		m_Dr_List.SetItemText(dwIndex, 0, BaseDllName);
		m_Dr_List.SetItemText(dwIndex, 1, DllBase);
		m_Dr_List.SetItemText(dwIndex, 2, SizeOfImage);
		m_Dr_List.SetItemText(dwIndex, 3, path);
		int Lenth = path.GetLength();
		/*头大小+字符大小+空字符大小*/
		/*都是宽字符需要*2*/
		InfoBuffer = (pDriverInfo)((char*)InfoBuffer+ ( sizeof(DriverInfo) + Lenth*2));
		++dwIndex;
	}
	CloseHandle(hDev);
	return VOID();
}

// 隐藏驱动
VOID ARK_DRIVER::HideDriver()
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
	int index = (int)m_Dr_List.GetFirstSelectedItemPosition();
	if (index == 0) return;
	--index;

	// 获取作为标识的信息 BASE
	CString DllName = m_Dr_List.GetItemText(index, 0);


	// 通用的消息传输结构体， 可以传输字符或数字
	INTERRACTINFO RecInfo = { CTL_HideDri,0,NULL,0 };
	wcscpy_s(RecInfo.Buffer, DllName.GetBuffer());
	RecInfo.Number = 0;
	INT SizeS = sizeof(INTERRACTINFO);

	DWORD dwRetSize_Sec;
	BOOL bRec = DeviceIoControl(hDev, CTL_HideDri,
		0, 0,
		&RecInfo, SizeS,
		&dwRetSize_Sec, NULL);

	if (RecInfo.Status)
		MessageBoxW(L"隐藏驱动成功", L"提示", MB_OK);
	else MessageBoxW(L"隐藏驱动失败", L"提示", MB_OK);

	CloseHandle(hDev);
	return VOID();
}


// 加载驱动信息
afx_msg LRESULT ARK_DRIVER::OnUmDriver(WPARAM wParam, LPARAM lParam)
{
	Kernel_EnumDriver();
	return 0;
}

// 隐藏驱动 
void ARK_DRIVER::OnHidedriver()
{
	HideDriver();
	// TODO: 在此添加命令处理程序代码
}

// 弹出菜单
void ARK_DRIVER::OnNMRClickDriverList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CMenu* pcMenu = m_Menu.GetSubMenu(NULL);

	CPoint cPoint;

	GetCursorPos(&cPoint);

	pcMenu->TrackPopupMenu(0, cPoint.x, cPoint.y, this);
}
