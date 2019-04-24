Mac OS X Build Instructions and Notes
====================================
* This guide will show you how to build the full set of Kore wallet binaries,

`kore-qt` - Full Qt GUI client

`kored` - daemon

`kore-cli` - daemon control client

`kore-tx` - tx client


Notes
-----

* Build process tested on MacOS Mojave (10.14)

* Build Result tested on MacOS Sierra (10.12), MacOS High Sierra (10.13) & MacOS Mojave (10.14)

* All of the commands should be executed in a Terminal application. The
built-in one is located in `/Applications/Utilities`.

* This process relies on the  depends folder for all its build dependencies

Preparation
-----------

You need to install XCode with all the options checked so that the compiler
and everything is available in /usr not just /Developer. You can get the
latest for your MacOS version from https://developer.apple.com/xcode/. You will
need to install the XCode developer tools.

XCode developer tools can be installed seperatley by runing `xcode-select --install`

Note: you must also install this SDK headers package located here:
`/MacOS/Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg`

There's also an assumption that you already have `git` installed. If
not, It is  available via Homebrew. `brew install git`

You will also need to install [Homebrew](http://brew.sh) in order to install library
dependencies.

The installation of the actual dependencies is covered in the Instructions
sections below.

Instructions: Homebrew
----------------------

### Build environment dependencies using Homebrew

        brew install git autoconf automake libtool pkg-config librsvg


### Cloning the source files.

1. Clone the github tree to get the source code and go into the directory.

        git clone https://github.com/KORE-Project/KORE.git
        cd KORE


### Building the depends folder

1. `make -C depends HOST=x86_64-apple-darwin`

2. `./autogen.sh`

3. `CONFIG_SITE=$PWD/depends/x86_64-apple-darwin/share/config.site ./configure --prefix=/ --disable-tests --enable-obfuscation`

4. `make`

5. `make deploy` (optional task to create a portable .dmg image)


Use Qt Creator as IDE
------------------------
You can use Qt Creator as IDE, for debugging and for manipulating forms, etc.
Download Qt Creator from http://www.qt.io/download/. Download the "community edition" and only install Qt Creator (uncheck the rest during the installation process).

1. Make sure you installed everything through homebrew mentioned above
2. Do a proper ./configure --with-gui=qt5 --enable-debug
3. In Qt Creator do "New Project" -> Import Project -> Import Existing Project
4. Enter "kore-qt" as project name, enter src/qt as location
5. Leave the file selection as it is
6. Confirm the "summary page"
7. In the "Projects" tab select "Manage Kits..."
8. Select the default "Desktop" kit and select "Clang (x86 64bit in /usr/bin)" as compiler
9. Select LLDB as debugger (you might need to set the path to your installtion)
10. Start debugging with Qt Creator


Running
-------

It's now available at `./kored`, provided that you are still in the `src`
directory. We have to first create the RPC configuration file, though.

Run `./kored` to get the filename where it should be put, or just try these
commands:

    echo -e "rpcuser=korerpc\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "/Users/${USER}/Library/Application Support/KORE/kore.conf"
    chmod 600 "/Users/${USER}/Library/Application Support/KORE/kore.conf"

The next time you run it, it will start downloading the blockchain, but it won't
output anything while it's doing this. This process may take several hours;
you can monitor its process by looking at the debug.log file, like this:

    tail -f $HOME/Library/Application\ Support/KORE/debug.log

Other commands:
-------

    ./kore-qt # start the Full Qt client
    ./kored -daemon # to start the kore daemon.
    ./kore-cli --help  # for a list of command-line options.
    ./kore-cli help    # When the daemon is running, to get a list of RPC commands
