#include <format.h>
#include <config.h>
#include <char.h>
#include <response.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

static int CreateServerSocket(void)
{
    int iError = 0;
    int iServerSocket = -1;
    struct sockaddr_in tServerAddr;

    /* ipv4 Ĭ�� tcp Э�� */
    iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == iServerSocket){
        perror("CreateServerSocket::socket");
        return -1;
    }

    tServerAddr.sin_family      = AF_INET;    /* ����IPv4 */
    tServerAddr.sin_port        = htons(SERVER_PORT); /* host to net, short */
    tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* ������е�ip INADDR_ANY = 0.0.0.0*/
    memset(tServerAddr.sin_zero, 0, 8);       /* �̶�Ϊ 0 ���ַ� */

    iError = bind(iServerSocket, (const struct sockaddr *)&tServerAddr,
                sizeof(struct sockaddr));
    if(iError){
        perror("CreateServerSocket::bind");
		close(iServerSocket);
		return -1;
    }

    iError = listen(iServerSocket, MAX_LISTEN);
    if(iError){
        perror("CreateServerSocket::listen");
		close(iServerSocket);
		return -1;
    }

	return iServerSocket;
}

#if 0
static void *HandleRequest(void *Data)
{
    int iClientSocketFd = *(int *)Data;
    struct RequestHeader tReqHeader;
    int iError = 0;

    memset(&tReqHeader, 0, sizeof(struct RequestHeader));

    DebugPrint("Run to begin of HandleRequest ----\n");

    /* ��ȡ�ͻ�������ͷ�� */
    iError = GetRequestHeader(iClientSocketFd, &tReqHeader);
    if(iError){
        close(iClientSocketFd);
        pthread_exit(NULL);
    }

    /* ��Ӧ���� */
    PutResponseHeader(iClientSocketFd, &tReqHeader);
    /* 1.cgi */
    /* 2.regular file */

    DebugPrint("Method = %s\n",   tReqHeader.strMethod);
    DebugPrint("URL = %s\n",      tReqHeader.strURL);
    DebugPrint("Version = %s\n",  tReqHeader.strVersion);
    DebugPrint("PostArgs = %s\n", tReqHeader.strPostArgs);
    DebugPrint("GetArgs = %s\n",  tReqHeader.strGetArgs);

    DebugPrint("Run to end of HandleRequest\n");

    close(iClientSocketFd);
    pthread_exit(NULL);
}
#else
static void HandleRequest(int iClientSocketFd)
{
    struct RequestHeader tReqHeader;
    int iError = 0;

    memset(&tReqHeader, 0, sizeof(struct RequestHeader));

    DebugPrint("Run to begin of HandleRequest ----\n");

    /* ��ȡ�ͻ�������ͷ�� */
    iError = GetRequestHeader(iClientSocketFd, &tReqHeader);
    if(iError){
        close(iClientSocketFd);
        exit(0);
    }

    /* ��Ӧ���� */
    PutResponseHeader(iClientSocketFd, &tReqHeader);
    /* 1.cgi */
    /* 2.regular file */

    DebugPrint("Method = %s\n",   tReqHeader.strMethod);
    DebugPrint("URL = %s\n",      tReqHeader.strURL);
    DebugPrint("Version = %s\n",  tReqHeader.strVersion);
    DebugPrint("PostArgs = %s\n", tReqHeader.strPostArgs);
    DebugPrint("GetArgs = %s\n",  tReqHeader.strGetArgs);

    DebugPrint("Run to end of HandleRequest\n");

    close(iClientSocketFd);
    exit(0);
}
#endif

int main(int argc, char *argv[])
{
    int iError = 0;
    int iServerSocketFd = -1;
    int iClientSocketFd = -1;
    struct sockaddr_in tClientSockaddr;
	socklen_t ClientSockLen = 0;
    pthread_t HandleRequestFd;

    iError = FormatMgrInit();
    if(iError){
        DebugPrint("FormatMgrInit error\n");
        return -1;
    }

    iServerSocketFd = CreateServerSocket();
	if(-1 == iServerSocketFd){
        perror("main::CreateServerSocket");
		return -1;
    }

    if(fork()){    /* ����Ƿ��ӽ��̣����˳���ͬʱ�ر� socket */
        close(iServerSocketFd);
        return 0; 
    }

    /* ��ֹ�����ӽ������ڸ������˳���
     *��������û�е��� waitpid/wait ���½�ʬ���� 
     *������� SIGCHLD �ź�
     */
    signal(SIGCHLD,SIG_IGN);

    while(1){
        iClientSocketFd = accept(iServerSocketFd, (struct sockaddr *)&tClientSockaddr, &ClientSockLen);
        if(-1 == iClientSocketFd){
            perror("main::accept");
			
        }else{
#if 0
            DebugPrint("Got a connect\n");
            iError = pthread_create(&HandleRequestFd, NULL, HandleRequest, &iClientSocketFd);
            if(iError){
                perror("main::pthread_create");
				close(iClientSocketFd);
            }
#else
            if(fork() == 0){
                HandleRequest(iClientSocketFd);
            }else{
                /* �ӽ�������رգ�����������ҲҪ�ر�
                   * ֻ��ȫ���ر��ˣ������ͻ��˲��ܹ����ر�
                   */
                close(iClientSocketFd);
            }
#endif
        }
    }

    close(iServerSocketFd);

    return 0;
}

