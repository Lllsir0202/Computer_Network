#include <iostream>
#include <ctime>

class timer
{
public:
    void set_duration(int time) { __time = time; };
    bool starttimer();

private:
    int __time = 0;
};