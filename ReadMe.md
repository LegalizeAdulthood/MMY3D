[![CMake workflow](https://github.com/LegalizeAdulthood/MMY3D/actions/workflows/cmake.yml/badge.svg)](https://github.com/LegalizeAdulthood/MMY3D/actions/workflows/cmake.yml)

# MMY3D

This repository contains the source code for MMY3D, a 3D fractal movie maker by Yann Le Bihan.

The code is experimental and in a prototype state.

The binary can be downloaded from [Karl Runmo's site](http://www.chillheimer.de/kallesfraktaler/mmy/)
and there is a [tutorial](http://www.arpege-expertise.com/mmy3d/tutorial.html) as well.

# Building

To build this repository, you will need CMake to generate a Visual Studio project.

After cloning the repository, in a command-line prompt use this command in the source code directory:

```
cmake --workflow --preset default
```

A sibling to your source code directory will be created called `build-`*dir*`-default`, where *dir*
is the name of the cloned source directory, `MMY3D` by default, e.g. `build-MMY3D-default`.  In the
build directory you will find a Visual Studio solution file you can open.
