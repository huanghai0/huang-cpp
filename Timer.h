#include <iostream>
#include <chrono>

class Timer
{
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {};
    ~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Elapsed time: " << duration.count() << " ms" << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point start;
};