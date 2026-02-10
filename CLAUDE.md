# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Enshrouded-Monitor (EMon)**: A lightweight, htop-inspired CLI tool written in C to monitor an Enshrouded dedicated server on Debian. Provides real-time system performance alongside game-specific telemetry like player counts and server versioning.

**Target Environment:**
- Host OS: Debian Linux
- Language: C with ncurses UI
- Target Process: EnshroudedServer.exe (running via Wine/Proton)
- Default Log Path: `./logs/enshrouded_server.log`
- Network Ports: UDP 15636 (Game), UDP 15637 (Query)

## Development Commands

**Build:**
```bash
make              # Build main application
make test         # Build test utilities
make debug        # Build with debug symbols
make clean        # Remove build artifacts
```

**Usage:**
```bash
./emon <host> [port]           # Run monitor (host required, port default: 15637)
./test_a2s <host> [port]       # Test A2S query independently

# Examples
./emon 10.0.2.33               # Monitor server at 10.0.2.33:15637
./emon 10.0.2.33 15637         # Specify custom port
./test_a2s 10.0.2.33           # Test query to 10.0.2.33:15637
```

**Requirements:**
- ncurses library: `sudo apt-get install libncurses5-dev libncursesw5-dev`
- gcc with C11 support
- Linux kernel with /proc filesystem

## Architecture

### Core Components

**Telemetry Sources:**
1. **A2S Query (Source A)** - `a2s_query.c/h`: UDP socket implementation to query localhost:15637 using Source Engine Query protocol
   - Implements full A2S_INFO protocol with challenge-response support
   - 2-second timeout for non-responsive servers
   - Parses server name, version, map, player count, and status
   - Status detection: Lobby, Loading, or Host_Online based on server name/map
   - Independent test utility: `test_a2s.c`

2. **Log Tailing (Source B)** - Phase 3: inotify-based log file watcher
   - Monitors `enshrouded_server.log` for new entries
   - Extracts player names, IPs, and connection timestamps via regex
   - Captures "World Saved" notifications

**UI Components (ncurses):**
- CPU/RAM visualization bars
- RAM danger threshold at 12GB
- Server status indicator: Lobby, Loading, or Host_Online
- Player table showing Name, IP, and connection duration
- Server version and player count (current/max slots, e.g., 4/16)
- Process-specific uptime (via `/proc/[pid]` stat.st_ctime)

**Performance Considerations:**
- ncurses refresh rate must be independent of UDP query timeout to prevent UI stutter
- Efficient log parsing with inotify to minimize CPU usage

### Implementation Phases

1. **Phase 1 (Complete)**: CPU/RAM bars and PID discovery for Enshrouded process
2. **Phase 2 (Complete)**: A2S_INFO integration for live version and player numbers
3. **Phase 3 (Next)**: Detailed player table with Name, IP, and connection duration via log tailing
4. **Phase 4 (Planned)**: Background log scrubber for "World Saved" notifications

## Key Technical Details

**Process Discovery:**
- Locate EnshroudedServer.exe process via `/proc` filesystem
- Calculate process-specific uptime using stat.st_ctime

**Log Parsing:**
- Watch for pattern: `[Session] 'PlayerName' joined` to extract player info
- Extract public IP addresses from connection events

**Network Protocol:**
- Implement Steam A2S_INFO protocol for server queries
- Handle UDP socket communication with proper timeout handling
