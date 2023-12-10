#pragma once
#include <string>
#include <occi.h>
#include "SeckKeyNodeInfo.h"
using namespace std;
using namespace oracle::occi;


class OCCIOP
{
public:
	OCCIOP();
	~OCCIOP();

	// ��ʼ�������������ݿ�
	bool connectDB(string user, string passwd, string connstr);
	// �õ�keyID -> ����ʵ��ҵ�������װ��С����
	int getKeyID();
	bool updataKeyID(int keyID);
	bool writeSecKey(NodeSecKeyInfo *pNode);
	void closeDB();

private:
	// ��ȡ��ǰʱ��, ����ʽ��Ϊ�ַ���
	string getCurTime();

private:
	Environment* m_env;
	Connection* m_conn;
};

