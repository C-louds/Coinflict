# Coinflict

Coinflict is a C++ desktop application for managing and analyzing personal finances. It provides an intuitive GUI built with [ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot), and [GLFW](https://www.glfw.org/). The app supports importing transactions (including from PDFs), visualizing analytics, and tracking spending over time.

---

## Table of Contents
- [Features](#features)  
- [Installation](#installation)  
- [Usage](#usage)  
- [Configuration](#configuration)  
- [Dependencies](#dependencies)  
- [Troubleshooting](#troubleshooting)  
- [Contributors](#contributors)  
- [License](#license)  

---

## Features
- **Dashboard**: Overview of balance, spending trends, and pending transactions.  
- **Transactions**: Import, view, and filter all financial transactions.  
- **Analytics**: Charts and statistics powered by ImPlot.  
- **PDF Import**: Parse financial statements into structured transaction data.  
- **Date Picker**: Intuitive time-based filtering of data(Yet to implement).  
- Checkout [TODO.md](TODO.md) if intrested.
---

## Installation

### Prerequisites
- C++17 or newer  
- [CMake](https://cmake.org/) ≥ 3.10  
- [GLFW](https://www.glfw.org/)  
- [ImGui](https://github.com/ocornut/imgui) (included in `lib/`)  
- [ImPlot](https://github.com/epezent/implot) (included in `lib/`)  

### Build Instructions
```bash
# Clone repository
git clone https://github.com/yourusername/coinflict.git
cd coinflict

# Create build directory
mkdir build && cd build

# Run CMake
cmake ..

# Build project
make
