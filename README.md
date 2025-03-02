# System Information Utility

A comprehensive system monitoring tool built with Qt that provides detailed information about various hardware and system components on Linux systems.

## Features

- **CPU Information**: View detailed CPU specifications including processor name, vendor, core count, frequency, cache sizes, and CPU flags.
- **Memory Usage**: Monitor RAM and swap usage with real-time graphs and detailed statistics.
- **Disk Information**: View disk partitions, usage, and file system details.
- **Network Information**: Monitor network interfaces and connection statistics.
- **USB Devices**: Track connected USB devices and their properties.
- **System Services**: View and manage system services.
- **Dark/Light Theme**: Toggle between dark and light themes for comfortable viewing.

## Screenshots

*Screenshots will be added soon*

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
- Qt 5 (Core and Widgets modules)
- Qt Charts module (optional, for memory usage graphs)

## Building

```bash
Clone the repository
git clone https://github.com/yourusername/system-info.git
cd system-info
Create build directory
mkdir build
cd build
Configure and build
cmake ..
make
Run the application
./QTSystemStats
```

## Optional Dependencies

- **Qt Charts**: For memory usage visualization
  - Ubuntu/Debian: `sudo apt install libqt5charts5-dev`
  - Fedora/CentOS: `sudo dnf install qt5-qtcharts-devel`

## Project Structure

- `src/`: Source files containing the implementation
- `include/`: Header files defining the interfaces
- `resources/`: Icons and other resources

## Implementation Details

- Uses `/proc` filesystem to gather CPU, memory, and network information
- Parses system files like `/proc/cpuinfo`, `/proc/meminfo`, and `/sys/devices` for hardware details
- Monitors system services through systemd interfaces
- Implements a theme manager for consistent styling across the application
- Uses Qt Charts for visualizing memory and CPU usage over time

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Acknowledgments

- Qt framework for the GUI components
- Linux kernel for providing system information interfaces