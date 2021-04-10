#pragma once


// ARK_SSDT 对话框

class ARK_SSDT : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_SSDT)

public:
	ARK_SSDT(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_SSDT();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumSSDT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	// 沟通内核 遍历SSDT
	VOID Kernel_EnumSSDT();

public:
	CListCtrl m_SSDT_List;

public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg LRESULT OnUmSsdt(WPARAM wParam, LPARAM lParam);
};
