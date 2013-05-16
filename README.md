# __d2__ - A library-based approach to deadlock detection


## Building and installing the library
You will need the [Boost][boost_web] libraries (1.53.0 and later) and
[CMake][cmake_web].

Any other dependencies are included as submodules and only requires checking
them out. To do so, issue (from the root of the project):

```
    $ git submodule update --init
```


Then, you can generate the build files using CMake. From the root of the
project:

```
    $ mkdir build   # directory where the build files will be generated
    $ cd build
    $ cmake ..      # generate the build files
```


Finally, you can build __d2__. Inside the directory where the build files
were generated:

```
    $ make d2       # build the library
    $ make d2tool   # build the command line utility
    $ make install  # install both (if you want to)
```



### Building and running the tests
#### Unit tests
These tests require [Google Test][gtest_web], which is included as a submodule.
If the submodule is initialized, you can build and run the tests:

```
    $ make unit       # builds the unit tests
    $ make check_unit # runs them using CTest
```


#### Integration tests
These tests are enabled only when the compiler supports C++11. They test
several scenarios in which deadlocks should (or should not) be detected.
To build and run them:

```
    $ make scenarios       # builds the integration tests
    $ make check_scenarios # runs them using CTest
```


To build and run all the tests, you can do:

```
    $ make check
```


[boost_web]: http://boost.org
[cmake_web]: http://cmake.org
[gtest_web]: http://code.google.com/p/googletest
