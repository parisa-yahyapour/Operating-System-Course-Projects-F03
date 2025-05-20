#ifndef _PRODUCT_HPP_
#define _PRODUCT_HPP_

#include <string>
#include <vector>

using namespace std;

class Product
{
private:
    int remain_amount;
    double remain_price;
    string name;

public:
    Product()
    {
        remain_amount = 0;
        remain_price = 0;
    }
    int read_main_fd;
    int write_main_fd;
    void SaveProductName(string name_product) { name = name_product; };
    void ReadFromStores(int num_stores);
    void SendToMain();
    string GetName() { return name; };
};

#endif