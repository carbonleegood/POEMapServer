// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

#ifdef NDEBUG
#define ENCRYPT_VMP
#endif
// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
//�����������Ͷ���
#define RT_NO_RESULT  0
#define RT_SUCCESS    1
#define RT_EXIT       2//�˳�
#define RT_INIT_NOT_COMPLETE  3//��ʼ��û�гɹ�
#define RT_DISCONNECT          4//������
#define RT_WRITE_DATA_FAIL     5 //д������ʧ��
#define RT_INJECTED            6//����ע���
#define RT_UNINJECTED          7//����ûע���