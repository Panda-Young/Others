/***************************************************************************
 * Description: Implementation of a singleton class in C++
 * Version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-13 16:34:28
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <iostream>

class Singleton
{
public:
    // Static method to get the single instance of the class
    static Singleton *get_Instance();

private:
    // Private constructor to prevent instantiation
    Singleton();
    // Private destructor
    ~Singleton();
    // Static pointer to hold the single instance of the class
    static Singleton *Instance;
    // Delete copy constructor to prevent copying
    Singleton(const Singleton &) = delete;
    // Delete assignment operator to prevent assignment
    Singleton &operator=(const Singleton &) = delete;
    // Pointer for demonstration purposes
    void *test_ptr = nullptr;
};

// Initialize the static member variable
Singleton *Singleton::Instance = nullptr;

Singleton::Singleton()
{
    // No need to initialize test_ptr to nullptr here as it is already initialized
}

Singleton::~Singleton()
{
    // No need to free test_ptr here as it is not dynamically allocated
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
    Singleton *instance = Singleton::get_Instance(); // Get the single instance of the class
    std::cout << std::hex << instance << std::endl;

    // No need to delete the instance manually as it will be automatically deleted
    // when the program exits
    // delete Singleton::Instance; // Uncomment if you want to delete the instance manually

    return 0;
}
