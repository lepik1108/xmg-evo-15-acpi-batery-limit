#!/usr/bin/env bash
set -euo pipefail

MODULE_NAME="xmg_ec_battery"
KO_FILE="./build/${MODULE_NAME}.ko"
SYSFS_NODE="/sys/class/power_supply/BAT0/charge_control_end_threshold"

echo "=== Building Kernel Module ==="
make clean
make

echo "=== Reloading Kernel Module ==="
# Force rmmod both underscore and hyphen variants to prevent 'File exists' errors
sudo rmmod "${MODULE_NAME}" 2>/dev/null || true
sudo rmmod "${MODULE_NAME//_/-}" 2>/dev/null || true

echo "Inserting ${KO_FILE} with dynamic debug enabled..."
sudo insmod "${KO_FILE}" dyndbg="+p"

echo "=== Checking dmesg after init ==="
sudo dmesg | grep "xmg_battery" | tail -n 10 || true

echo "=== Testing sysfs Node ==="
if [ -f "${SYSFS_NODE}" ]; then
    echo "sysfs node found: ${SYSFS_NODE}"

    echo -n "Current value: "
    ORIG_VAL=$(cat "${SYSFS_NODE}")
    echo "${ORIG_VAL}"

    echo "Testing write: 80%"
    echo 80 | sudo tee "${SYSFS_NODE}" > /dev/null

    echo -n "Updated value: "
    NEW_VAL=$(cat "${SYSFS_NODE}")
    echo "${NEW_VAL}"

    echo "=== Recent WMBC Kernel Debug Logs ==="
    sudo dmesg | grep "xmg_battery" | tail -n 10 || true

    # Restore original setting if valid
    #if [ -n "${ORIG_VAL}" ] && [ "${ORIG_VAL}" -ne 0 ]; then
    #    echo "${ORIG_VAL}" | sudo tee "${SYSFS_NODE}" > /dev/null
    #fi
else
    echo "ERROR: sysfs node ${SYSFS_NODE} not found!" >&2
    exit 1
fi

