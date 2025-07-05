# S32K344 QEMU Development Environment

This project provides a Docker-based development environment for creating S32K344 microcontroller emulation in QEMU.

## Quick Start

1. **Build the Docker environment:**
   ```bash
   ./scripts/docker-dev.sh build
   ```

2. **Start the development container:**
   ```bash
   ./scripts/docker-dev.sh start
   ```

3. **Connect to the development environment:**
   ```bash
   ./scripts/docker-dev.sh shell
   ```

## Docker Environment Features

### Base Configuration
- **OS**: Ubuntu 24.04 LTS
- **Architecture**: ARM cross-compilation support
- **User**: Non-root development user (uid: 1000)
- **Shell**: Bash with development aliases

### Installed Tools
- **Build Tools**: GCC, Meson, Ninja, CMake
- **ARM Toolchain**: `gcc-arm-none-eabi`, `gdb-multiarch`
- **QEMU Dependencies**: All required libraries for building QEMU
- **Development Tools**: Git, Vim, GDB, debugging utilities
- **Python**: Python 3 with pip for QEMU scripts

### Volume Mounts
- **Project Source**: `.` → `/workspace/project` (read-write)
- **QEMU Source**: Persistent volume for QEMU source code
- **Build Artifacts**: Persistent volume for compiled binaries
- **Zephyr SDK**: Persistent volume for Zephyr development
- **Test Results**: Persistent volume for test artifacts

## Development Workflow

### Initial Setup
```bash
# Build the Docker image
./scripts/docker-dev.sh build

# Start the development environment
./scripts/docker-dev.sh start

# Connect to the container
./scripts/docker-dev.sh shell
```

### Inside the Container
```bash
# Your project files are mounted at /workspace/project
cd /workspace/project

# QEMU source will be at /workspace/qemu-src
# Build artifacts will be at /workspace/qemu-build
# Zephyr SDK will be at /workspace/zephyr
```

### Environment Variables
- `QEMU_SOURCE_DIR`: `/workspace/qemu-src`
- `QEMU_BUILD_DIR`: `/workspace/qemu-build`
- `ZEPHYR_BASE`: `/workspace/zephyr`
- `PROJECT_ROOT`: `/workspace/project`
- `CROSS_COMPILE`: `arm-none-eabi-`

## Docker Management Commands

### Container Operations
```bash
./scripts/docker-dev.sh build      # Build the Docker image
./scripts/docker-dev.sh start      # Start the development container
./scripts/docker-dev.sh stop       # Stop the development container
./scripts/docker-dev.sh shell      # Connect to the container shell
./scripts/docker-dev.sh status     # Show container status
./scripts/docker-dev.sh logs       # Show container logs
```

### Testing
```bash
./scripts/docker-dev.sh test       # Run tests in test container
```

### Cleanup
```bash
./scripts/docker-dev.sh clean      # Remove containers and images
./scripts/docker-dev.sh reset      # Reset all persistent volumes (WARNING: destroys data)
```

## Project Structure

```
.
├── Dockerfile                  # Docker image definition
├── docker-compose.yml          # Docker Compose configuration
├── scripts/
│   └── docker-dev.sh           # Docker environment manager
├── tasks/                      # Project task management
├── docs/                       # Documentation
└── README.md                   # This file
```

## Next Steps

After setting up the Docker environment, you can proceed with:

1. **Repository Setup**: Clone QEMU source and set up branches
2. **Build System**: Configure Meson build for S32K344 target
3. **Zephyr Integration**: Set up Zephyr SDK and board definitions
4. **Device Development**: Implement S32K344 peripherals (LPUART, GPIO, etc.)

## Troubleshooting

### Docker Not Starting
- Ensure Docker Desktop is running
- Check that you have sufficient disk space
- Verify Docker has permissions to access the project directory

### Container Build Issues
- Update Docker to the latest version
- Clear Docker cache: `docker system prune -a`
- Check internet connection for package downloads

### Permission Issues
- The container runs as user `dev` (uid: 1000)
- Ensure your host user has appropriate permissions
- Use `sudo` inside the container if needed for system operations

### Volume Data Loss
- Persistent volumes store build artifacts and downloads
- Use `./scripts/docker-dev.sh status` to check volume status
- Backup important data before using `reset` command

## Contributing

1. Make changes to the Dockerfile or docker-compose.yml
2. Test with `./scripts/docker-dev.sh build`
3. Verify functionality with `./scripts/docker-dev.sh test`
4. Update documentation as needed

## License

This project follows the same license as the QEMU project. 