#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <string>

class Order
{
public:

    int id;
    int ownerId;
    int size;
    int filledSize;
    int limitPrice;
    int entryTime;
    std::string status;

    Order(int _id, int _ownerId, int _size, int _limitPrice);
    void fill(int amount);
    void cancel();
    bool isFilled();
    bool isCanceled();
    void print();
};

struct Limit
{
    int limitPrice;
    int size;
    std::vector<Order*> orders;
};

bool operator==(const Limit& l, const Limit& r);

std::vector<Order*>::iterator find(std::vector<Order*>::iterator begin, std::vector<Order*>::iterator end, Order* order);

std::vector<int> keys(std::map<int, Limit*> map);


class OrderBookSide
{
    std::map<int, Limit*> limits;
public:
    void printInOrder();
    void printReverse();
    void add(Order* order);
    void cancel(Order* order);
    void execute(int limitPrice, int size);
    Limit* getMaxLimit();
    Limit* getMinLimit();

};

class OrderBook
{
    OrderBookSide buySide;
    OrderBookSide sellSide;

public:
    void print();
    void placeBid(int id, int limitPrice, int size, int ownerId);
    void placeAsk(int id, int limitPrice, int size, int ownerId);

};

#endif // ORDERBOOK_H