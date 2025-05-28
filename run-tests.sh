#!/bin/bash

# TP4-ACSO Test Runner - Flexible Docker/Native Version
# Executes all tests for both exercises in Docker or natively
# Author: Refactored for TP4 Assignment  
# Date: May 2025
# Version: 4.0 - Docker/Native Selection

set -euo pipefail
IFS=$'\n\t'

# Colors and configuration
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly WHITE='\033[1;37m'
readonly NC='\033[0m'

readonly DOCKER_IMAGE="tp4-acso"
readonly CONTAINER_NAME="tp4-test-runner"
readonly WORKSPACE_DIR="/tp4"
readonly LOG_DIR="$(pwd)/test-logs"
readonly TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
readonly LOG_FILE="${LOG_DIR}/test-run-${TIMESTAMP}.log"

# Global execution mode
USE_DOCKER=true

# Create log directory
mkdir -p "${LOG_DIR}"

# Unified logging function
log() {
    local level="$1"
    local message="$2"
    local timestamp="$(date '+%Y-%m-%d %H:%M:%S')"
    
    case "$level" in
        "INFO")    echo -e "${BLUE}[INFO]${NC} $message" ;;
        "SUCCESS") echo -e "${GREEN}[SUCCESS]${NC} $message" ;;
        "WARNING") echo -e "${YELLOW}[WARNING]${NC} $message" ;;
        "ERROR")   echo -e "${RED}[ERROR]${NC} $message" ;;
        "STEP")    echo -e "${CYAN}[STEP]${NC} $message" ;;
    esac
    
    echo "$timestamp - $level: $message" >> "${LOG_FILE}"
}

# Detect system architecture and OS
detect_system() {
    local arch=$(uname -m)
    local os=$(uname -s)
    local suggested_mode=""
    
    log "INFO" "Detected system: $os $arch"
    
    if [[ "$os" == "Linux" && "$arch" == "x86_64" ]]; then
        suggested_mode="NATIVE"
        log "SUCCESS" "Running on x86_64 Linux - native execution recommended"
    elif [[ "$os" == "Darwin" ]]; then
        suggested_mode="DOCKER"
        log "INFO" "Running on macOS - Docker execution recommended for x86_64 compatibility"
    else
        suggested_mode="DOCKER"
        log "WARNING" "Running on $os $arch - Docker execution recommended"
    fi
    
    if [[ "$USE_DOCKER" == true ]]; then
        log "INFO" "Using Docker execution mode"
    else
        log "INFO" "Using native execution mode"
        if [[ "$suggested_mode" == "DOCKER" ]]; then
            log "WARNING" "Native mode on non-x86_64 Linux may have compatibility issues"
        fi
    fi
}

# Print banner
print_banner() {
    clear
    echo -e "${BLUE}"
    echo "████████╗██████╗ ██╗  ██╗      █████╗  ██████╗███████╗ ██████╗ "
    echo "╚══██╔══╝██╔══██╗██║  ██║     ██╔══██╗██╔════╝██╔════╝██╔═══██╗"
    echo "   ██║   ██████╔╝███████║     ███████║██║     ███████╗██║   ██║"
    echo "   ██║   ██╔═══╝ ╚════██║     ██╔══██║██║     ╚════██║██║   ██║"
    echo "   ██║   ██║          ██║     ██║  ██║╚██████╗███████║╚██████╔╝"
    echo "   ╚═╝   ╚═╝          ╚═╝     ╚═╝  ╚═╝ ╚═════╝╚══════╝ ╚═════╝ "
    echo -e "${NC}"
    echo -e "${WHITE}TP4-ACSO Test Suite - Ring & Shell Implementation${NC}"
    echo -e "${WHITE}Version 4.0 - Docker/Native Selection${NC}"
    echo -e "${BLUE}================================================================${NC}"
    echo -e "${CYAN}Start Time: $(date)${NC}"
    echo -e "${CYAN}Log File: ${LOG_FILE}${NC}"
    if [[ "$USE_DOCKER" == true ]]; then
        echo -e "${CYAN}Execution Mode: Docker (x86_64 Ubuntu 22.04)${NC}"
    else
        echo -e "${CYAN}Execution Mode: Native ($(uname -s) $(uname -m))${NC}"
    fi
    echo -e "${BLUE}================================================================${NC}"
}

# Error handling and cleanup
cleanup() {
    local exit_code=$?
    log "INFO" "Performing cleanup operations..."
    
    if [[ "$USE_DOCKER" == true ]]; then
        # Stop and remove any running containers
        for container in $(docker ps -q --filter "name=${CONTAINER_NAME}" 2>/dev/null || true); do
            log "WARNING" "Stopping container: $container"
            docker stop "$container" >/dev/null 2>&1 || true
            docker rm "$container" >/dev/null 2>&1 || true
        done
    fi
    
    if [[ $exit_code -ne 0 ]]; then
        log "ERROR" "Script exited with error code: $exit_code"
        log "ERROR" "Check log file for details: ${LOG_FILE}"
    else
        log "SUCCESS" "Script completed successfully"
    fi
    
    exit $exit_code
}

# Set up signal handlers
trap cleanup EXIT
trap 'log "ERROR" "Script interrupted by user"; exit 130' INT TERM

# Validation functions
validate_environment() {
    log "STEP" "Validating environment prerequisites..."
    
    if [[ ! -d "src" ]] || [[ ! -d "tests" ]]; then
        log "ERROR" "Not in TP4-ACSO project directory"
        log "ERROR" "Please run this script from the project root directory"
        return 1
    fi
    
    local required_files=(
        "src/ej1/Makefile" "src/ej1/ring.c"
        "src/ej2/Makefile" "src/ej2/shell.c"
        "tests/ej1/test_ring.c" "tests/ej2/test_shell.c"
    )
    
    for file in "${required_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            log "ERROR" "Required file missing: $file"
            return 1
        fi
    done
    
    if [[ "$USE_DOCKER" == true && ! -f "Dockerfile" ]]; then
        log "ERROR" "Dockerfile missing for Docker execution mode"
        return 1
    fi
    
    log "SUCCESS" "Environment validation passed"
    return 0
}

check_docker() {
    if [[ "$USE_DOCKER" == false ]]; then
        return 0
    fi
    
    log "INFO" "Checking Docker availability..."
    if ! docker info >/dev/null 2>&1; then
        log "ERROR" "Docker is not running. Please start Docker or use --native flag."
        exit 1
    fi
    log "SUCCESS" "Docker is running"
}

build_docker_image() {
    if [[ "$USE_DOCKER" == false ]]; then
        return 0
    fi
    
    log "INFO" "Checking if Docker image '$DOCKER_IMAGE' exists..."
    
    if docker image inspect $DOCKER_IMAGE >/dev/null 2>&1; then
        log "SUCCESS" "Docker image '$DOCKER_IMAGE' found"
        return 0
    fi
    
    log "INFO" "Building Docker image '$DOCKER_IMAGE'..."
    if docker build --platform linux/amd64 -t $DOCKER_IMAGE .; then
        log "SUCCESS" "Docker image built successfully"
    else
        log "ERROR" "Failed to build Docker image"
        exit 1
    fi
}

# Execute commands (Docker or native)
run_command() {
    local commands="$1"
    
    if [[ "$USE_DOCKER" == true ]]; then
        local container_name="${CONTAINER_NAME}-$(date +%s)"
        docker run --rm \
            --name "$container_name" \
            --platform linux/amd64 \
            -v "$(pwd):$WORKSPACE_DIR" \
            -w "$WORKSPACE_DIR" \
            $DOCKER_IMAGE \
            bash -c "$commands"
    else
        bash -c "$commands"
    fi
}

# Build projects
build_projects() {
    if [[ "$USE_DOCKER" == true ]]; then
        log "STEP" "Building projects in Docker container..."
    else
        log "STEP" "Building projects natively..."
    fi
    
    local build_commands="
        echo '==== Building Ring (Exercise 1) ===='
        cd src/ej1
        make clean && make
        echo 'Ring build: SUCCESS'
        echo
        
        echo '==== Building Shell (Exercise 2) ===='
        cd ../ej2
        make clean && make
        echo 'Shell build: SUCCESS'
        echo
        
        echo '==== Building Tests ===='
        cd ../../tests
        make clean && make all
        echo 'Tests build: SUCCESS'
    "
    
    if run_command "$build_commands"; then
        log "SUCCESS" "All projects built successfully"
    else
        log "ERROR" "Build failed"
        exit 1
    fi
}

# Run tests
run_tests() {
    local test_type="$1"
    if [[ "$USE_DOCKER" == true ]]; then
        log "STEP" "Discovering and running $test_type tests in Docker..."
    else
        log "STEP" "Discovering and running $test_type tests natively..."
    fi
    
    local filter_pattern=""
    local test_description=""
    
    case "$test_type" in
        "ring")
            filter_pattern="(ring|ej1)"
            test_description="Ring Communication Tests (EJ1)"
            ;;
        "shell")
            filter_pattern="(shell|debug|ej2)"
            test_description="Shell Implementation Tests (EJ2)"
            ;;
        "all")
            filter_pattern=".*"
            test_description="All Available Tests"
            ;;
        *)
            log "ERROR" "Unknown test type: $test_type"
            return 1
            ;;
    esac
    
    # Generate test execution commands
    local test_commands="
        echo '==== $test_description ===='
        cd tests
        
        test_count=0
        passed_count=0
        failed_count=0
        
        echo 'Scanning for tests...'
        echo
        
        # Run EJ1 tests if we're doing ring or all tests
        if [[ '$test_type' == 'ring' || '$test_type' == 'all' ]]; then
            if [[ -d ej1 ]]; then
                echo \"========================================\"
                echo \"EJ1 (Ring Communication) Tests\"
                echo \"========================================\"
                cd ej1
                for test_exec in \$(find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sed 's|^\./||' | sort); do
                    echo \"Running: ej1/\$test_exec\"
                    echo \"----------------------------------------\"
                    
                    if timeout 30 ./\$test_exec; then
                        echo \"✅ ej1/\$test_exec: PASSED\"
                        passed_count=\$((passed_count + 1))
                    else
                        echo \"❌ ej1/\$test_exec: FAILED\"
                        failed_count=\$((failed_count + 1))
                    fi
                    test_count=\$((test_count + 1))
                    echo
                done
                cd ..
            fi
        fi
        
        # Run EJ2 tests if we're doing shell or all tests
        if [[ '$test_type' == 'shell' || '$test_type' == 'all' ]]; then
            if [[ -d ej2 ]]; then
                echo \"========================================\"
                echo \"EJ2 (Shell Implementation) Tests\"
                echo \"========================================\"
                cd ej2
                for test_exec in \$(find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sed 's|^\./||' | sort); do
                    echo \"Running: ej2/\$test_exec\"
                    echo \"----------------------------------------\"
                    
                    if timeout 30 ./\$test_exec; then
                        echo \"✅ ej2/\$test_exec: PASSED\"
                        passed_count=\$((passed_count + 1))
                    else
                        echo \"❌ ej2/\$test_exec: FAILED\"
                        failed_count=\$((failed_count + 1))
                    fi
                    test_count=\$((test_count + 1))
                    echo
                done
                cd ..
            fi
        fi
        
        # Run integration tests if we're doing all tests
        if [[ '$test_type' == 'all' ]]; then
            echo \"========================================\"
            echo \"Integration Tests\"
            echo \"========================================\"
            for test_exec in \$(find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sed 's|^\./||' | sort); do
                echo \"Running: \$test_exec\"
                echo \"----------------------------------------\"
                
                if timeout 30 ./\$test_exec; then
                    echo \"✅ \$test_exec: PASSED\"
                    passed_count=\$((passed_count + 1))
                else
                    echo \"❌ \$test_exec: FAILED\"
                    failed_count=\$((failed_count + 1))
                fi
                test_count=\$((test_count + 1))
                echo
            done
        fi
        
        echo \"========================================\"
        echo \"Test Summary for $test_description\"
        echo \"========================================\"
        echo \"Total tests run: \$test_count\"
        echo \"Passed: \$passed_count\"
        echo \"Failed: \$failed_count\"
        
        if [ \$failed_count -eq 0 ]; then
            echo \"🎉 All tests PASSED!\"
            exit 0
        else
            echo \"💥 Some tests FAILED!\"
            exit 1
        fi
    "
    
    if run_command "$test_commands"; then
        log "SUCCESS" "$test_type tests completed successfully"
    else
        log "ERROR" "Some $test_type tests failed"
        return 1
    fi
}

# Performance benchmark for ring implementation
run_benchmark() {
    if [[ "$USE_DOCKER" == true ]]; then
        log "STEP" "Running performance benchmark in Docker..."
    else
        log "STEP" "Running performance benchmark natively..."
    fi
    
    local benchmark_commands="
        echo '==== Performance Benchmark for Ring Implementation ===='
        cd src/ej1
        make clean && make
        
        echo 'Testing ring performance with different process counts:'
        echo
        printf '%-12s %-10s %-8s %-15s\n' 'Processes' 'Time(s)' 'Status' 'Result'
        printf '%-12s %-10s %-8s %-15s\n' '---------' '-------' '------' '-------'
        
        for n in 1 5 10 25 50; do
            start=\$(date +%s.%N)
            if timeout 15 ./ring \$n 0 0 >/tmp/result 2>/dev/null; then
                end=\$(date +%s.%N)
                duration=\$(echo \"\$end - \$start\" | bc -l 2>/dev/null || echo '0.000')
                result=\$(cat /tmp/result | tail -1 2>/dev/null || echo 'ERROR')
                
                if [ \"\$result\" = \"\$n\" ]; then
                    status='PASS'
                else
                    status='FAIL'
                fi
                printf '%-12s %-10.3f %-8s %-15s\n' \"\$n\" \"\$duration\" \"\$status\" \"\$result\"
            else
                printf '%-12s %-10s %-8s %-15s\n' \"\$n\" 'TIMEOUT' 'FAIL' 'TIMEOUT'
            fi
            rm -f /tmp/result
        done
        echo
    "
    
    if run_command "$benchmark_commands"; then
        log "SUCCESS" "Performance benchmark completed"
    else
        log "ERROR" "Performance benchmark failed"
        return 1
    fi
}

# Parse command line arguments
parse_arguments() {
    local test_mode=""
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --docker)
                USE_DOCKER=true
                shift
                ;;
            --native)
                USE_DOCKER=false
                shift
                ;;
            ring|shell|all|benchmark|list|clean)
                test_mode="$1"
                shift
                ;;
            help|--help|-h)
                test_mode="help"
                shift
                ;;
            *)
                log "ERROR" "Unknown option: $1"
                test_mode="help"
                shift
                ;;
        esac
    done
    
    echo "$test_mode"
}

# Auto-detect execution mode if not specified
auto_detect_mode() {
    local arch=$(uname -m)
    local os=$(uname -s)
    
    if [[ "$os" == "Linux" && "$arch" == "x86_64" ]]; then
        # Check if we have Docker available and if we're not explicitly choosing native
        if [[ "${1:-}" != "--native" ]] && docker info >/dev/null 2>&1; then
            # Default to Docker for consistency, but native is fine too
            USE_DOCKER=true
        else
            USE_DOCKER=false
        fi
    else
        # Non-Linux or non-x86_64, prefer Docker
        USE_DOCKER=true
    fi
}

# Main execution function
main() {
    # Parse arguments first to check for mode flags
    local remaining_args=()
    local explicit_mode=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --docker)
                USE_DOCKER=true
                explicit_mode=true
                shift
                ;;
            --native)
                USE_DOCKER=false
                explicit_mode=true
                shift
                ;;
            *)
                remaining_args+=("$1")
                shift
                ;;
        esac
    done
    
    # Auto-detect if no explicit mode was set
    if [[ "$explicit_mode" == false ]]; then
        auto_detect_mode
    fi
    
    # Now parse the remaining arguments
    local test_mode=$(parse_arguments "${remaining_args[@]}")
    
    print_banner
    detect_system
    
    case "${test_mode}" in
        "ring"|"RING")
            validate_environment
            check_docker
            build_docker_image
            build_projects
            run_tests "ring"
            log "SUCCESS" "Ring tests completed!"
            ;;
        "shell"|"SHELL")
            validate_environment
            check_docker
            build_docker_image
            build_projects
            run_tests "shell"
            log "SUCCESS" "Shell tests completed!"
            ;;
        "all"|"ALL")
            validate_environment
            check_docker
            build_docker_image
            build_projects
            run_tests "all"
            run_benchmark
            log "SUCCESS" "All tests and benchmarks completed!"
            ;;
        "benchmark")
            validate_environment
            check_docker
            build_docker_image
            build_projects
            run_benchmark
            ;;
        "clean")
            log "INFO" "Cleaning up resources..."
            if [[ "$USE_DOCKER" == true ]]; then
                docker system prune -f >/dev/null 2>&1 || true
                docker image rm $DOCKER_IMAGE >/dev/null 2>&1 || true
            fi
            make -C src/ej1 clean >/dev/null 2>&1 || true
            make -C src/ej2 clean >/dev/null 2>&1 || true
            make -C tests clean >/dev/null 2>&1 || true
            log "SUCCESS" "Cleanup completed"
            ;;
        *)
            echo -e "${WHITE}TP4-ACSO Test Runner - Docker/Native Selection${NC}"
            echo
            echo "Usage: $0 [--docker|--native] {ring|shell|all|benchmark|clean}"
            echo
            echo -e "${CYAN}Execution Modes:${NC}"
            echo "  --docker      - Run tests in Docker container (x86_64 Ubuntu 22.04)"
            echo "  --native      - Run tests directly on current system"
            echo "  (auto-detect) - Automatically choose based on system architecture"
            echo
            echo -e "${CYAN}Commands:${NC}"
            echo "  ring      - Run all EJ1 (Ring Communication) tests from tests/ej1/"
            echo "  shell     - Run all EJ2 (Shell Implementation) tests from tests/ej2/"  
            echo "  all       - Run ALL tests: EJ1 + EJ2 + Integration + benchmarks"
            echo "  benchmark - Run performance benchmarks for ring implementation"
            echo "  clean     - Clean up Docker resources and build artifacts"
            echo
            echo -e "${CYAN}Examples:${NC}"
            echo "  $0 all                    # Auto-detect mode, run all tests"
            echo "  $0 --docker shell         # Force Docker mode, test shell only"
            echo "  $0 --native ring          # Force native mode, test ring only"
            echo "  $0 --native all           # Native mode, complete test suite"
            echo
            echo -e "${CYAN}System Detection:${NC}"
            echo "  Current: $(uname -s) $(uname -m)"
            if [[ "$(uname -s)" == "Linux" && "$(uname -m)" == "x86_64" ]]; then
                echo "  Recommended: Both --docker and --native work well"
            else
                echo "  Recommended: --docker (for x86_64 compatibility)"
            fi
            echo
            echo -e "${YELLOW}Features:${NC}"
            echo "  ✅ Flexible execution: Docker or native"
            echo "  🐳 Auto-detects system architecture"
            echo "  📊 Detailed test reporting with pass/fail counts"
            echo "  📝 Comprehensive logging to ${LOG_DIR}/"
            echo "  🚀 Performance benchmarking for ring implementation"
            echo "  🔍 Automatic test discovery in organized directories"
            echo
            exit 1
            ;;
    esac
}

# Execute main function with all arguments
main "$@"
