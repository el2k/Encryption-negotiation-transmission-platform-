#include "TcpServer.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

TcpServer::TcpServer()
{
}

TcpServer::~TcpServer()
{
}

int TcpServer::setListen(unsigned short port)
{
    int ret=0;
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_add.s_addr=htonl(INADDR_ANY);
    //创建监听套接字
    m_lfd=socket(AF_INET,SOCK_STREAM,0);
    if(m_lfd==-1)
    {
        ret=errno;
        return ret;
    }
    int on=1;
    //设置端口复用
    ret=setsockopt(m_lfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    if(ret==-1)
    {
        ret=errno;
        return ret;
    }
    //绑定
    ret=bind(m_lfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if(ret==-1)
    {
        ret=errno;
        return ret;
    }
    //监听     
    ret=listen(m_lfd,128);
    if(ret==-1)
    {
        ret=errno;
        return ret;
    }
    //接收
    return ret;
}

TcpSocket *TcpServer::acceptConn(int timeout)
{
    int ret=0;
    if(timeout>0)
    {
        fd_set accept_fdset;
        struct timeval Timeout;
        FD_ZERO(&accept_fdset);
        FD_SET(m_lfd,&accept_fdset);
        Timeout.tv_sec=timeout;
        Timeout.tv_usec=0;
        do
        {
            ret=select(m_lfd+1,&accept_fdset,NULL,NULL,&Timeout);
        }while(ret<0&&errno=EINTR);
        if(ret<=0)
        {
            return NULL;
        }
        struct sockaddr_in addrCli;
        socklen_t addrlen=sizeof(struct sockaddr_in);
        int connfd=accept(m_lfd,(struct sockaddr*)&addrCli, &addrlen);//接收客户端套接字
        if(connfd==-1)
        {
            return NULL;
        }
        return new TcpSocket(connfd);
    }
    return nullptr;
}

void TcpServer::closefd()
{
    close(m_lfd);
}
