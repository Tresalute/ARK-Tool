#pragma once


// ARK_FILE 对话框

class ARK_FILE : public CDialogEx
{
	DECLARE_DYNAMIC(ARK_FILE)

public:
	ARK_FILE(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ARK_FILE();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EnumFile };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	// 自定义函数
public:
	// 沟通内核 遍历文件
	VOID Kernel_EnumFILE();

	// 树控件初始化
	VOID InitialTreeCtr();


	// 系统生成函数
public:
	virtual BOOL OnInitDialog();

public:
	CTreeCtrl m_File_Tree;
	CListCtrl m_File_List;

};
