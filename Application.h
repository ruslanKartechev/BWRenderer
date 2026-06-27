#ifndef RENDERGL_APPLICATION_H
#define RENDERGL_APPLICATION_H
#include <string>
#include <cstdint>

class Application {

public:
    static std::string RootPath;
    static std::string ResourcesPath;
    static double runningTime;
    static uint64_t frameCount;

};


#endif //RENDERGL_APPLICATION_H
