#include <iostream>
#include <windows.h>
using namespace std;

int main()
{
    SetConsoleOutputCP ( CP_UTF8 ) ;
    SOCKET test = socket(AF_INET, SOCK_STREAM, 0);
    cout << "你好";
    system("pause");
    return 0;
}