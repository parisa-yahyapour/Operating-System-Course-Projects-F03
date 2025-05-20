CXX = g++ -std=c++17

MAIN_SRC = main.cpp
STORE_SRC = store.cpp
PRODUCT_SRC = product.cpp

HEADER = statics.hpp
HEADER2 = types.hpp
HEADER3 = color.hpp
HEADER_STORE = store.hpp
HEADER_PRODUCT = product.hpp

MAIN_OUT = main.out
STORE_OUT = store.out
PRODUCT_OUT = product.out

all: $(MAIN_OUT) $(STORE_OUT) $(PRODUCT_OUT)

$(MAIN_OUT): $(MAIN_SRC) $(HEADER) $(HEADER2) $(HEADER3)
	$(CXX) $(MAIN_SRC) -o $(MAIN_OUT) 

$(STORE_OUT): $(STORE_SRC) $(HEADER) $(HEADER_STORE) $(HEADER3)
	$(CXX) $(STORE_SRC) -o $(STORE_OUT)

$(PRODUCT_OUT): $(PRODUCT_SRC) $(HEADER) $(HEADER_PRODUCT) $(HEADER3) $(HEADER2)
	$(CXX) $(PRODUCT_SRC) -o $(PRODUCT_OUT) 

clean:
	rm -f $(MAIN_OUT) $(STORE_OUT) $(PRODUCT_OUT)