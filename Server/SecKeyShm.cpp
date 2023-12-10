#include "SecKeyShm.h"
#include <string.h>
#include <iostream>
using namespace std;

SecKeyShm::SecKeyShm(int key, int maxNode)
	: BaseShm(key, maxNode * sizeof(NodeSecKeyInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::SecKeyShm(string pathName, int maxNode)
	: BaseShm(pathName, maxNode * sizeof(NodeSecKeyInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::~SecKeyShm()
{
}

void SecKeyShm::shmInit()
{
	if (m_shmAddr != NULL)
	{
		memset(m_shmAddr, 0, m_maxNode * sizeof(NodeSecKeyInfo));
	}
}

int SecKeyShm::shmWrite(NodeSecKeyInfo * pNodeInfo)
{
	int ret = -1;
	// ���������ڴ�
	NodeSecKeyInfo* pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}

	// �жϴ����������Կ�Ƿ��Ѿ�����
	NodeSecKeyInfo	*pNode = NULL;
	for (int i = 0; i < m_maxNode; i++)
	{
		// pNode����ָ��ÿ���ڵ���׵�ַ
		pNode = pAddr + i;
		cout << i << endl;
		cout << "clientID �Ƚ�: " << pNode->clientID << ", " << pNodeInfo->clientID << endl;
		cout << "serverID �Ƚ�: " << pNode->serverID << ", " << pNodeInfo->serverID << endl;
		cout << endl;
		if (strcmp(pNode->clientID, pNodeInfo->clientID) == 0 &&
			strcmp(pNode->serverID, pNodeInfo->serverID) == 0)
		{
			// ����ҵ��˸�������Կ�Ѿ�����, ʹ������Կ���Ǿɵ�ֵ
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
			unmapShm();
			cout << "д���ݳɹ�: ԭ���ݱ�����!" << endl;
			return 0;
		}
	}

	// ��û���ҵ���Ӧ����Ϣ, ��һ���սڵ㽫��Կ��Ϣд��
	int i = 0;
	NodeSecKeyInfo tmpNodeInfo; //�ս��
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (memcmp(&tmpNodeInfo, pNode, sizeof(NodeSecKeyInfo)) == 0)
		{
			ret = 0;
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
			cout << "д���ݳɹ�: ���µĽڵ����������!" << endl;
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
}

NodeSecKeyInfo SecKeyShm::shmRead(string clientID, string serverID)
{
	int ret = 0;
	// ���������ڴ�
	NodeSecKeyInfo *pAddr = NULL;
	pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		cout << "�����ڴ����ʧ��..." << endl;
		return NodeSecKeyInfo();
	}
	cout << "�����ڴ�����ɹ�..." << endl;

	//����������Ϣ
	int i = 0;
	NodeSecKeyInfo info;
	NodeSecKeyInfo	*pNode = NULL;
	// ͨ��clientID��serverID���ҽڵ�
	cout << "maxNode: " << m_maxNode << endl;
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		cout << i << endl;
		cout << "clientID �Ƚ�: " << pNode->clientID << ", " << clientID.data() << endl;
		cout << "serverID �Ƚ�: " << pNode->serverID << ", " << serverID.data() << endl;
		if (strcmp(pNode->clientID, clientID.data()) == 0 &&
			strcmp(pNode->serverID, serverID.data()) == 0)
		{
			// �ҵ��Ľڵ���Ϣ, ��������������
			info = *pNode;
			cout << "++++++++++++++" << endl;
			cout << info.clientID << " , " << info.serverID << ", "
				<< info.seckeyID << ", " << info.status << ", "
				<< info.seckey << endl;
			cout << "===============" << endl;
			break;
		}
	}

	unmapShm();
	return info;
}
