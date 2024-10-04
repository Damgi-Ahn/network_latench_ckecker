#include <iostream>
#include <asio.hpp>
#include <cstring>
#include <vector>

using asio::ip::tcp;

int main(int argc, char *argv[])
{
  try {
    int server_port = 8080;
    bool use_round_trip = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc)
        server_port = std::stoi(argv[++i]);
      else if (std::strcmp(argv[i], "--use-round-trip") == 0 && i + 1 < argc)
        use_round_trip = std::strcmp(argv[++i], "true") == 0;
    }

    std::cout << "server_port: " << server_port << std::endl;
    std::cout << "use_round_trip: " << use_round_trip << std::endl;

    asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), server_port));

    std::cout << "Server is listening on port " << server_port << "..." << std::endl;

    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      std::cout << "Client connected!" << std::endl;

      while (true) {
        std::vector<char> data(1024 * 1024 * 100);
        asio::error_code error;

        size_t length = socket.read_some(asio::buffer(data), error);

        if (error == asio::error::eof) {
          std::cout << "Connection closed by client." << std::endl;
          break;
        } else if (error) {
          throw asio::system_error(error);
        }

        if (use_round_trip) {
          try {
            asio::write(socket, asio::buffer(data, length));
            std::cout << "Echoed " << length << " bytes back to client." << std::endl;
          } catch (const asio::system_error &e) {
            std::cerr << "Write failed: " << e.what() << std::endl;
            break; // Exit loop if writing fails
          }
        } else {
          std::vector<char> ok_data({'o', 'k'});
          try {
            asio::write(socket, asio::buffer(ok_data, ok_data.size()));
            std::cout << "Echoed " << ok_data.size() << " bytes back to client." << std::endl;
          } catch (const asio::system_error &e) {
            std::cerr << "Write failed: " << e.what() << std::endl;
            break; // Exit loop if writing fails
          }
        }
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}