#include "ServerOP.h"
#include "TcpSocket.h"
#include "RequestFactory.h"
#include "RequestCodec.h"
#include "RespondCodec.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include <string>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <unistd.h>
#include "Hash.h"
using namespace std;
using namespace Json;

/*
	{
		"Port":9898
	}
*/
ServerOP::ServerOP(string json)
{
	// ����json�ļ�, ���ļ� -> Value
	ifstream ifs(json);
	Reader r;
	Value root;
	r.parse(ifs, root);
	// ��root�еļ�ֵ��valueֵȡ��
	m_port = root["Port"].asInt();
	m_serverID = root["ServerID"].asString();
	// ���ݿ���ص���Ϣ
	m_dbUser = root["UserDB"].asString();
	m_dbPwd = root["PwdDB"].asString();
	m_dbConnStr = root["ConnStrDB"].asString();

	// ʵ����һ������oracle���ݵĶ���
	m_occi.connectDB(m_dbUser, m_dbPwd, m_dbConnStr);

	// ʵ���������ڴ����
	// �������ļ��ж� key/pathname
	string shmKey = root["ShmKey"].asString();
	int maxNode = root["ShmMaxNode"].asInt();
	// �ͻ��˴洢����Կֻ��һ��
	m_shm = new SecKeyShm(shmKey, maxNode);
}


void ServerOP::startServer()
{
	m_server = new TcpServer;
	m_server->setListen(m_port);
	while (1)
	{
		cout << "�ȴ��ͻ�������..." << endl;
		TcpSocket* tcp = m_server->acceptConn();
		if (tcp == NULL)
		{
			continue;
		}
		cout << "��ͻ������ӳɹ�..." << endl;
		// ͨ��
		pthread_t tid;
		// ����ص���������ľ�̬����, �����Ԫ����, ��ͨ�ĺ���
		// ��Ԫ���������, ���ǲ����������
		// ��Ԫ�������Է��ʵ�ǰ���˽�г�Ա
		pthread_create(&tid, NULL, workHard, this);
		m_list.insert(make_pair(tid, tcp));
	}
}

void * ServerOP::working(void * arg)
{
	return nullptr;
}

string ServerOP::seckeyAgree(RequestMsg* reqMsg)
{
	// 0. ��ǩ������У�� -> ��Կ���� -> �õ���Կ
	// ���յ��Ĺ�Կ����д�뱾�ش���
	ofstream ofs("public.pem");
	ofs << reqMsg->data();
	ofs.close();
	// �����ǶԳƼ��ܶ���
	RespondInfo info;
	RsaCrypto rsa("public.pem", false);

	// ������ϣ����
	Hash sha(T_SHA1);
	sha.addData(reqMsg->data());
	cout << "1111111111111111" << endl;
	bool bl = rsa.rsaVerify(sha.result(), reqMsg->sign());
	cout << "00000000000000000000" << endl;
	if (bl == false)
	{
		cout << "ǩ��У��ʧ��..." << endl;
		info.status = false;
	}
	else
	{
		cout << "ǩ��У��ɹ�..." << endl;
		// 1. ��������ַ���
		//   �ԳƼ��ܵ���Կ, ʹ�öԳƼ����㷨 aes, ��Կ����: 16, 24, 32byte
		string key = getRandKey(Len16);
		cout << "���ɵ������Կ: " << key << endl;
		// 2. ͨ����Կ����
		cout << "aaaaaaaaaaaaaaaa" << endl;
		string seckey = rsa.rsaPubKeyEncrypt(key);
		cout << "����֮�����Կ: " << seckey << endl;
		// 3. ��ʼ���ظ�������
		info.clientID = reqMsg->clientid();
		info.data = seckey;
		info.serverID = m_serverID;
		info.status = true;	

		// �����ɵ�����Կд�뵽���ݿ��� -> ���� SECKEYINFO
		NodeSecKeyInfo node;
		strcpy(node.clientID, reqMsg->clientid().data());
		strcpy(node.serverID, reqMsg->serverid().data());
		strcpy(node.seckey, key.data());
		node.seckeyID = m_occi.getKeyID();	// ��Կ��ID
		info.seckeyID = node.seckeyID;
		node.status = 1;
		// ��ʼ��node����
		bool bl = m_occi.writeSecKey(&node);
		if(bl)
		{
			// �ɹ�
			m_occi.updataKeyID(node.seckeyID + 1);
			// д�����ڴ�
			m_shm->shmWrite(&node);
		}
		else
		{
			// ʧ��
			info.status = false;
		}
	}

	// 4. ���л�
	CodecFactory* fac = new RespondFactory(&info);
	Codec* c = fac->createCodec();
	string encMsg = c->encodeMsg();
	// 5. ��������
	return encMsg;

}

ServerOP::~ServerOP()
{
	if (m_server)
	{
		delete m_server;
	}
	delete m_shm;
}

// Ҫ��: �ַ����а���: a-z, A-Z, 0-9, �����ַ�
string ServerOP::getRandKey(KeyLen len)
{
	// ��������������� => ����ʱ��
	srand(time(NULL));
	int flag = 0;
	string randStr = string();
	char *cs = "~!@#$%^&*()_+}{|\';[]";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;	// 4���ַ�����
		switch (flag)
		{
		case 0:	// a-z
			randStr.append(1, 'a' + rand() % 26);
			break;
		case 1: // A-Z
			randStr.append(1, 'A' + rand() % 26);
			break;
		case 2: // 0-9
			randStr.append(1, '0' + rand() % 10);
			break;
		case 3: // �����ַ�
			randStr.append(1, cs[rand() % strlen(cs)]);
			break;
		default:
			break;
		}
	}
	return randStr;
}

void* workHard(void * arg)
{
	sleep(1);
	string data = string();
	// ͨ�����������ݵ�this����ת��
	ServerOP* op = (ServerOP*)arg;
	// ��op�н�ͨ�ŵ��׽��ֶ���ȡ��
	TcpSocket* tcp = op->m_list[pthread_self()];
	// 1. ���տͻ������� -> ����
	string msg = tcp->recvMsg();
	// 2. �����л� -> �õ�ԭʼ���� RequestMsg ����
	CodecFactory* fac = new RequestFactory(msg);
	Codec* c = fac->createCodec();
	RequestMsg* req = (RequestMsg*)c->decodeMsg();
	// 3. ȡ������
	// �жϿͻ�����ʲô����
	switch (req->cmdtype())
	{
	case 1:
		// ��ԿЭ��
		data = op->seckeyAgree(req);
		break;
	case 2:
		// ��ԿУ��
		break;
	default:
		break;
	}

	// �ͷ���Դ
	delete fac;
	delete c;
	// tcp������δ���
	tcp->sendMsg(data);
	tcp->disConnect();
	op->m_list.erase(pthread_self());
	delete tcp;

	return NULL;
}
