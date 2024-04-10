Torrent Viewer plugin for Double Commander
==========================================

Simple .torrent viewer lister (WLX) plugin for [Double Commander][dc].

Displays common torrent info usually available in torrent clients:

- torrent info hash(es)
- piece size and pieces count
- trackers list
- files list

Supports [BitTorrent v2][bt] and Hybrid torrents.

**Suitable only for Qt-based Double Commander**:

- doublecmd-qt5
- doublecmd-qt6

Build plugin with the same Qt version as Double Commander.

Dependencies
------------

The only external dependency (except Qt and Double Commander SDK)
is [libtorrent-rasterbar][lt], version 2.0 is required.

Building
--------

By default Qt6 is expected, building with Qt5 requires explicit option,
see the few first lines of CMakeLists.txt for details.

Double Commander SDK is not included in this repo, it should downloaded
separately and placed into 'sdk' folder.

[dc]: https://github.com/doublecmd/doublecmd
[bt]: https://blog.libtorrent.org/2020/09/bittorrent-v2/
[lt]: https://github.com/arvidn/libtorrent
