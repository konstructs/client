# Install and Dependencies

Clone the git repository and download/update all submodules.

    git clone https://github.com/konstructs/client.git
    git submodule update --init --recursive

## Ubuntu Linux

Install the needed build tools and dependencies

    sudo apt-get install build-essential cmake libglew-dev xorg-dev
    sudo apt-get build-dep glfw

## macOS

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
if you don\'t already have it. You may use [Homebrew](http://brew.sh) to simplify
the installation:

    brew install cmake

(You also need to figure out how to enable the C++ compiler, and other dependencies,
if your are a macOS user please make a PR or open a issue and provide instructions)

## Windows

The official Windows build are cross compiled from Docker container, to do that your self
you need Docker installed.

    make -C package windows

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)
and [MinGW](http://www.mingw.org/). Add `C:\MinGW\bin` to your `PATH`. Install
the needed dependences.

Use the following commands in place of the ones described in the next section.

    cmake -G "MinGW Makefiles"
    mingw32-make

The game builds under Virtual Studio on Windows, but that is rarely tested.

(If your are a Windows user please make a PR or open a issue and provide instructions)

## Compile and Run

Once you have the dependencies, run the following commands in your
terminal.

    mkdir build && cd build
    cmake .. && make
    ./konstructs -h
