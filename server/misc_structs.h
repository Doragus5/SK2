#include <map>
#include <string>
#include <vector>
using namespace std;

typedef struct RNG_t
{
private:
    unsigned int seed;
    unsigned long long number;
public:
} RNG;

struct event_msg
{
private:
    void validate();

public:
    event_msg();
    int len;
    int event_no;
    unsigned char event_type;
    string event_data;
    int crc32;
    string to_string();
};

typedef struct player_t
{
private:
    unsigned int last_msg_time_stamp;

public:
    const string name;
    const int socket;
    char player_number;
} player;

typedef struct client_t
{
    private:
    public:
        unsigned int last_msg_time_stamp;
}client;

typedef struct game_t
{
private:
    map<string, player> player_list;
    unsigned int height;
    unsigned int width;
    unsigned int game_id;
    unsigned int turn_speed;
    int player_direction[25];

public:
    vector<string> events;
    game_t(unsigned int height, unsigned int width, unsigned int turn_speed);
    game_t();
    unsigned int get_height() const;
    event_msg NEW_GAME_msg() const;

} game;

typedef struct server_t
{
private:
    game current_game;
    unsigned int height;
    unsigned int width;
    unsigned int turn_speed;
    unsigned int fps;
    unsigned int port;
    RNG rng;
    vector<client> clients;


public:
    server_t(unsigned int height, unsigned int width, unsigned int turn_speed, unsigned int fps, unsigned int port);
    void start();

} server;


