#include <iostream>
#include "orderbook.h"
#include <cstdlib>

int main()
{

    OrderBook ob;

    for(int i = 0; i < 10000; i++)
    {
        if(std::rand() % 2 == 0)
        {
            ob.placeBid(std::rand(), std::rand()%100000, std::rand(), std::rand());
        }
        else
        {
            ob.placeAsk(std::rand(), std::rand()%100000, std::rand(), std::rand());
        }
    }
    
    ob.print();

    return 0;
}