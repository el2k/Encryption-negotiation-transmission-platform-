#pragma once
#include <map>
#include "TcpServer.h"
#include "Message.pb.h"
#include "OCCIOP.h"
#include "SecKeyShm.h"
// ����ͻ�������
class ServerOP
{
public:
	enum KeyLen {Len16=16, Len24=24, Len32=32};
	ServerOP(string json);
	// ����������
	void startServer();
	// �̹߳������� -> �Ƽ�ʹ��
	static void* working(void* arg);
	// ��Ԫ�ƻ�����ķ�װ
	friend void* workHard(void* arg);
	// ��ԿЭ��
	string seckeyAgree(RequestMsg* reqMsg);
	~ServerOP();

private:
	string getRandKey(KeyLen len);

private:
	string m_serverID;	// ��ǰ��������ID
	string m_dbUser;
	string m_dbPwd;
	string m_dbConnStr;
	unsigned short m_port;
	map<pthread_t, TcpSocket*> m_list;
	TcpServer *m_server = NULL;
	// �������ݿ�ʵ������
	OCCIOP m_occi;
	SecKeyShm* m_shm;
};

void* workHard(void* arg);

