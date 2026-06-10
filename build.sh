#!/usr/bin/env bash
set -euo pipefail

PROJECT_NAME="PowerController"
DEFAULT_PRESET="Debug"
DEFAULT_CUBECLT_ROOT="C:/ST/STM32CubeCLT_1.21.0"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PRESET="${CMAKE_PRESET:-$DEFAULT_PRESET}"
CUBECLT_ROOT="${STM32CubeCLT_ROOT:-$DEFAULT_CUBECLT_ROOT}"
PROGRAMMER_CLI="${STM32_PROGRAMMER_CLI:-$CUBECLT_ROOT/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe}"
ELF_PATH="$SCRIPT_DIR/build/$PRESET/$PROJECT_NAME.elf"

usage() {
    cat <<EOF
Usage: $0 [all|build|flash|clean|configure] [preset]

Commands:
  all        Configure, build, then flash. This is the default.
  build      Configure and build only.
  flash      Flash the existing ELF only.
  clean      Remove build output for the selected preset.
  configure  Run CMake configure only.

Environment:
  CMAKE_PRESET          Default preset when the second argument is omitted.
  STM32CubeCLT_ROOT     STM32CubeCLT install path. Default: $DEFAULT_CUBECLT_ROOT
  STM32_PROGRAMMER_CLI  Full path to STM32_Programmer_CLI executable.
EOF
}

require_command() {
    local command_name="$1"

    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "Error: '$command_name' was not found in PATH." >&2
        exit 1
    fi
}

configure() {
    require_command cmake
    cmake --preset "$PRESET"
}

build() {
    configure
    cmake --build --preset "$PRESET"
}

flash() {
    if [[ ! -f "$ELF_PATH" ]]; then
        echo "Error: ELF file not found: $ELF_PATH" >&2
        echo "Run '$0 build $PRESET' first." >&2
        exit 1
    fi

    if [[ ! -x "$PROGRAMMER_CLI" && ! -f "$PROGRAMMER_CLI" ]]; then
        echo "Error: STM32_Programmer_CLI not found: $PROGRAMMER_CLI" >&2
        echo "Set STM32CubeCLT_ROOT or STM32_PROGRAMMER_CLI to the correct path." >&2
        exit 1
    fi

    "$PROGRAMMER_CLI" -c port=SWD mode=UR -w "$ELF_PATH" -v -rst
}

clean() {
    rm -rf "$SCRIPT_DIR/build/$PRESET"
}

COMMAND="${1:-all}"
if [[ "${2:-}" != "" ]]; then
    PRESET="$2"
    ELF_PATH="$SCRIPT_DIR/build/$PRESET/$PROJECT_NAME.elf"
fi

cd "$SCRIPT_DIR"

case "$COMMAND" in
    all)
        build
        flash
        ;;
    build)
        build
        ;;
    flash)
        flash
        ;;
    clean)
        clean
        ;;
    configure)
        configure
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        echo "Error: unknown command '$COMMAND'." >&2
        usage >&2
        exit 1
        ;;
esac