// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

#ifdef NDEBUG
#define ENCRYPT_VMP
#endif
// TODO:  在此处引用程序需要的其他头文件
//函数返回类型定义
#define RT_NO_RESULT  0
#define RT_SUCCESS    1
#define RT_EXIT       2//退出
#define RT_INIT_NOT_COMPLETE  3//初始化没有成功
#define RT_DISCONNECT          4//无连接
#define RT_WRITE_DATA_FAIL     5 //写入数据失败
#define RT_INJECTED            6//进程注入过
#define RT_UNINJECTED          7//进程没注入过