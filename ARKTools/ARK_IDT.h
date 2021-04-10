#pragma once


// ARK_IDT 对话框

class ARK_IDT : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_IDT)

public:
	ARK_IDT(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_IDT();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumIDT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()


public:
	// 沟通内核 遍历IDT
	VOID Kernel_EnumIDT();

public:
	CListCtrl m_IDT_List;

public:
	virtual BOOL OnInitDialog();

protected:
	afx_msg LRESULT OnUmIdt(WPARAM wParam, LPARAM lParam);
};
