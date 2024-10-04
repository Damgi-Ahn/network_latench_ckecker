#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;

int main()
{
  try {
    asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

    std::cout << "Server is listening on port 8080..." << std::endl;

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

        asio::write(socket, asio::buffer(data, length));
        std::cout << "Echoed " << length << " bytes back to client." << std::endl;
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}