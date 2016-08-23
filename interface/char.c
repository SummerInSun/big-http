#include <char.h>
#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>

int GetLineFromSock(int iSockFd, char *pcBuf, int iSize)
{
    int iError = 0;
	char cByte = '\0';    /* ��ʼ��Ϊһ�����ַ� */
    int iRecvSize = 0;
    struct timeval tWaitVal;
    fd_set ReadSet;
	
    tWaitVal.tv_sec  = 0;
	tWaitVal.tv_usec = 1000 * 1000;    /* ����ÿ�ζ�ȡ�����ȴ� 500 ms */
#if 0
    FD_ZERO(&ReadSet);
    FD_SET(iSockFd, &ReadSet);
	
    iError = select(iSockFd + 1, &ReadSet, NULL, NULL, &tWaitVal);
    if(!FD_ISSET(iSockFd, &ReadSet)){	/* ˵��û�����ݹ���Ҫ���� */
        DebugPrint("Out of receive time ****** \n");
        *pcBuf++ = '\n';	  /* �ַ������� */
        *pcBuf = '\0';        /* �ַ������� */
        return -1;
    }
#endif
    while((iRecvSize < iSize) && (cByte != '\n')){

        FD_ZERO(&ReadSet);
        FD_SET(iSockFd, &ReadSet);
        
        iError = select(iSockFd + 1, &ReadSet, NULL, NULL, &tWaitVal);
        if(!FD_ISSET(iSockFd, &ReadSet)){	/* ˵��û�����ݹ���Ҫ���� */
            DebugPrint("Out of receive time ****** \n");
            *pcBuf++ = '\n';	  /* �ַ������� */
            *pcBuf = '\0';		  /* �ַ������� */
            return -1;
        }

        iError = recv(iSockFd, &cByte, 1, 0);
        if(-1 == iError){
            perror("GetLineFromSock::recv");
            *pcBuf++ = '\n';	  /* �ַ������� */
            break;
        }

		/* ��β�� '\n' ���� "\r\n" ���� '\r' */
        if(cByte == '\r'){
            iError = recv(iSockFd, &cByte, 1, MSG_PEEK);
            if((iError > 0) && (cByte == '\n')){
                iError = recv(iSockFd, &cByte, 1, 0);
			}else{
                cByte = '\n';
            }
        }

        *pcBuf++ = cByte;
        iRecvSize ++;
    }

    *pcBuf = '\0';    /* �ַ������� */

    return iRecvSize;
}

int GetBytesFromSock(int iSockFd, char *pcBuf, int iSize)
{
    int iError = 0;
	char cByte = '\0';    /* ��ʼ��Ϊһ�����ַ� */
    int iRecvSize = 0;
    struct timeval tWaitVal;
    fd_set ReadSet;
	
    tWaitVal.tv_sec  = 0;
	tWaitVal.tv_usec = 1000 * 1000;    /* ����ÿ�ζ�ȡ�����ȴ� 500 ms */

    while(iRecvSize < iSize){

        FD_ZERO(&ReadSet);
        FD_SET(iSockFd, &ReadSet);
        
        iError = select(iSockFd + 1, &ReadSet, NULL, NULL, &tWaitVal);
        if(!FD_ISSET(iSockFd, &ReadSet)){	/* ˵��û�����ݹ���Ҫ���� */
            DebugPrint("Out of receive time ****** \n");
            *pcBuf++ = '\n';	  /* �ַ������� */
            *pcBuf = '\0';		  /* �ַ������� */
            return -1;
        }

        iError = recv(iSockFd, &cByte, 1, 0);
        if(-1 == iError){
            perror("GetLineFromSock::recv");
            *pcBuf++ = '\n';	  /* �ַ������� */
            break;
        }

        *pcBuf++ = cByte;
        iRecvSize ++;
    }

    *pcBuf = '\0';    /* �ַ������� */

    return iRecvSize;
}

int GetNonSpaceBlock(char pcDes[][256], char *pcSrc, int iLenSize)
{
    int i = 0;
    int j = 0;
	int k = 0;
static char bOutRange = 0;

    if((pcSrc[0] == '\n') || (pcSrc[0] == '\r') || (pcSrc[0] == '\0')){    /* ˵������Ϊ�� */
        pcDes[0][0] = '\0';
        return 0;
    }    

    i = 0;
    j = 0;
    k = 0;

    DebugPrint("%s", pcSrc);

    while(1){
        while(isspace(pcSrc[k])){    /* �Ե����еĿո��ַ� */
            if((pcSrc[k] == '\n') || (pcSrc[k] == '\0')){  /* �������л��߿��ַ�,˵���ַ���������� */
                return 0;
            }

            k ++;
        }

        while(!isspace(pcSrc[k])){
            if(i < iLenSize){
                pcDes[j][i] = pcSrc[k];
                k ++;
                i ++;
            }else{
                if(0 == bOutRange){
                    DebugPrint("pcSrc out of range\n");
                    bOutRange = 1;
                }
            }
        }

        pcDes[j][i] = '\0';    /* Ϊÿһ�м��Ͻ����ַ� */
        j ++;    /* ��ά���黻�� */
		i = 0;   /* i ������ 0 */
    }

    return 0;
}

