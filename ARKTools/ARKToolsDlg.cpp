
// ARKToolsDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ARKTools.h"
#include "ARKToolsDlg.h"
#include "afxdialogex.h"

// 自实现接口
#include "LDDRIVER.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*=================================自定消息=================================*/
#define UM_DRIVER	WM_USER+100
#define UM_PEM		WM_USER+200
#define UM_FILE		WM_USER+300
#define UM_REG		WM_USER+400
#define UM_IDT		WM_USER+500
#define UM_GDT		WM_USER+600
#define UM_SSDT		WM_USER+700

INT g_msg[] =
{
	UM_DRIVER,
	UM_PEM,
	UM_FILE,
	UM_REG,
	UM_IDT,
	UM_GDT,
	UM_SSDT
};

uDrive g_uDriveInfo[] = {
//{"EnumProcAndModule.sys","C:\\Users\\15pb-win7\\Desktop\\EnumProcAndModule.sys"},
{"ARKDriverFile.sys","C:\\Users\\15pb-win7\\Desktop\\ARKDriverFile.sys"},
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CARKToolsDlg 对话框



CARKToolsDlg::CARKToolsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ARKTOOLS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CARKToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Bady_Tab, m_BadyTab);
}

BEGIN_MESSAGE_MAP(CARKToolsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_Bady_Tab, &CARKToolsDlg::OnTcnSelchangeBadyTab)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CARKToolsDlg 消息处理程序

BOOL CARKToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	
	// 初始化驱动 
	// InitialDriver();
	// 初始化控件
	m_BadyTab.InsertItem(0, L"驱动");
	m_BadyTab.InsertItem(1, L"进程");
	m_BadyTab.InsertItem(2, L"文件");
	m_BadyTab.InsertItem(3, L"注册表");
	m_BadyTab.InsertItem(4, L"IDT");
	m_BadyTab.InsertItem(5, L"GDT");
	m_BadyTab.InsertItem(6, L"SSDT");

	// 绑定子窗口
	TabWnd[0] = new ARK_DRIVER;
	TabWnd[1] = new ARK_PEM; 
	TabWnd[2] = new ARK_FILE; 
	TabWnd[3] = new ARK_REG;
	TabWnd[4] = new ARK_IDT;
	TabWnd[5] = new ARK_GDT; 
	TabWnd[6] = new ARK_SSDT;

	TabWnd[0]->Create(IDD_EnumDrive,&m_BadyTab);
	TabWnd[1]->Create(IDD_EnumPEM  ,&m_BadyTab);
	TabWnd[2]->Create(IDD_EnumFile ,&m_BadyTab);
	TabWnd[3]->Create(IDD_EnumReg  ,&m_BadyTab);
	TabWnd[4]->Create(IDD_EnumIDT  ,&m_BadyTab);
	TabWnd[5]->Create(IDD_EnumGDT  ,&m_BadyTab);
	TabWnd[6]->Create(IDD_EnumSSDT ,&m_BadyTab);
	
	// 设置窗口大小
	// 以选项卡为准，重新设置窗口的位置
	CRect Rect = { 0 };
	m_BadyTab.GetClientRect(&Rect);
	Rect.DeflateRect(8, 33, 10, 10);
	for (int i = 0; i < 7; ++i)
		TabWnd[i]->MoveWindow(&Rect);


	ShowTabWnd(0);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CARKToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CARKToolsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CARKToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 初始化驱动
BOOL CARKToolsDlg::InitialDriver()
{
	// 驱动 加载/检查/卸载 操作接口
	LDDRIVER LdDriver;
	BOOL bRel_LdD;
	// 检查驱动是否被加载
	INT NumOfDrive = _countof(g_uDriveInfo);

	for (INT i = 0; i< NumOfDrive ; ++i)
	{
		// 加载驱动
		bRel_LdD = LdDriver.LoadDriver(g_uDriveInfo[i].SysName, g_uDriveInfo[i].SysPath);
		if (!bRel_LdD)
		{
			MessageBoxA(NULL, "驱动加载失败\r\n程序即将退出！", "错误", MB_OK);
			ExitProcess(0);
		}

		// 输出驱动加载调试信息
		{
			CString DriverDebugInfo(g_uDriveInfo[i].SysName);
			DriverDebugInfo.Append(L"——驱动加载成功！\n");
			OutputDebugStringW(DriverDebugInfo.GetString());
		}
	}	

	// 输出调试信息
	{
		DWORD ErrorCode;
		CString csErroCode;
		ErrorCode = GetLastError();
		csErroCode.Format(_T("驱动加载数量：%d\n"), NumOfDrive);
		OutputDebugStringW(csErroCode.GetString());	
	}

	return 0;
}

// 扫尾工作
BOOL CARKToolsDlg::ClearEnvironment()
{
	// 卸载驱动
	// 驱动 加载/检查/卸载 操作接口
	LDDRIVER LdDriver;
	BOOL bRel_LdD;
	// 检查驱动是否被加载
	INT NumOfDrive = _countof(g_uDriveInfo);
	for (INT i = 0; i < NumOfDrive; ++i)
	{
		bRel_LdD = LdDriver.UnLoadDriver(g_uDriveInfo[i].SysName);
	}

	return 0;
}

// 显示Table
VOID CARKToolsDlg::ShowTabWnd(int index)
{
	for (int i = 0; i < 7; ++i)
	{
		TabWnd[i]->ShowWindow(i == index ? SW_SHOWNORMAL : SW_HIDE);
	}
	return VOID();
}

// 当选项卡的选中项被改变时响应
void CARKToolsDlg::OnTcnSelchangeBadyTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	DWORD dwIndex = m_BadyTab.GetCurSel();
	ShowTabWnd(dwIndex);
	// 根据不同的选择向不同的窗口发送消息；
 	SendMessageA(TabWnd[dwIndex]->m_hWnd, g_msg[dwIndex], NULL, NULL);
	return;
}
