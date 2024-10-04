#include <iostream>
#include <chrono>
#include <asio.hpp>
#include <vector>
#include <limits>
#include <cstring>

using asio::ip::tcp;

std::vector<char> generate_data(size_t size)
{
  std::vector<char> data(size);
  std::fill(data.begin(), data.end(), 'A');
  return data;
}

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

  for (size_t i = 0; i < times.size(); ++i) {
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

void send_data_in_chunks(tcp::socket &socket, const std::vector<char> &data, size_t chunk_size)
{
  size_t total_sent = 0;
  while (total_sent < data.size()) {
    size_t to_send = std::min(chunk_size, data.size() - total_sent);
    asio::write(socket, asio::buffer(&data[total_sent], to_send));
    total_sent += to_send;
  }
}

void send_data_at_once(tcp::socket &socket, const std::vector<char> &data)
{
  asio::write(socket, asio::buffer(data));
}

int main(int argc, char *argv[])
{
  try {
    size_t chunk_size = 64 * 1024;
    bool use_chunks = false;
    bool use_round_trip = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--chunk-size") == 0 && i + 1 < argc) {
        chunk_size = std::stoul(argv[++i]); 
      } else if (std::strcmp(argv[i], "--use-chunks") == 0 && i + 1 < argc) {
        use_chunks = std::strcmp(argv[++i], "true") == 0;
      } else if (std::strcmp(argv[i], "--use-round-trip") == 0 && i + 1 < argc) {
        use_round_trip = std::strcmp(argv[++i], "true") == 0;
      }
    }
    std::cout << "chunk_size: " << chunk_size << std::endl;
    std::cout << "use_chunks: " << use_chunks << std::endl;
    std::cout << "use_round_trip: " << use_round_trip << std::endl;


    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8080"); // 서버 IP와 포트

    tcp::socket socket(io_context);
    socket.open(tcp::v4());
    socket.set_option(asio::ip::tcp::no_delay(true));
    asio::connect(socket, endpoints);

    std::vector<size_t> data_sizes = {100 * 1024, 1 * 1024 * 1024, 10 * 1024 * 1024};

    std::vector<char> data;

    for (auto data_size : data_sizes) {
      data = generate_data(data_size);
      std::vector<long long> times;
      std::vector<double> throughputs;

      std::cout << "\nSending data of size: " << data_size / 1024 << " KB, 10 times..." << std::endl;

      for (int i = 0; i < 10; ++i) {
        std::cout << data_size / (1024 * 1024) << "MB [" << (i + 1) << "/10] Sending..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        if (use_chunks) {
          send_data_in_chunks(socket, data, chunk_size);
        } else {
          send_data_at_once(socket, data);
        }

        if (use_round_trip) {
          std::vector<char> reply(data_size);
          asio::read(socket, asio::buffer(reply));
        } else {
          std::vector<char> reply(2);
          asio::read(socket, asio::buffer(reply));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(duration);

        double throughput = (data_size * 8.0) / (duration / 1000.0) / (1024 * 1024); // Mbps로 변환
        throughputs.push_back(throughput);
      }

      print_statistics(times, throughputs);
    }
    socket.close();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}