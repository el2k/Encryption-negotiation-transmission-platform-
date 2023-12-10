#pragma once
#include <iostream>
using namespace std;
/* 用于通信的套接字类 */
// 超时的时间
static const int TIMEOUT = 10000;
class TcpSocket
{
public:
    enum ErronType{ParamError=3001,TimeoutError,PeerCloseError,MallocError};
    TcpSocket();
    TcpSocket(int connfd);
    ~TcpSocket();
    //连接服务器
    int connectToHost(string ip,unsigned short port,int timeout=TIMEOUT);
    //发送数据
    int sendMsg(string sendData,int timeout=TIMEOUT);
    //接收数据
    string recvMsg(int timeout=TIMEOUT);
    //断开连接
    void disConnect();
private:
    //设置非阻塞
    int setNonBlock(int fd);
    //设置阻塞
    int setBlock(int fd);
    //超时读
    int readTimeout(unsigned int wait_seconds);
    //超时写
    int writeTimeout(unsigned int wait_seconds);
    //超时连接
    int connectTimeout(struct sockaddr_in *addr,unsigned int wait_seconds);
    //读字符
    int readn(void*buf,int count);
    //写字符
    int writen(const void*buf,int count);
    int m_socket;//用于通信的套接字
}