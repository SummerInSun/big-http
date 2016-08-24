#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DebugPrint printf

#define SERVER_PORT 80    /* Ĭ�϶˿��� 80 �˿ڣ�Ҳ�� www �Ķ˿� */
#define MAX_LISTEN  256   /* �ɼ�������Ĳ�����������Ϊ 256 */
#define SINGLE_UPLOAD_SIZE 1024*1024*2    /* ����һ���ϴ���СΪ 2M */

#define SERVER_ROOT  "/www"
#define CONFIG_FILE  "big-http.conf"
#define CGI_ROOT     "/www/cgi-bin"
#define DEFAULT_FILE "index.html"

#define SERVER_SOFTWARE   "Server: big-http/0.1-bigyellow"
#define SERVER_NAME       "www.yellowmax.org"
#define GATEWAY_INTERFACE "CGI/1.1"
#define SERVER_PROTOCOL   "HTTP/1.1"

#endif    /* config.h */

