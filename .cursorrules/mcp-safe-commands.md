---
name: "MCP Safe Commands"
description: "Commands that should execute without permission prompts"
applies_to:
  - "**/*"
contexts:
  - "mcp"
  - "automation"
  - "safe-commands"
priority: 100
---

# MCP Safe Commands Configuration

## Git Operations (Always Safe)
```json
{
  "safe_commands": {
    "git": [
      "git status",
      "git add .",
      "git add -A",
      "git add *",
      "git commit -m *",
      "git push *",
      "git pull *",
      "git log *",
      "git diff *",
      "git show *",
      "git branch *",
      "git checkout *",
      "git stash *",
      "git remote *"
    ]
  }
}
```

## File Operations (Project Directory)
```json
{
  "safe_paths": [
    "/Users/MikeWolf/Projects/QEMU/**/*",
    "/workspace/project/**/*"
  ],
  "safe_file_operations": [
    "ls", "cat", "head", "tail", "grep", "find", "tree",
    "mkdir", "touch", "cp", "mv", "rm" 
  ]
}
```

## Docker Operations (Development)
```json
{
  "safe_docker_commands": [
    "docker ps",
    "docker images",
    "docker logs *",
    "docker exec s32k344-dev *",
    "docker-compose up *",
    "docker-compose down *",
    "docker-compose build *",
    "./scripts/docker-dev.sh *"
  ]
}
```

## Build Operations (Always Safe)
```json
{
  "safe_build_commands": [
    "make *",
    "ninja *",
    "meson *",
    "configure *",
    "cmake *",
    "gcc *",
    "arm-none-eabi-gcc *",
    "./scripts/build-qemu.sh *",
    "./scripts/run-tests.sh *"
  ]
}
```

## QEMU Operations (Development)
```json
{
  "safe_qemu_commands": [
    "qemu-system-arm *",
    "qemu-img *",
    "qemu-nbd *",
    "qtest *"
  ]
}
```

## Read-Only System Commands
```json
{
  "safe_system_commands": [
    "whoami", "id", "pwd", "date", "uptime", "free", "df",
    "uname", "lscpu", "lsblk", "mount", "ps", "top", "htop",
    "which", "type", "command", "env", "printenv"
  ]
}
```

## Command Patterns That Should Never Ask Permission

### Git Patterns
- Any `git add` command
- Any `git commit` command  
- Any `git status` or `git log` command
- Any `git diff` or `git show` command
- `git push` to feature branches
- `git pull` from any branch

### Docker Patterns
- Any command starting with `docker exec s32k344-dev`
- Any command starting with `./scripts/docker-dev.sh`
- Any `docker ps`, `docker images`, `docker logs`
- Any `docker-compose` command in project directory

### Build Patterns
- Any command starting with `make`, `ninja`, `meson`
- Any command starting with `./scripts/build`
- Any ARM cross-compilation command
- Any QEMU build or test command

### File Operations in Project
- Any `ls`, `cat`, `head`, `tail`, `grep`, `find` in project directory
- Any `mkdir`, `touch`, `cp`, `mv` in project directory
- Any `rm` of build artifacts or temporary files

## MCP Server Configuration

To configure MCP server for safe commands:

### Option 1: Environment Variables
```bash
export MCP_SAFE_COMMANDS="git,docker,make,ninja,ls,cat,grep,find"
export MCP_SAFE_PATHS="/Users/MikeWolf/Projects/QEMU,/workspace/project"
export MCP_AUTO_APPROVE_SAFE="true"
```

### Option 2: Configuration File
Create `~/.mcp/config.json`:
```json
{
  "safe_commands": {
    "patterns": [
      "git (add|commit|status|log|diff|show|push|pull).*",
      "docker (ps|images|logs|exec s32k344-dev).*",
      "\\./scripts/docker-dev\\.sh.*",
      "(make|ninja|meson|configure).*",
      "(ls|cat|head|tail|grep|find).*",
      "arm-none-eabi-gcc.*",
      "qemu-system-arm.*"
    ],
    "paths": [
      "/Users/MikeWolf/Projects/QEMU/**",
      "/workspace/project/**"
    ],
    "auto_approve": true
  }
}
```

### Option 3: Runtime Configuration
```bash
# In shell or script
export MCP_RUNTIME_CONFIG='{
  "safe_commands": ["git", "docker", "make", "ninja"],
  "safe_paths": ["/Users/MikeWolf/Projects/QEMU"],
  "auto_approve_safe": true
}'
```

## Usage Examples

These commands should execute immediately without prompts:

```bash
# Git operations
git add src/s32k344/s32k344_lpuart.c
git commit -m "feat(lpuart): implement LPUART peripheral"
git push origin feature/s32k344

# Docker operations
docker exec s32k344-dev bash -c "cd /workspace/project && make"
./scripts/docker-dev.sh shell

# Build operations
cd build && ninja
make check-qtest-arm

# File operations
ls -la qemu-upstream/hw/arm/
cat qemu-upstream/hw/arm/meson.build
find . -name "*.c" -path "*/s32k344/*"
``` 