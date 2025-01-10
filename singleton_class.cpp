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
    static Singleton *get_Instance(); // 声明为静态成员函数

private:
    Singleton();
    ~Singleton();
    static Singleton *Instance;                  // 声明为静态成员变量
    Singleton(const Singleton &) = delete;            // 禁止拷贝构造
    Singleton &operator=(const Singleton &) = delete; // 禁止赋值操作
    void *test_ptr = nullptr;
};

// 在类外定义静态成员变量
Singleton *Singleton::Instance = nullptr;

Singleton::Singleton()
{
    // 这里不需要再设置 test_ptr 为 nullptr，因为它已经在类中初始化为 nullptr
}

Singleton::~Singleton()
{
    // 如果需要释放 test_ptr 指向的资源，请在这里做
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
    Singleton *instance = Singleton::get_Instance(); // 正确调用静态成员函数
    std::cout << std::hex << instance << std::endl;

    // 如果需要，可以在程序结束前删除单例实例
    // 但是通常单例会在程序结束时自动销毁
    // delete Singleton::Instance; // 谨慎使用，这可能会导致在程序结束前删除单例；因为当程序结束时，全局/静态对象会被自动销毁  

    return 0;
}
