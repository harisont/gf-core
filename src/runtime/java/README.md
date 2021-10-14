# Java bindings to C runtime

## Pre-requisites
1. You must have installed the PGF C runtime (see [`../c/README.md`](../c/README.md))
2. You will need a Java Development Kit, for instance the [open JDK](https://openjdk.java.net/)

## Installation
1. Run `make`. If this fails with an error such as 

   ```
   Makefile:33: *** No JNI headers found.  Stop.
   ```

   `locate` your `jni.h` and `jni_md.h` files and assign the respective containing folders to `JNI_INCLUDES` in the [Makefile](Makefile) as follows: 
   
   ```
   JNI_INCLUDES = -I FOLDER_CONTAINING_jni.h -I FOLDER_CONTAINING_jni_md.h
   ```
   
   On Windows, you might need to take a look and uncomment the lines around `WINDOWS_FLAGS`.
2. Run `make install`. The output will contain the folder where `jpgf` is installed, for instance
   
   ```
   Libraries have been installed in:
   /usr/local/lib
   ```
   
   Keep this in mind, as when running Java code using `jpgf` you'll have to set `java.library.path` to this path with the `-D` flag (as an example, see __Running tests__ below).

## Usage
See: https://www.grammaticalframework.org/doc/runtime-api.html#java

## Running tests
1. [Download `JUnit` and `hamcrest-core`'s `jar`s](https://github.com/junit-team/junit4/wiki/Download-and-Install) to the current folder 
2. Compile with
   ```
   javac -cp .:junit-4.13.2.jar TestBasic.java
   ```
3. Run with 
   ```
   java -Djava.library.path=PATH_TO_LIBS -cp .:junit-4.13.2.jar org.junit.runner.JUnitCore TestBasic
   ```
   where PATH_TO_LIBS is the path where `make install` has put `jpgf` (see Installation step 2).