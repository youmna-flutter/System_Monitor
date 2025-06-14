# Linux System Monitor

![Demo Screenshot](./screenshot.png) *(Add actual screenshot later)*

## Features
- Process listing with detailed stats
- System resource monitoring
- Keyboard-controlled interface
- Lightweight (~2MB compiled)

## Build & Run
```bash
# Dependencies
sudo apt install libncurses-dev

# Compile
make && ./sysmonitor
```
**Controls:**  
`q`=Quit | `c`=Sort by CPU | `m`=Sort by Memory

## Technical Details
- Pure C (C11 standard)
- Ncurses for TUI
- Linux `/proc` filesystem
- POSIX-compliant process handling

## Roadmap
- [ ] Network I/O monitoring
- [ ] Process killing functionality
- [ ] Configurable themes

## Recommended Repository Structure
linux-system-monitor/
├── src/
│   ├── main.c          # Entry point
│   ├── process.c       # Process listing logic
│   ├── stats.c         # CPU/memory calculations
│   └── ui.c           # Ncurses interface
├── include/
│   └── sysmonitor.h    # All headers
├── Makefile
├── README.md           # Project documentation
└── screenshot.png      # Demo image

