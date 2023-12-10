#include "RsaCrypto.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iostream>
#include <openssl/buffer.h>
#include <string.h>
RsaCrypto::RsaCrypto()
{
	m_publicKey = RSA_new();
	m_privateKey = RSA_new();
}

RsaCrypto::RsaCrypto(string fileName, bool isPrivate)
{
	m_publicKey = RSA_new();
	m_privateKey = RSA_new();
	if (isPrivate)
	{
		initPrivateKey(fileName);
	}
	else
	{
		initPublicKey(fileName);
	}
}
RsaCrypto::~RsaCrypto()
{
	RSA_free(m_publicKey);
	RSA_free(m_privateKey);
}
// ����Կ/˽Կ�ַ������ݽ����� RSA ������
void RsaCrypto::parseKeyString(string keystr, bool pubKey)
{
	// �ַ������� -> BIO������
	BIO* bio = BIO_new_mem_buf(keystr.data(), keystr.size());
	// ��Կ�ַ���
	if (pubKey)
	{
		PEM_read_bio_RSAPublicKey(bio, &m_publicKey, NULL, NULL);
	}
	else
	{
		// ˽Կ�ַ���
		PEM_read_bio_RSAPrivateKey(bio, &m_privateKey, NULL, NULL);
	}
	BIO_free(bio);
}

void RsaCrypto::generateRsakey(int bits, string pub, string pri)
{
	RSA* r = RSA_new();
	// ����RSA��Կ��
	// ����bignum����
	BIGNUM* e = BN_new();
	// ��ʼ��bignum����
	BN_set_word(e, 456787);
	RSA_generate_key_ex(r, bits, e, NULL);

	// ����bio�ļ�����
	BIO* pubIO = BIO_new_file(pub.data(), "w");
	// ��Կ��pem��ʽд�뵽�ļ���
	PEM_write_bio_RSAPublicKey(pubIO, r);
	// �����е�����ˢ���ļ���
	BIO_flush(pubIO);
	BIO_free(pubIO);

	// ����bio����
	BIO* priBio = BIO_new_file(pri.data(), "w");
	// ˽Կ��pem��ʽд���ļ���
	PEM_write_bio_RSAPrivateKey(priBio, r, NULL, NULL, 0, NULL, NULL);
	BIO_flush(priBio);
	BIO_free(priBio);

	// �õ���Կ��˽Կ
	m_privateKey = RSAPrivateKey_dup(r);
	m_publicKey = RSAPublicKey_dup(r);

	// �ͷ���Դ
	BN_free(e);
	RSA_free(r);
}
bool RsaCrypto::initPublicKey(string pubfile)
{
	// ͨ��BIO���ļ�
	BIO* pubBio = BIO_new_file(pubfile.data(), "r");
	// ��bio�е�pem���ݶ���
	if (PEM_read_bio_RSAPublicKey(pubBio, &m_publicKey, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(pubBio);
	return true;
}

bool RsaCrypto::initPrivateKey(string prifile)
{
	// ͨ��bio���ļ�
	BIO* priBio = BIO_new_file(prifile.data(), "r");
	// ��bio�е�pem���ݶ���
	if (PEM_read_bio_RSAPrivateKey(priBio, &m_privateKey, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(priBio);
	return true;
}
string RsaCrypto::rsaPubKeyEncrypt(string data)
{
	cout << "�������ݳ���: " << data.size() << endl;
	// ���㹫Կ����
	int keyLen = RSA_size(m_publicKey);
	cout << "pubKey len: " << keyLen << endl;
	// �����ڴ�ռ�
	char* encode = new char[keyLen + 1];
	// ʹ�ù�Կ����
	int ret = RSA_public_encrypt(data.size(), (const unsigned char*)data.data(),
		(unsigned char*)encode, m_publicKey, RSA_PKCS1_PADDING);
	string retStr = string();
	if (ret >= 0)
	{
		// ���ܳɹ�
		cout << "ret: " << ret << ", keyLen: " << keyLen << endl;
		retStr = toBase64(encode, ret);
	}
	else
	{
		ERR_print_errors_fp(stdout);
	}
	// �ͷ���Դ
	delete[]encode;
	return retStr;
}

string RsaCrypto::rsaPriKeyDecrypt(string encData)
{
	// textָ����ڴ���Ҫ�ͷ�
	char* text = fromBase64(encData);
	// ����˽Կ����
	//cout << "�������ݳ���: " << text.size() << endl;
	int keyLen = RSA_size(m_privateKey);
	// ʹ��˽Կ����
	char* decode = new char[keyLen + 1];
	// ���ݼ������֮��, ���ĳ��� == ��Կ����
	int ret = RSA_private_decrypt(keyLen, (const unsigned char*)text,
		(unsigned char*)decode, m_privateKey, RSA_PKCS1_PADDING);
	string retStr = string();
	if (ret >= 0)
	{
		retStr = string(decode, ret);
	}
	else
	{
		cout << "˽Կ����ʧ��..." << endl;
		ERR_print_errors_fp(stdout);
	}
	delete[]decode;
	delete[]text;
	return retStr;
}

string RsaCrypto::rsaSign(string data, SignLevel level)
{
	unsigned int len;
	char* signBuf = new char[1024];
	memset(signBuf, 0, 1024);
	int ret = RSA_sign(level, (const unsigned char*)data.data(), data.size(), (unsigned char*)signBuf,
		&len, m_privateKey);
	if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	cout << "sign len: " << len << ", ret: " << ret << endl;
	string retStr = toBase64(signBuf, len);
	delete[]signBuf;
	return retStr;
}

bool RsaCrypto::rsaVerify(string data, string signData, SignLevel level)
{
	// ��֤ǩ��
	int keyLen = RSA_size(m_publicKey);
	char* sign = fromBase64(signData);
	int ret = RSA_verify(level, (const unsigned char*)data.data(), data.size(),
		(const unsigned char*)sign, keyLen, m_publicKey);
	delete[]sign;
	if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	if (ret != 1)
	{
		return false;
	}
	return true;
}
string RsaCrypto::toBase64(const char* str, int len)
{
	BIO* mem = BIO_new(BIO_s_mem());
	BIO* bs64 = BIO_new(BIO_f_base64());
	// mem��ӵ�bs64��
	bs64 = BIO_push(bs64, mem);
	// д����
	BIO_write(bs64, str, len);
	BIO_flush(bs64);
	// �õ��ڴ����ָ��
	BUF_MEM *memPtr;
	BIO_get_mem_ptr(bs64, &memPtr);
	string retStr = string(memPtr->data, memPtr->length - 1);
	BIO_free_all(bs64);
	return retStr;
}

char* RsaCrypto::fromBase64(string str)
{
	int length = str.size();
	BIO* bs64 = BIO_new(BIO_f_base64());
	BIO* mem = BIO_new_mem_buf(str.data(), length);
	BIO_push(bs64, mem);
	char* buffer = new char[length];
	memset(buffer, 0, length);
	BIO_read(bs64, buffer, length);
	BIO_free_all(bs64);

	return buffer;
}
