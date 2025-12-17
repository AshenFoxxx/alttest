# ALT Linux Branch Package Comparator

Shared library + CLI utility for comparing binary packages between ALT Linux branches using official RDB API.

## Features

- Compares any branches: `sisyphus`, `p11`, `p10`, `p9`, etc.
- RPM-compatible EVR comparison (`version-release`)
- All architectures: `x86_64`, `aarch64`, `i586`, `armh`, etc.
- JSON output:
  * `only_in_branch1` - packages only in branch1
  * `only_in_branch2` - packages only in branch2  
  * `newer_in_branch1` - packages newer in branch1 vs branch2
- Interactive mode + CLI mode
- FHS-compliant installation
## Quick Start

### 1. Container (Recommended - Docker/Podman)
### Build
```bash
sudo podman build -t alttest . # or docker build
```
### Test sisyphus vs p11 (x86_64)
```bash
sudo podman run --rm alttest
```
### Custom branches + save JSON locally
```bash
sudo podman run --rm -v $(pwd):/app alttest x86_64 sisyphus p10
ls -lh comparison.json # 3.8MB JSON result!
```

### 2. Native Install (ALT Workstation K 11.1)
- Dependencies (from ALT p11 repo)
```bash
sudo apt-get install gcc make pkg-config libjansson-devel libcurl-devel
```
- Build & Install

```bash
git clone <repo>
cd alttest
make
sudo make install
```
- Usage

```bash
alttest x86_64 sisyphus p11
```
- Interactive mode (no args)
- alttestDependencies (from ALT p11 repo)

```bash
sudo apt-get install gcc make pkg-config libjansson-devel libcurl-devel
```
## Usage

### CLI Mode
```
alttest <arch> <branch1> <branch2>
```

### Examples:
```bash
alttest x86_64 sisyphus p11
alttest aarch64 p10 p9
alttest i586 sisyphus p10
```
### Interactive Mode
```
alttest
```
```

Enter arch: x86_64
Enter branch1: sisyphus
Enter branch2: p11
```
### Output
- Console: **STATS** (total packages, only_in_*, newer_in_*)
- File: **`comparison.json`** (3-4MB detailed result)

## JSON Structure
```json
{
    "branch1": "sisyphus",
    "branch2": "p11",
    "arch": "x86_64",
    "total_branch1": 38315,
    "total_branch2": 36018,
    "only_in_branch1": [
        "pkg1",
        "pkg2"
    ],
    "only_in_branch2": [
        "pkg3"
    ],
    "newer_in_branch1": [
        {
            "name": "glibc",
            "branch1": "2.38-alt1",
            "branch2": "2.37-alt2"
        }
    ]
}

```


## FHS Installation Paths
```
| Path | Description |
|------|-------------|
| `/usr/local/lib64/libalttest.so.1.0.0` | Shared library |
| `/usr/local/lib64/libalttest.so.1` | Symlink |
| `/usr/local/lib64/libalttest.so` | Symlink |
| `/usr/local/bin/alttest` | CLI utility |
```

```bash
sudo make install # Install
sudo make uninstall # Remove
```

## Build & Development
```bash
make # Build lib + CLI
make clean # Cleanup
make install # FHS install
```
## Example Results (sisyphus vs p11, x86_64)
```bash
STATS:
sisyphus : 38315 pkgs
p11 : 36018 pkgs
only_in_b1 : 5037 pkgs
only_in_b2 : 2740 pkgs
newer_in_b1 : 9863 pkgs
```

**comparison.json**: 3.8MB detailed comparison

## Supported Branches
- `sisyphus`, `p11`, `p10`, `p9`
- Any branch from [rdb.altlinux.org/api](https://rdb.altlinux.org/api/)


