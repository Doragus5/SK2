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

void server::start()
{
    int sock;
    int flags, sflags;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    char buffer[BUFFER_SIZE];
    socklen_t snda_len, rcva_len;
    ssize_t len, snd_len;

    sock = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if (sock < 0)
        syserr("socket");
    // after socket() call; we should close(sock) on any execution path;

    server_address.sin_family = AF_INET;                // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port);              // default port for receiving is PORT_NUM

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *)&server_address,
             (socklen_t)sizeof(server_address)) < 0)
        syserr("bind");

    snda_len = (socklen_t)sizeof(client_address);
    for (;;)
    {
        do
        {
            rcva_len = (socklen_t)sizeof(client_address);
            flags = 0; // we do not request anything special
            len = recvfrom(sock, buffer, sizeof(buffer), flags,
                           (struct sockaddr *)&client_address, &rcva_len);
            if (len < 0)
                syserr("error on datagram from client socket");
            else
            {
                (void)printf("read from socket: %zd bytes: %.*s\n", len,
                             (int)len, buffer);
                sflags = 0;
                snd_len = sendto(sock, buffer, (size_t)len, sflags,
                                 (struct sockaddr *)&client_address, snda_len);
                if (snd_len != len)
                    syserr("error on sending datagram to client socket");
            }
        } while (len > 0);
        (void)printf("finished exchange\n");
    }
}