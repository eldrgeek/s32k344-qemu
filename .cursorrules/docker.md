---
name: "Docker Development Environment"
description: "Rules for Docker container operations and build environment"
applies_to:
  - "Dockerfile"
  - "docker-compose.yml"
  - "scripts/docker-*.sh"
contexts:
  - "docker"
  - "containers"
  - "build-environment"
priority: 90
---

# Docker Development Environment Rules

## Container Operations (Always Safe)

### Build and Management
- `docker build`, `docker-compose build` - Always safe to rebuild
- `docker run`, `docker-compose up` - Safe for development containers
- `docker exec` - Safe for executing commands in existing containers
- `docker logs` - Always safe to check container logs
- `docker ps`, `docker images` - Safe status checks

### Development Container Commands
Within the `s32k344-dev` container, ALL commands are safe:
- File operations within `/workspace/project`
- Build operations: `make`, `ninja`, `meson`, `configure`
- ARM cross-compilation with `arm-none-eabi-gcc`
- QEMU build and test operations
- Git operations on project repository

### Container Best Practices
- Use persistent volumes for project data
- Keep containers stateless except for development tools
- Rebuild containers when dependencies change
- Use `.dockerignore` to exclude unnecessary files

### Scripts Usage
- `./scripts/docker-dev.sh build` - Safe rebuild
- `./scripts/docker-dev.sh start` - Safe container start
- `./scripts/docker-dev.sh shell` - Safe interactive shell
- `./scripts/docker-dev.sh stop` - Safe container stop

### Volume Management
- Project directory mounted as `/workspace/project`
- Build artifacts in `/workspace/project/build`
- Virtual environment in `/workspace/project/venv`
- Persistent development tools and caches

### Quick Commands
Execute without hesitation:
```bash
# Build environment
./scripts/docker-dev.sh build

# Start development environment
./scripts/docker-dev.sh start

# Get shell in container
./scripts/docker-dev.sh shell

# Execute commands in container
docker exec s32k344-dev bash -c "cd /workspace/project && make"
``` 