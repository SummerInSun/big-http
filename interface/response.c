#include <response.h>
#include <char.h>
#include <config.h>
#include <format.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

void SocketPrintf(int iSockFd, const char *pcData)
{
    char strBuf[1024];

    sprintf(strBuf, "%s\r\n", pcData);
    send(iSockFd, strBuf, strlen(strBuf), 0);
}

void NoSuchFile(int iSockFd)
{	
    SocketPrintf(iSockFd, "HTTP/1.0 404 NOT FOUND");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Not Found</title> \
            <body>                   \
                <h2>Can't find this file</h2> \
            </body>                  \
         </html>                     \
        ");
}

void ErrorExec(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 500 Internal Server Error");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Error exec</title> \
            <body>                   \
                <h2>Error prohibited CGI execution.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void bad_request(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 400 BAD REQUEST");
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Error request</title> \
            <body>                   \
                <h2>Your browser sent a bad request.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void HtmlHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
}

void CgiHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
}

void PlainHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/plain\r\n");
}

static void SetEnv(struct RequestHeader *ptReqHeader)
{
    char strArgs[1024] = "\0";
    char strMethod[256] = "\0";
    char strContType[256] = "\0";

    /* ���� */
    if(!strcmp(ptReqHeader->strMethod, "GET")){
        snprintf(strArgs, 256, "QUERY_STRING=%s", ptReqHeader->strGetArgs);
        strArgs[255] = '\0';
        putenv(strArgs);
    }else if(!strcmp(ptReqHeader->strMethod, "POST")){
        /* ��Ҫ���������Ҫ�� cgi ����ʹ�ã��Ի�ô������Ĳ��� */
        snprintf(strContType, 256, "CONTENT_TYPE=%s", ptReqHeader->strContType);
        strContType[255] = '\0';
        putenv(strContType);

        snprintf(strArgs, 256, "CONTENT_LENGTH=%d", ptReqHeader->iContLen);
        strArgs[255] = '\0';
        putenv(strArgs);
    }

    /* ���� */
    snprintf(strMethod, 255, "REQUEST_METHOD=%s", ptReqHeader->strMethod);
    strMethod[255] = '\0';
    putenv(strMethod);
}

int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader)
{
    int iRecvNum = 0;
    char strBuf[1024];
	char strTmp[4][256];
	int iPostArgLen = 0;
    char *strIndex;

    memset(strBuf, 0, sizeof(strBuf));
    memset(strTmp, 0, sizeof(strTmp[0]) * 4);

    /* ���Ȼ�ȡ��һ�У���һ�����ǰ��������󷽷���url �Լ� http �汾�� */
    iRecvNum = GetLineFromSock(iSockFd, strBuf, sizeof(strBuf));
    if(-1 == iRecvNum){
        DebugPrint("GetRequestHeader::GetLineFromSock\n");
        return -1;
    }
    /* ���ո�ָ��ַ� */
    GetNonSpaceBlock(strTmp, strBuf, sizeof(strTmp[0]));

    memcpy(ptReqHeader->strMethod, strTmp[0], sizeof(strTmp[0]));
    memcpy(ptReqHeader->strURL, strTmp[1], sizeof(strTmp[0]));
    memcpy(ptReqHeader->strVersion, strTmp[2], sizeof(strTmp[0]));

    /* ����ͷ��������һ��������Ϊ���� */
    while(strBuf[0] != '\n'){
        iRecvNum = GetLineFromSock(iSockFd, strBuf, sizeof(strBuf));
        if(-1 == iRecvNum){
            perror("GetRequestHeader::GetLineFromSock");
            return -1;
        }

        if(!strncasecmp("Content-Length:", strBuf, 15)){
            GetNonSpaceBlock(strTmp, strBuf, sizeof(strTmp[0]));
            iPostArgLen = atoi(strTmp[1]);
            DebugPrint("Content-Length = %d\n", iPostArgLen);
        }

        if(!strncasecmp("Content-Type:", strBuf, 13)){
            strIndex = strrchr(strBuf, ':');
            if(strIndex){
                strIndex += 2;   /* ���� ": " */
                memcpy(ptReqHeader->strContType, strIndex, sizeof(ptReqHeader->strContType));
            }
            DebugPrint("Content-Type = %s\n", ptReqHeader->strContType);
        }
    }

    ptReqHeader->strPostArgs = malloc(iPostArgLen);    /* ����ռ� */
    if(NULL == ptReqHeader->strPostArgs){
        perror("GetRequestHeader::malloc");
        return -1;
    }
    memset(ptReqHeader->strPostArgs, 0, iPostArgLen);  /* ��� */
    ptReqHeader->iContLen = iPostArgLen;

	if(iPostArgLen){
        iRecvNum = GetBytesFromSock(iSockFd, ptReqHeader->strPostArgs, iPostArgLen);
        if(-1 == iRecvNum){
            perror("GetRequestHeader::GetLineFromSock");
            return -1;
        }
    }

    DebugPrint("strPostArgs = %s \n", ptReqHeader->strPostArgs);

    return 0;
}

int PutResponseHeader(int iSockFd, struct RequestHeader *ptReqHeader)
{
    int iError = 0;
    struct stat tReqFStat;
	char strPath[256]  = "\0";
    char *pcIndex;

    /* ����в����Ļ����Ѳ���ȡ���� */
    pcIndex = strrchr(ptReqHeader->strURL, '?');
    if(pcIndex != NULL){
        *pcIndex = '\0';
        pcIndex ++;
        memcpy(ptReqHeader->strGetArgs, pcIndex, sizeof(ptReqHeader->strGetArgs));
    }

    snprintf(strPath, sizeof(strPath), SERVER_ROOT"%s", ptReqHeader->strURL);

    iError = stat(strPath, &tReqFStat);
    if(-1 == iError){
        NoSuchFile(iSockFd);
        return 0;
    }

    if ((tReqFStat.st_mode & S_IFMT) == S_IFDIR){  /* ���ļ���һ���ļ��� */
        strcat(strPath, "/"DEFAULT_FILE);			 /* Ĭ����Ѱ����� index.html */

		iError = stat(strPath, &tReqFStat);
        if(-1 == iError){
            NoSuchFile(iSockFd);
            return 0;
        }
    }

    DebugPrint("Method = %s \n", ptReqHeader->strMethod);

    SetEnv(ptReqHeader);    /* ���û������� */
    RegularFileExec(iSockFd, strPath, ptReqHeader);

    free(ptReqHeader->strPostArgs);
    ptReqHeader->strPostArgs = NULL;

    DebugPrint("QUERY_STRING = %s \n", getenv("QUERY_STRING"));
    DebugPrint("REQUEST_METHOD = %s \n", getenv("REQUEST_METHOD"));

    return 0;
}

