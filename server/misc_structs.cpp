#include "misc_structs.h"
#include "parser.h"
//-------------CONSTRUCTORS-------------
game::game_t(unsigned int height, unsigned int width, unsigned int turn_speed) : height{height}, width{width}, turn_speed{turn_speed} {}
event_msg::event_msg() : len{0}, crc32{0} {}
server::server_t(unsigned int height, unsigned int width, unsigned int turn_speed, unsigned int fps, unsigned int port) : height{height}, width{width}, turn_speed{turn_speed}, fps{fps}, port{port} {
    current_game = game(height, width, turn_speed);
}
//-------------METHODS-------------
void event_msg::validate()
{
    if (len != 0)
        return;
    //TODO
}
string event_msg::to_string()
{
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
    }
    msg.event_data = part_msg;
    return msg;
}

unsigned int game::get_height() const
{
    return height;
}