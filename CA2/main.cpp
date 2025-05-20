#include <iostream>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>
#include <wait.h>
#include "statics.hpp"
#include "types.hpp"

using namespace std;
using namespace std::filesystem;

vector<path> GetFilesPath(string main_path)
{
    vector<path> file_pathes;
    string unwanted = main_path + "/Parts.csv";
    for (const auto &entry : directory_iterator(main_path))
    {
        if (entry.path() != unwanted)
        {
            file_pathes.push_back(entry.path());
        }
    }
    return file_pathes;
}

void print_path(const vector<path> &pathes)
{
    for (int i = 0; i < pathes.size(); i++)
    {
        cout << pathes[i] << endl;
    }
}

void SetStoreInfo(const vector<path> &csv, vector<StoreInfo> &info)
{
    cout << BLUE << MAIN_PATH << RESET << "Saving path of stores and their IDs" << endl;
    for (int i = 0; i < csv.size(); i++)
    {
        info[i].csv_path = csv[i];
        info[i].id = i + 1;
    }
}

void CreateStoreProcess(vector<StoreInfo> &stores)
{
    for (size_t i = 0; i < stores.size(); i++)
    {
        int pipefd[2];
        int pipefd2[2];
        if (pipe(pipefd) == -1 || pipe(pipefd2) == -1)
        {
            cout << "Error in creating unnamed pipe\n";
            exit(1);
        }
        pid_t forkpid = fork();
        if (forkpid == 0)
        {
            cout << BLUE << MAIN_PATH << RESET << "Fork for store " << stores[i].id << " is created successfully" << endl;
            close(pipefd[1]);
            close(pipefd2[0]);
            string read_fd = to_string(pipefd[0]);
            string write_fd = to_string(pipefd2[1]);
            execl(OUT_STORE.c_str(), read_fd.c_str(), write_fd.c_str(), nullptr);
            exit(1);
        }
        else if (forkpid < 0)
        {
            cout << "fork failed\n";
        }
        close(pipefd[0]);
        close(pipefd2[1]);
        string message = (stores[i].csv_path);
        message += MESSAGE_DELIMITER;
        stores[i].read_fd = pipefd2[0];
        stores[i].write_fd = pipefd[1];
        cout << BLUE << MAIN_PATH << RESET << "Sending csv path to its process: " << message << endl;
        write(stores[i].write_fd, message.c_str(), message.size() + 1);
        usleep(10000);
    }
}

void CreateProductProcess(vector<ProductInfo> &products)
{
    for (size_t i = 0; i < products.size(); i++)
    {
        int pipefd[2];
        int pipefd2[2];
        if (pipe(pipefd) == -1 || pipe(pipefd2) == -1)
        {
            cout << "Error in creating unnamed pipe\n";
            exit(1);
        }
        pid_t forkpid = fork();
        if (forkpid == 0)
        {
            cout << BLUE << MAIN_PATH << RESET << "Fork for product " << products[i].name << " is created successfully" << endl;
            close(pipefd[1]);
            close(pipefd2[0]);
            string read_fd = to_string(pipefd[0]);
            string write_fd = to_string(pipefd2[1]);
            execl(OUT_PRODUCT.c_str(), read_fd.c_str(), write_fd.c_str(), nullptr);
            exit(1);
        }
        else if (forkpid < 0)
        {
            cout << "fork failed\n";
        }
        close(pipefd[0]);
        close(pipefd2[1]);
        string message = products[i].name + MESSAGE_DELIMITER;
        products[i].read_fd = pipefd2[0];
        products[i].write_fd = pipefd[1];
        cout << BLUE << MAIN_PATH << RESET << "Sending product name to its process: " << message << endl;
        write(pipefd[1], message.c_str(), message.size() + 1);
        usleep(10000);
    }
}

void Terminate(const vector<StoreInfo> &stores, const vector<ProductInfo> &products)
{
    for (int i = 0; i < stores.size(); i++)
    {
        wait(NULL);
    }
    for (int i = 0; i < products.size(); i++)
    {
        wait(NULL);
    }
    for (int i = 0; i < stores.size(); i++)
    {
        close(stores[i].read_fd);
        close(stores[i].write_fd);
    }
    for (int i = 0; i < products.size(); i++)
    {
        close(products[i].read_fd);
        close(products[i].write_fd);
    }
}

int SearchInProducts(string product_name, const vector<ProductInfo> &products)
{
    for (int i = 0; i < products.size(); i++)
    {
        if (products[i].name == product_name)
        {
            return i;
        }
    }
    return -1;
}

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

vector<BenefitInfo> ExtractBenefit(const vector<string> &messages, const vector<ProductInfo> &products)
{
    vector<BenefitInfo> benefits;
    for (int i = 0; i < messages.size(); i++)
    {
        istringstream line(messages[i]);
        string temp;
        BenefitInfo sample;
        getline(line, temp, SEPERATOR);
        sample.name = temp;
        getline(line, temp, SEPERATOR);
        sample.benefit = stod(temp);
        sample.index = SearchInProducts(sample.name, products);
        benefits.push_back(sample);
    }
    return benefits;
}

vector<RemainderInfo> ExtractRemainder(const vector<string> &messages, const vector<ProductInfo> &products)
{
    vector<RemainderInfo> remainders;
    for (int i = 0; i < messages.size(); i++)
    {
        istringstream line(messages[i]);
        string temp;
        RemainderInfo sample;
        getline(line, temp, SEPERATOR);
        sample.name = temp;
        getline(line, temp, SEPERATOR);
        sample.amount = stoi(temp);
        getline(line, temp, SEPERATOR);
        sample.price = stod(temp);
        sample.index = SearchInProducts(sample.name, products);
        remainders.push_back(sample);
    }
    return remainders;
}

void SaveBenefit(vector<ProductInfo> &products, vector<BenefitInfo> benefits)
{
    for (int i = 0; i < benefits.size(); i++)
    {
        products[benefits[i].index].benefit += benefits[i].benefit;
    }
}

void SaveRemainder(vector<ProductInfo> &products, vector<RemainderInfo> remainders)
{
    for (int i = 0; i < remainders.size(); i++)
    {
        products[remainders[i].index].reaminder_amount = remainders[i].amount;
        products[remainders[i].index].remainder_price = remainders[i].price;
    }
}

void ReadFromStores(const vector<StoreInfo> &stores, vector<ProductInfo> &products)
{
    int num_stores = stores.size();
    cout << BLUE << MAIN_PATH << RESET << "Start Reading from stores in main process" << endl;
    for (int i = 0; i < stores.size(); i++)
    {
        char buffer[1024];
        int num = read(stores[i].read_fd, buffer, sizeof(buffer));
        if (num > 0)
        {
            buffer[num] = '\0';
            cout << BLUE << MAIN_PATH << RESET << "Recieved in main: " << buffer << endl;
            string benefit_info = buffer;
            vector<string> messages = SplitMessage(benefit_info);
            vector<BenefitInfo> benefits = ExtractBenefit(messages, products);
            SaveBenefit(products, benefits);
        }
    }
    cout << BLUE << MAIN_PATH << RESET << "Recieving from stores in main has finished successfully" << endl;
}

void ReadFromProducts(vector<ProductInfo> &products)
{
    cout << BLUE << MAIN_PATH << RESET << "Strat Reading from products in main process" << endl;
    for (int i = 0; i < products.size(); i++)
    {
        if (products[i].wanted)
        {
            char buffer[1024];
            int num = read(products[i].read_fd, buffer, sizeof(buffer));
            if (num > 0)
            {
                buffer[num] = '\0';
                cout << BLUE << MAIN_PATH << RESET << "Recieved in main: " << buffer << " from product " << products[i].name << endl;
                string remainder_info = buffer;
                vector<string> messages = SplitMessage(remainder_info);
                vector<RemainderInfo> remainders = ExtractRemainder(messages, products);
                SaveRemainder(products, remainders);
                i += messages.size() - 1;
            }
        }
    }
    cout << BLUE << MAIN_PATH << RESET << "Recieving from products in main has finished successfully" << endl;
}

void PrintInfo(const vector<ProductInfo> &products)
{
    for (int i = 0; i < products.size(); i++)
    {
        if (products[i].wanted)
        {
            cout << MAGNETA << MAIN_PATH << "Info of product: " << products[i].name << endl;
            cout << MAGNETA << MAIN_PATH << "Remainder amount: " << products[i].reaminder_amount << endl;
            cout << MAGNETA << MAIN_PATH << "Remainder price: " << products[i].remainder_price << endl;
            cout << MAGNETA << MAIN_PATH << "Benefit: " << products[i].benefit << endl;
        }
    }
}

vector<ProductInfo> SetProducts(string path)
{
    ifstream stream_csv_file(path);
    string temp_string;
    vector<ProductInfo> products;
    while (getline(stream_csv_file, temp_string, ','))
    {
        ProductInfo temp;
        temp.name = temp_string;
        temp.benefit = 0;
        temp.wanted = false;
        products.push_back(temp);
    }
    return products;
}

string GetProductNames(vector<ProductInfo> products)
{
    string temp;
    for (int i = 0; i < products.size(); i++)
    {
        temp += products[i].name + ",";
    }
    temp.pop_back();
    return temp;
}

string CreateMenu(vector<ProductInfo> products)
{
    string temp = "Choose from these products:\n";
    for (int i = 0; i < products.size(); i++)
    {
        temp += to_string(i + 1) + ") " + products[i].name + "\n";
    }
    return temp;
}

void SendChoiceToStore(vector<StoreInfo> stores, string choice)
{
    choice += MESSAGE_DELIMITER;
    for (int i = 0; i < stores.size(); i++)
    {
        cout << BLUE << MAIN_PATH << RESET << "Sending choice to store: " << stores[i].csv_path << endl;
        write(stores[i].write_fd, choice.c_str(), choice.size() + 1);
        usleep(10000);
    }
}

void SendNumStoresToProducts(vector<ProductInfo> products, int num)
{
    string amount = to_string(num) + MESSAGE_DELIMITER;
    for (int i = 0; i < products.size(); i++)
    {
        cout << BLUE << MAIN_PATH << RESET << "Sending number of stores to product: " << products[i].name << endl;
        write(products[i].write_fd, amount.c_str(), amount.size());
        usleep(10000);
    }
}

void CreateNamedPipes(vector<ProductInfo> products)
{
    for (int i = 0; i < products.size(); i++)
    {
        string path = NAMEDPIPE_PATH + products[i].name;
        if (mkfifo(path.c_str(), 0666) == -1)
        {
            perror("fuck");
        }
        else
        {
            cout << BLUE << MAIN_PATH << RESET << "Create pipe:" << i + 1 << " in path:" << path << endl;
        }
    }
}

void ClosePipes(vector<ProductInfo> products)
{
    cout << BLUE << MAIN_PATH << RESET << "Start Closing named pipes" << endl;
    for (int i = 0; i < products.size(); i++)
    {
        string path = NAMEDPIPE_PATH + products[i].name;
        if (unlink(path.c_str()) == -1)
        {
            perror("error");
        }
        else
        {
            cout << BLUE << MAIN_PATH << RESET << "pipe: " << path << " has closed successfully" << endl;
        }
    }
}

void StartRecievingInProducts(const vector<ProductInfo> &products)
{
    for (int i = 0; i < products.size(); i++)
    {
        if (products[i].wanted)
        {
            cout << BLUE << MAIN_PATH << RESET << "Sending signal of recieve to product: " << products[i].name << endl;
            write(products[i].write_fd, START_SIGNAL.c_str(), START_SIGNAL.size() + 1);
            usleep(10000);
        }
        else
        {
            cout << BLUE << MAIN_PATH << RESET << "Sending signal of nothing to product: " << products[i].name << endl;
            write(products[i].write_fd, NO_SIGNAL.c_str(), NO_SIGNAL.size() + 1);
            usleep(10000);
        }
    }
}

void SaveChoices(string choices, vector<ProductInfo> &products)
{
    istringstream line(choices);
    string temp;
    while (getline(line, temp, ','))
    {
        int index = SearchInProducts(temp, products);
        products[index].wanted = true;
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        cout << "Invalid Argument!\n";
    }
    string store_path = argv[1];
    string parts_path = store_path + PARTS_PATH;
    vector<path> csv = GetFilesPath(store_path);
    vector<StoreInfo> stores(csv.size());
    vector<ProductInfo> products = SetProducts(parts_path);
    string menu = CreateMenu(products);
    cout << CYAN << menu;
    string choice;
    cin >> choice;
    SaveChoices(choice, products);
    cout << CYAN << "your choices are: " << choice << endl;
    CreateNamedPipes(products);
    SetStoreInfo(csv, stores);
    CreateProductProcess(products);
    CreateStoreProcess(stores);
    string names = GetProductNames(products);
    SendNumStoresToProducts(products, stores.size());
    StartRecievingInProducts(products);
    SendChoiceToStore(stores, choice);
    ReadFromStores(stores, products);
    ReadFromProducts(products);
    ClosePipes(products);
    Terminate(stores, products);
    PrintInfo(products);
    cout << BLUE << MAIN_PATH << RESET << "Main process has done its job" << endl;
    exit(1);
    return 0;
}