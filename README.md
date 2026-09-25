# XMG EVO 15 (E25) Battery Charge Control

Prolong your XMG laptop battery life with: 
Linux battery charge threshold management for the **XMG EVO 15 (E25)** and compatible TongFang AMD Strix Point platforms (AMI BIOS). 

This project provides a native Linux kernel module that interfaces directly with the Embedded Controller (EC) register `0x07B9` via the ACPI WMI pipeline (`\_SB.AMW0.WMBC`). By registering standard power supply attributes, it natively unlocks battery threshold controls in desktop environments like KDE Plasma, GNOME, and Niri/Waybar setups.

---

## How it looks (niri/iNiR)
git push -u origin main
![Settings Screenshot](docs/charge_limit_niri_ui.png)
*(niri/iNir: Setting->System->Power->Battery->Charge limit was disabled (greyed out), so I've dug in and now it works like a charm.)*

---

## Hardware & Technical Details

- **Target Platform:** TongFang AMD Strix Point (XMG EVO 15 E25)
- **EC Offset:** `0x07B9` (Integer range `1`–`100`)
- **ACPI Pipeline:** Routed via `\_SB.AMW0.WMBC`
  - 3-Step EC Register Sequence: Unlock/Initialize (`0x0001`), Write Payload (`0x00[Val_Hex]07B9`), and Commit/Sync.

---

## Repository Structure

```shell
xmg-acpi-bat/
├── docs/
│   └── charge_limit_niri_ui.png
├── kernel_module/
│   ├── Makefile
│   ├── build_and_test.sh
│   └── xmg_ec_battery.c
├── systemd_service/
│   ├── enable_and_test.sh
│   ├── xmg-battery-limit.service
│   └── xmg-set-charge-limit
└── README.md
```

## Installation & Usage

### 1. Prerequisites
Ensure you have your kernel headers and a build toolchain installed:
- Arch Linux: `sudo pacman -S linux-headers base-devel clang llvm`
- Ubuntu/Debian: `sudo apt install build-essential linux-headers-$(uname -r)`

### 2. Build and Test the Kernel Module
Navigate into the `kernel_module/` directory and run the helper script to compile and insert the module dynamically:
```shell
cd kernel_module
./build_and_test.sh
```

Once loaded, check your sysfs node and current status:
Bash

```shell
cat /sys/class/power_supply/BAT0/charge_control_end_threshold
echo 80 | sudo tee /sys/class/power_supply/BAT0/charge_control_end_threshold
```

### 3. Make It Permanent on Boot
To ensure the kernel module loads automatically on every system boot:

    1. Copy or install your compiled .ko module to your kernel modules tree.

    2. Register the module in /etc/modules-load.d/xmg-battery.conf:

```text
    xmg_ec_battery
```

## Useful References & Acknowledgments

- EC Hacking Guide: Writing a custom EC driver / investigating ACPI interfaces

- Community Context: TongFang/Clevo ACPI reverse-engineering patterns for Linux power management.
