#ifndef _STORE_HPP_
#define _STORE_HPP_

#include <string>
#include <vector>
#include "statics.hpp"

using namespace std;

class ProductInfo
{
private:
    string name;
    double price;
    int amount;
    double benefit;

public:
    void SaveProductInfo(string product_name, int product_amount, double product_price, double product_benefit);
    string GetName() { return name; };
    int GetAmount() { return amount; };
    double GetPrice() { return price; };
    void Decrease(int amount_d) { amount -= amount_d; };
    void Empty() { amount = 0; };
    void IncreaseBenefit(double new_benefit) { benefit += new_benefit; };
    double GetBenefit() { return benefit; };
    void IncreaseAmount(int new_amount) { amount += new_amount; };
    void IncreasePrice(double new_amount) { price += new_amount; };
};

class Store
{
private:
    string name;
    vector<ProductInfo> input_records;
    vector<ProductInfo> output_records;
    vector<ProductInfo> wanted;

public:
    int read_fd_main;
    int write_fd_main;
    void SaveStoreName(string path);
    void SaveStoreRecords(vector<string> records);
    string GetName() { return name; };
    void CalculateProductBenefit();
    void CalculateRemainder();
    void SendToProduct();
    void SeparateChoices(string choices);
    void SendBenefitToMain(int fd);
};

#endif