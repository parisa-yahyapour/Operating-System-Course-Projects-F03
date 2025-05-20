#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "statics.hpp"
#include <signal.h>
#include <vector>
#include <poll.h>

using namespace std;


int RecognizeRequest(string command, bool is_playing)
{
    command.pop_back();
    int message_code;
    if (command.find(SEND_NAME_COMMAND) != string::npos)
    {
        return SEND_NAME;
    }
    else if (command.find(CHOOSE_OPTION_COMMAND) != string::npos)
    {
        return SEND_PORT;
    }
    else if (command.find(PLAY_AGAIN) != string::npos)
    {
        return AGAIN;
    }
    else if (is_playing)
    {
        return PLAY_IN_GAME;
    }

    return NO_MATCH_REQUEST;
}

long int GetPortNum(char *buffer)
{
    string message = buffer;
    string number = message.substr(SEND_PORT_COMMAND.size(), message.size() - SEND_PORT_COMMAND.size());
    return strtol(number.c_str(), NULL, 10);
}

pair<int, sockaddr_in> CreateTCPSocket(char *ip, int port)
{
    struct sockaddr_in address;
    int fd, opt = 1;

    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &(address.sin_addr)) == -1)
        perror("FAILED: Input ipv4 address invalid");

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable failed");

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    memset(address.sin_zero, 0, sizeof(address.sin_zero));
    address.sin_port = htons(port);

    return pair<int, sockaddr_in>(fd, address);
}

pair<int, sockaddr_in> CreateBroadcastSocket()
{
    int fd, opt = 1, broadcast = 1;
    struct sockaddr_in bc_address;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(8080);
    bc_address.sin_addr.s_addr = inet_addr("127.255.255.255");
    return pair<int, sockaddr_in>(fd, bc_address);
}

int FindFd(const vector<pollfd> &fds, int wanted_fd)
{
    for (int i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == wanted_fd)
        {
            return i;
        }
    }
    return -1;
}

void CloseAllConnections(const vector<pollfd> &fds)
{
    for (int i = 0; i < fds.size(); i++)
    {
        close(fds[i].fd);
    }
}

int answer_status = USER_ENTERED;

void SendGameRespone(int room_fd, char *buffer)
{
    if (answer_status == NOT_CHOSEN_YET)
    {
        const char *no_choice = "-1\n";
        send(room_fd, no_choice, strlen(no_choice), 0);
        answer_status = USER_ENTERED;
        return;
    }
    send(room_fd, buffer, strlen(buffer), 0);
}

void alarm_handler(int sig)
{
    answer_status = NOT_CHOSEN_YET;
}

void HandleNoInput(int fd)
{
    string message = to_string(NOT_CHOSEN_YET);
    const char *converted = message.c_str();
    send(fd, converted, strlen(converted), 0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        perror("Invalid Arguments");

    char *ipaddr = argv[1];
    struct sockaddr_in server_addr, room_addr, broadcast_addr;
    int server_fd, room_fd, broadcast_fd, opt = 1;
    vector<pollfd> file_descriptors;
    bool is_playing = false;

    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ipaddr, &(server_addr.sin_addr)) == -1)
        perror("FAILED: Input ipv4 address invalid");

    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable failed");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    server_addr.sin_port = htons(strtol(argv[2], NULL, 10));

    if (connect(server_fd, (sockaddr *)(&server_addr), sizeof(server_addr)))
        perror("FAILED: Connect");

    file_descriptors.push_back(pollfd{server_fd, POLLIN, 0});
    file_descriptors.push_back(pollfd{STDIN, POLLIN, 0});

    pair<int, sockaddr_in> braodcast_connection = CreateBroadcastSocket();
    broadcast_addr = braodcast_connection.second;
    broadcast_fd = braodcast_connection.first;
    if (bind(broadcast_fd, (const struct sockaddr *)(&broadcast_addr), sizeof(broadcast_addr)) == -1)
        perror("FAILED: Bind unsuccessfull");
    file_descriptors.push_back(pollfd{broadcast_fd, POLLIN, 0});

    while (1)
    {
        if (poll(file_descriptors.data(), (nfds_t)(file_descriptors.size()), -1) == -1)
            perror("FAILED: Poll");

        for (size_t i = 0; i < file_descriptors.size(); ++i)
        {
            if (file_descriptors[i].revents & POLLIN)
            {
                if (file_descriptors[i].fd == server_fd)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(server_fd, buffer, BUFFER_SIZE, 0);
                    string message = buffer;
                    if (message.find(SEND_PORT_COMMAND) != string ::npos)
                    {
                        int port_num = GetPortNum(buffer);
                        pair<int, sockaddr_in> new_connection = CreateTCPSocket(ipaddr, port_num);
                        room_fd = new_connection.first;
                        room_addr = new_connection.second;
                        if (connect(room_fd, (sockaddr *)(&room_addr), sizeof(room_addr)))
                            perror("FAILED: Connect To Room");
                        file_descriptors.push_back(pollfd{room_fd, POLLIN, 0});
                    }
                    else
                    {
                        write(STDOUT, buffer, strlen(buffer));
                    }
                }
                else if (file_descriptors[i].fd == STDIN)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    read(STDIN, buffer, BUFFER_SIZE);
                    string message = buffer;
                    long int request_code = RecognizeRequest(message, is_playing);
                    switch (request_code)
                    {
                    case SEND_NAME:
                    {
                        send(server_fd, buffer, strlen(buffer), 0);
                    }
                    break;
                    case SEND_PORT:
                    {
                        send(server_fd, buffer, strlen(buffer), 0);
                    }
                    break;
                    case PLAY_IN_GAME:
                    {
                        send(room_fd, buffer, strlen(buffer), 0);
                    }
                    break;
                    case AGAIN:
                    {
                        send(server_fd, buffer, strlen(buffer), 0);
                    }
                    break;
                    default:
                        break;
                    }
                }
                else if (file_descriptors[i].fd == room_fd)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(room_fd, buffer, BUFFER_SIZE, 0);
                    write(STDOUT, buffer, strlen(buffer));
                    string message = buffer;
                    if (message.find(GAME_MENU_TEXT) != string ::npos)
                    {
                        signal(SIGALRM, alarm_handler);
                        siginterrupt(SIGALRM, 1);
                        char buffer[BUFFER_SIZE];
                        memset(buffer, 0, BUFFER_SIZE);
                        is_playing = true;
                        alarm(10);
                        read(STDIN, buffer, BUFFER_SIZE);
                        alarm(0);
                        SendGameRespone(room_fd, buffer);
                    }
                    else if (message.find(CURRENT_GAME_IS_FINISHED) != string ::npos)
                    {
                        int index_room = FindFd(file_descriptors, room_fd);
                        file_descriptors.erase(file_descriptors.begin() + index_room);
                        close(room_fd);
                        is_playing = false;
                    }
                }
                else if (file_descriptors[i].fd == broadcast_fd)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    socklen_t adrr = sizeof(broadcast_addr);
                    int recieve_status = recv(file_descriptors[i].fd, buffer, BUFFER_SIZE, 0);
                    string message = buffer;
                    write(STDOUT, buffer, strlen(buffer));
                    if (message.find(CLOSE_PROGRAM) != string ::npos)
                    {
                        CloseAllConnections(file_descriptors);
                        exit(0);
                    }
                }
            }
        }
    }
}
