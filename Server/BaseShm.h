#pragma once
#include <iostream>
using namespace std;

class BaseShm
{
public:
    BaseShm(int Key);
    BaseShm(int key,int size);
    BaseShm(string name);
    BaseShm(string name,int size);
    void * mapShm();
    int unmapShm();
    int delSum();
    ~BaseShm();
private:
    int getShmID(key_t key,int shmSize;int flag);
private:
    int m_shmID;
protect:
    void *m_shmAddr=NULL;
};