#pragma once
#include <iostream>
using namespace std;
/* ����ͨ�ŵ��׽����� */
// ��ʱ��ʱ��
static const int TIMEOUT = 10000;
class TcpSocket
{
public:
    enum ErronType{ParamError=3001,TimeoutError,PeerCloseError,MallocError};
    TcpSocket();
    TcpSocket(int connfd);
    ~TcpSocket();
    //���ӷ�����
    int connectToHost(string ip,unsigned short port,int timeout=TIMEOUT);
    //��������
    int sendMsg(string sendData,int timeout=TIMEOUT);
    //��������
    string recvMsg(int timeout=TIMEOUT);
    //�Ͽ�����
    void disConnect();
private:
    //���÷�����
    int setNonBlock(int fd);
    //��������
    int setBlock(int fd);
    //��ʱ��
    int readTimeout(unsigned int wait_seconds);
    //��ʱд
    int writeTimeout(unsigned int wait_seconds);
    //��ʱ����
    int connectTimeout(struct sockaddr_in *addr,unsigned int wait_seconds);
    //���ַ�
    int readn(void*buf,int count);
    //д�ַ�
    int writen(const void*buf,int count);
    int m_socket;//����ͨ�ŵ��׽���
}