FROM ubuntu:20.04

ENV TZ=Europe/Stockholm

# Install build tools for C runtime
RUN apt-get update \
 && apt-get install --yes --no-install-recommends tzdata \
 && apt-get install --yes --no-install-recommends \
    autoconf automake libtool libtool-bin make g++ 

# Install Java stuff
RUN apt-get install --yes --no-install-recommends default-jre default-jdk \
    && apt-get install --yes --no-install-recommends junit4

# Build C runtime
COPY src/runtime/c/m4/ax_prog_doxygen.m4 /tmp/gf-core/src/runtime/c/m4/
COPY src/runtime/c/pgf/*.h \
    src/runtime/c/pgf/*.cxx \
    /tmp/gf-core/src/runtime/c/pgf/
COPY src/runtime/c/configure.ac \
    src/runtime/c/libpgf.pc.in \
    src/runtime/c/Makefile.am \
    /tmp/gf-core/src/runtime/c/
WORKDIR /tmp/gf-core/src/runtime/c
RUN autoreconf -i && \
    ./configure && \
    make && \
    make install
ENV LD_LIBRARY_PATH=/usr/local/lib

# Build Java bindings
COPY src/runtime/java/*.c \
    src/runtime/java/*.h \
    src/runtime/java/*.java \
    src/runtime/java/Makefile \
    /tmp/gf-core/src/runtime/java/
COPY src/runtime/java/org/grammaticalframework/pgf/*.java \
    /tmp/gf-core/src/runtime/java/org/grammaticalframework/pgf/
WORKDIR /tmp/gf-core/src/runtime/java
RUN make && make install

# Copy test grammar
COPY src/runtime/haskell/tests/basic.pgf \
    src/runtime/haskell/tests/basic.gf \
     /tmp/gf-core/src/runtime/haskell/tests/

# Build and run testsuite
WORKDIR /tmp/gf-core/src/runtime/java
RUN javac -cp .:/usr/share/java/junit4.jar -encoding ISO-8859-1 TestBasic.java \
    && java -Djava.library.path=$LD_LIBRARY_PATH -cp .:/usr/share/java/junit4.jar org.junit.runner.JUnitCore TestBasic
