// ARK_REG.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_REG.h"
#include "afxdialogex.h"

/*自定义头*/
#include "Sources.h"

// ARK_REG 对话框

IMPLEMENT_DYNAMIC(ARK_REG, CDialogEx)

ARK_REG::ARK_REG(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumReg, pParent)
{

}

ARK_REG::~ARK_REG()
{
}

void ARK_REG::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REG_TREE, m_REG_Tree);
	DDX_Control(pDX, IDC_REG_LIST, m_REG_List);
}


BEGIN_MESSAGE_MAP(ARK_REG, CDialogEx)
END_MESSAGE_MAP()


BOOL ARK_REG::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// 初始化树控件
		// 对树控件进行操作，参数二不写就是添加到最外层
	HTREEITEM RootNode = m_REG_Tree.InsertItem(L"User");
	m_REG_Tree.InsertItem(L"HKEY_CLASSES_ROOT", RootNode);
	m_REG_Tree.InsertItem(L"HKEY_CURRENT_USER", RootNode);
	m_REG_Tree.InsertItem(L"HKEY_LOCAL_MACHINE", RootNode);
	m_REG_Tree.InsertItem(L"HKEY_USERS", RootNode);
	m_REG_Tree.InsertItem(L"HKEY_CURRENT_CONFIG", RootNode);
	HTREEITEM SubNode3 = m_REG_Tree.InsertItem(L"子节点三", RootNode);
	m_REG_Tree.InsertItem(L"孙节点一", SubNode3);

	HKEY_LOCAL_MACHINE;
	// 初始化列表控件

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


// ARK_REG 消息处理程序



/*==================================用户自定义处理程序==================================*/
// 沟通内核 遍历注册表
VOID ARK_REG::Kernel_EnumReg()
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

	CHAR BufferToUser[] = "这是来自3环的测试字符串_CTL_Test";
	DWORD dwSizeOfGetBuffer = sizeof(BufferToUser);
	CHAR BufferToKernel[0x100] = { 0 };		/*输出缓冲区*/
	DWORD dwSizeOfPutBuffer = 0x100;	/*输出缓冲区的字节数*/

	DWORD dwRetSize;
	DeviceIoControl(hDev, CTL_EnumReg,
		BufferToKernel, dwSizeOfPutBuffer,
		BufferToKernel, dwSizeOfPutBuffer,
		&dwRetSize, NULL);

	/* 添加至 列表控件中*/

	CloseHandle(hDev);
	return VOID();
}


