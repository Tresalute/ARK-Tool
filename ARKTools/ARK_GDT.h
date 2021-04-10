#pragma once


// ARK_GDT 对话框

class ARK_GDT : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_GDT)

public:
	ARK_GDT(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_GDT();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumGDT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	// 沟通内核 遍历GDT
	VOID Kernel_EnumGDT();

	// 段描述符支持
	VOID SE_GDTInsert(INT index, UINT64 GDTab);
	// 门描述符支持
	VOID DOR_GDTInsert(INT index, UINT64 GDTab);

public:
	CListCtrl m_GDT_List;

public:
	virtual BOOL OnInitDialog();

protected:
	afx_msg LRESULT OnUmGdt(WPARAM wParam, LPARAM lParam);
};
