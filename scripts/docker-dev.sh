#!/bin/bash

# S32K344 QEMU Docker Development Environment Manager
# This script provides convenient commands for managing the Docker development environment

set -e

# Configuration
PROJECT_NAME="s32k344-qemu"
DEV_CONTAINER="s32k344-dev"
TEST_CONTAINER="s32k344-test"
DOCKER_COMPOSE_FILE="docker-compose.yml"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

# Check if Docker is running
check_docker() {
    if ! docker info > /dev/null 2>&1; then
        print_error "Docker is not running. Please start Docker Desktop first."
        exit 1
    fi
}

# Check if docker-compose file exists
check_compose_file() {
    if [ ! -f "$DOCKER_COMPOSE_FILE" ]; then
        print_error "docker-compose.yml not found in current directory"
        exit 1
    fi
}

# Build the Docker image
build() {
    print_header "Building Docker Image"
    check_docker
    check_compose_file
    
    print_status "Building $PROJECT_NAME development image..."
    docker-compose build s32k344-dev
    
    print_status "Build completed successfully!"
}

# Start the development container
start() {
    print_header "Starting Development Container"
    check_docker
    check_compose_file
    
    print_status "Starting $DEV_CONTAINER container..."
    docker-compose up -d s32k344-dev
    
    print_status "Development container started!"
    print_status "Use './scripts/docker-dev.sh shell' to connect"
}

# Stop the development container
stop() {
    print_header "Stopping Development Container"
    check_docker
    
    print_status "Stopping $DEV_CONTAINER container..."
    docker-compose down
    
    print_status "Development container stopped!"
}

# Connect to the development container shell
shell() {
    print_header "Connecting to Development Container"
    check_docker
    
    if ! docker ps | grep -q "$DEV_CONTAINER"; then
        print_warning "Development container is not running. Starting it first..."
        start
        sleep 2
    fi
    
    print_status "Connecting to $DEV_CONTAINER shell..."
    docker exec -it $DEV_CONTAINER /bin/bash
}

# Run tests in the test container
test() {
    print_header "Running Tests"
    check_docker
    check_compose_file
    
    print_status "Running tests in $TEST_CONTAINER container..."
    docker-compose run --rm s32k344-test /bin/bash -c "
        echo 'Test environment ready'
        echo 'QEMU Source: $QEMU_SOURCE_DIR'
        echo 'QEMU Build: $QEMU_BUILD_DIR'
        echo 'Zephyr Base: $ZEPHYR_BASE'
        echo 'Project Root: $PROJECT_ROOT'
        if [ -f ./scripts/run-tests.sh ]; then
            ./scripts/run-tests.sh
        else
            echo 'No test script found. Create ./scripts/run-tests.sh'
        fi
    "
}

# Clean up Docker artifacts
clean() {
    print_header "Cleaning Docker Artifacts"
    check_docker
    
    print_status "Stopping containers..."
    docker-compose down 2>/dev/null || true
    
    print_status "Removing containers..."
    docker rm -f $DEV_CONTAINER $TEST_CONTAINER 2>/dev/null || true
    
    print_status "Removing images..."
    docker rmi ${PROJECT_NAME}:dev 2>/dev/null || true
    
    print_status "Cleanup completed!"
}

# Show container status
status() {
    print_header "Container Status"
    check_docker
    
    print_status "Running containers:"
    docker ps --filter "name=$DEV_CONTAINER" --filter "name=$TEST_CONTAINER"
    
    print_status "Docker images:"
    docker images --filter "reference=${PROJECT_NAME}:dev"
    
    print_status "Docker volumes:"
    docker volume ls --filter "name=qemu"
}

# Show logs
logs() {
    print_header "Container Logs"
    check_docker
    
    if docker ps | grep -q "$DEV_CONTAINER"; then
        print_status "Showing logs for $DEV_CONTAINER..."
        docker logs $DEV_CONTAINER
    else
        print_warning "Development container is not running"
    fi
}

# Reset volumes (WARNING: This will delete all persistent data)
reset() {
    print_header "Resetting Volumes (WARNING: This will delete all data!)"
    
    read -p "Are you sure you want to delete all persistent data? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Stopping containers..."
        docker-compose down 2>/dev/null || true
        
        print_status "Removing volumes..."
        docker volume rm qemu-src qemu-build zephyr-data test-artifacts build-cache 2>/dev/null || true
        
        print_status "Volumes reset completed!"
    else
        print_status "Reset cancelled."
    fi
}

# Show help
help() {
    print_header "S32K344 QEMU Docker Development Environment"
    
    echo "Usage: $0 [COMMAND]"
    echo
    echo "Commands:"
    echo "  build     Build the Docker development image"
    echo "  start     Start the development container"
    echo "  stop      Stop the development container"
    echo "  shell     Connect to the development container shell"
    echo "  test      Run tests in the test container"
    echo "  status    Show container and image status"
    echo "  logs      Show container logs"
    echo "  clean     Clean up Docker artifacts"
    echo "  reset     Reset all persistent volumes (WARNING: destroys data)"
    echo "  help      Show this help message"
    echo
    echo "Examples:"
    echo "  $0 build              # Build the development image"
    echo "  $0 start              # Start the development environment"
    echo "  $0 shell              # Connect to the development container"
    echo "  $0 test               # Run the test suite"
    echo
    echo "Quick Start:"
    echo "  1. $0 build"
    echo "  2. $0 start"
    echo "  3. $0 shell"
    echo
}

# Main command processing
case "${1:-help}" in
    build)
        build
        ;;
    start)
        start
        ;;
    stop)
        stop
        ;;
    shell)
        shell
        ;;
    test)
        test
        ;;
    status)
        status
        ;;
    logs)
        logs
        ;;
    clean)
        clean
        ;;
    reset)
        reset
        ;;
    help|--help|-h)
        help
        ;;
    *)
        print_error "Unknown command: $1"
        help
        exit 1
        ;;
esac 