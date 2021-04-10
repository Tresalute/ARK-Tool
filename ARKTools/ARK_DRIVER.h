#pragma once


// ARK_DRIVER 对话框

class ARK_DRIVER : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_DRIVER)

public:
	ARK_DRIVER(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_DRIVER();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumDrive };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

public:
	// 沟通内核 遍历驱动
	VOID Kernel_EnumDriver();
	// 隐藏驱动
	VOID HideDriver();
public:
	// 驱动信息显示列表
	CListCtrl m_Dr_List;
	// 菜单
	CMenu m_Menu;
protected:
	afx_msg LRESULT OnUmDriver(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnHidedriver();
	afx_msg void OnNMRClickDriverList(NMHDR *pNMHDR, LRESULT *pResult);
};
