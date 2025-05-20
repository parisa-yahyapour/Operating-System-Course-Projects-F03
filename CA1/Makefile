CXX = g++

SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

HEADER = statics.hpp

SERVER_OUT = server.out
CLIENT_OUT = client.out

all: $(SERVER_OUT) $(CLIENT_OUT)

$(SERVER_OUT): $(SERVER_SRC) $(HEADER)
	$(CXX) $(SERVER_SRC) -o $(SERVER_OUT) 

$(CLIENT_OUT): $(CLIENT_SRC) $(HEADER)
	$(CXX) $(CLIENT_SRC) -o $(CLIENT_OUT) 

clean:
	rm -f $(SERVER_OUT) $(CLIENT_OUT)