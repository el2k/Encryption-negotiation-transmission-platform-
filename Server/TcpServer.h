#pragma once
#include "TcpSocket.h"
class TcpServer
{
public:
    TcpServer();
    ~TcpServer();
    //¼àÌý
    int setListen(unsigned short port);
    //Á¬½Ó
    TcpSocket* acceptConn(int timeout=10000);
    void closefd();
private:
    int m_lfd;//¼àÌýÌ×½Ó×Ö
}