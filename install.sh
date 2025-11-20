#!/usr/bin/env bash
set -euo pipefail

# install_vscode_gpp.sh
# Idempotent installer: snapd -> VS Code (snap) -> g++/gdb -> VS Code C++ extension
# Also creates a sample project at ~/cpp-project with .vscode configuration.
#
# Notes:
# - This script uses snap for VS Code installation (classic confinement).
# - It uses the system package manager to install g++/gdb (apt, dnf or yum).
# - Run as a user with sudo privileges.

# ---------- Configuration ----------
PROJECT_DIR="${HOME}/cpp-project"
VSCODE_EXT="ms-vscode.cpptools"
# Compiler and debugger paths (assumed standard on Linux distros)
COMPILER_PATH="/usr/bin/g++"
DEBUGGER_PATH="/usr/bin/gdb"

# ---------- Helpers ----------
log() { printf "\n[INFO] %s\n" "$1"; }
err() { printf "\n[ERROR] %s\n" "$1" >&2; exit 1; }

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

# Detect package manager (apt/dnf/yum)
detect_pkg_mgr() {
  if command_exists apt-get; then
    echo "apt"
  elif command_exists dnf; then
    echo "dnf"
  elif command_exists yum; then
    echo "yum"
  else
    echo ""
  fi
}

# ---------- Start ----------
if [ "$EUID" -eq 0 ]; then
  # Running as root: prefer running with sudo for operations that can be done as normal user.
  # But this script performs system installs (snapd, packages) which require root.
  log "Running as root. Proceeding."
fi

PKG_MGR="$(detect_pkg_mgr)"
if [ -z "$PKG_MGR" ]; then
  err "No supported package manager detected (apt, dnf, or yum required)."
fi
log "Detected package manager: $PKG_MGR"

# Update package lists where appropriate
if [ "$PKG_MGR" = "apt" ]; then
  log "Updating apt package lists..."
  sudo apt-get update -y
elif [ "$PKG_MGR" = "dnf" ]; then
  log "Updating dnf metadata..."
  sudo dnf makecache --refresh -y || true
elif [ "$PKG_MGR" = "yum" ]; then
  log "Updating yum metadata..."
  sudo yum makecache -y || true
fi

# Install snapd if missing
if ! command_exists snap; then
  log "snap not found: installing snapd..."
  if [ "$PKG_MGR" = "apt" ]; then
    sudo apt-get install -y snapd
    # Enable and start snapd socket/service if systemd present
    if command_exists systemctl; then
      sudo systemctl enable --now snapd.socket || true
    fi
  elif [ "$PKG_MGR" = "dnf" ]; then
    sudo dnf install -y snapd
    if command_exists systemctl; then
      sudo systemctl enable --now snapd.socket || true
    fi
    # optional: create classic symlink for snap classic support
    sudo ln -s /var/lib/snapd/snap /snap 2>/dev/null || true
  elif [ "$PKG_MGR" = "yum" ]; then
    sudo yum install -y snapd
    if command_exists systemctl; then
      sudo systemctl enable --now snapd.socket || true
    fi
    sudo ln -s /var/lib/snapd/snap /snap 2>/dev/null || true
  fi
else
  log "snap already installed."
fi

# Ensure snap path available in current shell (snap puts binaries in /snap/bin)
if [ -d /snap/bin ] && [[ ":$PATH:" != *":/snap/bin:"* ]]; then
  log "Adding /snap/bin to PATH for current session."
  export PATH="/snap/bin:$PATH"
fi

# Install/refresh VS Code via snap
if snap list | awk 'NR>1 {print $1}' | grep -xq "code"; then
  log "VS Code (snap 'code') already installed. Attempting refresh..."
  sudo snap refresh code || true
else
  log "Installing Visual Studio Code via snap (classic)..."
  # classic confinement is required for the official vscode snap
  sudo snap install --classic code
fi

# Install g++ and gdb via system package manager
if [ "$PKG_MGR" = "apt" ]; then
  # Use build-essential which provides g++ plus make etc.
  log "Installing build-essential (g++) and gdb via apt..."
  sudo apt-get install -y build-essential gdb
elif [ "$PKG_MGR" = "dnf" ]; then
  log "Installing gcc-c++ (g++) and gdb via dnf..."
  sudo dnf install -y gcc-c++ gdb
elif [ "$PKG_MGR" = "yum" ]; then
  log "Installing gcc-c++ (g++) and gdb via yum..."
  sudo yum install -y gcc-c++ gdb
fi

# Verify compiler and debugger existence
if ! command_exists g++; then
  err "g++ not found after install. Aborting."
fi
if ! command_exists gdb; then
  log "gdb not found or failed to install. Debugging in VS Code will not work until gdb is present."
fi

log "g++ location: $(command -v g++ || echo 'not found')"
log "gdb location: $(command -v gdb || echo 'not found')"

# Install the C/C++ extension for VS Code
# The 'code' CLI is provided by the snap; ensure it's available
if ! command_exists code; then
  err "'code' command not found in PATH. You may need to restart your shell or ensure /snap/bin is in PATH."
fi

# Check if extension is installed
if code --list-extensions | grep -xq "$VSCODE_EXT"; then
  log "VS Code extension $VSCODE_EXT already installed. Attempting update..."
  code --install-extension "$VSCODE_EXT" --force >/dev/null 2>&1 || true
else
  log "Installing VS Code extension: $VSCODE_EXT ..."
  code --install-extension "$VSCODE_EXT" --force
fi

# Create a sample project directory and .vscode configuration
log "Creating sample project directory at: $PROJECT_DIR"
mkdir -p "$PROJECT_DIR"
cd "$PROJECT_DIR"

VSCODE_DIR="${PROJECT_DIR}/.vscode"
mkdir -p "$VSCODE_DIR"

# tasks.json: compile current file or all .cpp to executable named after file
cat > "${VSCODE_DIR}/tasks.json" <<'EOF'
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build active file",
      "type": "shell",
      "command": "/usr/bin/g++",
      "args": [
        "-g",
        "-std=c++17",
        "${file}",
        "-o",
        "${fileDirname}/${fileBasenameNoExtension}"
      ],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "presentation": {
        "reveal": "always",
        "panel": "shared"
      },
      "problemMatcher": [
        "$gcc"
      ]
    },
    {
      "label": "Build project (all cpp)",
      "type": "shell",
      "command": "/bin/sh",
      "args": [
        "-c",
        "g++ -g -std=c++17 *.cpp -o app || true"
      ],
      "group": {
        "kind": "build",
        "isDefault": false
      },
      "presentation": {
        "reveal": "always",
        "panel": "shared"
      },
      "problemMatcher": [
        "$gcc"
      ]
    }
  ]
}
EOF

# launch.json: debug the compiled binary (default: executable named after file) using cpptools.
cat > "${VSCODE_DIR}/launch.json" <<EOF
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "g++ build and debug active file",
      "type": "cppdbg",
      "request": "launch",
      "program": "\${fileDirname}/\${fileBasenameNoExtension}",
      "args": [],
      "stopAtEntry": false,
      "cwd": "\${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "${DEBUGGER_PATH}",
      "preLaunchTask": "Build active file",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    },
    {
      "name": "Debug compiled project (app)",
      "type": "cppdbg",
      "request": "launch",
      "program": "\${workspaceFolder}/app",
      "args": [],
      "stopAtEntry": false,
      "cwd": "\${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "${DEBUGGER_PATH}"
    }
  ]
}
EOF

# c_cpp_properties.json: point to compilerPath and common include settings
cat > "${VSCODE_DIR}/c_cpp_properties.json" <<EOF
{
  "configurations": [
    {
      "name": "Linux",
      "includePath": [
        "\${workspaceFolder}/**",
        "/usr/include",
        "/usr/local/include"
      ],
      "defines": [],
      "compilerPath": "${COMPILER_PATH}",
      "cStandard": "c11",
      "cppStandard": "c++17",
      "intelliSenseMode": "gcc-x64"
    }
  ],
  "version": 4
}
EOF

# Optional settings.json: small tweaks
cat > "${VSCODE_DIR}/settings.json" <<'EOF'
{
  "files.autoSave": "off",
  "editor.formatOnSave": false,
  "C_Cpp.intelliSenseEngine": "Tag Parser",
  "C_Cpp.default.compilerPath": "/usr/bin/g++"
}
EOF

# Create a starter main.cpp if none exists
if [ ! -f "${PROJECT_DIR}/main.cpp" ] && [ ! -f "${PROJECT_DIR}/hello.cpp" ]; then
  cat > "${PROJECT_DIR}/main.cpp" <<'CPP'
#include <iostream>

int main() {
    std::cout << "Hello, C++ world!" << std::endl;
    return 0;
}
CPP
  log "Created sample source: ${PROJECT_DIR}/main.cpp"
fi

log "Installation and configuration complete."

cat <<SUMMARY

What I installed/configured:
 - snapd (if it was not already installed)
 - Visual Studio Code via snap (package name: 'code', classic confinement)
 - GNU C++ toolchain (g++) and gdb via system package manager ($PKG_MGR)
 - VS Code extension: $VSCODE_EXT
 - Sample project created at: $PROJECT_DIR
 - .vscode/tasks.json  -> build tasks (Build active file / Build project)
 - .vscode/launch.json -> debug configurations (gdb / cppdbg)
 - .vscode/c_cpp_properties.json -> compilerPath set to ${COMPILER_PATH}
 - .vscode/settings.json -> minimal recommended settings

How to use:
 1) Open the project in VS Code:
      code "$PROJECT_DIR"
 2) Open main.cpp (or your own .cpp file).
 3) To build the active file: Run the 'Run Build Task' (Ctrl+Shift+B) or choose Tasks > Run Build Task.
 4) To debug: Set a breakpoint and press F5 (use the "g++ build and debug active file" configuration).

Notes / Caveats:
 - The script installs g++/gdb via the system package manager (apt/dnf/yum). There is no widely-adopted snap package for g++ that is recommended, so system packages are used to minimize complexity and ensure standard paths.
 - If you prefer a different compiler path or have a custom toolchain, update .vscode/c_cpp_properties.json and .vscode/launch.json accordingly.
 - If 'code' CLI is not available immediately after snap install, restart your shell or log out / log in; /snap/bin must be in PATH.

SUMMARY

exit 0
