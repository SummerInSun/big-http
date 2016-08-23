#include <format.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

static int isSupportCgi(struct FileDesc *ptFileDesc);
static int CgiWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader);
//static int CgiReadFromClient(int iClient, char *strPath);

#define STDOUT 1
#define STDIN  0

struct FormatMgr g_tCgiFormatMgr = {
    .name = "cgi",
    .isSupport = isSupportCgi,
    .WriteToClient  = CgiWriteToClient,
    //.ReadFromClient = CgiReadFromClient,
};

static int isSupportCgi(struct FileDesc *ptFileDesc)
{
    const char aStrCgi[] = {0x7f, 0x45, 0x4c, 0x46, 0};
    unsigned char *pucMem = ptFileDesc->pucMem;

    if (strncmp((char *)pucMem, aStrCgi, 3) == 0){
        return 1;
    }

    if ((ptFileDesc->tFStat.st_mode & S_IXUSR) ||
        (ptFileDesc->tFStat.st_mode & S_IXGRP) ||
        (ptFileDesc->tFStat.st_mode & S_IXOTH)   )
    {
        return 1;
    }

    /* ֻ֧�� elf �ļ�,���ִ�нű��ļ� */
    return 0;
}

static int CgiWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader)
{
	int iCgiOutFd[2];
	int iCgiInFd[2];
    pid_t ForkPid;
    char cByte = '\0';
    fd_set WriteSet;
    char *strTmp;
    int i = 0;

    munmap(ptFileDesc->pucMem, ptFileDesc->tFStat.st_size);
    close(ptFileDesc->iFd);

    DebugPrint("Run to CgiWriteToClient\n");

    /* �����ܵ� */
    if (pipe(iCgiOutFd) < 0) {
        perror("CgiWriteToClient::pipe");
        ErrorExec(iClient);
        return -1;
	}
    if (pipe(iCgiInFd) < 0) {
        perror("CgiWriteToClient::pipe");
        ErrorExec(iClient);
        return -1;
    }

    /* fork �߳� */
    ForkPid = fork();
    if(ForkPid < 0){
        perror("CgiWriteToClient::pipe");
        ErrorExec(iClient);
        return -1;
    }

    /* ִ�� cgi ������д��ͻ��� */
    if(ForkPid == 0){    /* ���߳� */
        dup2(iCgiOutFd[1], STDOUT);    /* �ѱ�׼������ӵ��ܵ���� */
        dup2(iCgiInFd[0], STDIN);	   /* �ѱ�׼�������ӵ��ܵ����� */
        close(iCgiOutFd[0]);
        close(iCgiInFd[1]);

        execl(ptFileDesc->strFName, NULL, NULL);
        exit(0);
	}else{    /* ���߳� */
        close(iCgiOutFd[1]);
        close(iCgiInFd[0]);

        CgiHeader(iClient);

        /* cgi ����ӱ�׼�����ò��� */
        strTmp = ptReqHeader->strPostArgs;
        write(iCgiInFd[1], strTmp, ptReqHeader->iContLen);

        /* ���ӽ��̵�����˶�ȡ���ݣ����͵��ͻ��˽�����ʾ */
        while (read(iCgiOutFd[0], &cByte, 1) > 0){
            send(iClient, &cByte, 1, 0);
        }

        /* �رչܵ����ȴ��ӽ����˳� */
        close(iCgiOutFd[0]);
        close(iCgiInFd[1]);
        waitpid(ForkPid, NULL, 0);
    }

    return 0;
}

int CgiInit(void)
{
    RegisterFormatMgr(&g_tCgiFormatMgr);
    return 0;
}




