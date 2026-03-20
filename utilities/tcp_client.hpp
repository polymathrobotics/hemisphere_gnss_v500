#ifndef HEMISPHERE_GNSS_V500_DRIVER__TCP_CLIENT_HPP_
#define HEMISPHERE_GNSS_V500_DRIVER__TCP_CLIENT_HPP_

// C++ stdlib
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <atomic>
<<<<<<< HEAD
=======
#include <chrono>
>>>>>>> main
#include <functional>
#include "rclcpp/rclcpp.hpp"
// C / POSIX networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
<<<<<<< HEAD
=======
#include <fcntl.h>
>>>>>>> main
#include <poll.h>
#include <unistd.h>       // close()

// Errors / utilities
#include <cerrno>
#include <cstring>

namespace hemisphere_gnss_v500_driver
{

    class TCP_Client
    {
        public:
            /**
             * @brief Construct a new TCP Client for the V500
             * @param ip_address The IP address of the GNSS unit (e.g., "192.168.100.1")
             * @param port The data port (usually 2000 or 1000)
             * @param timeout_s Timeout for the poll() loop in seconds
             */
<<<<<<< HEAD
            TCP_Client(std::string ip_address, int port, int timeout_s, int buffer_size);
=======
            TCP_Client(std::string ip_address, int port, int timeout_s, int buffer_size, int connect_retry_interval_s, rclcpp::Logger logger);
>>>>>>> main

            /**
             * @brief Default Constructor
            **/
            TCP_Client();
            /**
             * @brief Destructor ensures clean disconnection and thread joining
             */
            ~TCP_Client();

            /**
             * @brief Initializes and connects the socket
             */
            void loadSocketConfigurations();

            /**
             * @brief Initiates the TCP handshake with the V500
             */
            void connectToServer();

            /**
             * @brief Closes the connection and stops the RX thread
             */
            void disconnectFromServer();

            /**
             * @brief Spawns the background thread to begin receiving data
             */
            void run();

            /**
             * @brief The internal loop that handles poll() and recv()
             */
            void receiveBytes();

            /**
             * @brief Sends a $JASC or NMEA command to the V500
             * @param command The string command (e.g., "$JASC,GPGGA,10")
             * @return true if successfully sent
             */
            bool sendCommand(const std::string& command);

            /**
             * @brief Sets the function to be called when new bytes arrive
             */
            void setBytesCallback(std::function<void(const std::vector<uint8_t>&)> callback);

        private:
            // Connection parameters
            std::string ip_address_;
            int port_;
            int timeout_s_;
            int buffer_size_;
<<<<<<< HEAD
=======
            int connect_retry_interval_s_;
            rclcpp::Logger logger_;
>>>>>>> main

            // Thread-safe state control
            std::atomic<bool> running_;

            // Socket resources
            int sock_fd_ = -1;
            sockaddr_in server_addr_ {};

            // Data handling
            std::vector<uint8_t> buf;
            std::function<void(const std::vector<uint8_t>&)> bytes_callback_;

            // Thread handle
            std::thread rx_thread_;
    };

} // namespace hemisphere_gnss_v500_driver

#endif  // HEMISPHERE_GNSS_V500_DRIVER__TCP_CLIENT_HPP_
