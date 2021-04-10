// ARK_GDT.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_GDT.h"
#include "afxdialogex.h"

/*自定义头*/
#include "Sources.h"

/*自定义消息*/
#define UM_GDT		WM_USER+600

/*自定义结构体*/
//typedef struct _GDTINFO
//{
//	ULONG uSelector;		// 段选择子
//	ULONG uBase;			// 基址
//	ULONG uLimit;			// 界限
//	ULONG uPG;				// 粒度
//	ULONG uDPL;				// 段特权级
//	ULONG uType;			// 类型
//}GDTInfo,*pGDTInfo;
/*段描述符结构体*/
typedef struct _GDT_SE
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
}GDT_SE, *PGDT_SE;
/*门描述结构体*/
typedef struct _GDT_DOR
{
	UINT64 Offset0_15 : 16;
	UINT64 Selector : 16;
	UINT64 ParCoun : 5;
	UINT64 Unkown : 3;
	UINT64 Type : 4;
	UINT64 S : 1;
	UINT64 DPL : 2;
	UINT64 P : 1;
	UINT64 Offset16_31 : 16;
}GDT_DOR,*PGDT_DOR;
// ARK_GDT 对话框

IMPLEMENT_DYNAMIC(ARK_GDT, CDialogEx)

ARK_GDT::ARK_GDT(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumGDT, pParent)
{

}

ARK_GDT::~ARK_GDT()
{
}

void ARK_GDT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GDT_LIST, m_GDT_List);
}


BEGIN_MESSAGE_MAP(ARK_GDT, CDialogEx)
	ON_MESSAGE(UM_GDT, &ARK_GDT::OnUmGdt)
END_MESSAGE_MAP()


// ARK_GDT 消息处理程序


BOOL ARK_GDT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// ListInitial
	m_GDT_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | WS_VSCROLL);
	m_GDT_List.EnsureVisible(m_GDT_List.GetItemCount() - 1, FALSE);

	const WCHAR* wTtile[] = {
			L"所属段",
			L"基址/偏移",
			L"界限/选择子",
			L"P",
			L"段特权级",
			L"类型",
			L"段粒度",
	};
	m_GDT_List.InsertColumn(0, wTtile[0], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(1, wTtile[1], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(2, wTtile[2], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(3, wTtile[3], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(4, wTtile[4], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(5, wTtile[5], LVCFMT_CENTER, 100);
	m_GDT_List.InsertColumn(6, wTtile[6], LVCFMT_CENTER, 100);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// 消息调用
afx_msg LRESULT ARK_GDT::OnUmGdt(WPARAM wParam, LPARAM lParam)
{
	Kernel_EnumGDT();
	return 0;
}

// 沟通内核 遍历GDT
VOID ARK_GDT::Kernel_EnumGDT()
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
	DeviceIoControl(hDev, CTL_EnumGDT,
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
	DeviceIoControl(hDev, CTL_EnumGDT,
		BuufferToUser_Sec, sizeof(buffer_Get),
		BuufferKernel_Sec, dwSizeOfPutBuffer_Sec,
		&dwRetSize_Sec, NULL);

	//=================================添加至列表控件中=================================//


	m_GDT_List.DeleteAllItems();

	UINT64* InfoBuffer = (UINT64*)((char*)BuufferKernel_Sec + 8);
	INT dwIndex = 0;
	while (dwIndex<100)
	{
		// 如果是 系统段->> 调用 DOR_GDTInsert
		if (((GDT_DOR*)InfoBuffer)[dwIndex].S == 0)
			DOR_GDTInsert(dwIndex, InfoBuffer[dwIndex]);
		// 如果是 代码或数据段->> 调用 SE_GDTInsert
		else SE_GDTInsert(dwIndex, InfoBuffer[dwIndex]);

		++dwIndex;
	}

	CloseHandle(hDev);
	return VOID();
}

VOID ARK_GDT::SE_GDTInsert(INT index, UINT64 _GDTab)
{
	GDT_SE GDTab = *(GDT_SE*)(&_GDTab);

	CString SE_Base;
	CString SE_Limit;
	CString SE_P;
	CString SE_DPL;
	CString SE_Type;
	CString SE_G;

	ULONG GDT_Base = MAKE_ULONG(GDTab.Base0_23, GDTab.Base24_31);
	SE_Base.Format(_T("0x%08X"), GDT_Base);

	ULONG GDT_limit = MAKE_LONG(GDTab.Limit0_15, GDTab.Limit16_19);
	SE_Limit.Format(_T("0x%08X"), GDT_limit);

	SE_P.Format(_T("%d"), GDTab.P);
	if (GDTab.G == 0)
		SE_G.Format(_T("Byte"));
	else SE_G.Format(_T("Page"));
	
	SE_DPL.Format(_T("%d"), GDTab.DPL);
	SE_Type.Format(_T("0x%X"), GDTab.Type);

	// 添加行
	m_GDT_List.InsertItem(index, L"");
	m_GDT_List.UpdateWindow();

	// 插入数据
	//m_GDT_List.SetItemText(dwIndex, 0, uSelector);
	
	if (GDTab.Type>>3 == 0)
		m_GDT_List.SetItemText(index, 0, L"数据段");
	else if(GDTab.Type >> 3 == 1)
		m_GDT_List.SetItemText(index, 0, L"代码段");
	else m_GDT_List.SetItemText(index, 0, L"Error");
	m_GDT_List.SetItemText(index, 1, SE_Base);
	m_GDT_List.SetItemText(index, 2, SE_Limit);
	m_GDT_List.SetItemText(index, 3, SE_P);
	m_GDT_List.SetItemText(index, 4, SE_DPL);
	m_GDT_List.SetItemText(index, 5, SE_Type);
	m_GDT_List.SetItemText(index, 6, SE_G);


	return VOID();
}

VOID ARK_GDT::DOR_GDTInsert(INT index, UINT64 _GDTab)
{
	GDT_DOR GDTab = *(GDT_DOR*)(&_GDTab);

	CString Dor_Offset;
	CString Dor_Selector;
	CString Dor_P;
	CString Dor_DPL;
	CString Dor_Type;


	ULONG GDT_Offset = MAKE_LONG(GDTab.Offset0_15, GDTab.Offset16_31);
	Dor_Offset.Format(_T("0x%08X"), GDT_Offset);

	Dor_Selector.Format(_T("0x%X"), GDTab.Selector);

	Dor_DPL.Format(_T("%d"), GDTab.DPL);

	Dor_Type.Format(_T("0x%X"), GDTab.Type);

	Dor_P.Format(_T("%d"), GDTab.P);


	// 添加行
	m_GDT_List.InsertItem(index, L"");
	m_GDT_List.UpdateWindow();

	m_GDT_List.SetItemText(index, 0, L"系统段");
	m_GDT_List.SetItemText(index, 1, Dor_Offset);
	m_GDT_List.SetItemText(index, 2, Dor_Selector);
	m_GDT_List.SetItemText(index, 3, Dor_P);
	m_GDT_List.SetItemText(index, 4, Dor_DPL);
	m_GDT_List.SetItemText(index, 5, Dor_Type);
	m_GDT_List.SetItemText(index, 6, L"null");

	return VOID();
}





