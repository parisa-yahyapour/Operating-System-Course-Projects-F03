#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>

#include "product.hpp"
#include "statics.hpp"
#include "types.hpp"

using namespace std;

vector<string> SplitMessage(string message)
{
    istringstream line(message);
    vector<string> messages;
    string temp;
    while (getline(line, temp, DELIMITER))
    {
        messages.push_back(temp);
    }
    return messages;
}

vector<RemainderInfo> ExtractRemainder(const vector<string> &messages)
{
    vector<RemainderInfo> remainders;
    for (int i = 0; i < messages.size(); i++)
    {
        istringstream line(messages[i]);
        string temp;
        RemainderInfo sample;
        getline(line, temp, SEPERATOR);
        sample.amount = stoi(temp);
        getline(line, temp, SEPERATOR);
        sample.price = stod(temp);
        remainders.push_back(sample);
    }
    return remainders;
}

void Product::ReadFromStores(int num_stores)
{
    string path = NAMEDPIPE_PATH + name;
    cout << GREEN << PRODUCT_PATH << RESET << "Opening fd for reading from stores in product: " << name << endl;
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("error");
        close(fd);
    }
    while (num_stores != 0)
    {
        char buffer[1024];
        int num = read(fd, buffer, sizeof(buffer));
        if (num > 0)
        {
            buffer[num] = '\0';
            string message = buffer;
            cout << GREEN << PRODUCT_PATH << RESET << "In product: " << name << " recieved " << message << endl;
            vector<string> messages = SplitMessage(message);
            vector<RemainderInfo> remainders = ExtractRemainder(messages);
            for (int i = 0; i < remainders.size(); i++)
            {
                remain_amount += remainders[i].amount;
                remain_price += remainders[i].price;
            }
            num_stores -= remainders.size();
        }
    }
    cout << GREEN << PRODUCT_PATH << RESET<< "Recieving from stores in product: " << name << " has finished" << endl;
    close(fd);
}

void Product::SendToMain()
{
    cout << GREEN << PRODUCT_PATH << RESET<< "Start sending remainder data of product: " << name << " to main process" << endl;
    string message = name + "," + to_string(remain_amount) + "," + to_string(remain_price) + MESSAGE_DELIMITER;
    write(write_main_fd, message.c_str(), message.size());
    usleep(10000);
}

int main(int argc, char const *argv[])
{
    cout << GREEN << PRODUCT_PATH << RESET << "Execl for product was successful" << endl;
    Product new_product;
    int read_fd = stoi(argv[0]);
    int write_fd = stoi(argv[1]);
    new_product.read_main_fd = read_fd;
    new_product.write_main_fd = write_fd;
    char buffer[1024];
    int num = read(new_product.read_main_fd, buffer, sizeof(buffer));
    if (num > 0)
    {
        buffer[num] = '\0';
    }
    string name = buffer;
    name.pop_back();
    new_product.SaveProductName(name);
    cout << GREEN << PRODUCT_PATH << RESET << "Product name is recieved successfully: " << new_product.GetName() << endl;

    char buffer2[1024];
    int num2 = read(new_product.read_main_fd, buffer2, sizeof(buffer2));
    if (num2 > 0)
    {
        buffer2[num2] = '\0';
    }
    string data = buffer2;
    data.pop_back();
    cout << GREEN << PRODUCT_PATH << RESET<< "Number of stores has been recieved successfully in product: " << name << endl;
    int num_stores = stoi(data);

    char buffer3[1024];
    int num3 = read(new_product.read_main_fd, buffer3, sizeof(buffer3));
    if (num3 > 0)
    {
        buffer3[num3] = '\0';
    }
    string status = buffer3;
    cout << GREEN << PRODUCT_PATH << RESET << "Status in product " << new_product.GetName() << " is " << status << endl;

    if (status == START_SIGNAL)
    {
        cout << GREEN << PRODUCT_PATH << RESET<< "Starting recieving from stores in product: " << name << endl;
        new_product.ReadFromStores(num_stores);
        new_product.SendToMain();
    }
    else
    {
        cout<< GREEN << PRODUCT_PATH << RESET << "Nothing to recieve in product " << new_product.GetName() << endl;
    }
    cout << GREEN << PRODUCT_PATH << RESET << "Product: " << new_product.GetName() << " has finished its job" << endl;
    return 0;
}
