
Debian
====================
This directory contains files used to package kored/kore-qt
for Debian-based Linux systems. If you compile kored/kore-qt yourself, there are some useful files here.

## kore: URI support ##


kore-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install kore-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your koreqt binary to `/usr/bin`
and the `../../share/pixmaps/kore128.png` to `/usr/share/pixmaps`

kore-qt.protocol (KDE)

