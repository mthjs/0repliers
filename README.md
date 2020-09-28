# 0repliers

> Only listen as a single ZMQ_ROUTER socket to provide a server which can
> handle multiple clients.

## Why?

For fun ofcourse, but also to see if there is a way to reduce the blast radius
of ZMQ in an existing code-base.

## Getting Started

> Although this is a somewhat functional solution, I doubt you'll want to use
> this. I won't stop you though and contributions welcome!

If you have it installed, I did provide a docker-container for ease of use.
All that needs to be done to get up to speed are the following commands (from
the root of this project):

```bash
$ docker build -t 0repliers ./container
$ docker run --rm -it -v `pwd`:/opt 0repliers bash
```

> If you don't have docker, or aren't keen on installing docker, you can also
> do the next steps without (although I haven't tested any other environment
> than the provided docker-container).
> Granted, you'll have to have the following installed:
> * cmake
> * make
> * gcc
> * zeromq

Now, to build we'll do the following:

```bash
mkdir -p build
cd build
cmake ..
make -j6 0repliers
```

### Running Tests

Before you can run the tests, you'll need to initialize the `git submodules`.
Which is as quite easy. At the root of this project, run the following:

```bash
$ git submodule update --init --recursive
```

After which, we should have the wonderful test-framework [Catch2](Catch2)
available. Presuming we're still at the root of this project, we'll do the
following to build and run the tests:

```bash
mkdir -p build
cd build
cmake ..
make -j6 tests; ./tests/test
```

Catch2: https://github.com/catchorg/Catch2
