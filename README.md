# YourNesPal
This is a command line tool that takes sprite PNGs and processes them into a format that can be used on the NES. 

It outputs indexed PNGs and binary files because these can be read by [NesFab](https://github.com/pubby/nesfab).

It is intended for use with [MMC3](https://www.nesdev.org/wiki/MMC3) mapper, because this mapper allows CHR bank switching and has large CHR ROMS (256K).
# Status 
This project is a big WIP, it is not yet usable.

I am making to brush up on my vanilla C++ after a time spent exclusively with Unreal, I also want to learn unit testing.

This is also made available for potential employers to see my programming, so for that reason I won't be accepting significant code contributions.
# Project Architecture
The build system is CMake with vcpkg as a package manager. 

The CMake build is split into three targets:
- The actual executable ("cmd")
- The static library that contains the projects functionality ("main")
- The test executable ("main_test")

Google Test is used as the testing library
