#ifndef VIM_H_INCLUDED
#define VIM_H_INCLUDED
#include <cstdlib>
#include <cstdio>
#include <windows.h>
using namespace std;
// Edit by Kelukin to support the file modify operation in file system
void displayVim();
// �Ƿ�Ҫ�����ײ��Ĺ�������ʱ�д�����
// �����ڻ��������е����ݴ�ӡ���ն���Ļ��

void runVim();
//����Vim

void clearScreen();
//�����Ļ

void workChar(char &x);
//����������ַ��Ĳ�ͬ�����в�ͬ�Ĳ���
//��ʱ����Ĺ����ַ��пɼ��ַ�, ESC���˳�����ͨģʽ

void initBuffer();
//�����ļ����������ݼ��ؽ�Buffer����

void saveVim();
//����ǰ�ļ�����Ϣд��ĳ��ϵͳ��

void quitVim();
//�˳���ǰVim
#endif // VIM_H_INCLUDED
