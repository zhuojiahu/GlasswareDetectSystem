#pragma once
#include "stdafx.h"

extern "C"
{
	//////////////////////����ͨ�ź���
    _declspec(dllexport) unsigned int LED_DIST_Connect(char* address, int port);//������
    _declspec(dllexport) unsigned int LED_DIST_Disconnect(unsigned int socket);//�ر�����

    //////////////////////�������ú���
    _declspec(dllexport) int LED_DIST_New_SetArea(unsigned int socket, int style, int area, int value);//���õ�����
    _declspec(dllexport) int LED_DIST_New_SetAll(unsigned int socket, int style, int arr[60]);//������������
    _declspec(dllexport) int LED_DIST_New_SelectStyle(unsigned int socket, int style);//����������ʽ��1/2��
    _declspec(dllexport) int LED_DIST_New_SaveAll(unsigned int socket);//���ñ�������
    _declspec(dllexport) int LED_DIST_New_SetLightTime(unsigned int socket, int time);//���õ���ʱ��us
    
    //////////////////////�������ú���
    _declspec(dllexport) int LED_DIST_New_SoftTri(unsigned int socket);//����
    _declspec(dllexport) int LED_DIST_New_SetTriTime(unsigned int socket, int time);//�����ڴ�������
    _declspec(dllexport) int LED_DIST_New_SoftTriStart(unsigned int socket);//��ʼ�ڴ���
    _declspec(dllexport) int LED_DIST_New_SoftTriStop(unsigned int socket);//ֹͣ�ڴ���
    
    //////////////////////��ȡ����
    _declspec(dllexport) int LED_DIST_New_ReadInfo(unsigned int socket, char* arr);//��ȡ��Դ״̬
    _declspec(dllexport) int LED_DIST_New_ReadAll(unsigned int socket, int style, char* arr);//��ȡ��������
    _declspec(dllexport) int LED_DIST_New_ReadVersion(unsigned int socket, char* arr);//��ȡ��Դ�汾��
}
