#include <iostream>
#include <chrono>
#include <asio.hpp>
#include <vector>
#include <limits>
#include <cstring> // For std::strcmp

using asio::ip::tcp;

// 전송 크기별 데이터 생성 함수
std::vector<char> generate_data(size_t size)
{
  std::vector<char> data(size);
  std::fill(data.begin(), data.end(), 'A'); // 'A'로 데이터 채우기
  return data;
}

// 소요 시간 및 초당 전송량 통계 출력 함수
void print_statistics(const std::vector<long long> &times, const std::vector<double> &throughputs)
{
  if (times.empty() || throughputs.empty())
    return;

  long long min_time = std::numeric_limits<long long>::max();
  long long max_time = std::numeric_limits<long long>::min();
  long long total_time = 0;

  double min_throughput = std::numeric_limits<double>::max();
  double max_throughput = std::numeric_limits<double>::min();
  double total_throughput = 0;

  for (size_t i = 0; i < times.size(); ++i)
  {
    long long t = times[i];
    double tp = throughputs[i];

    if (t < min_time)
      min_time = t;
    if (t > max_time)
      max_time = t;
    total_time += t;

    if (tp < min_throughput)
      min_throughput = tp;
    if (tp > max_throughput)
      max_throughput = tp;
    total_throughput += tp;
  }

  double avg_time = total_time / static_cast<double>(times.size());
  double avg_throughput = total_throughput / static_cast<double>(throughputs.size());

  std::cout << "Min time: " << min_time << " ms" << std::endl;
  std::cout << "Max time: " << max_time << " ms" << std::endl;
  std::cout << "Avg time: " << avg_time << " ms" << std::endl;

  std::cout << "Min throughput: " << min_throughput << " Mbps" << std::endl;
  std::cout << "Max throughput: " << max_throughput << " Mbps" << std::endl;
  std::cout << "Avg throughput: " << avg_throughput << " Mbps" << std::endl;
}

// 청크 단위로 데이터를 전송하는 함수
void send_data_in_chunks(tcp::socket &socket, const std::vector<char> &data, size_t chunk_size)
{
  size_t total_sent = 0;
  while (total_sent < data.size())
  {
    size_t to_send = std::min(chunk_size, data.size() - total_sent);
    asio::write(socket, asio::buffer(&data[total_sent], to_send));
    total_sent += to_send;
  }
}

// 한 번에 데이터를 전송하는 함수
void send_data_at_once(tcp::socket &socket, const std::vector<char> &data)
{
  asio::write(socket, asio::buffer(data));
}

int main(int argc, char *argv[])
{
  try
  {
    // 기본 값
    size_t chunk_size = 64 * 1024; // 기본 청크 크기 (64KB)
    bool use_chunks = true;        // 기본적으로 청크 단위 전송

    // 커맨드 라인 인자 처리
    for (int i = 1; i < argc; ++i)
    {
      if (std::strcmp(argv[i], "--chunk-size") == 0 && i + 1 < argc)
      {
        chunk_size = std::stoul(argv[++i]);
      }
      else if (std::strcmp(argv[i], "--use-chunks") == 0 && i + 1 < argc)
      {
        use_chunks = std::strcmp(argv[++i], "true") == 0;
      }
    }

    // ASIO IO 컨텍스트
    asio::io_context io_context;

    // 서버 주소와 포트 설정
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8080"); // 서버 IP와 포트

    // 소켓 생성 및 TCP_NODELAY 설정
    tcp::socket socket(io_context);
    socket.open(tcp::v4());
    socket.set_option(asio::ip::tcp::no_delay(true)); // Nagle 알고리즘 비활성화
    asio::connect(socket, endpoints);

    // 전송할 데이터 크기 (100KB, 1MB, 10MB, 100MB)
    std::vector<size_t> data_sizes = {100 * 1024, 1 * 1024 * 1024, 10 * 1024 * 1024};

    // 데이터 버퍼 재사용을 위한 벡터
    std::vector<char> data;

    // 각 데이터 크기별로 10번씩 전송
    for (auto data_size : data_sizes)
    {
      data = generate_data(data_size);
      std::vector<long long> times;
      std::vector<double> throughputs;

      std::cout << "\nSending data of size: " << data_size / 1024 << " KB, 10 times..." << std::endl;

      for (int i = 0; i < 10; ++i)
      {
        // 진행 상황 출력
        std::cout << data_size / (1024 * 1024) << "MB [" << (i + 1) << "/10] Sending..." << std::endl;

        // 시작 시간 측정
        auto start = std::chrono::high_resolution_clock::now();

        if (use_chunks)
        {
          // 청크 단위로 데이터 전송
          send_data_in_chunks(socket, data, chunk_size);
        }
        else
        {
          // 한 번에 데이터 전송
          send_data_at_once(socket, data);
        }

        // 서버로부터 응답 수신
        std::vector<char> reply(data_size);
        asio::read(socket, asio::buffer(reply));

        // 종료 시간 측정
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(duration);

        // Throughput 계산 (Mbps 단위)
        double throughput = (data_size * 8.0) / (duration / 1000.0) / (1024 * 1024); // Mbps로 변환
        throughputs.push_back(throughput);
      }

      // 통계 출력 (시간과 초당 전송량)
      print_statistics(times, throughputs);
    }

    // 소켓 닫기
    socket.close();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}