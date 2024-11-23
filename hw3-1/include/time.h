#include <iostream>
#include <ctime>
#include "defs.h"

class timer
{
public:
    void set_duration(int time) { __time = time; };
    bool starttimer();

private:
    int __time = TIMEOUT;
};