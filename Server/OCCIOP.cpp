#include "OCCIOP.h"
#include <iostream>
#include <string.h>
#include <time.h>
using namespace std;
using namespace oracle::occi;

OCCIOP::OCCIOP()
{
}


OCCIOP::~OCCIOP()
{
}

bool OCCIOP::connectDB(string user, string passwd, string connstr)
{
	// 1. ��ʼ������
	m_env = Environment::createEnvironment("utf8", "utf8");
	// 2. ��������
	m_conn = m_env->createConnection(user, passwd, connstr);
	if (m_conn == NULL)
	{
		return false;
	}
	cout << "���ݿ����ӳɹ�..." << endl;
	return true;
}

int OCCIOP::getKeyID()
{
	// ��ѯ���ݿ�
	// for update: ��ʱ�����ݱ����
	string sql = "select ikeysn from SECMNG.KEYSN for update";
	Statement* stat = m_conn->createStatement(sql);
	// ִ�����ݲ�ѯ
	ResultSet* resSet = stat->executeQuery();
	// �ñ�ֻ��һ����¼
	int keyID = -1;
	if (resSet->next())
	{
		keyID = resSet->getInt(1);
	}
	stat->closeResultSet(resSet);
	m_conn->terminateStatement(stat);

	return keyID;
}

// ��ԿID�ڲ����ʱ����Զ�����, Ҳ�����ֶ�����
bool OCCIOP::updataKeyID(int keyID)
{
	// �������ݿ�
	string sql = "update SECMNG.KEYSN set ikeysn = " + to_string(keyID);
	Statement* stat = m_conn->createStatement(sql);
	// �����Զ��ύ
	stat->setAutoCommit(true);	
	// ִ�����ݲ�ѯ
	int ret = stat->executeUpdate();
	m_conn->terminateStatement(stat);
	if (ret <= 0)
	{
		return false;
	}
	return true;
}

// �����ɵ���Կд�����ݿ�
// ������Կ���
bool OCCIOP::writeSecKey(NodeSecKeyInfo *pNode)
{
	// ��֯�������sql���
	char sql[1024] = { 0 };
	sprintf(sql, "Insert Into SECMNG.SECKEYINFO(clientid, serverid, keyid, createtime, state, seckey) \
					values ('%s', '%s', %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss') , %d, '%s') ", 
		pNode->clientID, pNode->serverID, pNode->seckeyID, 
		getCurTime().data(), 1, pNode->seckey);
	cout << "insert sql: " << sql << endl;
	Statement* stat = m_conn->createStatement();
	// ���ݳ�ʼ��
	stat->setSQL(sql);
	// ִ��sql
	stat->setAutoCommit(true);
	bool bl = stat->executeUpdate(sql);
	m_conn->terminateStatement(stat);
	if (bl)
	{
		return true;
	}
	return false;
}

void OCCIOP::closeDB()
{
	m_env->terminateConnection(m_conn);
	Environment::terminateEnvironment(m_env);
}

string OCCIOP::getCurTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

	return tmp;
}
