#pragma once
#include <Windows.h>

class LDDRIVER
{
	// �ṩ�ⲿ�ӿ�
public:
	// �������
	BOOL CheckDriverIsLoad(CHAR* SysName);
	// ��������
	BOOL LoadDriver(CHAR* SysName,CHAR* SysPath);
	// ����ж��
	BOOL UnLoadDriver(CHAR* SysName);
};

