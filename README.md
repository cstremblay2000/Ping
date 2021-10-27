# Ping
Implementation of Ping in C

---
# How to make it
There is an included make file, so the make command should work to create the executable.  **The Makefile does call sudo** and this is because it needs set the "s" bit in the permissions. The program uses **raw sockets** which require root access to create. 
---
# Usage
./ping [-c count] [-i wait] [-s size] host

---
# Important notes
This does not support the full set of flags that ping would on a linux distrubutions. 
