/***************************************************************************
 * Description: queue_mapper.cpp
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-01-06 23:07:50
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <iostream>
#include <set>
#include <queue>

class PositionMapper {
private:
    std::set<int> usedIDs;
    std::queue<int> inputQueue;

    // Find an unused ID from 0 to 100
    int findUnusedID() {
        for (int id = 0; id < 100; ++id) {
            if (usedIDs.find(id) == usedIDs.end()) {
                return id;
            }
        }
        std::cout << "No available ID found" << std::endl;
        return -1;
    }

public:
    // Add an input value to the queue and map it to an unused ID
    void addInput(int inputValue) {
        inputQueue.push(inputValue);

        int id = findUnusedID();
        if (id != -1) {
            usedIDs.insert(id);
            std::cout << "Mapped input " << inputValue << " to ID " << id << std::endl;
        }
    }

    // Release an ID and map the next input value in the queue to the released ID
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

    // Add some input values
    mapper.addInput(200);  
    mapper.addInput(100);  
    mapper.addInput(300);  
    mapper.addInput(101);  
    mapper.addInput(302);  

    // Release some IDs
    mapper.releaseID(2);
    mapper.releaseID(4);

    // Add more input values
    mapper.addInput(303);  
    mapper.addInput(304);  
    mapper.addInput(305);  
    mapper.addInput(306);  

    return 0;
}
