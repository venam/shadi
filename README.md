#A Shadi knowledge generator

Inspired by [dbag](https://github.com/iotek/dbag/), mixed with and idea of
[pranomostro](https://nixers.net/showthread.php?tid=1872) and using markov
chains of [c9x](http://c9x.me/cbits/).

There's a fortune file and a markov chain.

##Usage

```
./shadi.sh #install the fortune file - rebuild it
forturne shadi
```

Or

```
make
./markov  -c 4 shadi.plain
```
