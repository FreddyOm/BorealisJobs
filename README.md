# BorealisJobs
![CMake Multi Platform Build](https://github.com/FreddyOm/BorealisJobs/actions/workflows/cmake-multi-platform.yml/badge.svg)

**A multi-platform, fiber based job system based on a thread pool written in C and C++.**

## License
This project is authored by Frederik Omlor and may not be used for commercial or non-commercial purposes other than those specifically granted by the author.
The project, aswell as all other public projects on the authors GitHub account, may be cloned and used for private uses as long as they are not published in any way.

## Setup

***NOTE: This project is currently work in progress and therefore only available for Windows platform!***

To setup the project, create a ```build``` folder in this directory, navigate inside the folder and execute the command ```cmake ..```. This will setup the project for your respective platform.

Be aware that the library file (specifically the *BorealisJobs.dll*) will not be copied automatically. In order to execute the test project (see: *BorealisJobsTest/src/main.cpp*) you have to move/copy the library file manually next to the resulting executable!

Required CMake Version: 3.19 or newer. 

## Features

**TODO:** 
- [x] Thread pool
- [x] Fibers
- [x] Jobs
- [x] Spinlocks
- [x] Scoped Spinlocks
- [ ] Use *boost* to make the project compatible for multiple platforms
- [ ] Upgrade the test project to use coorperative concurrency and yield to other jobs mid-job
- [ ] Add JobContext being handed to any job including the thread id, parameters, etc.
- [ ] Visualizing the jobs as graph / DAG
 
## Dependencies

Currently, ***BorealisJobs*** has no external dependencies other than the Windows API (Win32). 

The Windows API is used for the implementation of the fibers. In the future this will be replaced by the multi-platform implementation of [boost](https://www.boost.org).

## Sources
*All of this code (except the spinlock class) was written by Frederik Omlor but is inspired and influencd by multiple sources. No AI was used for writing this projects code.*

The sources used for this project are listed below:

- [Game Engine Architecture - Jason Gregory](https://gameenginebook.com/)
- [Parallelizing the Naughty Dog engine using fibers - Christian Gyrling](https://www.youtube.com/watch?v=HIVBhKj7gQU)
- [Fiber in C++: Understanding the Basics - A Graphics Guy's Notes](https://agraphicsguynotes.com/posts/fiber_in_cpp_understanding_the_basics/)
- [Fibers, Oh My! - Dale Weiler](https://graphitemaster.github.io/fibers/)
- [Correctly implementing a spinlock in C++ - Erik Rigtorp](https://rigtorp.se/spinlock/)
