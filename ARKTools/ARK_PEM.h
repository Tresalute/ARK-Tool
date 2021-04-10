#pragma once





// ARK_PEM 对话框

class ARK_PEM : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_PEM)

public:
	ARK_PEM(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_PEM();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumPEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	// 沟通内核 遍历进程
	VOID Kernel_EnumPEM();

	// 隐藏进程
	VOID HideProcess();

	// 结束进程
	VOID KillProcess();

	// 保护进程
	VOID ProtectProcess();

public:
	CListCtrl m_PEM_List;
	CMenu m_Menu;
public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg LRESULT OnUmPem(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnHideProcess();
	afx_msg void OnNMRClickPemList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKillprocess();
	afx_msg void OnProtectProcess();
};

/*==========================工具函数==========================*/
INT MyGetLenth(char* buffer);