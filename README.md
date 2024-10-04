# Network Latency Checker

This project is a tool for measuring network latency and data transmission performance between a client and a server. It is written in C++ using the ASIO library.

Send 100 KB, 1 MB and 10 MB of data from the client to the server, 
performing each transmission 10 times. Measure and report the round-trip time for each size, 
including the minimum, maximum, average times, and data throughput.

## Operating System

- Linux

## Developer

- Damgi

## Build Instructions

1. **Install CMake**: Make sure you have CMake installed. If it's not installed, you can install it using your package manager.

   ```bash
   sudo apt update
   sudo apt install cmake
2. Download the Source Code: Clone the repository.

   ```bash
   git clone https://github.com/your_username/network_latency_checker.git
   cd network_latency_checker

3. Create a Build Directory: Create and navigate to a build directory.

   ```bash
   mkdir build
   cd build
   
4. Build with CMake: Run CMake to build the project.

   ```bash
   cmake ..
   make
   
5. Executable Files: Once the build is complete, 
the network_latency_checker_client and network_latency_checker_server executables will be generated.

## Usage

1. Run the Server: Start the server first with the following command:

   ```bash
   ./network_latency_checker_server --port 8080
   ./network_latency_checker_server --use-round-trip true

  - --port: Set server port number. The default is 8080.
  - --use-round-trip: Set round trip chunks. It can be specified as true or false. The default is False.

3. Run the Client: With the server running, execute the client. You can pass necessary arguments as follows:

   ```bash
   ./network_latency_checker_client
   ./network_latency_checker_client --chunk-size 65536 --use-chunks true
   ./network_latency_checker_client --use-round-trip true

  - --port: Set server port number. The default is 8080.
  - --chunk-size: Specify the chunk size of the data to be transmitted. The default is 64KB.
  - --use-chunks: Set whether to transmit in chunks. It can be specified as true or false. The default is False.
  - --use-round-trip: Set round trip chunks. It can be specified as true or false. The default is False.

## License

MIT License. Use this software under the following conditions.
   
