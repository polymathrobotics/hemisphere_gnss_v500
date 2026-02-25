#include "hemisphere_gnss_v500_driver/utilities/udp_socket_driver.hpp"


namespace hemisphere_gnss_v500_driver
{

UDP_SOCKET_DRIVER::UDP_SOCKET_DRIVER(std::string ip_address, int port, int timeout_s, int buf_size)
{
    this->ip_address_ = ip_address;
    this->port_ = port;
    this->timeout_s_ = timeout_s;
    this->running_ = false;
    this->buf.resize(buf_size);
}


void UDP_SOCKET_DRIVER::loadSocketConfigurations()
{
    if (sock_fd_ >= 0) {
        return;
    } else {
        this->setupUDPSocket();
    }
}

void UDP_SOCKET_DRIVER::setupUDPSocket()
{
    // Basic validation
    if (port_ <= 0 || port_ > 65535) {
        throw std::invalid_argument("setupUDPSocket: port out of range");
    }
    if (timeout_s_ <= 0) {
        timeout_s_ = 1;
    }
    if (ip_address_.empty()) {
        ip_address_ = "0.0.0.0";
    }

    // Create UDP socket
    sock_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0) {
        throw std::runtime_error(std::string("socket() failed: ") + std::strerror(errno));
    }

    // Allow quick restart/rebind
    int reuse = 1;
    if (::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ::close(sock_fd_);
        sock_fd_ = -1;
        throw std::runtime_error(std::string("setsockopt(SO_REUSEADDR) failed: ") + std::strerror(errno));
    }

    // Build bind address
    std::memset(&bind_addr_, 0, sizeof(bind_addr_));
    bind_addr_.sin_family = AF_INET;
    bind_addr_.sin_port = htons(static_cast<uint16_t>(port_));

    if (::inet_pton(AF_INET, ip_address_.c_str(), &bind_addr_.sin_addr) != 1) {
        ::close(sock_fd_);
        sock_fd_ = -1;
        throw std::invalid_argument("inet_pton() failed: invalid bind IP '" + ip_address_ + "'");
    }

    // Bind
    if (::bind(sock_fd_, reinterpret_cast<sockaddr*>(&bind_addr_), sizeof(bind_addr_)) < 0) {
        ::close(sock_fd_);
        sock_fd_ = -1;
        throw std::runtime_error(std::string("bind() failed: ") + std::strerror(errno));
    }

    running_ = true;
    // set up bytes buffer here if needed
}


void UDP_SOCKET_DRIVER::setBytesCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    this->bytes_callback_ = callback;
}


void UDP_SOCKET_DRIVER::run(){

    std::thread t(&UDP_SOCKET_DRIVER::receiveBytes, this);
    // include other logic later
}

void UDP_SOCKET_DRIVER::receiveBytes()
{
    if (sock_fd_ < 0) {
        throw std::runtime_error("receiveBytes(): socket not set up (sock_fd_ < 0). Call setupUDPSocket() first.");
    }
    // Convert seconds to milliseconds for poll()
    const int timeout_ms = (timeout_s_ <= 0) ? 1000 : (timeout_s_ * 1000);

    while (running_) {
        pollfd pfd{};
        pfd.fd = sock_fd_;
        pfd.events = POLLIN;

        int pret = ::poll(&pfd, 1, timeout_ms);
        if (pret < 0) {
            if (errno == EINTR) {
                continue;  // interrupted by signal; retry
            }
            throw std::runtime_error(std::string("poll() failed: ") + std::strerror(errno));
        }

        if (pret == 0) {
            continue;
        }

        if (!(pfd.revents & POLLIN)) {
            continue;
        }

        sockaddr_in sender{};
        socklen_t sender_len = sizeof(sender);

        ssize_t n = ::recvfrom(sock_fd_,
                               buf.data(),
                               buf.size(),
                               0,
                               reinterpret_cast<sockaddr*>(&sender),
                               &sender_len);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            // If we are shutting down and socket got closed, exit cleanly
            if (!running_) {
                break;
            }
            throw std::runtime_error(std::string("recvfrom() failed: ") + std::strerror(errno));
        }

        if (n == 0) {
            continue;
        }

        // --- "Process it" step: right now we just print debug info ---
        // Replace this block later with:
        //   - calling a callback, or
        //   - pushing bytes into a queue for your NMEA framer/parser.
        char sender_ip[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &sender.sin_addr, sender_ip, sizeof(sender_ip));

        // Debug statements for now
        std::string s(reinterpret_cast<const char*>(buf.data()), reinterpret_cast<const char*>(buf.data()) + n);
        std::cerr << s << "\n";
        std::cerr << "[UDP] rx " << n << " bytes from "
            << sender_ip << ":" << ntohs(sender.sin_port) << "\n";

        if (bytes_callback_) {
            // need to make this production ready later, but for now we can just pass the raw bytes as a vector
            bytes_callback_(std::vector<uint8_t>(buf.begin(), buf.begin() + n));
        }

    }
}


void UDP_SOCKET_DRIVER::shutdownUDPSocket()
{
    // Signal any receive loop to stop
    this->running_ = false;

    // clean up all threads

    // Closing the socket will help unblock select()/recvfrom()
    if (sock_fd_ >= 0) {
        ::close(sock_fd_);
        sock_fd_ = -1;
    }
}

UDP_SOCKET_DRIVER::~UDP_SOCKET_DRIVER()
{
    shutdownUDPSocket();
}

} // namespace hemisphere_gnss_v500_driver
