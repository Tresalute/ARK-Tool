#include "pch.h"
#include "LDDRIVER.h"

#include <Winsvc.h>

//************************************
// 方法:   CheckDriverIsLoad
// 全名:   LDDRIVER::CheckDriverIsLoad
// 访问:   protected 
// 返回:   BOOL
// 作用：  检查驱动是否加载
// 限定符:
// 参数:   CHAR * SysName
//************************************
BOOL LDDRIVER::CheckDriverIsLoad(CHAR* SysName)
{
	HANDLE hFile;
	hFile = CreateFileA(SysName, GENERIC_READ | GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode;
		CString csErroCode;
		ErrorCode = GetLastError();
		csErroCode.Format(_T("驱动未加载"));
		OutputDebugStringW(csErroCode.GetString());
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}

//************************************
// 方法:   LoadDriver
// 全名:   LDDRIVER::LoadDriver
// 访问:   protected 
// 返回:   BOOL
// 作用： 加载驱动 
// 限定符:
// 参数:   CHAR * SysName
//************************************
BOOL LDDRIVER::LoadDriver(CHAR * DriverName,CHAR* szFilePath)
{
	SC_HANDLE hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hServiceMgr)
	{
		OutputDebugStringW(_T("打开服务控制器失败,请检查是否以管理员权限运行"));
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}
	OutputDebugStringW(_T("Loading----"));
	SC_HANDLE m_hServiceDDK = CreateServiceA(
		hServiceMgr,//SMC句柄
		DriverName,//驱动服务名称(驱动程序的在注册表中的名字)
		DriverName,//驱动服务显示名称(注册表驱动程序的DisplayName值)
		SERVICE_ALL_ACCESS,//权限(所有访问权限)
		SERVICE_KERNEL_DRIVER,//服务类型(驱动程序)
		SERVICE_DEMAND_START,//启动方式(需要时启动,注册表驱动程序的Start值)
		SERVICE_ERROR_IGNORE,//错误控制(忽略,注册表驱动程序的ErrorControl值)
		szFilePath,//服务的二进制文件路径(驱动程序文件路径, 注册表驱动程序的ImagePath值)
		NULL,//加载组命令
		NULL,//TagId
		NULL,//依存关系
		NULL,//服务启动名
		NULL);//密码
	// 调试信息
	{
		OutputDebugStringA(DriverName);
		OutputDebugStringA(szFilePath);
	}
	if (!m_hServiceDDK)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			OutputDebugStringW(_T("驱动已经存在"));
			if (!m_hServiceDDK)m_hServiceDDK = OpenServiceA(hServiceMgr, DriverName, SERVICE_ALL_ACCESS);
		}
		else {
			OutputDebugStringW(_T("Error while Install ,error code:" + GetLastError()));
		}

	}
	else {
		OutputDebugStringW(_T("驱动安装成功!"));
		OpenServiceA(hServiceMgr, DriverName, SERVICE_ALL_ACCESS);
	}
	if (!StartService(m_hServiceDDK, NULL, NULL))
	{
		DWORD ErrorCode = GetLastError();
		switch (ErrorCode)
		{
		case ERROR_SERVICE_ALREADY_RUNNING:
			OutputDebugStringW(_T("驱动已经运行"));
			break;
		case ERROR_SERVICE_NOT_FOUND:
			OutputDebugStringW(_T("驱动未找到"));
			break;
		default:
			CString msg("启动失败，错误码 %d " + ErrorCode);
			OutputDebugStringW(msg);
			break;
		}
		OutputDebugStringW(_T("启动失败"));
	}
	else {
		OutputDebugStringW(_T("启动成功"));
	}
	CloseServiceHandle(hServiceMgr);
	CloseServiceHandle(m_hServiceDDK);
	return TRUE;
}

//************************************
// 方法:   UnLoadDriver
// 全名:   LDDRIVER::UnLoadDriver
// 访问:   protected 
// 返回:   BOOL
// 作用：  停止并卸载驱动
// 限定符:
// 参数:   CHAR * SysName
//************************************
BOOL LDDRIVER::UnLoadDriver(CHAR * SysName)
{
	// 打开SCM管理器
	SC_HANDLE hServiceMgr = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_打开SCM管理器失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}
	// 打开一个服务
	SC_HANDLE hServiceDDK = OpenServiceA(hServiceMgr, SysName, SERVICE_STOP | DELETE);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_打开服务失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	// 停止驱动
	SERVICE_STATUS svcsta = { 0 };
	BOOL bRet_Stop = ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &svcsta);
	if (!bRet_Stop)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_停止驱动失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	// 卸载驱动
	BOOL bRet_Over =DeleteService(hServiceDDK);
	if (!bRet_Over)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_卸载驱动失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	OutputDebugStringW(L"卸载成功");

	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
	return TRUE;
}
