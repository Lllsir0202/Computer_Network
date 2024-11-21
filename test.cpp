#include <iostream>
#include <windows.h>
using namespace std;

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SOCKET test = socket(AF_INET, SOCK_STREAM, 0);
    cout << "你好";
    int *a = new int[2];
    a[0] = 1;
    a[1] = 2;
    int *b = new int[4];
    memcpy(b, a, 2 * sizeof(int));
    cout << b[0] << b[1] << endl;
    delete[] b;
    cout << a[0] << a[1] << endl;
    delete[] a;
    system("pause");
    return 0;
}