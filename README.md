# General

Guayadeque Music Player 0.4.6 Beta
Juan Rios anonbeat@gmail.com
see LICENSE

Please email with bugs, suggestions, requests, translations to anonbeat@gmail.com
or post them in our forums http://guayadeque.org

Special Thanks to Mrmotinjo (Stefan Bogdanovic http://evilsun.carbonmade.com)
for the icon and splash designed for guayadeque.

---

## Extra audio playback support

Various formats support (like `DSD/DSF`) requires ``gstreamer1.0-libav`` package:

```bash
sudo apt install gstreamer1.0-libav
```

---

# Build

Need installed taglib, sqlite3, libcurl, gstreamer1.0, wxWidgets 3.0, libdbus-1, libgio, libwxsqlite3

Its been developed in XUbuntu

---

## Dependencies

### Ubuntu (pre 20.0):

```bash
sudo apt install libindicate-dev libgdk-pixbuf2.0-dev libtag-extras-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev
```

---

### Ubuntu 20.04

```bash
sudo apt install libgpod-dev libjsoncpp-dev libgdk-pixbuf2.0-dev libtag-extras-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev libwxsqlite3-3.0-dev libwxbase3.0-dev libtag1-dev libcurl4-gnutls-dev
```

---

## Build

### Normal build

```bash
./build 
sudo make install
```

---

### Faster build on multi-core systems

```bash
./build \
	-j$(nproc) \
	-j$(nproc)
sudo make install
```

