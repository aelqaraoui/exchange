#include "orderbook.h"
#include "assert.h"
#include <ctime>

Order::Order(int _id, int _ownerId, int _size, int _limitPrice):id(_id), ownerId(_ownerId), size(_size), filledSize(0), limitPrice(_limitPrice), entryTime(int(time(0))), status("Open")
{

}

void Order::fill(int amount)
{
    filledSize += amount;
        
    if(filledSize == size)
    {
        status = "Filled";
    }
}

void Order::cancel()
{
    if(status != "Filled")
    {
        status = "Canceled";
    }
    else
    {
        std::cout << "Order is already Filled!" << std::endl; 
    }
}

bool Order::isFilled()
{
    return (status == "Filled");
}

bool Order::isCanceled()
{
    return (status == "Canceled");
}

void Order::print()
{
    std::cout << "Order : " << id << ", Size : " << size << ", FilledSize : " << filledSize << ", Status : " << status << std::endl;
}

bool operator==(const Limit& l, const Limit& r)
{
    bool truth = true;
    if(l.orders.size() == r.orders.size())
    {   
        for(int i = 0; i < l.orders.size(); i++)
        {
            truth = truth && l.orders[i] == r.orders[i];
        }
    }   
    else
    {
        return false;
    }  
    return truth && l.limitPrice == r.limitPrice && l.size == r.size;
}

std::vector<Order*>::iterator find(std::vector<Order*>::iterator begin, std::vector<Order*>::iterator end, Order* order)
{
    std::vector<Order*>::iterator it;
    for(it = begin; it != end; it++)
    {
        if(*it == order)
        {
            return it;
        }
    }
    return it;
}

std::vector<int> keys(std::map<int, Limit*> map)
{
    std::vector<int> keys;
    std::map<int, Limit*>::iterator it;
    for(it = map.begin(); it != map.end(); it++)
    {
        keys.push_back(it->first);
    }
    return keys;
}


void OrderBookSide::printInOrder()
{
    for(std::map<int, Limit*>::iterator it = limits.begin(); it != limits.end(); it++)
    {
        std::cout << "Price : " << it->first << ", Number of orders : " << it->second->orders.size() << std::endl;
        for(int i = 0; i < it->second->orders.size(); i++)
        {
            std::cout << "\t";
            it->second->orders[i]->print();
        }
    }
}

void OrderBookSide::printReverse()
{
    std::vector<int> k = keys(limits);
    for(int i = k.size()-1; i > -1; i--)
    {
        std::cout << "Price : " << limits[k[i]]->limitPrice << ", Number of orders : " << limits[k[i]]->orders.size() << ", Size : " << limits[k[i]]->size << std::endl;
        for(int j = 0; j < limits[k[i]]->orders.size(); j++)
        {
            std::cout << "\t";
            limits[k[i]]->orders[j]->print();
        }
    }
}

void OrderBookSide::add(Order* order)
{
    if(limits.find(order->limitPrice) == limits.end())
    {
        limits[order->limitPrice] = new Limit;
        limits[order->limitPrice]->limitPrice = order->limitPrice;
        limits[order->limitPrice]->size = order->size;
        limits[order->limitPrice]->orders.push_back(order);
    }
    else 
    {
        limits[order->limitPrice]->size += order->size;
        limits[order->limitPrice]->orders.push_back(order);
    }
}

void OrderBookSide::cancel(Order* order)
{
    if(limits.find(order->limitPrice) == limits.end())
    {
        std::cout << "Order not found!" << std::endl;
    }
    else
    {
        std::vector<Order*>::iterator it = find(limits[order->limitPrice]->orders.begin(), limits[order->limitPrice]->orders.end(), order);
        if(it == limits[order->limitPrice]->orders.end())
        {
            std::cout << "Order not found!" << std::endl;
        }
        else
        {
            limits[order->limitPrice]->size -= order->size;
            (*it)->cancel();
            limits[order->limitPrice]->orders.erase(it);
        }
    }
}

void OrderBookSide::execute(int limitPrice, int size)
{
    int filledSize = 0;

    for(int i = 0; i < limits[limitPrice]->orders.size(); i++)
    {
        int orderSize = limits[limitPrice]->orders[i]->size - limits[limitPrice]->orders[i]->filledSize;

        limits[limitPrice]->orders[i]->fill(int(double(orderSize) * double(size) / double(limits[limitPrice]->size)));
        filledSize += int(double(orderSize) * double(size) / double(limits[limitPrice]->size));
        
        if(limits[limitPrice]->orders[i]->isFilled())
        {
            limits[limitPrice]->orders.erase(limits[limitPrice]->orders.begin() + i);
        }
    }

    limits[limitPrice]->orders.erase(
        std::remove_if(
            limits[limitPrice]->orders.begin(), 
            limits[limitPrice]->orders.end(),
            [](Order* o) {return o->isFilled();}
        ), 
        limits[limitPrice]->orders.end());

    limits[limitPrice]->orders[0]->fill(size - filledSize);

    limits[limitPrice]->orders.erase(
        std::remove_if(
            limits[limitPrice]->orders.begin(), 
            limits[limitPrice]->orders.end(),
            [](Order* o) {return o->isFilled();}
        ), 
        limits[limitPrice]->orders.end());

    limits[limitPrice]->size -= size;

    if(limits[limitPrice]->orders.size() == 0)
    {
        limits.erase(limitPrice);
    }
}

Limit* OrderBookSide::getMaxLimit()
{
    std::vector<int> k = keys(limits);
    if(k.size() == 0)
        return NULL;

    return limits[k.at(k.size()-1)];
}

Limit* OrderBookSide::getMinLimit()
{
    std::vector<int> k = keys(limits);
    if(k.size() == 0)
        return NULL;

    return limits[k.at(0)];
}

void OrderBook::print()
{
    sellSide.printReverse();
    std::cout << "****************************************" << std::endl;
    buySide.printReverse();
}

void OrderBook::placeBid(int id, int limitPrice, int size, int ownerId)
{
    Limit* AskLimit = sellSide.getMinLimit();
    int remainingSize = size;

    if(AskLimit == NULL)
    {
        buySide.add(new Order(id, ownerId, remainingSize, limitPrice));
        return;
    }

    while(AskLimit->limitPrice <= limitPrice && remainingSize > 0)
    {
        remainingSize -= std::min(AskLimit->size, remainingSize);
        sellSide.execute(AskLimit->limitPrice, std::min(AskLimit->size, remainingSize));

        AskLimit = sellSide.getMinLimit();

        if(AskLimit == NULL)
            break;
    }

    if(remainingSize > 0)
        buySide.add(new Order(id, ownerId, remainingSize, limitPrice));

}

void OrderBook::placeAsk(int id, int limitPrice, int size, int ownerId)
{
    Limit* BidLimit = buySide.getMaxLimit();
    int remainingSize = size;

    if(BidLimit == NULL)
    {
        sellSide.add(new Order(id, ownerId, remainingSize, limitPrice));
        return;
    }

    while(BidLimit->limitPrice >= limitPrice && remainingSize > 0)
    {
        remainingSize -= std::min(BidLimit->size, remainingSize);
        buySide.execute(BidLimit->limitPrice, std::min(BidLimit->size, remainingSize));

        BidLimit = buySide.getMaxLimit();

        if(BidLimit == NULL)
            break;
    }
    
    if(remainingSize > 0)
        sellSide.add(new Order(id, ownerId, remainingSize, limitPrice));

}


