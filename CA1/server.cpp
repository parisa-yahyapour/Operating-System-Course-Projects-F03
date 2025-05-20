#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include "statics.hpp"
#include <string>
#include <thread>
#include <chrono>

using namespace std;

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

vector<Room> CreateRooms(int num_rooms, char *ip, vector<pollfd> &fds)
{
    vector<Room> rooms;
    for (int i = 1; i <= num_rooms; i++)
    {
        struct sockaddr_in address;
        Room new_room;
        new_room.num_present_players = 0;
        new_room.num_room = i;
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        address.sin_family = AF_INET;
        address.sin_port = htons(BASE_PORT_ROOM + i);
        inet_pton(AF_INET, ip, &(address.sin_addr));
        bind(fd, (struct sockaddr *)&address, sizeof(address));
        listen(fd, MAX_QUEUE_ROOM);
        new_room.room_file_descriptor = fd;
        new_room.port = BASE_PORT_ROOM + i;
        new_room.is_started = false;
        new_room.num_answered = 0;
        rooms.push_back(new_room);
        fds.push_back(pollfd{fd, POLLIN, 0});
    }
    return rooms;
}

string ConvertEmptyRoomDataToMessage(map<int, Room> option_to_empty_rooms)
{
    string message = "Available Rooms:";
    int index = 1;
    for (map<int, Room>::iterator it = option_to_empty_rooms.begin(); it != option_to_empty_rooms.end(); ++it)
    {
        message += "\noption " + to_string(index) + ") Room" + to_string(it->second.num_room) + " has " +
                   to_string(MAX_NUM_PRESENT_PLAYERS - it->second.num_present_players) + " empty spot/spots";
        index++;
    }
    message += "\n";
    message += "Please write with this format: room <n>\n";
    return message;
}

map<int, Room> GetEmptyRooms(const vector<Room> &rooms)
{
    map<int, Room> option_to_empty_rooms;
    for (int i = 0; i < rooms.size(); i++)
    {
        if (rooms[i].num_present_players != MAX_NUM_PRESENT_PLAYERS)
        {
            option_to_empty_rooms.insert(pair<int, Room>(rooms[i].num_room, rooms[i]));
        }
    }
    return option_to_empty_rooms;
}

int FindByDescriptor(int fd, const vector<Player> &players)
{
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].player_file_descriptor == fd)
        {
            return i;
        }
    }
    return -1;
}

Player CreateNewPlayer(int fd)
{
    Player temp;
    temp.player_file_descriptor = fd;
    temp.name = NO_NAME;
    temp.room_port = NO_ROOM;
    temp.room_fd = NO_ROOM;
    temp.num_victory = NO_PLAYED_GAME;
    return temp;
}

Command RecognizeOperation(string command)
{
    command.pop_back();
    Command converted_command;
    if (command.find(SEND_NAME_COMMAND) != string::npos)
    {
        converted_command.code = SEND_NAME;
        converted_command.arguemnets = command.substr(SEND_NAME_COMMAND.size() + 1, command.size() - SEND_NAME_COMMAND.size() - 1);
    }
    else if (command.find(CHOOSE_OPTION_COMMAND) != string::npos)
    {
        converted_command.code = SEND_PORT;
        converted_command.arguemnets = command.substr(CHOOSE_OPTION_COMMAND.size() + 1, command.size() - CHOOSE_OPTION_COMMAND.size() - 1);
    }
    else if (command.find(PLAY_AGAIN) != string::npos)
    {
        converted_command.code = AGAIN;
    }
    else
    {
        converted_command.code = -1;
        converted_command.arguemnets = command;
    }
    return converted_command;
}

bool IsRoomFd(int fd, const vector<Room> &rooms)
{
    for (int i = 0; i < rooms.size(); i++)
    {
        if (rooms[i].room_file_descriptor == fd)
        {
            return true;
        }
    }
    return false;
}

int FindRoomByFd(int fd, const vector<Room> &rooms)
{
    for (int i = 0; i < rooms.size(); i++)
    {
        if (fd == rooms[i].room_file_descriptor)
        {
            return i;
        }
    }
    return -1;
}

void ChangeRoomInfo(int client_fd, Room *chosen_room)
{
    chosen_room->socket_to_choice.insert(pair<int, int>(client_fd, NOT_CHOSEN_YET));
    chosen_room->num_present_players++;
}

void StartGame(Room current_room)
{
    map<int, int> players = current_room.socket_to_choice;
    for (map<int, int>::iterator it = players.begin(); it != players.end(); ++it)
    {
        send(it->first, GAME_MENU_TEXT, strlen(GAME_MENU_TEXT), 0);
    }
}

int FindRoomByPlayer(int player_fd, const vector<Room> &rooms)
{
    for (int i = 0; i < rooms.size(); i++)
    {
        if (rooms[i].socket_to_choice.find(player_fd) != rooms[i].socket_to_choice.end())
        {
            return i;
        }
    }
    return -1;
}

void SavePlayerChoice(int fd, Room *current_room, int choice)
{
    current_room->socket_to_choice[fd] = GAME_MENU[choice];
    current_room->num_answered++;
}

int Judge(Room current_room) // return the winner
{
    pair<int, int> player1, player2; // first is fd second is choice
    int index = 0;
    for (map<int, int>::iterator it = current_room.socket_to_choice.begin(); it != current_room.socket_to_choice.end(); ++it)
    {
        if (index == 0)
        {
            player1.first = it->first;
            player1.second = it->second;
            index++;
        }
        else
        {
            player2.first = it->first;
            player2.second = it->second;
        }
    }
    if (player1.second == player2.second)
    {
        return TIE;
    }

    if (player1.second == ROCK && player2.second == SCISSORS ||
        player1.second == SCISSORS && player2.second == PAPER ||
        player1.second == PAPER && player2.second == ROCK ||
        player2.second == NOT_CHOSEN_YET)
    {
        return player1.first;
    }
    return player2.first;
}

int FindPlayerByPort(int port, const vector<Player> &players)
{
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].room_port == port && players[i].room_fd == NO_ROOM)
        {
            return i;
        }
    }
    return -1;
}

int FindPlayerByRoom(int fd, const vector<Player> &players)
{
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].room_fd == fd)
        {
            return i;
        }
    }
    return -1;
}

void DeleteConnection(Room *current_room, vector<pollfd> &fds)
{
    vector<int> clients;
    for (map<int, int>::iterator it = current_room->socket_to_choice.begin(); it != current_room->socket_to_choice.end(); ++it)
    {
        send(it->first, CURRENT_GAME_IS_FINISHED, strlen(CURRENT_GAME_IS_FINISHED), 0);
        clients.push_back(it->first);
    }
    for (int i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == clients[0] || fds[i].fd == clients[1])
        {
            fds.erase(fds.begin() + i);
        }
    }
    close(clients[0]);
    close(clients[1]);
    current_room->socket_to_choice.clear();
    current_room->num_answered = 0;
    current_room->num_present_players = 0;
    current_room->is_started = false;
}

void SaveResult(int winner, vector<Player> &players)
{
    players[winner].num_victory++;
}

void PrintPlayers(const vector<Player> &players)
{
    for (int i = 0; i < players.size(); i++)
    {
        string message = "Player <" + players[i].name + "> has won " + to_string(players[i].num_victory) + " matches\n";
        const char *converted_messgae = message.c_str();
        write(1, converted_messgae, strlen(converted_messgae));
    }
}

string CreateEndGameResult(const vector<Player> &players)
{
    string result = CLOSE_PROGRAM;
    for (int i = 0; i < players.size(); i++)
    {
        string message = "Player <" + players[i].name + "> has won " + to_string(players[i].num_victory) + " matches\n";
        result += message;
    }
    return result;
}

void CloseAllConnections(const vector<pollfd> &fds)
{
    for (int i = 0; i < fds.size(); i++)
    {
        close(fds[i].fd);
    }
}

bool IsRoomAvailabe(int room_number, const map<int, Room> &empty_rooms)
{
    if (empty_rooms.find(room_number) != empty_rooms.end())
    {
        return true;
    }
    return false;
}

// for running server use this format s.out <IP> <port> <num rooms>
int main(int argc, char *argv[])
{
    char *ipaddr = argv[1];
    struct sockaddr_in server_addr, broadcast_addr;
    int server_fd, broadcast_fd, opt = 1, broadcast = 1;
    vector<pollfd> file_descriptors;
    int num_rooms = strtol(argv[3], NULL, 10);
    vector<Room> all_rooms = CreateRooms(num_rooms, ipaddr, file_descriptors);
    vector<Player> players;

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

    if (bind(server_fd, (const struct sockaddr *)(&server_addr), sizeof(server_addr)) == -1)
        perror("FAILED: Bind unsuccessfull");

    if (listen(server_fd, 20) == -1)
        perror("FAILED: Listen unsuccessfull");

    write(STDOUT, START_SERVER, strlen(START_SERVER));
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
                if (file_descriptors[i].fd == server_fd) // new user
                {
                    struct sockaddr_in new_addr;
                    socklen_t new_size = sizeof(new_addr);
                    int new_fd = accept(server_fd, (struct sockaddr *)(&new_addr), &new_size);
                    write(1, NEW_CONNECTION, strlen(NEW_CONNECTION));
                    send(new_fd, REGISTERATION_SENTENCE, strlen(REGISTERATION_SENTENCE), 0);
                    file_descriptors.push_back(pollfd{new_fd, POLLIN, 0});
                    Player new_player = CreateNewPlayer(new_fd);
                    players.push_back(new_player);
                }
                else if (IsRoomFd(file_descriptors[i].fd, all_rooms)) // room message
                {
                    int found_room = FindRoomByFd(file_descriptors[i].fd, all_rooms);
                    if (all_rooms[found_room].num_present_players != MAX_NUM_PRESENT_PLAYERS)
                    {
                        struct sockaddr_in new_addr;
                        socklen_t new_size = sizeof(new_addr);
                        int new_fd = accept(file_descriptors[i].fd, (struct sockaddr *)(&new_addr), &new_size);
                        write(1, CONNECTING_TO_ROOM, strlen(CONNECTING_TO_ROOM));
                        send(new_fd, SUCCESSFULLY_ENTERED, strlen(SUCCESSFULLY_ENTERED), 0);
                        file_descriptors.push_back(pollfd{new_fd, POLLIN, 0});
                        ChangeRoomInfo(new_fd, &all_rooms[found_room]);
                        int entered_player = FindPlayerByPort(all_rooms[found_room].port, players);
                        players[entered_player].room_fd = new_fd;
                        if (all_rooms[found_room].num_present_players == MAX_NUM_PRESENT_PLAYERS)
                        {
                            all_rooms[found_room].is_started = true;
                        }
                    }
                    if (all_rooms[found_room].is_started)
                    {
                        StartGame(all_rooms[found_room]);
                    }
                }
                else if (file_descriptors[i].fd == STDIN)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    read(STDIN, buffer, BUFFER_SIZE);
                    string message = buffer;
                    if (message.compare(END_GAME) == 0)
                    {
                        string result = CreateEndGameResult(players);
                        const char *converted = result.c_str();
                        int status = sendto(broadcast_fd, converted, strlen(converted), 0, (struct sockaddr *)&broadcast_addr,
                                            sizeof(broadcast_addr));
                        this_thread ::sleep_for(chrono ::seconds(1));
                    }
                }
                else if (file_descriptors[i].fd == broadcast_fd)
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    socklen_t adrr = sizeof(broadcast_addr);
                    int recieve_status = recv(file_descriptors[i].fd, buffer, BUFFER_SIZE, 0);
                    write(STDOUT, buffer, strlen(buffer));
                    string message = buffer;
                    if (message.find(CLOSE_PROGRAM) != string ::npos)
                    {
                        CloseAllConnections(file_descriptors);
                        exit(0);
                    }
                }
                else // message from user
                {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int recieve_status = recv(file_descriptors[i].fd, buffer, BUFFER_SIZE, 0);
                    int waiting_room = FindRoomByPlayer(file_descriptors[i].fd, all_rooms);
                    int founded_player = FindByDescriptor(file_descriptors[i].fd, players);
                    map<int, Room> option_to_empty_rooms = GetEmptyRooms(all_rooms);
                    string choose_room_message = ConvertEmptyRoomDataToMessage(option_to_empty_rooms);
                    string recieved_message = buffer;
                    Command converted_command = RecognizeOperation(buffer);
                    if (waiting_room != -1)
                    {
                        converted_command.code = PLAY_IN_GAME;
                    }
                    if (recieve_status != 0)
                    {
                        switch (converted_command.code)
                        {
                        case SEND_NAME:
                        {
                            const char *converted_message = choose_room_message.c_str();
                            players[founded_player].name = converted_command.arguemnets;
                            send(file_descriptors[i].fd, converted_message, strlen(converted_message), 0);
                        }
                        break;
                        case SEND_PORT:
                        {
                            int room_number = stoi(converted_command.arguemnets);
                            bool is_available = IsRoomAvailabe(room_number, option_to_empty_rooms);
                            if (!is_available)
                            {
                                send(file_descriptors[i].fd, ROOM_IS_NOT_AVAILABLE, strlen(ROOM_IS_NOT_AVAILABLE), 0);
                                string room_message = ConvertEmptyRoomDataToMessage(option_to_empty_rooms);
                                const char *converted = room_message.c_str();
                                send(file_descriptors[i].fd, converted, strlen(converted), 0);
                            }
                            else
                            {
                                int port_num = option_to_empty_rooms[room_number].port;
                                players[founded_player].room_port = port_num;
                                string selected_port = SEND_PORT_COMMAND + to_string(port_num);
                                const char *converted_port = selected_port.c_str();
                                send(file_descriptors[i].fd, converted_port, strlen(converted_port), 0);
                            }
                        }
                        break;
                        case PLAY_IN_GAME:
                        {
                            int choice = stoi(converted_command.arguemnets);
                            SavePlayerChoice(file_descriptors[i].fd, &all_rooms[waiting_room], choice);
                            if (all_rooms[waiting_room].num_answered == BOTH_PLAYER_CHOSE)
                            {
                                int winner = Judge(all_rooms[waiting_room]);
                                if (winner == TIE)
                                {
                                    int status = sendto(broadcast_fd, TIE_RESULT,
                                                        strlen(TIE_RESULT), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
                                    this_thread ::sleep_for(chrono ::seconds(1));
                                }
                                else
                                {
                                    int winner_player = FindPlayerByRoom(winner, players);
                                    string result = WINNER_ANNOUNCEMENT + players[winner_player].name + "\n";
                                    const char *announcement = result.c_str();
                                    int status = sendto(broadcast_fd, announcement, strlen(announcement), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
                                    this_thread ::sleep_for(chrono ::seconds(1));
                                    SaveResult(winner_player, players);
                                }
                                DeleteConnection(&all_rooms[waiting_room], file_descriptors);
                            }
                        }
                        break;
                        case AGAIN:
                        {
                            const char *converted_message = choose_room_message.c_str();
                            send(file_descriptors[i].fd, converted_message, strlen(converted_message), 0);
                        }
                        break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}
