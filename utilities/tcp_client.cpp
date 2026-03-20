#include "tcp_client.hpp"

namespace hemisphere_gnss_v500_driver
{


// need to add a method to send gga to the console

TCP_Client::TCP_Client(std::string ip_address, int port, int timeout_s, int buffer_size, int connect_retry_interval_s, rclcpp::Logger logger)
    : ip_address_(ip_address), port_(port), timeout_s_(timeout_s), running_(false), buffer_size_(buffer_size), connect_retry_interval_s_(connect_retry_interval_s), sock_fd_(-1), logger_(logger)
{
    this->buf.resize(4096);
}

void TCP_Client::loadSocketConfigurations()
{
    // Don't reconnect if we are already up
    if (sock_fd_ >= 0) {
        return;
    }
    this->connectToServer();
}

void TCP_Client::connectToServer()
{
    std::memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port_));

    if (::inet_pton(AF_INET, ip_address_.c_str(), &server_addr_.sin_addr) != 1) {
        throw std::invalid_argument("Invalid V500 IP format: " + ip_address_);
    }

    while (true) {
        sock_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd_ < 0) {
            RCLCPP_WARN(logger_, "[TCP] socket() failed: %s, retrying in %ds", std::strerror(errno), connect_retry_interval_s_);
            std::this_thread::sleep_for(std::chrono::seconds(connect_retry_interval_s_));
            continue;
        }

        // Set non-blocking so connect() returns immediately with EINPROGRESS
        ::fcntl(sock_fd_, F_SETFL, ::fcntl(sock_fd_, F_GETFL, 0) | O_NONBLOCK);

        int ret = ::connect(sock_fd_, reinterpret_cast<sockaddr*>(&server_addr_), sizeof(server_addr_));
        if (ret < 0 && errno != EINPROGRESS) {
            RCLCPP_WARN(logger_, "[TCP] connect() to V500 failed: %s, retrying in %ds", std::strerror(errno), connect_retry_interval_s_);
            ::close(sock_fd_);
            sock_fd_ = -1;
            std::this_thread::sleep_for(std::chrono::seconds(connect_retry_interval_s_));
            continue;
        }

        // Wait up to connect_retry_interval_s_ for the connection to complete
        pollfd pfd{};
        pfd.fd = sock_fd_;
        pfd.events = POLLOUT;
        int pret = ::poll(&pfd, 1, connect_retry_interval_s_ * 1000);

        if (pret <= 0) {
            RCLCPP_WARN(logger_, "[TCP] connect() to V500 timed out, retrying in %ds", connect_retry_interval_s_);
            ::close(sock_fd_);
            sock_fd_ = -1;
            continue;
        }

        // Check if the connection actually succeeded
        int sock_err = 0;
        socklen_t len = sizeof(sock_err);
        ::getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, &sock_err, &len);
        if (0 != sock_err) {
            RCLCPP_WARN(logger_, "[TCP] connect() to V500 failed: %s, retrying in %ds", std::strerror(sock_err), connect_retry_interval_s_);
            ::close(sock_fd_);
            sock_fd_ = -1;
            std::this_thread::sleep_for(std::chrono::seconds(connect_retry_interval_s_));
            continue;
        }

        // Restore blocking mode for the receive loop
        ::fcntl(sock_fd_, F_SETFL, ::fcntl(sock_fd_, F_GETFL, 0) & ~O_NONBLOCK);

        RCLCPP_INFO(logger_, "[TCP] Connected to V500 at %s:%d", ip_address_.c_str(), port_);
        running_ = true;
        break;
    }
}

bool TCP_Client::sendCommand(const std::string& command)
{
    if (sock_fd_ < 0 || !running_) return false;

    // Logic: Hemisphere commands MUST end in \r\n
    std::string formatted_cmd = command;
    if (formatted_cmd.size() < 2 || formatted_cmd.substr(formatted_cmd.size() - 2) != "\r\n") {
        while(!formatted_cmd.empty() && (formatted_cmd.back() == '\n' || formatted_cmd.back() == '\r')) {
            formatted_cmd.pop_back();
        }
        formatted_cmd += "\r\n";
    }

    ssize_t n = ::send(sock_fd_, formatted_cmd.c_str(), formatted_cmd.size(), 0);
    return (n > 0);
}

void TCP_Client::run()
{
    // Only spawn the thread if we aren't already running
    if (running_ && !rx_thread_.joinable()) {
        rx_thread_ = std::thread(&TCP_Client::receiveBytes, this);
    }
}

void TCP_Client::receiveBytes()
{
    const int timeout_ms = (timeout_s_ <= 0) ? 1000 : (timeout_s_ * 1000);
    while (running_) {
        pollfd pfd{};
        pfd.fd = sock_fd_;
        pfd.events = POLLIN;

        int pret = ::poll(&pfd, 1, timeout_ms);

        if (pret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (pret == 0 || !(pfd.revents & POLLIN)) continue;

        ssize_t n = ::recv(sock_fd_, buf.data(), buf.size(), 0);

        if (n < 0) {
            if (errno == EINTR) continue;
            RCLCPP_ERROR(logger_, "[TCP] recv() error: %s", std::strerror(errno));
            // fall through to reconnect
        } else if (n == 0) {
            RCLCPP_WARN(logger_, "[TCP] V500 closed the connection.");
            // fall through to reconnect
        } else {
            if (bytes_callback_) {
                bytes_callback_(std::vector<uint8_t>(buf.begin(), buf.begin() + n));
            }
            continue;
        }

        // Lost connection — close socket and reconnect
        ::shutdown(sock_fd_, SHUT_RDWR);
        ::close(sock_fd_);
        sock_fd_ = -1;
        running_ = false;
        RCLCPP_WARN(logger_, "[TCP] Attempting to reconnect to V500...");
        connectToServer();
    }
}

void TCP_Client::setBytesCallback(std::function<void(const std::vector<uint8_t>&)> callback)
{
    this->bytes_callback_ = callback;
}

void TCP_Client::disconnectFromServer()
{
    running_ = false;

    if (sock_fd_ >= 0) {
        ::shutdown(sock_fd_, SHUT_RDWR);
        ::close(sock_fd_);
        sock_fd_ = -1;
    }

    if (rx_thread_.joinable()) {
        rx_thread_.join();
    }
}

TCP_Client::~TCP_Client()
{
    disconnectFromServer();
}

} // namespace hemisphere_gnss_v500_driver
