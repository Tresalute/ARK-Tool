// ARK_SSDT.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_SSDT.h"
#include "afxdialogex.h"

/*自定义头*/
#include "Sources.h"

/*自定义消息*/
#define UM_SSDT		WM_USER+700

/*自定义结构体*/
typedef struct _SSDTINFO
{
	LONG Num;				// 调用号
	INT Addr;				// 地址
}SSDTINFO, *PSSDTINFO;

// ARK_SSDT 对话框

IMPLEMENT_DYNAMIC(ARK_SSDT, CDialogEx)

ARK_SSDT::ARK_SSDT(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumSSDT, pParent)
{

}

ARK_SSDT::~ARK_SSDT()
{
}

void ARK_SSDT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SSDT_LIST, m_SSDT_List);
}




BEGIN_MESSAGE_MAP(ARK_SSDT, CDialogEx)
	ON_MESSAGE(UM_SSDT, &ARK_SSDT::OnUmSsdt)
END_MESSAGE_MAP()


// ARK_SSDT 消息处理程序


BOOL ARK_SSDT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// ListInitial
	m_SSDT_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | WS_VSCROLL);
	m_SSDT_List.EnsureVisible(m_SSDT_List.GetItemCount() - 1, FALSE);

	const WCHAR* wTtile[] = {
			L"调用号",
			L"地址",
	};
	m_SSDT_List.InsertColumn(0, wTtile[0], LVCFMT_CENTER, 100);
	m_SSDT_List.InsertColumn(1, wTtile[1], LVCFMT_CENTER, 150);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


// 沟通内核 遍历SSDT
VOID ARK_SSDT::Kernel_EnumSSDT()
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
	DeviceIoControl(hDev, CTL_EnumSSDT,
		&BufferToUser_Fir, dwSizeOfGetBuffer_Fir,
		&BufferToKernel_Fir, dwSizeOfPutBuffer_Fir,
		&dwRetSize_Fir, NULL);
	//=================================第二次发送=================================//
		/*用获取的数据大小申请缓冲区，存储数据*/
	DWORD dwSize = BufferToKernel_Fir.bufferSize + 8;

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
	DeviceIoControl(hDev, CTL_EnumSSDT,
		BuufferToUser_Sec, sizeof(buffer_Get),
		BuufferKernel_Sec, dwSizeOfPutBuffer_Sec,
		&dwRetSize_Sec, NULL);

	//=================================添加至列表控件中=================================//
	m_SSDT_List.DeleteAllItems();

	SSDTINFO* InfoBuffer = (SSDTINFO*)((char*)BuufferKernel_Sec + 8);

	INT index = 0;
	while (InfoBuffer[index].Num != -1)
	{
		CString csNum;
		CString csAddr;


		// 格式化
		csNum.Format(_T("%Xh"), InfoBuffer[index].Num);
		csAddr.Format(_T("0x%08X"), InfoBuffer[index].Addr);


		// 添加行
		m_SSDT_List.InsertItem(index, L"");
		m_SSDT_List.UpdateWindow();
		

		// 插入行
		m_SSDT_List.SetItemText(index, 0, csNum);
		m_SSDT_List.SetItemText(index, 1, csAddr);

		++index;
	}

	CloseHandle(hDev);
	return VOID();
}




afx_msg LRESULT ARK_SSDT::OnUmSsdt(WPARAM wParam, LPARAM lParam)
{
	Kernel_EnumSSDT();
	return 0;
}
