#include <string>
#include <map>
using namespace std;

const int STDIN = 0;
const int STDOUT = 1;
const int STDERR = 2;
const int BUFFER_SIZE = 1024;
const int BASE_PORT_ROOM = 3000;
const int BROADCAST_PORT = 8080;
const int MAX_QUEUE_ROOM = 2;
const int MAX_NUM_PRESENT_PLAYERS = 2;
const int NO_MATCH_REQUEST = -1;
const int NOT_CHOSEN_YET = -1;
const int USER_ENTERED = 0;
const int TIE = -1;
const int NO_ROOM = -1;
const int NO_PLAYED_GAME = 0;
const int BOTH_PLAYER_CHOSE = 2;


const bool FULL = true;
const bool HAS_CAPACITY = false;

const char *START_SERVER = "Server is started\n";
const char *NEW_CONNECTION = "New client is connected to server\n";
const char *REGISTERATION_SENTENCE = "Please send your name with this format: name <your_name>\n";
const char *CONNECTING_TO_ROOM = "User is connected to room\n";
const char *SUCCESSFULLY_ENTERED = "You have entered the room successfully!\nPlease Wait\n";
const char *GAME_MENU_TEXT = "Please choose:\n1) Rock\n2) Paper\n3) Scissors\n";
const char *BROADCAST_IP = "255.255.255.255";
const char *CURRENT_GAME_IS_FINISHED = "Your current game is finished\nPlease enter <again> for playing\n";
const char *TIE_RESULT = "There is no winner! Tie!\n";
const char *ROOM_IS_NOT_AVAILABLE = "The requested room is not available!\nPlease choose other room\n";

const string CHOOSE_OPTION_COMMAND = "room";
const string SEND_NAME_COMMAND = "name";
const string NO_NAME = "no name";
const string WINNER_ANNOUNCEMENT = "Winner is ";
const string PLAY_AGAIN = "again";
const string END_GAME = "end_game\n";
const string PICK_CHOICE = "o";
const string CLOSE_PROGRAM = "Thank you for participating!\nResults:\n";
const string SEND_PORT_COMMAND = "port ";

typedef struct pollfd pollfd;

struct Room
{
    int room_file_descriptor;
    int port;
    int num_room;
    int num_present_players;
    map<int, int> socket_to_choice;
    bool is_started;
    int num_answered;
};

struct Player
{
    string name;
    int player_file_descriptor;
    int room_port;
    int room_fd;
    int broadcast_discriptor;
    int num_victory;
};

struct Command
{
    int code;
    string arguemnets;
};

Player NULL_PLAYER{"Null", -1, -1};

enum Choice
{
    NOTHING,
    ROCK,
    PAPER,
    SCISSORS
};

enum Operation
{
    SEND_NAME,
    SEND_PORT,
    PLAY_IN_GAME,
    AGAIN,
    RECIEVE_PORT
};

map<int, int> GAME_MENU = {{1, ROCK}, {2, PAPER}, {3, SCISSORS}};