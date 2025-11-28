#pragma once

#include <thread>
#include <vector>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/registered_buffer.hpp>
#include <asio/write.hpp>

#include "Events.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "ServerLaunch.hpp"

#define MAX_PLAYERS 4

class UDPSocket {
    public:
        UDPSocket(asio::io_context &io_context, int port) : _socket(asio::ip::udp::socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))) {};
        ~UDPSocket() = default;

        asio::ip::udp::socket &get() {
            return _socket;
        };

    private:
        asio::ip::udp::socket _socket;
};

class NetworkServer : public APlugin
{
    public:
    NetworkServer(Registery& r, EntityLoader& l)
        : APlugin(r, l, {}, {})
    {
        this->_registery.get().on<ServerLaunching>([this](ServerLaunching const &s){
            this->_threads.emplace_back([this, s](){this->launch_server(s);});
        });
    }

    ~NetworkServer() override
    {
        for (auto& t : this->_threads) {
            t.join();
        }
    }

    private:

        asio::ip::tcp::socket accept_socket(asio::ip::tcp::acceptor &a) {
            asio::ip::tcp::socket s(this->_io_c);
            a.accept(s);
            LOGGER("server", LogLevel::DEBUG, "client accepted");
            return s;
        }

        void launch_server(ServerLaunching const &s) {
            UDPSocket socket(_io_c, s.port);
            this->_registery.get().on<SendMessage>([this, &socket](SendMessage const &m) {
                for (auto &it : this->_clients) {
                    socket.get().send_to(asio::buffer(m.message), it);
                }
                // utiliser des semaphores
            });
            LOGGER("server", LogLevel::DEBUG, "thread started");

            std::array<char, 1> recv_buf;
            for (size_t i = 0; i < MAX_PLAYERS ;++i) {
                socket.get().receive_from(asio::buffer(recv_buf), _clients[i]);

                std::error_code ignored_error;
                socket.get().send_to(asio::buffer("hey"), _clients[i], 0, ignored_error);
            }
        }
        
        void handle_receive(const std::error_code& error, std::size_t) {
            // socket.async_send_to(asio::buffer(*message), _clients,
            //     std::bind(&udp_server::handle_send, this, message,
            //         asio::placeholders::error,
            //         asio::placeholders::bytes_transferred));

        }

        asio::io_context _io_c;
        std::vector<std::thread> _threads;
        std::array<asio::ip::udp::endpoint, MAX_PLAYERS> _clients;
};
