#include "misc_structs.h"
#include "parser.h"
#include <math.h>

//-------------CONSTRUCTORS-------------
server::server_t(int32_t height, int32_t width, uint32_t turn_speed, uint32_t fps, uint32_t port)
    : height{height}, width{width}, turn_speed{turn_speed}, fps{fps}, port{port}
{
    rng = RNG();
    current_game = game(height, width, turn_speed, &rng, this);
}
//-------------METHODS-------------
string server::client_id_to_str(client_key key)
{
    string s = "<";
    s += std::to_string(key.first);
    s += ',';
    s += std::to_string(key.second.first.first);
    s += std::to_string(key.second.first.second);
    s += std::to_string(key.second.second.first);
    s += std::to_string(key.second.second.second);
    s += '>';
    return s;
}
client_key server::client_identifier(sockaddr_in6 *client_address)
{
    return std::make_pair(client_address->sin6_port, std::make_pair(std::make_pair(client_address->sin6_addr.__in6_u.__u6_addr32[0], client_address->sin6_addr.__in6_u.__u6_addr32[1]), std::make_pair(client_address->sin6_addr.__in6_u.__u6_addr32[2], client_address->sin6_addr.__in6_u.__u6_addr32[3])));
}

void server::broadcast(string msg)
{
    std::cout << "SERVER::broadcast\n";
    string datagram = int_to_fb(current_game.game_id);
    datagram += msg;
    sflags = 0;
    for (auto it : clients)
    {
        auto client = it.second;
        sendto(sock, datagram.c_str(), (size_t)datagram.length(), sflags,
               (struct sockaddr *)&(client.client_address), snda_len);
    }
    errno = 0;
}

void server::broadcast_many_events(sockaddr_in6 *client_address, uint32_t no_event)
{
    std::cout << "SERVER::broadcast_many_events\n";
    uint64_t total_events = current_game.events.size();
    if (no_event >= current_game.events.size())
        return;
    string datagram = int_to_fb(current_game.game_id);
    uint64_t curr_len = 4;
    while (no_event < total_events && curr_len + current_game.events[no_event].size() < 550)
    {
        datagram += current_game.events[no_event++];
    }
    sendto(sock, datagram.c_str(), (size_t)datagram.length(), sflags,
           (struct sockaddr *)client_address, snda_len);
    errno = 0;
    broadcast_many_events(client_address, no_event);
}

uint32_t server::check_client(sockaddr_in6 *client_address, client_msg msg)
{
    std::cout << "SERVER::check_client\n";
    auto it = clients.find(client_identifier(client_address));
    if (it == clients.end() && !current_game.player_exists(msg.player_name))
        return 0;
    if (it == clients.end())
        return 2;
    auto client = it->second;
    if (client.session_id == msg.session_id && client.name == msg.player_name)
        return 1;
    if (client.session_id < msg.session_id)
    {
        clients.erase(client_identifier(client_address));
        return 0;
    }
    return 2;
}

void server::handle_new_client(sockaddr_in6 *client_address, client_msg msg)
{
    std::cout << "SERVER::handle_new_client\n";
    if (clients.size() == 25 || current_game.no_of_participants == 25)
        return;
    client new_client(frame_number, *client_address, msg.session_id, msg.player_name);
    new_client.dir = msg.turn_dir;
    if (new_client.dir == 2)
        new_client.dir = -1;
    auto id = client_identifier(client_address);
    clients.insert(std::make_pair(id, new_client));
    if (!current_game.game_started)
        current_game.add_player(msg.player_name);
    broadcast_many_events(client_address, msg.next_event_no);
}

void server::handle_old_client(sockaddr_in6 *client_address, client_msg msg)
{
    std::cout << "SERVER::handle_old_client\n";
        auto client = clients.find(client_identifier(client_address))->second;
    client.last_msg_time_stamp = frame_number;
    if (current_game.player_exists(msg.player_name))
        current_game.player_turning[current_game.player_number(msg.player_name)];
    broadcast_many_events(client_address, msg.next_event_no);
}

void server::start()
{

    struct sockaddr_in6 server_address;
    struct sockaddr_in6 client_address;

    char buffer[33];

    ssize_t len;

    struct pollfd p;
    int timerfd;
    struct itimerspec timerValue;

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;

    /* clear pollfd */
    bzero(&p, sizeof(p));

    /* set timerfd */
    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    if (timerfd < 0)
    {
        printf("failed to create timer fd\n");
        exit(1);
    }
    bzero(&timerValue, sizeof(timerValue));
    timerValue.it_value.tv_sec = 0;
    timerValue.it_value.tv_nsec = 1000 * 1000 * 1000 / fps;
    timerValue.it_interval.tv_sec = 0;
    timerValue.it_interval.tv_nsec = 1000 * 1000 * 1000 / fps;

    /* set events */
    p.fd = timerfd;
    p.revents = 0;
    p.events = POLLIN;

    /* start timer */
    if (timerfd_settime(timerfd, 0, &timerValue, NULL) < 0)
    {
        printf("could not start timer\n");
        exit(1);
    }

    sock = socket(PF_INET6, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if (sock < 0)
        syserr("socket");
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
    // after socket() call; we should close(sock) on any execution path;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;  // IPv4
    server_address.sin6_addr = in6addr_any; // listening on all interfaces
    server_address.sin6_port = htons(port); // default port for receiving is PORT_NUM

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *)&server_address,
             (socklen_t)sizeof(server_address)) < 0)
        syserr("bind");

    snda_len = (socklen_t)sizeof(client_address);
    frame_number = 0;
    for (;;)
    {

        uint32_t numEvents = poll(&p, 1, -1);
        if (numEvents > 0)
        {
            frame_number++;
            std::cout << "\n\n\nSERVER::start::Frame number:" << frame_number << "\n";
            uint64_t timersElapsed = 0;
            (void)read(p.fd, &timersElapsed, 8);
            std::cout << "SERVER::start::Trying generating new frame\n";
            std::cout << "SERVER::start::Total players " << current_game.total_players << "\n";
            if (!current_game.game_started && current_game.total_players > 1)
            {
                std::cout << "SERVER::start::Trying starting new game\n";
                uint32_t i = 0;
                for (auto it : clients)
                {
                    auto client = it.second;
                    if (client.name != "" && client.dir)
                        i++;
                }
                if (i == current_game.total_players)
                {
                    std::cout << "SERVER::start::Starting new game\n";
                    current_game.start_game();
                }
            }
            else if (current_game.game_started && !current_game.game_ended)
            {
                std::cout << "SERVER::start::Generating new frame\n";
                current_game.next_frame();
            }
            if (current_game.game_ended)
            {
                std::cout << "SERVER::start::Generating new game\n";
                current_game = game(height, width, turn_speed, &rng, this);
                for (auto it : clients)
                {
                    auto client = it.second;
                    current_game.add_player(client.name);
                }
            }
        }
        rcva_len = (socklen_t)sizeof(client_address);
        flags = 0; // we do not request anything special
        len = recvfrom(sock, buffer, sizeof(buffer), flags,
                       (struct sockaddr *)&client_address, &rcva_len);
        if (errno == 11)
        {
            errno = 0;
            continue;
        }

        if (len >= 13)
        {
            (void)printf("SERVER::start::read from socket: %zd bytes: %.*s\n", len,
                         (int32_t)len, buffer);
            client_msg received(len, buffer);
            uint32_t client_type = check_client(&client_address, received);
            std::cout << "SERVER::case " << client_type << "\n";
            switch (client_type)
            {
            case 0:
                std::cout << "SERVER::start::new client " << client_id_to_str(client_identifier(&client_address)) << "\n";
                handle_new_client(&client_address, received);
                break;
            case 1:
                handle_old_client(&client_address, received);
                break;

            default:
                break;
            }
            sflags = 0;
            sendto(sock, buffer, (size_t)len, sflags,
                   (struct sockaddr *)&client_address, snda_len);
        }
    }
}