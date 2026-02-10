# Enshrouded Monitor (EMon)

A lightweight, htop-inspired CLI tool for monitoring Enshrouded dedicated servers on Debian Linux.

## Features

### Phase 1 (Current)
- ✅ Real-time CPU usage monitoring with visual bar
- ✅ RAM usage monitoring with danger threshold (>12GB warning)
- ✅ Process discovery for EnshroudedServer.exe (Wine/Proton)
- ✅ Process-specific uptime calculation
- ✅ Memory usage per process

### Phase 2 (Current)
- ✅ A2S_INFO protocol integration for server version
- ✅ Live player count via Source Engine Query (UDP port 15637)
- ✅ Server status detection (Lobby/Loading/Host_Online)
- ✅ Server name, map, and game information display
- ✅ Challenge-response support for secured servers
- ✅ Query timeout handling (2 second timeout)

### Phase 3 (Planned)
- 🔲 Player table with names and IPs
- 🔲 Individual connection duration tracking
- 🔲 Log file tailing with inotify

### Phase 4 (Planned)
- 🔲 "World Saved" notifications
- 🔲 Background log scrubber

## Requirements

- Debian Linux (or compatible distribution)
- ncurses library
- gcc compiler
- make

## Installation

Install dependencies:
```bash
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev build-essential
```

Build the project:
```bash
make
```

## Usage

Run the monitor:
```bash
./emon 10.0.2.33              # Monitor server at 10.0.2.33:15637
./emon 10.0.2.33 15637        # Specify custom port
./emon 192.168.1.100          # Monitor different server
```

**Controls:**
- `q` - Quit the application

**Arguments:**
- `host` - Server hostname or IP address (required)
- `port` - Query port (optional, default: 15637)

## Architecture

**System Monitoring:**
- Reads `/proc/stat` for CPU usage calculation
- Reads `/proc/meminfo` for memory statistics
- Independent refresh rate to prevent UI stutter

**Process Monitoring:**
- Scans `/proc` filesystem to find EnshroudedServer process
- Reads `/proc/[pid]/cmdline` to detect Wine processes
- Calculates process-specific uptime using boot time and starttime

**UI Design:**
- ncurses-based interface with color support
- Real-time visual progress bars
- Danger threshold highlighting for RAM (>12GB)

## Development

Build with debug symbols:
```bash
make debug
```

Clean build artifacts:
```bash
make clean
```

Build and run:
```bash
make run
```

Test A2S query independently:
```bash
make test
./test_a2s 10.0.2.33           # Test server at 10.0.2.33:15637
./test_a2s 10.0.2.33 15637     # Specify custom port
./test_a2s 192.168.1.10 25637  # Test different server/port
```

## Technical Details

**Target Server:**
- Process: EnshroudedServer.exe (running via Wine/Proton)
- Game Port: UDP 15636
- Query Port: UDP 15637
- Default Log: `./logs/enshrouded_server.log`

**Performance:**
- Refresh interval: 1000ms
- Minimal CPU overhead
- Non-blocking I/O for log monitoring (Phase 3)

## License

MIT License - See LICENSE file for details
