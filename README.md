# matriochqa
A csv &amp; markdown front end for QEMU

## Requirements
- MS-Windows, for the moment - although it should work on Linux as well.
- Qt 5.12, no way to escape
- [Hugo](https://gohugo.io/) to render the server status pages into a browser.
- Hugo them [Book](https://github.com/alex-shpak/hugo-book). matriochqa may work with other themes, but I tested only this one.
- [QHttpServer](https://blog.qt.io/blog/2019/01/25/introducing-qt-http-server/), to interact with the emulator instances from the web panel. Still, you can compile matriochqa without web commands support.
- [QEMU](https://www.qemu.org/) of course

## Build
As usual:
```
qmake matriochqa.pro
make
```

## Status
**NOT FUNCTIONAL, WORKING ON IT !**
