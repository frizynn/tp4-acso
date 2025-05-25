#!/bin/bash

# TP4-ACSO Test Runner - Refactored Version
# Executes all tests for both exercises in an isolated x86_64 environment
# Author: Refactored for TP4 Assignment  
# Date: May 2025
# Version: 3.0 - Simplified and Optimized

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
    echo -e "${WHITE}Version 3.0 - Automated Test Discovery${NC}"
    echo -e "${BLUE}================================================================${NC}"
    echo -e "${CYAN}Start Time: $(date)${NC}"
    echo -e "${CYAN}Log File: ${LOG_FILE}${NC}"
    echo -e "${CYAN}Architecture: x86_64 (Docker Ubuntu 22.04)${NC}"
    echo -e "${BLUE}================================================================${NC}"
}

# Error handling and cleanup
cleanup() {
    local exit_code=$?
    log "INFO" "Performing cleanup operations..."
    
    # Stop and remove any running containers
    for container in $(docker ps -q --filter "name=${CONTAINER_NAME}" 2>/dev/null || true); do
        log "WARNING" "Stopping container: $container"
        docker stop "$container" >/dev/null 2>&1 || true
        docker rm "$container" >/dev/null 2>&1 || true
    done
    
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
    
    if [[ ! -f "Dockerfile" ]] || [[ ! -d "src" ]] || [[ ! -d "tests" ]]; then
        log "ERROR" "Not in TP4-ACSO project directory"
        log "ERROR" "Please run this script from the project root directory"
        return 1
    fi
    
    local required_files=(
        "src/ej1/Makefile" "src/ej1/ring.c"
        "src/ej2/Makefile" "src/ej2/shell.c"
        "tests/ej1/test_ring.c" "tests/ej2/test_shell.c"
        "tests/test_integration.c"
    )
    
    for file in "${required_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            log "ERROR" "Required file missing: $file"
            return 1
        fi
    done
    
    log "SUCCESS" "Environment validation passed"
    return 0
}

check_docker() {
    log "INFO" "Checking Docker availability..."
    if ! docker info >/dev/null 2>&1; then
        log "ERROR" "Docker is not running. Please start Docker and try again."
        exit 1
    fi
    log "SUCCESS" "Docker is running"
}

build_docker_image() {
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

# Execute commands in Docker container
run_in_docker() {
    local commands="$1"
    local container_name="${CONTAINER_NAME}-$(date +%s)"
    
    docker run --rm \
        --name "$container_name" \
        --platform linux/amd64 \
        -v "$(pwd):$WORKSPACE_DIR" \
        -w "$WORKSPACE_DIR" \
        $DOCKER_IMAGE \
        bash -c "$commands"
}

# Build projects in Docker
build_projects() {
    log "STEP" "Building projects in Docker container..."
    
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
    
    if run_in_docker "$build_commands"; then
        log "SUCCESS" "All projects built successfully"
    else
        log "ERROR" "Build failed"
        exit 1
    fi
}

# Discover and run tests automatically
get_available_tests() {
    local filter="$1"
    local test_dir="$2"
    
    if [[ -n "$test_dir" ]]; then
        # Get tests from specific subdirectory
        run_in_docker "cd tests/$test_dir && find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sed 's|^\./||' | sort" 2>/dev/null | grep -E "$filter" || true
    else
        # Get tests from root tests directory and subdirectories
        run_in_docker "
            cd tests
            # Root level tests
            find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sed 's|^\./||' | sort
            # EJ1 tests
            if [[ -d ej1 ]]; then
                find ej1 -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sort
            fi
            # EJ2 tests
            if [[ -d ej2 ]]; then
                find ej2 -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' | sort
            fi
        " 2>/dev/null | grep -E "$filter" || true
    fi
}

run_tests() {
    local test_type="$1"
    log "STEP" "Discovering and running $test_type tests..."
    
    local filter_pattern=""
    local test_description=""
    local test_dir=""
    
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
    
    if run_in_docker "$test_commands"; then
        log "SUCCESS" "$test_type tests completed successfully"
    else
        log "ERROR" "Some $test_type tests failed"
        return 1
    fi
}
# Performance benchmark for ring implementation
run_benchmark() {
    log "STEP" "Running performance benchmark..."
    
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
    
    if run_in_docker "$benchmark_commands"; then
        log "SUCCESS" "Performance benchmark completed"
    else
        log "ERROR" "Performance benchmark failed"
        return 1
    fi
}

# List all available tests in the repository
list_tests() {
    log "INFO" "Discovering all available tests..."
    
    local list_commands="
        echo '==== Available Tests in Repository ===='
        echo
        cd tests
        
        echo '📁 EJ1 (Ring Communication) Tests:'
        if [[ -d ej1 ]]; then
            echo '  Source Files:'
            find ej1 -name '*.c' -type f | sort | sed 's|^|    📄 |'
            echo '  Executables:'
            find ej1 -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' 2>/dev/null | sort | sed 's|^|    🚀 |' || echo '    (No executables found - run build first)'
        else
            echo '    (No ej1 directory found)'
        fi
        echo
        
        echo '📁 EJ2 (Shell Implementation) Tests:'
        if [[ -d ej2 ]]; then
            echo '  Source Files:'
            find ej2 -name '*.c' -type f | sort | sed 's|^|    📄 |'
            echo '  Executables:'
            find ej2 -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' 2>/dev/null | sort | sed 's|^|    🚀 |' || echo '    (No executables found - run build first)'
        else
            echo '    (No ej2 directory found)'
        fi
        echo
        
        echo '📁 Integration Tests:'
        echo '  Source Files:'
        find . -maxdepth 1 -name '*.c' -type f 2>/dev/null | sort | sed 's|^\./|  📄 |' || echo '  (No integration test sources found)'
        echo '  Executables:'
        find . -maxdepth 1 -type f -executable ! -name '*.c' ! -name 'Makefile' ! -name 'README.md' 2>/dev/null | sort | sed 's|^\./|  🚀 |' || echo '  (No integration test executables found - run build first)'
        echo
        
        echo 'Test Categories:'
        echo '  🔄 Ring Tests (ej1/): Basic ring communication, advanced features, strict validation, extreme cases'
        echo '  🐚 Shell Tests (ej2/): Shell functionality, pipe handling, debug utilities'
        echo '  🔗 Integration Tests: Cross-exercise compatibility and full system tests'
        echo
        echo 'Usage:'
        echo '  ./run-tests.sh ring  - Run all ring communication tests'
        echo '  ./run-tests.sh shell - Run all shell implementation tests'
        echo '  ./run-tests.sh all   - Run all tests (ring + shell + integration)'
        echo
    "
    
    run_in_docker "$list_commands"
}

# Main execution function
main() {
    print_banner
    
    case "${1:-help}" in
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
        "list")
            validate_environment
            check_docker
            build_docker_image
            build_projects
            list_tests
            ;;
        "clean")
            log "INFO" "Cleaning up Docker resources..."
            docker system prune -f >/dev/null 2>&1 || true
            docker image rm $DOCKER_IMAGE >/dev/null 2>&1 || true
            log "SUCCESS" "Cleanup completed"
            ;;
        *)
            echo -e "${WHITE}TP4-ACSO Test Runner - Organized Test Discovery${NC}"
            echo
            echo "Usage: $0 {ring|shell|all|benchmark|list|clean}"
            echo
            echo -e "${CYAN}Commands:${NC}"
            echo "  ring      - Run all EJ1 (Ring Communication) tests from tests/ej1/"
            echo "  shell     - Run all EJ2 (Shell Implementation) tests from tests/ej2/"  
            echo "  all       - Run ALL tests: EJ1 + EJ2 + Integration + benchmarks"
            echo "  benchmark - Run performance benchmarks for ring implementation"
            echo "  list      - List all available tests organized by exercise"
            echo "  clean     - Clean up Docker resources and images"
            echo
            echo -e "${CYAN}Test Organization:${NC}"
            echo "  📁 tests/ej1/     - Ring communication tests"
            echo "  📁 tests/ej2/     - Shell implementation tests"
            echo "  📁 tests/         - Integration and cross-exercise tests"
            echo
            echo -e "${CYAN}Examples:${NC}"
            echo "  $0 all       # Complete organized test suite (recommended)"
            echo "  $0 ring      # Test only ring implementation (EJ1)"
            echo "  $0 shell     # Test only shell implementation (EJ2)"
            echo "  $0 list      # See organized test structure"
            echo "  $0 benchmark # Performance testing only"
            echo
            echo -e "${YELLOW}Features:${NC}"
            echo "  ✅ Organized test structure by exercise"
            echo "  🐳 Runs in isolated x86_64 Docker environment"
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
