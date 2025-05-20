#include <vector>
#include <string>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

struct StoreInfo
{
    path csv_path;
    int id;
    int write_fd;
    int read_fd;
};

struct ProductInfo
{
    string name;
    int write_fd;
    int read_fd;
    double benefit;
    bool wanted;
    int reaminder_amount;
    double remainder_price;
};

struct BenefitInfo
{
    string name;
    double benefit;
    int index;
};

struct RemainderInfo
{
    string name;
    int amount;
    double price;
    int index;
};



