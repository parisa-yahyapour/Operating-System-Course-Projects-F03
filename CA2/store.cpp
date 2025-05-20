#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>

#include "store.hpp"

using namespace std;

vector<string> ReadCSV(string file_path)
{
    ifstream stream_csv_file(file_path);
    string temp_string;
    vector<string> data_of_csv_file;
    while (getline(stream_csv_file, temp_string))
    {
        data_of_csv_file.push_back(temp_string);
    }
    return data_of_csv_file;
}

void ProductInfo ::SaveProductInfo(string product_name, int product_amount, double product_price, double product_benefit)
{
    name = product_name;
    amount = product_amount;
    price = product_price;
    benefit = product_benefit;
}

void Store::SaveStoreRecords(vector<string> csv_data)
{
    for (int i = 0; i < csv_data.size(); i++)
    {
        ProductInfo temp;
        istringstream line(csv_data[i]);
        string token, name, type;
        int amount;
        double price;
        getline(line, token, ',');
        name = token;
        getline(line, token, ',');
        price = stod(token);
        getline(line, token, ',');
        amount = stoi(token);
        getline(line, token, ',');
        type = token;
        type.pop_back();
        temp.SaveProductInfo(name, amount, price, 0);
        if (type == INPUT_TYPE)
        {
            input_records.push_back(temp);
        }
        else
        {
            output_records.push_back(temp);
        }
    }
}

void Store::SaveStoreName(string path)
{
    int start;
    for (int i = path.size() - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            start = i + 1;
            break;
        }
    }
    name = path.substr(start, path.size() - start);
}

void Store::CalculateProductBenefit()
{
    for (int z = 0; z < wanted.size(); z++)
    {
        string product = wanted[z].GetName();
        cout << YELLOW << STORE_PATH << RESET << "Start calculating benefit in store " << name << " for product " << product << endl;
        for (int i = 0; i < output_records.size(); i++)
        {
            if (output_records[i].GetName() == product)
            {
                for (int j = 0; j < input_records.size(); j++)
                {
                    if (output_records[i].GetAmount() > input_records[j].GetAmount() &&
                        input_records[j].GetName() == product)
                    {
                        output_records[i].Decrease(input_records[j].GetAmount());
                        wanted[z].IncreaseBenefit(input_records[j].GetAmount() *
                                                  (output_records[i].GetPrice() - input_records[j].GetPrice()));
                        input_records[j].Empty();
                    }
                    else if (output_records[i].GetAmount() <= input_records[j].GetAmount() &&
                             input_records[j].GetName() == product)
                    {
                        wanted[z].IncreaseBenefit(output_records[i].GetAmount() *
                                                  (output_records[i].GetPrice() - input_records[j].GetPrice()));
                        input_records[j].Decrease(output_records[i].GetAmount());
                        break;
                    }
                }
            }
        }
        cout << YELLOW << STORE_PATH << RESET << "Benefit in store " << name << " for product "
             << product << " is " << wanted[z].GetBenefit() << endl;
    }
}

void Store::CalculateRemainder()
{
    for (int z = 0; z < wanted.size(); z++)
    {
        string product = wanted[z].GetName();
        cout << YELLOW << STORE_PATH << RESET << "Start calculating remainder of " << product << " in store " << name << endl;
        for (int i = 0; i < input_records.size(); i++)
        {
            if (input_records[i].GetName() == product)
            {
                wanted[z].IncreaseAmount(input_records[i].GetAmount());
                wanted[z].IncreasePrice(input_records[i].GetPrice() * input_records[i].GetAmount());
            }
        }
        cout << YELLOW << STORE_PATH << RESET << "Remainder in store " << name << " for product " << product
             << ": amount=" << wanted[z].GetAmount() << " price=" << wanted[z].GetPrice() << endl;
    }
}

void Store::SendToProduct()
{
    for (int i = 0; i < wanted.size(); i++)
    {
        string path_pipe = NAMEDPIPE_PATH + wanted[i].GetName();
        int fd = open(path_pipe.c_str(), O_WRONLY);
        if (fd == -1)
        {
            perror("error");
            exit(1);
        }
        cout << YELLOW << STORE_PATH << RESET << "Opening fd for writing to " << path_pipe << " in store " << name << endl;
        string message = to_string(wanted[i].GetAmount()) + "," + to_string(wanted[i].GetPrice()) + MESSAGE_DELIMITER;
        int nd = write(fd, message.c_str(), message.size() + 1);
        cout << YELLOW << STORE_PATH << RESET << "Sending to " << path_pipe << " in store " << name << " has been done" << endl;
        usleep(10000);
        close(fd);
    }
}

void Store::SeparateChoices(string choices)
{
    vector<string> products;
    istringstream line(choices);
    string temp;
    while (getline(line, temp, ','))
    {
        ProductInfo sample;
        sample.SaveProductInfo(temp, 0, 0, 0);
        wanted.push_back(sample);
    }
}
void Store::SendBenefitToMain(int fd)
{
    string benefit_info = "";
    for (int i = 0; i < wanted.size(); i++)
    {
        cout << YELLOW << STORE_PATH << RESET << "Start sending benefit " << wanted[i].GetName()
             << " in store " << name << " to main" << endl;
        benefit_info += wanted[i].GetName() + "," + to_string(wanted[i].GetBenefit()) + MESSAGE_DELIMITER;
    }
    write(fd, benefit_info.c_str(), benefit_info.size());
    usleep(10000);
    cout << YELLOW << STORE_PATH << RESET << "Store " << name << " has finished sending data tp main" << endl;
}

int main(int argc, char const *argv[])
{
    cout << YELLOW << STORE_PATH << RESET << "Execl for store was successful" << endl;
    Store new_store;
    int read_fd = stoi(argv[0]);
    int write_fd = stoi(argv[1]);
    new_store.read_fd_main = read_fd;
    new_store.write_fd_main = write_fd;
    char buffer[1024];
    int num = read(new_store.read_fd_main, buffer, sizeof(buffer));
    if (num > 0)
    {
        buffer[num] = '\0';
    }
    string path = buffer;
    path.pop_back();
    cout << YELLOW << STORE_PATH << RESET << "Store path is recieved successfully: " << path << endl;
    new_store.SaveStoreName(path);
    cout << YELLOW << STORE_PATH << RESET << "Start reading from csv in store: " << new_store.GetName() << endl;
    vector<string> csv_data = ReadCSV(path);
    cout << YELLOW << STORE_PATH << RESET << "Reading from csv in store: " << new_store.GetName() << " has finished successfully" << endl;
    new_store.SaveStoreRecords(csv_data);

    char buffer2[1024];
    int num2 = read(new_store.read_fd_main, buffer2, sizeof(buffer2));
    if (num2 > 0)
    {
        buffer2[num2] = '\0';
    }
    string choice = buffer2;
    choice.pop_back();
    cout << YELLOW << STORE_PATH << RESET << "Choice of user has been recieved in store: " << new_store.GetName() << " choices are " << choice << endl;
    new_store.SeparateChoices(choice);
    new_store.CalculateProductBenefit();
    new_store.CalculateRemainder();
    cout << YELLOW << STORE_PATH << RESET << "Start sending remainder in store " << new_store.GetName() << " to product " << choice << endl;
    new_store.SendToProduct();
    new_store.SendBenefitToMain(new_store.write_fd_main);
    cout << YELLOW << STORE_PATH << RESET << "Store " << new_store.GetName() << " has finished its job" << endl;
    return 0;
}
