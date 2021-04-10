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
BOOL LDDRIVER::LoadDriver(CHAR * SysName,CHAR* SysPath)
{
	if (SysName == nullptr || SysPath == nullptr)
	{
			OutputDebugStringW(L"空指针");
			return FALSE;
	}


	// 打开SCM管理器
	SC_HANDLE hServiceMgr = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_打开SCM管理器失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	{		
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString()); 
	}
	__asm int 3;
	// 创建一个服务
	SC_HANDLE hServiceDDK = CreateServiceA(
		hServiceMgr, 
		SysName,				// 注册表中的名字
		SysName,				// 注册表驱动程序的DisplayName值
		SERVICE_ALL_ACCESS,		// 权限(所有访问权限)
		SERVICE_KERNEL_DRIVER,	// 服务类型(驱动程序)
		SERVICE_DEMAND_START,	// 启动方式(需要时启动,注册表驱动程序的Start值)
		SERVICE_ERROR_IGNORE,	// 错误控制(忽略,注册表驱动程序的ErrorControl值)
		SysPath, NULL, NULL, NULL, NULL, NULL);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_创建服务失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// 打开服务
	hServiceDDK = OpenServiceA(hServiceMgr, SysName, SERVICE_START);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_打开服务失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// 启动服务
	BOOL bRes = StartServiceA(hServiceDDK, NULL, NULL);
	if (!bRes)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_启动服务失败 错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("错误码：%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// 清理句柄
	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
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

	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
	return TRUE;
}
