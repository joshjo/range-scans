#include <iostream>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <ctime>


using namespace std;


int main(int argc, char** argv) {
    srand (time(NULL));
    int key_domain_size = 1000000;
    int range_size = key_domain_size / 10;

    if (argc != 2) {
        cout << "Please enter a query size, domain, and leaf size: ./reads 1000" << endl;
        return 0;
    }

    for (int i = 0; i < 10; i += 1) {
        int R1 = rand() % key_domain_size;
        cout << R1 << endl;
    }

    return 0;
}
