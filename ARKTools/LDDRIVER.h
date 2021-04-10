#pragma once
#include <Windows.h>

class LDDRIVER
{
	// 提供外部接口
public:
	// 驱动检查
	BOOL CheckDriverIsLoad(CHAR* SysName);
	// 驱动加载
	BOOL LoadDriver(CHAR* SysName,CHAR* SysPath);
	// 驱动卸载
	BOOL UnLoadDriver(CHAR* SysName);
};

