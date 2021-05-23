#include <map>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "err.h"
using namespace std;

typedef struct RNG_t
{
private:
    uint32_t seed;
    uint64_t number;

public:
} RNG;

struct event_msg
{
private:
    void validate();

public:
    event_msg();
    uint32_t len;
    uint32_t event_no;
    uint8_t event_type;
    string event_data;
    uint32_t crc32;
    string to_string();
};

typedef struct player_t
{
private:
    uint32_t last_msg_time_stamp;

public:
    const string name;
    const uint32_t socket;
    char player_number;
} player;

typedef struct client_t
{
    private:
    public:
        uint32_t last_msg_time_stamp;
        struct sockaddr *client_address;
}client;

typedef struct game_t
{
private:
    map<string, player> player_list;
    uint32_t height;
    uint32_t width;
    uint32_t game_id;
    uint32_t turn_speed;
    uint32_t player_direction[25];

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


