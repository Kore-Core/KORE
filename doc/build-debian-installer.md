
DEBIAN INSTALLER BUILD NOTES
=============================

Below are the basic instructions for mounting a Debian installation package.

Requirements
-------------

- Linux 16.04 or higher.
- Go 1.11.0 or later.

After completing the compilation of the Kore Express project for Linux, run the following command sequence to create an installer for Linux Ubuntu and Debian.

Compiling the Kore Express project
-----------------------------------

Before assembling the package it is necessary to compile the project. The instructions are available in the [build-unix.md](./build-unix.md) file.

Mounting the Debian installation
-----------------------------------

It is necessary to compile the Obfs4Proxy plugin, to include the package.

> Note: You must be in the root directory of the project (expresskore).

Change the directory to the directory of the Obfs4Proxy plugin:
```
cd src/obfs4
```

Download the Obfs4Proxy project:

```
git submodule init
git submodule update
```

Compile the Obfs4Proxy project.

> Note: Requires Go 1.11 or later.

```
go build -o obfs4proxy/obfs4proxy ./obfs4proxy
```

Go back to the root directory and mount the package:

```
cd ../../
make deploy
```

The installer is generated in the directory below:

```
expresskore/share/
``` 