#include "pch.h"
#include "LDDRIVER.h"

#include <Winsvc.h>

//************************************
// ����:   CheckDriverIsLoad
// ȫ��:   LDDRIVER::CheckDriverIsLoad
// ����:   protected 
// ����:   BOOL
// ���ã�  ��������Ƿ����
// �޶���:
// ����:   CHAR * SysName
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
		csErroCode.Format(_T("����δ����"));
		OutputDebugStringW(csErroCode.GetString());
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}

//************************************
// ����:   LoadDriver
// ȫ��:   LDDRIVER::LoadDriver
// ����:   protected 
// ����:   BOOL
// ���ã� �������� 
// �޶���:
// ����:   CHAR * SysName
//************************************
BOOL LDDRIVER::LoadDriver(CHAR * SysName,CHAR* SysPath)
{
	if (SysName == nullptr || SysPath == nullptr)
	{
			OutputDebugStringW(L"��ָ��");
			return FALSE;
	}


	// ��SCM������
	SC_HANDLE hServiceMgr = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_��SCM������ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	{		
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("�����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString()); 
	}
	__asm int 3;
	// ����һ������
	SC_HANDLE hServiceDDK = CreateServiceA(
		hServiceMgr, 
		SysName,				// ע����е�����
		SysName,				// ע������������DisplayNameֵ
		SERVICE_ALL_ACCESS,		// Ȩ��(���з���Ȩ��)
		SERVICE_KERNEL_DRIVER,	// ��������(��������)
		SERVICE_DEMAND_START,	// ������ʽ(��Ҫʱ����,ע������������Startֵ)
		SERVICE_ERROR_IGNORE,	// �������(����,ע������������ErrorControlֵ)
		SysPath, NULL, NULL, NULL, NULL, NULL);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_��������ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("�����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// �򿪷���
	hServiceDDK = OpenServiceA(hServiceMgr, SysName, SERVICE_START);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_�򿪷���ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("�����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// ��������
	BOOL bRes = StartServiceA(hServiceDDK, NULL, NULL);
	if (!bRes)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("Load_��������ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("�����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
	}
	__asm int 3;

	// ������
	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
	return TRUE;
}

//************************************
// ����:   UnLoadDriver
// ȫ��:   LDDRIVER::UnLoadDriver
// ����:   protected 
// ����:   BOOL
// ���ã�  ֹͣ��ж������
// �޶���:
// ����:   CHAR * SysName
//************************************
BOOL LDDRIVER::UnLoadDriver(CHAR * SysName)
{
	// ��SCM������
	SC_HANDLE hServiceMgr = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_��SCM������ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}
	// ��һ������
	SC_HANDLE hServiceDDK = OpenServiceA(hServiceMgr, SysName, SERVICE_STOP | DELETE);
	if (hServiceDDK == NULL)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_�򿪷���ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	// ֹͣ����
	SERVICE_STATUS svcsta = { 0 };
	BOOL bRet_Stop = ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &svcsta);
	if (!bRet_Stop)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_ֹͣ����ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	// ж������
	BOOL bRet_Over =DeleteService(hServiceDDK);
	if (!bRet_Over)
	{
		DWORD dwErrorCode = GetLastError();
		CString csErrorCode;
		csErrorCode.Format(_T("UnLoad_ж������ʧ�� �����룺%d"), dwErrorCode);
		OutputDebugStringW(csErrorCode.GetString());
		return FALSE;
	}

	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
	return TRUE;
}
