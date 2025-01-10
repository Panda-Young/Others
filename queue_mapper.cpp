/* **************************************************************
 * @Description: position mapper
 * @Date: 2024-05-20 13:47:42
 * @Version: 0.1.0
 * @Author: error: git config user.email & please set dead value or install git
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved. 
 **************************************************************/

#include <iostream>
#include <set>
#include <queue>

class PositionMapper {
private:
    std::set<int> usedIDs;
    std::queue<int> inputQueue;

    int findUnusedID() {
        for (int id = 0; id < 6; ++id) {
            if (usedIDs.find(id) == usedIDs.end()) {
                return id;
            }
        }
        std::cout << "No available ID found" << std::endl;
        return -1;
    }

public:
    void addInput(int inputValue) {
        inputQueue.push(inputValue);

        int id = findUnusedID();
        if (id != -1) {
            usedIDs.insert(id);
            std::cout << "Mapped input " << inputValue << " to ID " << id << std::endl;
        }
    }

    void releaseID(int id) {
        usedIDs.erase(id);

        if (!inputQueue.empty()) {
            int inputValue = inputQueue.front();
            inputQueue.pop();

            // usedIDs.insert(id);
            std::cout << "Mapped release " << inputValue << " ID " << id << std::endl;
        }
    }
};

int main() {
    PositionMapper mapper;

    // 添加一些输入
    // for (int i = 0; i < 3; ++i) {
    //     mapper.addInput(i);
    // }
    mapper.addInput(200);  
    mapper.addInput(100);  
    mapper.addInput(300);  
    mapper.addInput(101);  
    mapper.addInput(302);  

    mapper.releaseID(2);
    mapper.releaseID(4);

    mapper.addInput(303);  
    mapper.addInput(304);  
    mapper.addInput(305);  
    mapper.addInput(306);  

    // 假设我们在某个时刻释放了一些ID

    return 0;
}
