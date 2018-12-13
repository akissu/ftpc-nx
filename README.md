# ftpc WIP

FTP Client for Switch

Code resused from:
- https://github.com/mtheall/ftpd
- https://github.com/mkulke/ftplibpp

## Features

- Authentication (hardcoded for now)

## Build and install

You must set up the [development environment](https://devkitpro.org/wiki/Getting_Started).

### NRO

The following pacman packages are required to build `nro`:

    devkitA64
    libnx
    switch-tools

They are available as part of the `switch-dev` meta-package.

Build `nro`:

    make nro

## Supported Commands

- CDUP
- CWD
- HELP
- LIST
- NLST
- GET

## TODO

- Remove mtheall/ftpd code
- Resolve/debug workarounds for mkulke/ftplibpp issues
- Add mkulke/ftplibpp or a fork of as a submodule
