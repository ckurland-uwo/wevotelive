# WebSocketTestServer

A lightweight WebSocket server using [IXWebSocket](https://github.com/machinezone/IXWebSocket), written in C++17.

---

## 📦 Prerequisites

- CMake ≥ 3.5
- A C++17-compatible compiler (GCC or MSVC)
- [IXWebSocket](https://github.com/machinezone/IXWebSocket) cloned inside the `server/` directory

---

## 📁 Project Structure

```
wevotelive/
├── server/
│   ├── CMakeLists.txt         # Main build script
│   ├── server.cpp             # Your server source
│   ├── IXWebSocket/           # Clone of the IXWebSocket repo
│   └── build/                 # Will be created during build
```

---

## 🖥️ How to Build (Windows - MSYS2 UCRT64)

> 💡 These steps assume you are using **MSYS2 UCRT64 shell** and have installed the necessary development tools via pacman.

### ✅ 1. Clone IXWebSocket into the `server/` folder:

```bash
git clone https://github.com/machinezone/IXWebSocket.git
```

Make sure the folder structure looks like:
```
server/
├── IXWebSocket/
├── server.cpp
└── CMakeLists.txt
```

### ✅ 2. Install MSYS2 packages (if you haven't already):

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-zlib
```

### ✅ 3. Build IXWebSocket

```bash
cd ~/your/project/path/server/IXWebSocket
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build .
```

### ✅ 4. Build the project

```bash
cd ~/your/project/path/server
mkdir build-win && cd build-win
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build .
```

### ✅ 5. Run the server

```bash
./server.exe
```

---

## 🐧 How to Build (Linux / WSL / Ubuntu)

### ✅ 1. Install dependencies

```bash
sudo apt update
sudo apt install build-essential cmake zlib1g-dev
```

### ✅ 2. Clone IXWebSocket into the project

```bash
cd ~/your/project/path/server
git clone https://github.com/machinezone/IXWebSocket.git
```

### ✅ 3. Build IXWebSocket

```bash
cd ~/your/project/path/server/IXWebSocket
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### ✅ 4. Build everything
```bash
cd ~/your/project/path/server
mkdir build-linux && cd build-linux
cmake ..
make -j$(nproc)
```

### ✅ 5. Run the server

```bash
./server
```

---

## ✅ Notes

- This project uses `add_subdirectory()` to build IXWebSocket along with your server — no need to precompile anything.
- On Linux, `pthread` and `zlib` are linked automatically.
- On Windows, ensure you're using **UCRT64** toolchain in MSYS2.
