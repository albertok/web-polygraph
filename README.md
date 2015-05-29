### Web-Polygraph

HTTP benchmarking tool. http://www.web-polygraph.org

### Changes from source

* Minor changes so it compiles on Debian
* SSL cert unique file names

To Build

```sh
apt-get install libssl-dev openssl-dev build-essentials build-tools gnuplot zlib 
./configure
make && make install
```

