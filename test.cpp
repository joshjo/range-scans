// discrete_distribution
#include <iostream>
#include <random>
#include "zipf.h"

int main()
{
    srand(100);
    std::random_device rd;
    std::mt19937 gen(100);
    zipf_distribution<> zipf(100000);

    for (int i = 0; i < 20; i++)
        printf("draw %d %d\n", i, zipf(gen));

  return 0;
}
