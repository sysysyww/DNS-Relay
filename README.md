# DNS-Relay

This project is a DNS relay server implemented in C, designed to handle DNS queries by either responding from a local cache or forwarding the request to an external DNS server.

## Features

- **Local Cache**: Stores DNS query results to improve response times for frequently accessed domains.
- **Multithreading**: Utilizes a thread pool to handle multiple client requests concurrently, enhancing performance under load.
- **Domain Blocking**: Supports blocking specific domains by mapping them to `0.0.0.0` in the `dnsrelay.txt` file.
- **Configuration File**: Uses `dnsrelay.txt` to map domain names to IP addresses, allowing for easy customization.

## Getting Started

### Prerequisites

- A POSIX-compliant operating system (e.g., Linux, macOS).
- GCC compiler.

### Installation

1. **Clone the repository**:
   
    ```bash
    git clone https://github.com/sysysyww/DNS-Relay.git
    ```
   
2. **Navigate to the project directory**:
   
    ```bash
    cd DNS-Relay
    ```
3. **Compile the source code**:

    ```bash
    gcc -o dns_relay main.c cache.c communication.c encapsulation.c hash_table.c ip_address.c log.c parser.c pthread_pool.c serve.c -lpthread
    ```

### Usage
1. **Edit the dnsrelay.txt file**:
  * Add domain-to-IP mappings in the following format:
    ```
    127.0.0.1 localhost
    0.0.0.0 blocked-domain.com
    ```
    
2. **Run the DNS relay server**:

    ```bash
    sudo ./dns_relay
    ```
    Note: Root privileges are required to bind to port 53.

3. **Configure your system's DNS settings**:
  * Set the DNS server to 127.0.0.1 to route queries through the DNS relay.
