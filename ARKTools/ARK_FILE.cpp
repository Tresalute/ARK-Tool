// ARK_FILE.cpp: 实现文件
//

#include "pch.h"
#include "ARKTools.h"
#include "ARK_FILE.h"
#include "afxdialogex.h"

/*自定义头*/
#include "Sources.h"

// ARK_FILE 对话框

IMPLEMENT_DYNAMIC(ARK_FILE, CDialogEx)

ARK_FILE::ARK_FILE(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EnumFile, pParent)
{

}

ARK_FILE::~ARK_FILE()
{
}

void ARK_FILE::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_TREE, m_File_Tree);
	DDX_Control(pDX, IDC_File_LIST, m_File_List);
}

BOOL ARK_FILE::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//InitialTreeCtr();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BEGIN_MESSAGE_MAP(ARK_FILE, CDialogEx)
END_MESSAGE_MAP()


// ARK_FILE 消息处理程序

// 沟通内核 遍历文件
VOID ARK_FILE::Kernel_EnumFILE()
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
	DeviceIoControl(hDev, CTL_EnumFile,
		BufferToKernel, dwSizeOfPutBuffer,
		BufferToKernel, dwSizeOfPutBuffer,
		&dwRetSize, NULL);

	/* 添加至 列表控件中*/

	CloseHandle(hDev);
	return VOID();
}

// 树控件初始化
VOID ARK_FILE::InitialTreeCtr()
{
	CString filePath = L".\\";
	TCHAR buffer[128] = { 0 };
	GetLogicalDriveStrings(128, buffer);
	TCHAR* p = buffer;

	while (*p != NULL)
	{
		TCHAR volName[64];
		TCHAR fileSystemName[64];
		GetVolumeInformation(
			p, /*根文件夹名,一般是一个盘符,例如:C:\*/
			volName, sizeof(volName),
			NULL,
			NULL,
			NULL,
			fileSystemName, /*文件系统名*/
			sizeof(fileSystemName));
		CString szBuffer;
		szBuffer.Format(L"%s[%s]%s", volName, p, fileSystemName);
		// 插入到树控件
		HTREEITEM hItem = m_File_Tree.InsertItem(p);
		wchar_t* pBuff = _wcsdup(p);
		m_File_Tree.SetItemData(hItem, (DWORD_PTR)pBuff);
		p += 4;
	}
	return VOID();
}


