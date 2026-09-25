#!/usr/bin/env bash
set -euo pipefail

# Get absolute path to the directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SCRIPT_SRC="${SCRIPT_DIR}/xmg-set-charge-limit"
SERVICE_SRC="${SCRIPT_DIR}/xmg-battery-limit.service"

SCRIPT_DEST="/usr/local/bin/xmg-set-charge-limit"
SERVICE_DEST="/etc/systemd/system/xmg-battery-limit.service"

# Ensure source script is executable
chmod +x "${SCRIPT_SRC}"

# Create symlinks (-f forces overwrite if old symlinks exist)
echo "Creating symlinks..."
sudo ln -sf "${SCRIPT_SRC}" "${SCRIPT_DEST}"
sudo ln -sf "${SERVICE_SRC}" "${SERVICE_DEST}"

# Reload systemd configuration
echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

# Enable and run immediately
echo "Enabling and starting service..."
sudo systemctl enable --now xmg-battery-limit.service

# Check service status
echo "Checking service status..."
sudo systemctl status xmg-battery-limit.service

