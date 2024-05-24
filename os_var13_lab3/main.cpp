#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

// Ініціалізація основної пам'яті та кеш-пам'яті
size_t mainMemorySize = 50; // Розмір основної пам'яті (1 : 10)
vector<long> mainMemory(mainMemorySize, -1);

size_t cacheSize = mainMemorySize / 10; // Розмір кеш-пам'яті (1 : 10)
unordered_map<long, pair<long, size_t>> cache;

// Функція для виведення карти пам'яті
void printMemoryMap() {
    cout << "Memory Map:" << endl;
    // Виводимо стан основної пам'яті
    cout << "Main Memory:" << endl;
    for (size_t i = 0; i < mainMemory.size(); i++) {
        if (mainMemory[i] == -1) {
            ;
            cout << "Block " << i << ":\tData: -1" << endl;
        }
        else
            cout << "Block " << i << ":\tData: " << mainMemory[i] << endl;
    }
    // Виведення ділянок пам'яті з кешу
    cout << "Cache Memory:" << endl;
    for (const auto &entry : cache) {
        long physicalAddr = entry.first;
        pair<long, size_t> data = entry.second;
        cout << "Address " << physicalAddr << ":\tData: " <<
            data.first << ", \tOld label: " << data.second << endl; // processId = data
    }
}

// Функція для перетворення віртуальної адреси у фізичну
long virtualToPhysical(long physicalAddr) {
    return physicalAddr;
}

// Функція для зміни розміру кеш-пам'яті
void changeCacheSize(size_t newCacheSize) {
    cache.clear();
    // Змінюємо розмір кеш-пам'яті
    if (newCacheSize > 0) {
        cache.reserve(newCacheSize);
        cout << "Cache size changed to: " << newCacheSize << endl;
    }
    else
        cout << "Incorrect cache size for changing" << endl;
    cacheSize = newCacheSize;
}

// Функція для встановлення модифікації (визначення найстарішого елемента в кеші)
void updateUsage() {
    for (auto &entry : cache)
        entry.second.second++;
}

// Функція для заміщення даних в кеші
void replaceCacheData(long physicalAddr, long data) {
    cout << "Searching the oldest data in cache for replacing..." << endl;
    if (physicalAddr >= 0) {
        // Знаходимо найбільш старий елемент
        auto oldest = cache.begin();
        for (auto it = cache.begin(); it != cache.end(); it++)
            if (it->second.second > oldest->second.second)
                oldest = it;
        // Замінюємо дані
        cout << "Replacing data in main memory at address: " << oldest->first << endl;
        mainMemory[oldest->first] = oldest->second.first;
        cout << "Data modified in main memory at: "
            << oldest->first << " with data "<< oldest->second.first << endl;
        // Додаємо нові дані
        long addr = physicalAddr;
        if (cache.find(physicalAddr) != cache.end())
            addr = oldest->first;
        cache.erase(oldest);
        cache[addr] = make_pair(data, 0);
        cout << "New data added to cache at address: " << addr << " with data " << data << endl;
        //printMemoryMap();
        // Оновлюємо ознаки використання
        updateUsage();
    }
    else
        cout << "Incorrect physicalAddr replaceCacheData" << endl;
}

// Функція для додавання даних в кеш
void addToCache(long physicalAddr, long data) {
    if (physicalAddr >= 0) {
        // Перевірка розміру кешу
        if (cache.size() >= cacheSize) {
            // Кеш заповнений, потрібно виконати заміщення даних
            cout << "Cache is full. Replacing data..." << endl;
            replaceCacheData(physicalAddr, data);
        }
        else {
            // Кеш ще не заповнений, просто додаємо дані
            cache[physicalAddr] = make_pair(data, 0);
            updateUsage();
            cout << "Data added to cache at address: " << physicalAddr << " with data " << data << endl;
        }
        //printMemoryMap();
    }
    else
        cout << "Incorrect physicalAddr addToCache" << endl;
}

// Функція для зчитування даних з кешу
long read(long virtualAddr) {
    if (virtualAddr >= 0 && virtualAddr < mainMemorySize) {
        long physicalAddr = virtualToPhysical(virtualAddr);
        // Перевірка чи дані є в кеші
        auto it = cache.find(physicalAddr);
        if (it != cache.end()) {
            // Дані знайдено в кеші
            cout << "[R] Data found in cache at address: "
                << physicalAddr << " with data " << it->second.first << endl;
            //printMemoryMap();
            return it->second.first;
        }
        else {
            // Дані відсутні в кеші, тому зчитуємо їх з основної пам'яті
            long data = mainMemory[physicalAddr];
            cout << "[R] Data not found in cache.\n[R] Reading from main memory at address: "
                << physicalAddr << " with data " << data << endl;
            if (data != -1) {
                //printMemoryMap();
                // Додавання даних в кеш
                cout << "[R] Data found in cache.\n[R] Adding data to cache..." << endl;
                addToCache(physicalAddr, data);
                return data;
            }
            else {
                cout << "[R] Data not found in main memory. Do nothing." << endl;
                //printMemoryMap();
                return -1; // Повертаємо -1 у випадку відсутності даних в основній пам'яті
            }
        }
    }
    else {
        cout << "[R] Incorrect virtualAddr read (mainMemorySize)" << endl;
        return -1; // Повертаємо -1 у випадку некоректної адреси
    }
}

// Функція для запису даних в кеш
void write(long virtualAddr, long data) {
    if (virtualAddr >= 0 && virtualAddr < mainMemorySize) {
        long physicalAddr = virtualToPhysical(virtualAddr);
        // Перевірка чи дані вже є в кеші
        auto it = cache.find(physicalAddr);
        if (it != cache.end()) {
            // Дані знайдено в кеші, оновлюємо їх значення
            cout << "[W] Data found in cache at address: " << physicalAddr << " with "
                << it->second.first << ". Updating with data " << data << endl;
            replaceCacheData(physicalAddr, data);
        }
        else {
            // Дані відсутні в кеші, тому додаємо їх
            cout << "[W] Data not found in cache at address: " << physicalAddr << " with data " << data << endl;
            cout << "[W] Adding data to main memory..." << endl;
            // Перевіряємо, чи дані є в основній пам'яті
            if (mainMemory[physicalAddr] != -1) {
                // Дані знайдено в основній пам'яті, додаємо їх до кешу
                cout << "[W] Data found in main memory at address: "
                    << physicalAddr << " with data " << mainMemory[physicalAddr] << endl;
                cout << "Adding to cache..." << endl;
                addToCache(physicalAddr, data);
            }
            else {
                // Дані відсутні в основній пам'яті, записуємо їх безпосередньо туди
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

// Функція виділення пам'яті для процесу
void allocateMemoryForProcess(const long processId, const size_t memorySize) { 
    // Перевіряємо, чи вказаний розмір пам'яті для виділення не перевищує розмір доступної пам'яті
    if (memorySize > mainMemorySize) {
        cout << "[P] Insufficient memory available" << endl;
        return;
    }
    // Пошук першого вільного блоку пам'яті
    long freeIndex = -1;
    for (long i = 0; i < mainMemorySize; i++)
        if (mainMemory[i] == -1) {
            freeIndex = i;
            break;
        }
    // Перевірка, чи була знайдена вільна область пам'яті
    if (freeIndex == -1) {
        cout << "[P] No free memory available, making rewrite..." << processId << endl;
        // Запис при переповненій пам'яті відбувається з початкових блоків основної пам'яті
        for (long i = 0; i < memorySize; i++)
            write(i, processId);
    }
    else { // Запис даних процесу у вільну область основної пам'яті
        long countBlock = 0;
        for (long i = freeIndex; i < mainMemorySize; i++) {
            if (i == mainMemorySize || countBlock >= memorySize)
                break;
            write(i, processId);
            countBlock++;
        }
        if (countBlock < memorySize) { // Вільні блоки закінчились, відбувається перезапиз з початку
            cout << "[P] All free blocks are filled, making rewrite from the start..." << endl;
            for (long i = 0; i < memorySize - countBlock; i++)
                write(i, processId);
        }
    }
    cout << "[P] Memory allocated for process " << processId << " with size " << memorySize << endl;
    printMemoryMap();
}

// Функція для читання пам'яті процесом
void readMemoryByProcess(const long processId, const size_t memorySize) { // processId = data
    // Перевіряємо, чи вказаний розмір пам'яті для виділення не перевищує розмір доступної пам'яті
    if (memorySize > mainMemorySize) {
        cout << "[P] Insufficient memory available" << endl;
        return;
    }
    // Пошук існуючих блоків для читання процесом
    bool flag = true;
    for (long i = 0; i < mainMemorySize; i++)
        if (mainMemory[i] == processId) {
            flag = false;
            cout << "[P] A process already exists with ID: " << processId << endl;
            for (long j = i; j < i + memorySize; j++) {
                // Зчитування зупиняється при виході за межі основної пам'яті
                // Зчитування зупиняється якщо в блоці знайдено не шукані дані
                if (j == mainMemorySize || mainMemory[i] != processId)
                    break;
                read(j);
            }
            break;
        }
    if (flag) // Читання при відсутності шуканих даних починається з початкових блоків основної пам'яті
        for (long i = 0; i < memorySize; i++)
            read(i);
    printMemoryMap();
}

// Функція для звільнення пам'яті, використовується при завершенні процесу
void freeMemoryForProcess(const long processId) {
    // Вивільнення пам'яті процесу
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
    // Ініціалізація кешу
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
