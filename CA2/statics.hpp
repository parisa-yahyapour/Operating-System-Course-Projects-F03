#ifndef _STATIC_HPP_
#define _STATIC_HPP_

#include <string>
#include <vector>
#include <iostream>
#include "color.hpp"

using namespace std;

const string OUT_STORE = "./store.out";
const string OUT_PRODUCT = "./product.out";
const string INPUT_TYPE = "input";
const string OUTPUT_TYPE = "output";
const string PARTS_PATH = "/Parts.csv";
const string START_SIGNAL = "start@";
const string NO_SIGNAL = "nothing@";
const string MESSAGE_DELIMITER = "@";
const string MAIN_PATH = "[main.cpp]: ";
const string STORE_PATH = "[store.cpp]: ";
const string PRODUCT_PATH = "[product.cpp]: ";

const char* NAMEDPIPE_PATH = "./fifo/";

const char DELIMITER = '@';
const char SEPERATOR = ',';

#endif