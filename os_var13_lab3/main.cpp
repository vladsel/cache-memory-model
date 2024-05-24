#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

// ����������� ������� ���'�� �� ���-���'��
size_t mainMemorySize = 50; // ����� ������� ���'�� (1 : 10)
vector<long> mainMemory(mainMemorySize, -1);

size_t cacheSize = mainMemorySize / 10; // ����� ���-���'�� (1 : 10)
unordered_map<long, pair<long, size_t>> cache;

// ������� ��� ��������� ����� ���'��
void printMemoryMap() {
    cout << "Memory Map:" << endl;
    // �������� ���� ������� ���'��
    cout << "Main Memory:" << endl;
    for (size_t i = 0; i < mainMemory.size(); i++) {
        if (mainMemory[i] == -1) {
            ;
            cout << "Block " << i << ":\tData: -1" << endl;
        }
        else
            cout << "Block " << i << ":\tData: " << mainMemory[i] << endl;
    }
    // ��������� ������ ���'�� � ����
    cout << "Cache Memory:" << endl;
    for (const auto &entry : cache) {
        long physicalAddr = entry.first;
        pair<long, size_t> data = entry.second;
        cout << "Address " << physicalAddr << ":\tData: " <<
            data.first << ", \tOld label: " << data.second << endl; // processId = data
    }
}

// ������� ��� ������������ ��������� ������ � �������
long virtualToPhysical(long physicalAddr) {
    return physicalAddr;
}

// ������� ��� ���� ������ ���-���'��
void changeCacheSize(size_t newCacheSize) {
    cache.clear();
    // ������� ����� ���-���'��
    if (newCacheSize > 0) {
        cache.reserve(newCacheSize);
        cout << "Cache size changed to: " << newCacheSize << endl;
    }
    else
        cout << "Incorrect cache size for changing" << endl;
    cacheSize = newCacheSize;
}

// ������� ��� ������������ ����������� (���������� ����������� �������� � ����)
void updateUsage() {
    for (auto &entry : cache)
        entry.second.second++;
}

// ������� ��� �������� ����� � ����
void replaceCacheData(long physicalAddr, long data) {
    cout << "Searching the oldest data in cache for replacing..." << endl;
    if (physicalAddr >= 0) {
        // ��������� ������� ������ �������
        auto oldest = cache.begin();
        for (auto it = cache.begin(); it != cache.end(); it++)
            if (it->second.second > oldest->second.second)
                oldest = it;
        // �������� ���
        cout << "Replacing data in main memory at address: " << oldest->first << endl;
        mainMemory[oldest->first] = oldest->second.first;
        cout << "Data modified in main memory at: "
            << oldest->first << " with data "<< oldest->second.first << endl;
        // ������ ��� ���
        long addr = physicalAddr;
        if (cache.find(physicalAddr) != cache.end())
            addr = oldest->first;
        cache.erase(oldest);
        cache[addr] = make_pair(data, 0);
        cout << "New data added to cache at address: " << addr << " with data " << data << endl;
        //printMemoryMap();
        // ��������� ������ ������������
        updateUsage();
    }
    else
        cout << "Incorrect physicalAddr replaceCacheData" << endl;
}

// ������� ��� ��������� ����� � ���
void addToCache(long physicalAddr, long data) {
    if (physicalAddr >= 0) {
        // �������� ������ ����
        if (cache.size() >= cacheSize) {
            // ��� ����������, ������� �������� �������� �����
            cout << "Cache is full. Replacing data..." << endl;
            replaceCacheData(physicalAddr, data);
        }
        else {
            // ��� �� �� ����������, ������ ������ ���
            cache[physicalAddr] = make_pair(data, 0);
            updateUsage();
            cout << "Data added to cache at address: " << physicalAddr << " with data " << data << endl;
        }
        //printMemoryMap();
    }
    else
        cout << "Incorrect physicalAddr addToCache" << endl;
}

// ������� ��� ���������� ����� � ����
long read(long virtualAddr) {
    if (virtualAddr >= 0 && virtualAddr < mainMemorySize) {
        long physicalAddr = virtualToPhysical(virtualAddr);
        // �������� �� ��� � � ����
        auto it = cache.find(physicalAddr);
        if (it != cache.end()) {
            // ��� �������� � ����
            cout << "[R] Data found in cache at address: "
                << physicalAddr << " with data " << it->second.first << endl;
            //printMemoryMap();
            return it->second.first;
        }
        else {
            // ��� ������ � ����, ���� ������� �� � ������� ���'��
            long data = mainMemory[physicalAddr];
            cout << "[R] Data not found in cache.\n[R] Reading from main memory at address: "
                << physicalAddr << " with data " << data << endl;
            if (data != -1) {
                //printMemoryMap();
                // ��������� ����� � ���
                cout << "[R] Data found in cache.\n[R] Adding data to cache..." << endl;
                addToCache(physicalAddr, data);
                return data;
            }
            else {
                cout << "[R] Data not found in main memory. Do nothing." << endl;
                //printMemoryMap();
                return -1; // ��������� -1 � ������� ��������� ����� � ������� ���'��
            }
        }
    }
    else {
        cout << "[R] Incorrect virtualAddr read (mainMemorySize)" << endl;
        return -1; // ��������� -1 � ������� ���������� ������
    }
}

// ������� ��� ������ ����� � ���
void write(long virtualAddr, long data) {
    if (virtualAddr >= 0 && virtualAddr < mainMemorySize) {
        long physicalAddr = virtualToPhysical(virtualAddr);
        // �������� �� ��� ��� � � ����
        auto it = cache.find(physicalAddr);
        if (it != cache.end()) {
            // ��� �������� � ����, ��������� �� ��������
            cout << "[W] Data found in cache at address: " << physicalAddr << " with "
                << it->second.first << ". Updating with data " << data << endl;
            replaceCacheData(physicalAddr, data);
        }
        else {
            // ��� ������ � ����, ���� ������ ��
            cout << "[W] Data not found in cache at address: " << physicalAddr << " with data " << data << endl;
            cout << "[W] Adding data to main memory..." << endl;
            // ����������, �� ��� � � ������� ���'��
            if (mainMemory[physicalAddr] != -1) {
                // ��� �������� � ������� ���'��, ������ �� �� ����
                cout << "[W] Data found in main memory at address: "
                    << physicalAddr << " with data " << mainMemory[physicalAddr] << endl;
                cout << "Adding to cache..." << endl;
                addToCache(physicalAddr, data);
            }
            else {
                // ��� ������ � ������� ���'��, �������� �� ������������� ����
                cout << "[W] Data not found in main memory.\n[W] Writing directly to main memory at address: "
                    << physicalAddr << " with data " << data << endl;
                mainMemory[physicalAddr] = data;
                //printMemoryMap();
            }
        }
    }
    else
        cout << "Incorrect virtualAddr write (mainMemorySize)" << endl;
}

// ������� �������� ���'�� ��� �������
void allocateMemoryForProcess(const long processId, const size_t memorySize) { 
    // ����������, �� �������� ����� ���'�� ��� �������� �� �������� ����� �������� ���'��
    if (memorySize > mainMemorySize) {
        cout << "[P] Insufficient memory available" << endl;
        return;
    }
    // ����� ������� ������� ����� ���'��
    long freeIndex = -1;
    for (long i = 0; i < mainMemorySize; i++)
        if (mainMemory[i] == -1) {
            freeIndex = i;
            break;
        }
    // ��������, �� ���� �������� ����� ������� ���'��
    if (freeIndex == -1) {
        cout << "[P] No free memory available, making rewrite..." << processId << endl;
        // ����� ��� ����������� ���'�� ���������� � ���������� ����� ������� ���'��
        for (long i = 0; i < memorySize; i++)
            write(i, processId);
    }
    else { // ����� ����� ������� � ����� ������� ������� ���'��
        long countBlock = 0;
        for (long i = freeIndex; i < mainMemorySize; i++) {
            if (i == mainMemorySize || countBlock >= memorySize)
                break;
            write(i, processId);
            countBlock++;
        }
        if (countBlock < memorySize) { // ³��� ����� ����������, ���������� ��������� � �������
            cout << "[P] All free blocks are filled, making rewrite from the start..." << endl;
            for (long i = 0; i < memorySize - countBlock; i++)
                write(i, processId);
        }
    }
    cout << "[P] Memory allocated for process " << processId << " with size " << memorySize << endl;
    printMemoryMap();
}

// ������� ��� ������� ���'�� ��������
void readMemoryByProcess(const long processId, const size_t memorySize) { // processId = data
    // ����������, �� �������� ����� ���'�� ��� �������� �� �������� ����� �������� ���'��
    if (memorySize > mainMemorySize) {
        cout << "[P] Insufficient memory available" << endl;
        return;
    }
    // ����� �������� ����� ��� ������� ��������
    bool flag = true;
    for (long i = 0; i < mainMemorySize; i++)
        if (mainMemory[i] == processId) {
            flag = false;
            cout << "[P] A process already exists with ID: " << processId << endl;
            for (long j = i; j < i + memorySize; j++) {
                // ���������� ����������� ��� ����� �� ��� ������� ���'��
                // ���������� ����������� ���� � ����� �������� �� ����� ���
                if (j == mainMemorySize || mainMemory[i] != processId)
                    break;
                read(j);
            }
            break;
        }
    if (flag) // ������� ��� ��������� ������� ����� ���������� � ���������� ����� ������� ���'��
        for (long i = 0; i < memorySize; i++)
            read(i);
    printMemoryMap();
}

// ������� ��� ��������� ���'��, ��������������� ��� ��������� �������
void freeMemoryForProcess(const long processId) {
    // ���������� ���'�� �������
    for (long i = 0; i < mainMemorySize; i++)
        if (mainMemory[i] == processId)
            mainMemory[i] = -1;
    cout << "[P] Memory freed for process with ID: " << processId << endl;
    printMemoryMap();
}

void printMenu() {
    cout << "Menu: (processId = data)" << endl;
    cout << "1. Write data" << endl;
    cout << "2. Read data" << endl;
    cout << "3. Free data" << endl;
    cout << "4. Change cache size" << endl;
    cout << "5. Clear cache" << endl;
    cout << "6. Clear main memory" << endl;
    cout << "7. Print memory map" << endl;
    cout << "8. Exit" << endl;
}

void startModel() {
    // ����������� ����
    cout << "Main memory size changed to: " << mainMemorySize << endl;
    changeCacheSize(cacheSize);
    while (true) {
        printMenu();
        int choice = 0;
        cout << "Your choice: ";
        cin >> choice;
        while (choice < 1 || choice > 8) {
            cout << "Error, your choice: ";
            cin >> choice;
        }
        long sw1 = 0;
        size_t sw2 = 0;
        switch (choice) {
            case 1:
                cout << "\nEnter the process ID and the number of blocks for it" << endl;
                cout << "process ID: ";
                cin >> sw1;
                while (sw1 < 1) {
                    cout << "Error, process ID: ";
                    cin >> sw1;
                }
                cout << "number of blocks: ";
                cin >> sw2;
                while (sw2 < 1) {
                    cout << "Error, number of blocks: ";
                    cin >> sw2;
                }
                allocateMemoryForProcess(sw1, sw2);
                break;
            case 2:
                cout << "\nEnter the process ID and the number of blocks for it" << endl;
                cout << "process ID: ";
                cin >> sw1;
                while (sw1 < 1) {
                    cout << "Error, process ID: ";
                    cin >> sw1;
                }
                cout << "number of blocks: ";
                cin >> sw2;
                while (sw2 < 1) {
                    cout << "Error, number of blocks: ";
                    cin >> sw2;
                }
                readMemoryByProcess(sw1, sw2);
                break;
            case 3:
                cout << "\nEnter the process ID" << endl;
                cout << "process ID: ";
                cin >> sw1;
                while (sw1 < 1) {
                    cout << "Error, process ID: ";
                    cin >> sw1;
                }
                freeMemoryForProcess(sw1);
                break;
            case 4:
                cout << "\nEnter the new cache size" << endl;
                cout << "cache size: ";
                cin >> sw2;
                while (sw2 < 1) {
                    cout << "Error, cache size: ";
                    cin >> sw2;
                }
                changeCacheSize(sw2);
                break;
            case 5:
                cout << "\nCache has been cleared" << endl;
                cache.clear();
                cache.reserve(mainMemorySize / 10);
                printMemoryMap();
                break;
            case 6:
                cout << "\nMain memory has been cleared" << endl;
                mainMemory.clear();
                mainMemory.assign(mainMemorySize, -1);
                printMemoryMap();
                break;
            case 7:
                printMemoryMap();
                break;
            case 8:
                cout << "\nProgram is finished" << endl;
                return;
            default:
                break;
        }
    }
}

int main() {
    try {
        startModel();
    }
    catch (const std::exception &ex) {
        std::cerr << "main error: " << ex.what() << endl;
        return 1;
    }
    return 0;
}
