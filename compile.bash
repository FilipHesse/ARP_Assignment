mkdir -p Release

g++ -o Release/processP -std=c++11 src/processP.cpp
g++ -o Release/processS -std=c++11 src/processS.cpp
g++ -o Release/processL -std=c++11 src/processL.cpp
g++ -o Release/processG -std=c++11 src/processG.cpp
g++ -o Release/multi_process -std=c++11 src/multi_process.cpp
