#ifndef __CHAR_H__
#define __CHAR_H__

struct RequestHeader
{
    char strMethod[10];     /* ���󷽷� */
	char strURL[256];       /* ������Դ���� */
	char strVersion[15];    /* http �汾�� */
    char strGetArgs[256];   /* ����� get ��ʽ����������� */
	char strPostArgs[1024]; /* ����� post ��ʽ����������� */
};

/*
struct ResponseHeader
{
    
};
*/

int GetLineFromSock(int iSockFd, char *pcBuf, int iSize);
int GetNonSpaceBlock(char pcDes[][256], char *pcSrc, int iSize);
int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader);

#endif    /* char.h */

