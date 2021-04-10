// ARK_IDT.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_IDT.h"
#include "afxdialogex.h"

/*自定义头*/
#include "Sources.h"

/*自定义消息*/
#define UM_IDT		WM_USER+500

/*自定义结构体*/
// 5号->IDT构体
typedef struct _IDTINFO
{
	ULONG uAddrOffset;
	ULONG uNum;
	ULONG uSelector;
	ULONG uType;
	ULONG uLevel;
}IDTInfo, *pIDTInfo;

// ARK_IDT 对话框

IMPLEMENT_DYNAMIC(ARK_IDT, CDialogEx)

ARK_IDT::ARK_IDT(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumIDT, pParent)
{

}

ARK_IDT::~ARK_IDT()
{
}

void ARK_IDT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IDT_LIST, m_IDT_List);
}


BEGIN_MESSAGE_MAP(ARK_IDT, CDialogEx)
	ON_MESSAGE(UM_IDT, &ARK_IDT::OnUmIdt)
END_MESSAGE_MAP()


// ARK_IDT 消息处理程序


BOOL ARK_IDT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ListInitial
	m_IDT_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | WS_VSCROLL);
	m_IDT_List.EnsureVisible(m_IDT_List.GetItemCount() - 1, FALSE);

	const WCHAR* wTtile[] = {
			L"中断地址",
			L"中断号",
			L"段选择子",
			L"类型",
			L"特权等级",
	};
	m_IDT_List.InsertColumn(0, wTtile[0], LVCFMT_CENTER, 100);
	m_IDT_List.InsertColumn(1, wTtile[1], LVCFMT_CENTER, 150);
	m_IDT_List.InsertColumn(2, wTtile[2], LVCFMT_CENTER, 150);
	m_IDT_List.InsertColumn(3, wTtile[3], LVCFMT_CENTER, 100);
	m_IDT_List.InsertColumn(4, wTtile[4], LVCFMT_CENTER, 100);


	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



// 沟通内核 遍历IDT
VOID ARK_IDT::Kernel_EnumIDT()
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
	DeviceIoControl(hDev, CTL_EnumIDT,
		&BufferToUser_Fir, dwSizeOfGetBuffer_Fir,
		&BufferToKernel_Fir, dwSizeOfPutBuffer_Fir,
		&dwRetSize_Fir, NULL);


	//=================================第二次发送=================================//
		/*用获取的数据大小申请缓冲区，存储数据*/
	DWORD dwSize = BufferToKernel_Fir.bufferSize + 8;
	//CHAR* buffer_Get = new CHAR[dwSize];
	//memset(buffer_Get, 0, dwSize);

	CommintInfo buffer_Get;
	pCommintInfo BuufferToUser_Sec = (pCommintInfo)&buffer_Get;
	DWORD dwSizeOfGetBuffer_Sec = dwSize;
	BuufferToUser_Sec->bNum = 1;// 第二次发送

	CHAR* buffer_Put = new CHAR[dwSize];
	memset(buffer_Put, 0, dwSize);
	pCommintInfo BuufferKernel_Sec = (pCommintInfo)buffer_Put;
	DWORD dwSizeOfPutBuffer_Sec = dwSize;
	BuufferKernel_Sec->bNum = 1;

	DWORD dwRetSize_Sec;
	DeviceIoControl(hDev, CTL_EnumIDT,
		BuufferToUser_Sec, sizeof(buffer_Get),
		BuufferKernel_Sec, dwSizeOfPutBuffer_Sec,
		&dwRetSize_Sec, NULL);


	/*================================= 添加至列表控件中=================================*/
	/*
	// 5号->IDT构体
		typedef struct _IDTINFO
		{
			ULONG uAddrOffset;		中断地址
			ULONG uNum;				中断号
			ULONG uSelector;		选择子
			ULONG uType;			类型
			ULONG uLevel;			权限
		}IDTInfo, *pIDTInfo;
	*/
	m_IDT_List.DeleteAllItems();

	pIDTInfo InfoBuffer = (pIDTInfo)((char*)BuufferKernel_Sec + 8);
	DWORD dwIndex = 0;
	while (dwIndex<100)
	{
		CString csAddrOffset;
		CString csNum;
		CString csSelector;
		CString csType;
		CString csLevel;

		csAddrOffset.Format(_T("0x%08x"), InfoBuffer[dwIndex].uAddrOffset);
		csNum.Format(_T("%d"), InfoBuffer[dwIndex].uNum);
		csSelector.Format(_T("%d"), InfoBuffer[dwIndex].uSelector);
		csType.Format(_T("%d"), InfoBuffer[dwIndex].uType);
		csLevel.Format(_T("%d"), InfoBuffer[dwIndex].uLevel);

		// 添加行
		m_IDT_List.InsertItem(dwIndex, L"");
		m_IDT_List.UpdateWindow();

		// 插入数据
		m_IDT_List.SetItemText(dwIndex, 0, csAddrOffset);
		m_IDT_List.SetItemText(dwIndex, 1, csNum);
		m_IDT_List.SetItemText(dwIndex, 2, csSelector);
		m_IDT_List.SetItemText(dwIndex, 3, csType);
		m_IDT_List.SetItemText(dwIndex, 4, csLevel);

		
		dwIndex++;
	}

	CloseHandle(hDev);
	return VOID();
}

afx_msg LRESULT ARK_IDT::OnUmIdt(WPARAM wParam, LPARAM lParam)
{
	Kernel_EnumIDT();
	return 0;
}
