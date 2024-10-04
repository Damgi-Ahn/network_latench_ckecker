#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;

int main()
{
  try {
    // ASIO IO 컨텍스트
    asio::io_context io_context;

    // 서버 소켓 설정 (localhost:8080)
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true)
    {
      // 클라이언트 연결 수락
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      std::cout << "Client connected!" << std::endl;

      // 클라이언트로부터 데이터 읽기 및 에코
      while (true) {
        std::vector<char> data(1024 * 1024 * 100); // 최대 100MB 버퍼
        asio::error_code error;

        // 데이터 읽기
        size_t length = socket.read_some(asio::buffer(data), error);

        if (error == asio::error::eof) {
          std::cout << "Connection closed by client." << std::endl;
          break; // 클라이언트가 연결을 닫으면 종료
        } else if (error) {
          throw asio::system_error(error); // 다른 에러 발생 시 예외 처리
        }

        // 읽은 데이터를 클라이언트에게 다시 전송 (에코)
        asio::write(socket, asio::buffer(data, length));
        std::cout << "Echoed " << length << " bytes back to client." << std::endl;
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}