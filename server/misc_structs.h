#include <map>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <set>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <iostream>

#include "err.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;
typedef pair<uint16_t, pair<pair<uint32_t, uint32_t>, pair<uint32_t, uint32_t>>> client_key;
typedef struct server_t server;
typedef struct RNG_t
{
private:
    uint32_t seed;
    uint64_t number;

public:
    RNG_t(uint32_t seed);
    RNG_t();
    uint32_t next();
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

struct client_msg
{
private:
    void validate();

public:
    client_msg(uint32_t len, const char *buff);

    uint32_t len;
    uint32_t next_event_no;
    uint8_t turn_dir;
    string player_name;
    uint64_t session_id;

    string to_string();
};

typedef struct player_tt
{
private:
public:
    player_tt(string name);
    bool eliminated = false;
    string name;
    uint8_t player_number;
    uint32_t direction;
    int32_t px;
    int32_t py;
    double x;
    double y;
} player_t;

typedef struct client_t
{
private:
public:
    client_t(uint64_t last_msg_time_stamp, struct sockaddr_in6 client_address, uint64_t session_id, string name);

    uint64_t last_msg_time_stamp;
    struct sockaddr_in6 client_address;
    uint64_t session_id;
    string name;
} client;

typedef struct game_t
{
private:
    map<string, player_t> player_list;
    int32_t height;
    int32_t width;
    uint32_t turn_speed;
    uint32_t event_no = 0;
    server *conn_handle;
    RNG *rng;
    set<pair<uint32_t, uint32_t>> eaten;

    event_msg NEW_GAME_msg() const;
    event_msg PIXEL_msg(string player_name) const;
    event_msg PLAYER_ELIMINATED_msg(string player_name) const;
    event_msg GAME_OVER_msg() const;

public:
    uint32_t total_players = 0;
    uint32_t eliminated_players = 0;
    uint32_t game_id;
    bool game_started = false;
    bool game_ended = false;
    int32_t player_turning[25] = {};
    uint32_t no_of_participants = 0;
    vector<string> events;

    uint32_t player_number(string name);
    bool player_exists(string name);
    game_t(int32_t height, int32_t width, uint32_t turn_speed, RNG *rng, server *conn_handle);
    game_t();
    uint32_t get_height() const;
    void start_game();
    void next_frame();
    void add_player(string name);

} game;

struct server_t
{
private:
    game current_game;
    int32_t height;
    int32_t width;
    uint32_t turn_speed;
    uint32_t fps;
    uint32_t port;
    uint64_t frame_number = 0;
    int32_t sock;
    int32_t flags, sflags;
    RNG rng;
    socklen_t snda_len, rcva_len;
    map<client_key, client> clients;

    client_key client_identifier(sockaddr_in6 *client_address);
    uint32_t check_client(sockaddr_in6 *client_address, client_msg msg);
    void handle_new_client(sockaddr_in6 *client_address, client_msg msg);
    void handle_old_client(sockaddr_in6 *client_address, client_msg msg);
    void broadcast_many_events(sockaddr_in6 *client_address, uint32_t no_event);

public:
    server_t(int32_t height, int32_t width, uint32_t turn_speed, uint32_t fps, uint32_t port);

    void broadcast(string msg);
    void start();
};
