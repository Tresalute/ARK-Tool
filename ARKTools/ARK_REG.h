#pragma once


// ARK_REG 对话框

class ARK_REG : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_REG)

public:
	ARK_REG(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_REG();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumReg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	// 沟通内核 遍历注册表
	VOID Kernel_EnumReg();

public:
	virtual BOOL OnInitDialog();

private:
	// 注册表树控件
	CTreeCtrl m_REG_Tree;
	// 注册表列表控件
	CListCtrl m_REG_List;
};
