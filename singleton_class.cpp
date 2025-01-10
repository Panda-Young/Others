/* **************************************************************
 * @Description: singleton class
 * @Date: 2024-05-13 16:34:28
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/

#include <iostream>

class Singleton
{
public:
    static Singleton *get_Instance(); // ����Ϊ��̬��Ա����

private:
    Singleton();
    ~Singleton();
    static Singleton *Instance;                  // ����Ϊ��̬��Ա����
    Singleton(const Singleton &) = delete;            // ��ֹ��������
    Singleton &operator=(const Singleton &) = delete; // ��ֹ��ֵ����
    void *test_ptr = nullptr;
};

// �����ⶨ�徲̬��Ա����
Singleton *Singleton::Instance = nullptr;

Singleton::Singleton()
{
    // ���ﲻ��Ҫ������ test_ptr Ϊ nullptr����Ϊ���Ѿ������г�ʼ��Ϊ nullptr
}

Singleton::~Singleton()
{
    // �����Ҫ�ͷ� test_ptr ָ�����Դ������������
}

Singleton *Singleton::get_Instance()
{
    if (Instance == nullptr) {
        Instance = new Singleton();
    }
    return Instance;
}

int main(int argc, char *argv[])
{
    Singleton *instance = Singleton::get_Instance(); // ��ȷ���þ�̬��Ա����
    std::cout << std::hex << instance << std::endl;

    // �����Ҫ�������ڳ������ǰɾ������ʵ��
    // ����ͨ���������ڳ������ʱ�Զ�����
    // delete Singleton::Instance; // ����ʹ�ã�����ܻᵼ���ڳ������ǰɾ����������Ϊ���������ʱ��ȫ��/��̬����ᱻ�Զ�����  

    return 0;
}
