#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <char.h>

struct RequestHeader
{
    char strMethod[10];     /* ���󷽷� */
	char strURL[256];       /* ������Դ���� */
	char strVersion[15];    /* http �汾�� */
    char strContType[256];  /* ����� Content-Type */
    char strGetArgs[256];   /* ����� get ��ʽ��������������г������� */
	char *strPostArgs;      /* ����� post ��ʽ����������� */
    int iContLen;           /* �����峤�� */
};

int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader);
int PutResponseHeader(int iSockFd, struct RequestHeader *ptReqHeader);
void NoSuchFile(int iSockFd);
void bad_request(int iSockFd);
void ErrorExec(int iSockFd);
void HtmlHeader(int iSockFd);
void CgiHeader(int iSockFd);
void PlainHeader(int iSockFd);

#endif    /* response.h */

