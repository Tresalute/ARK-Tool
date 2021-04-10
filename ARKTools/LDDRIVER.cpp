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
BOOL LDDRIVER::LoadDriver(CHAR * DriverName,CHAR* szFilePath)
{
	SC_HANDLE hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hServiceMgr)
	{
		OutputDebugStringW(_T("�򿪷��������ʧ��,�����Ƿ��Թ���ԱȨ������"));
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}
	OutputDebugStringW(_T("Loading----"));
	SC_HANDLE m_hServiceDDK = CreateServiceA(
		hServiceMgr,//SMC���
		DriverName,//������������(�����������ע����е�����)
		DriverName,//����������ʾ����(ע������������DisplayNameֵ)
		SERVICE_ALL_ACCESS,//Ȩ��(���з���Ȩ��)
		SERVICE_KERNEL_DRIVER,//��������(��������)
		SERVICE_DEMAND_START,//������ʽ(��Ҫʱ����,ע������������Startֵ)
		SERVICE_ERROR_IGNORE,//�������(����,ע������������ErrorControlֵ)
		szFilePath,//����Ķ������ļ�·��(���������ļ�·��, ע������������ImagePathֵ)
		NULL,//����������
		NULL,//TagId
		NULL,//�����ϵ
		NULL,//����������
		NULL);//����
	// ������Ϣ
	{
		OutputDebugStringA(DriverName);
		OutputDebugStringA(szFilePath);
	}
	if (!m_hServiceDDK)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			OutputDebugStringW(_T("�����Ѿ�����"));
			if (!m_hServiceDDK)m_hServiceDDK = OpenServiceA(hServiceMgr, DriverName, SERVICE_ALL_ACCESS);
		}
		else {
			OutputDebugStringW(_T("Error while Install ,error code:" + GetLastError()));
		}

	}
	else {
		OutputDebugStringW(_T("������װ�ɹ�!"));
		OpenServiceA(hServiceMgr, DriverName, SERVICE_ALL_ACCESS);
	}
	if (!StartService(m_hServiceDDK, NULL, NULL))
	{
		DWORD ErrorCode = GetLastError();
		switch (ErrorCode)
		{
		case ERROR_SERVICE_ALREADY_RUNNING:
			OutputDebugStringW(_T("�����Ѿ�����"));
			break;
		case ERROR_SERVICE_NOT_FOUND:
			OutputDebugStringW(_T("����δ�ҵ�"));
			break;
		default:
			CString msg("����ʧ�ܣ������� %d " + ErrorCode);
			OutputDebugStringW(msg);
			break;
		}
		OutputDebugStringW(_T("����ʧ��"));
	}
	else {
		OutputDebugStringW(_T("�����ɹ�"));
	}
	CloseServiceHandle(hServiceMgr);
	CloseServiceHandle(m_hServiceDDK);
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

	OutputDebugStringW(L"ж�سɹ�");

	CloseServiceHandle(hServiceDDK);
	CloseServiceHandle(hServiceMgr);
	return TRUE;
}
