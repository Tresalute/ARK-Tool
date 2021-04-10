
// ARKToolsDlg.h: 头文件
//

#pragma once
#include "Sources.h"
#include "ARK_DRIVER.h"
#include "ARK_FILE.h"
#include "ARK_GDT.h"
#include "ARK_IDT.h"
#include "ARK_PEM.h"
#include "ARK_REG.h"
#include "ARK_SSDT.h"

// CARKToolsDlg 对话框
class CARKToolsDlg : public CDialogEx
{
// 构造
public:
	CARKToolsDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ARKTOOLS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// 函数声明
public:
	// 初始化驱动
	BOOL InitialDriver();

	// 扫尾工作
	BOOL ClearEnvironment();

	// 工具函数
public:

	// 显示Table
	VOID ShowTabWnd(int index);

	// 共有数据遍历
public:
	//主Tab控件
	CTabCtrl m_BadyTab;


	// 私有数据变量
private:
	// 驱动窗口
	ARK_DRIVER* Ark_Driver = nullptr;
	// 文件窗口
	ARK_FILE* Ark_File = nullptr;
	// GDT窗口
	ARK_GDT* Ark_GDT = nullptr;
	// IDT窗口
	ARK_IDT* Ark_IDT = nullptr;
	// 线程，进程，模块窗口
	ARK_PEM* Ark_PEM = nullptr;
	// 注册表窗口
	ARK_REG* Ark_Reg = nullptr;
	// SSDT窗口
	ARK_SSDT* Ark_SSDT = nullptr;

	// 保存窗口的数组
	CDialogEx* TabWnd[7] = { 0 };


public:
	
	afx_msg void OnTcnSelchangeBadyTab(NMHDR *pNMHDR, LRESULT *pResult);
};
