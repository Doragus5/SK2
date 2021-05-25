#include "misc_structs.h"
#include "parser.h"
#include <math.h>
//-------------CONSTANTS-------------
const double pi = acos(-1);
//-------------CONSTRUCTORS-------------
client_msg::client_msg(uint32_t len, const char *buff) : len{len}
{
    string eb = "";
    for (uint32_t i = 0; i < 8; i++)
        eb += buff[i];
    session_id = eb_to_long(eb);
    turn_dir = buff[8];
    string fb = "";
    for (uint32_t i = 9; i <= 12; i++)
        fb += buff[i];
    next_event_no = fb_to_int(fb);
    player_name = "";
    for (uint32_t i = 13; i < len; i++)
        player_name += buff[i];
}
game::game_t(int32_t height, int32_t width, uint32_t turn_speed, RNG *rng, server *conn_handle)
    : height{height}, width{width}, turn_speed{turn_speed}, conn_handle{conn_handle}, rng{rng} {}
game::game_t() : conn_handle{NULL}, rng{NULL} {}
event_msg::event_msg() : len{0}, crc32{0} {}
player_t::player_tt(string name) : name{name} {}
client::client_t(uint64_t last_msg_time_stamp, sockaddr_in6 client_address, uint64_t session_id, string name) : last_msg_time_stamp{last_msg_time_stamp}, client_address{client_address}, session_id{session_id}, name{name} {}
RNG::RNG_t(uint32_t seed) : number{seed} {}
RNG::RNG_t() : number{time(NULL)} {}
//-------------METHODS-------------
uint32_t RNG::next()
{
    std::cout << "        RNG::next number:" << number << "\n";
    uint32_t result = number;
    number = (number * 279410273) % 4294967291;
    return result;
}

void event_msg::validate()
{
    std::cout << "        EVENT_MSG::validate\n";
    if (len != 0)
        return;
    len = 4 + 1 + event_data.length();
    crc32 = control_sum(this->substring());
}

string event_msg::substring()
{
    std::cout << "        EVENT_MSG::substring\n";
    string msg = "";
    msg += int_to_fb(len);
    msg += int_to_fb(event_no);
    msg += event_type;
    msg += event_data;
    return msg;
}

string event_msg::to_string()
{
    std::cout << "        EVENT_MSG::to_string\n";
    string msg = "";
    this->validate();
    msg += int_to_fb(len);
    msg += int_to_fb(event_no);
    msg += event_type;
    msg += event_data;
    msg += int_to_fb(crc32);
    return msg;
}

event_msg game::NEW_GAME_msg() const
{
    std::cout << "    GAME::NEW_GAME_msg\n";
    event_msg msg;
    msg.event_no = 0;
    msg.event_type = '\0';
    string part_msg = "";
    part_msg += int_to_fb(width);
    part_msg += int_to_fb(height);
    char i = 0;
    for (auto player : player_list)
    {
        part_msg += player.first;
        part_msg += '\0';
        player.second.player_number = i;
        i++;
        std::cout << "    GAME::NEW_GAME_msg::Player '" << player.first << "' has number " << i - 1 << "=" << (uint32_t)player.second.player_number << "\n";
    }
    msg.event_data = part_msg;
    return msg;
}

event_msg game::PIXEL_msg(string player_name) const
{
    std::cout << "    GAME::PIXEL_msg\n";
    event_msg msg;
    msg.event_no = 0;
    msg.event_type = '\0';
    string part_msg = "";
    auto player = player_list.find(player_name)->second;
    part_msg += (char)player.player_number;
    part_msg += int_to_fb(player.px);
    part_msg += int_to_fb(player.py);
    msg.event_data = part_msg;
    return msg;
}

event_msg game::PLAYER_ELIMINATED_msg(string player_name) const
{
    std::cout << "    GAME::PLAYER_ELIMINATED_msg\n";
    event_msg msg;
    msg.event_no = 0;
    msg.event_type = '\2';
    string part_msg = "";
    part_msg += (char)player_list.find(player_name)->second.player_number;
    msg.event_data = part_msg;
    return msg;
}

event_msg game::GAME_OVER_msg() const
{
    std::cout << "    GAME::GAME_OVER_msg\n";
    event_msg msg;
    msg.event_no = 0;
    msg.event_type = '\3';
    string part_msg = "";
    msg.event_data = part_msg;
    return msg;
}

uint32_t game::get_height() const
{
    return height;
}

void game::start_game()
{
    game_started = true;
    std::cout << "    GAME::start_game::Starting a new game\n";
    game_id = rng->next();
    string event = NEW_GAME_msg().to_string();
    events.push_back(event);
    conn_handle->broadcast(event);
    for (auto it : player_list)
    {
        auto player = it.second;
        player.px = rng->next() % width;
        player.py = rng->next() % height;
        player.x = (double)player.px + 0.5;
        player.y = (double)player.py + 0.5;
        player.direction = rng->next() % 360;
        std::cout << "    GAME::start_game::Inserting " << player.px << "," << player.py << "\n";
        if (eaten.insert(std::make_pair(player.px, player.py)).second)
        {
            std::cout << "    GAME::start_game::PIXEL '" << player.name << "'" << player.px << "," << player.py << "\n";
            string event = PIXEL_msg(player.name).to_string();
            events.push_back(event);
            conn_handle->broadcast(event);
        }
        else
        {
            std::cout << "    GAME::start_game::PLAYER_ELIMINATED '" << player.name << "'\n";
            string event = PLAYER_ELIMINATED_msg(player.name).to_string();
            events.push_back(event);
            conn_handle->broadcast(event);
            player.eliminated = true;
            eliminated_players++;
            if (eliminated_players + 1 >= total_players)
            {
                std::cout << "    GAME::start_game::GAME_OVER\n";
                string event2 = GAME_OVER_msg().to_string();
                events.push_back(event);
                conn_handle->broadcast(event);
                game_ended = true;
                return;
            }
        }
    }
}

void game::next_frame()
{
    if (game_ended)
        return;
    for (auto it : player_list)
    {
        auto player = it.second;
        std::cout << "    GAME::next_frame::Moving player '" << std::to_string(player.player_number) << "'\n";
        if (player.eliminated)
            continue;
        player.direction += turn_speed * player_turning[player.player_number];
        player.direction %= 360;
        player.x += cos(player.direction / 180 * pi);
        player.y += sin(player.direction / 180 * pi);
        if (player.px == (int32_t)floor(player.x) && player.py == (int32_t)floor(player.y))
            continue;
        player.px = (int32_t)floor(player.x);
        player.py = (int32_t)floor(player.y);
        std::cout << "    GAME::next_frame::player " << std::to_string(player.player_number) << " cooridnates " << player.px << "," << player.py << "\n";
        if (player.px >= 0 && player.py >= 0 && player.px < width && player.py < height && eaten.insert(std::make_pair(player.px, player.py)).second)
        {
            string event = PIXEL_msg(player.name).to_string();
            events.push_back(event);
            conn_handle->broadcast(event);
        }
        else
        {
            std::cout << "    GAME::next_frame::PLAYER_ELIMINATED '" << player.name << "'\n";
            string event = PLAYER_ELIMINATED_msg(player.name).to_string();
            events.push_back(event);
            conn_handle->broadcast(event);
            player.eliminated = true;
            eliminated_players++;
            if (eliminated_players + 1 >= total_players)
            {
                std::cout << "    GAME::next_frame::GAME_OVER\n";
                string event2 = GAME_OVER_msg().to_string();
                events.push_back(event);
                conn_handle->broadcast(event);
                game_ended = true;
                return;
            }
        }
    }
}

bool game::player_exists(string name)
{
    return player_list.find(name) != player_list.end();
}

uint32_t game::player_number(string name)
{
    auto player = player_list.find(name)->second;
    return player.player_number;
}

void game::add_player(string name)
{
    std::cout << "    GAME::add_player(" << name << ")\n";
    no_of_participants++;
    if (name == "")
        return;
    total_players++;
    player_list.insert(std::make_pair(name, player_t(name)));
}