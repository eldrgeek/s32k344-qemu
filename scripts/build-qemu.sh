#!/bin/bash

# S32K344 QEMU Build Script
# This script builds QEMU with S32K344 support for development and testing

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QEMU_SOURCE_DIR="${QEMU_SOURCE_DIR:-$PROJECT_ROOT/qemu-upstream}"
QEMU_BUILD_DIR="${QEMU_BUILD_DIR:-$PROJECT_ROOT/build}"
MAKE_JOBS="${MAKE_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

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

# Check build dependencies
check_dependencies() {
    print_header "Checking Build Dependencies"
    
    local missing_deps=()
    
    # Required tools
    local required_tools=("make" "gcc" "pkg-config" "python3")
    
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            missing_deps+=("$tool")
        fi
    done
    
    # Check for development libraries (these checks are basic and may vary by system)
    if ! pkg-config --exists glib-2.0 2>/dev/null; then
        missing_deps+=("libglib2.0-dev")
    fi
    
    if ! pkg-config --exists pixman-1 2>/dev/null; then
        missing_deps+=("libpixman-1-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "On Ubuntu/Debian, install with:"
        print_status "  sudo apt-get install ${missing_deps[*]}"
        print_status "On macOS with Homebrew:"
        print_status "  brew install ${missing_deps[*]}"
        return 1
    fi
    
    print_status "All required dependencies are available"
    return 0
}

# Validate QEMU source
validate_qemu_source() {
    print_header "Validating QEMU Source"
    
    if [ ! -d "$QEMU_SOURCE_DIR" ]; then
        print_error "QEMU source directory not found: $QEMU_SOURCE_DIR"
        print_status "Run the repository setup first to clone QEMU source"
        return 1
    fi
    
    if [ ! -f "$QEMU_SOURCE_DIR/configure" ] && [ ! -f "$QEMU_SOURCE_DIR/meson.build" ]; then
        print_error "QEMU source appears to be incomplete"
        return 1
    fi
    
    local qemu_version
    if [ -f "$QEMU_SOURCE_DIR/VERSION" ]; then
        qemu_version=$(cat "$QEMU_SOURCE_DIR/VERSION")
        print_status "QEMU version: $qemu_version"
    else
        print_warning "Could not determine QEMU version"
    fi
    
    return 0
}

# Configure QEMU build
configure_qemu() {
    print_header "Configuring QEMU Build"
    
    # Create build directory
    mkdir -p "$QEMU_BUILD_DIR"
    cd "$QEMU_BUILD_DIR"
    
    # QEMU configure options for S32K344 development
    local configure_opts=(
        "--target-list=arm-softmmu"
        "--enable-debug"
        "--enable-debug-info"
        "--disable-werror"
        "--enable-trace-backends=log"
        "--enable-slirp"
        "--disable-docs"
        "--disable-gtk"
        "--disable-sdl"
        "--disable-vnc"
        "--enable-curses"
    )
    
    # Add macOS-specific options if on macOS
    if [[ "$OSTYPE" == "darwin"* ]]; then
        configure_opts+=("--disable-cocoa")
    fi
    
    print_status "Configuration options: ${configure_opts[*]}"
    
    # Run configure
    if [ -f "$QEMU_SOURCE_DIR/configure" ]; then
        print_status "Using autotools configure"
        "$QEMU_SOURCE_DIR/configure" "${configure_opts[@]}"
    else
        print_status "Using meson build system"
        meson setup . "$QEMU_SOURCE_DIR" "${configure_opts[@]}"
    fi
    
    print_status "QEMU configuration completed"
}

# Build QEMU
build_qemu() {
    print_header "Building QEMU"
    
    cd "$QEMU_BUILD_DIR"
    
    local build_start=$(date +%s)
    
    # Determine build system and build
    if [ -f "Makefile" ]; then
        print_status "Building with make (using $MAKE_JOBS jobs)"
        make -j"$MAKE_JOBS"
    elif [ -f "build.ninja" ]; then
        print_status "Building with ninja"
        ninja
    else
        print_error "No build system found in build directory"
        return 1
    fi
    
    local build_end=$(date +%s)
    local build_time=$((build_end - build_start))
    
    print_status "Build completed in ${build_time} seconds"
    
    # Verify build artifacts
    if [ -f "qemu-system-arm" ]; then
        print_status "✓ qemu-system-arm binary created successfully"
        local qemu_size=$(stat -f%z "qemu-system-arm" 2>/dev/null || stat -c%s "qemu-system-arm" 2>/dev/null || echo "unknown")
        print_status "Binary size: $qemu_size bytes"
    else
        print_error "✗ qemu-system-arm binary not found"
        return 1
    fi
}

# Test basic QEMU functionality
test_qemu_basic() {
    print_header "Testing Basic QEMU Functionality"
    
    cd "$QEMU_BUILD_DIR"
    
    # Test QEMU version
    print_status "Testing QEMU version..."
    if ./qemu-system-arm --version; then
        print_status "✓ QEMU version check passed"
    else
        print_error "✗ QEMU version check failed"
        return 1
    fi
    
    # Test machine list (should include standard ARM machines)
    print_status "Testing machine list..."
    if ./qemu-system-arm -machine help | grep -q "arm"; then
        print_status "✓ ARM machines are available"
    else
        print_warning "ARM machines may not be properly configured"
    fi
    
    # Note: S32K344 machine will not be available until we implement it
    print_status "Note: S32K344 machine will be available after implementation"
}

# Create build summary
create_build_summary() {
    print_header "Build Summary"
    
    local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local summary_file="$PROJECT_ROOT/build-summary-$timestamp.txt"
    
    cat > "$summary_file" << EOF
S32K344 QEMU Build Summary
==========================
Generated: $timestamp
Project Root: $PROJECT_ROOT
QEMU Source: $QEMU_SOURCE_DIR
QEMU Build: $QEMU_BUILD_DIR

Build Environment:
- Operating System: $(uname -s)
- Architecture: $(uname -m)
- Make Jobs: $MAKE_JOBS
- Compiler: $(gcc --version 2>/dev/null | head -n1 || echo "Unknown")

Build Artifacts:
$(cd "$QEMU_BUILD_DIR" && ls -la qemu-system-* 2>/dev/null || echo "No QEMU binaries found")

Next Steps:
1. Implement S32K344 machine definition
2. Add peripheral device implementations  
3. Test with S32K344-specific binaries
4. Run integration tests

EOF
    
    print_status "Build summary saved to: $summary_file"
}

# Clean build directory
clean_build() {
    print_header "Cleaning Build Directory"
    
    if [ -d "$QEMU_BUILD_DIR" ]; then
        print_status "Removing build directory: $QEMU_BUILD_DIR"
        rm -rf "$QEMU_BUILD_DIR"
        print_status "Build directory cleaned"
    else
        print_status "Build directory does not exist, nothing to clean"
    fi
}

# Show help
show_help() {
    print_header "S32K344 QEMU Build Script"
    
    echo "Usage: $0 [COMMAND]"
    echo
    echo "Commands:"
    echo "  build     Build QEMU with S32K344 support (default)"
    echo "  clean     Clean the build directory"
    echo "  test      Test basic QEMU functionality"
    echo "  help      Show this help message"
    echo
    echo "Environment Variables:"
    echo "  QEMU_SOURCE_DIR  Path to QEMU source (default: $PROJECT_ROOT/qemu-upstream)"
    echo "  QEMU_BUILD_DIR   Path to build directory (default: $PROJECT_ROOT/build)"
    echo "  MAKE_JOBS        Number of parallel build jobs (default: auto)"
    echo
    echo "Examples:"
    echo "  $0                # Build QEMU"
    echo "  $0 clean          # Clean build directory"
    echo "  $0 test           # Test QEMU functionality"
    echo
}

# Main build function
main_build() {
    print_header "S32K344 QEMU Build Process"
    
    # Check dependencies
    if ! check_dependencies; then
        print_error "Dependency check failed"
        return 1
    fi
    
    # Validate source
    if ! validate_qemu_source; then
        print_error "Source validation failed"
        return 1
    fi
    
    # Configure
    if ! configure_qemu; then
        print_error "Configuration failed"
        return 1
    fi
    
    # Build
    if ! build_qemu; then
        print_error "Build failed"
        return 1
    fi
    
    # Test
    if ! test_qemu_basic; then
        print_warning "Basic tests failed, but build may still be usable"
    fi
    
    # Summary
    create_build_summary
    
    print_header "Build Completed Successfully"
    print_status "QEMU binary available at: $QEMU_BUILD_DIR/qemu-system-arm"
}

# Main execution
case "${1:-build}" in
    build)
        main_build
        ;;
    clean)
        clean_build
        ;;
    test)
        test_qemu_basic
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac 