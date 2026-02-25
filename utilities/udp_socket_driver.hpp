#pragma once

// C++ stdlib
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

// C / POSIX networking + select
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <poll.h>

#include <sys/select.h>   // select()
#include <unistd.h>       // close()

// Errors / utilities
#include <cerrno>         // errno
#include <cstring>        // std::strerror
#include <functional>

namespace hemisphere_gnss_v500_driver
{

    class UDP_SOCKET_DRIVER
    {

        public:
            UDP_SOCKET_DRIVER(std::string ip_address, int port, int timeout_s);
            ~UDP_SOCKET_DRIVER();

            void loadSocketConfigurations();
            void setupUDPSocket();
            void receiveBytes();
            void run();
            void shutdownUDPSocket();
            void setBytesCallback(std::function<void(const std::vector<uint8_t>&)> callback);

        private:

            std::string ip_address_;
            int port_;
            int timeout_s_;
            bool running_;
            int sock_fd_ = -1;
            sockaddr_in bind_addr_ {};
            std::vector<uint8_t> buf;
            std::function<void(const std::vector<uint8_t>&)> bytes_callback_;

    };

}
