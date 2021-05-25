#include "misc_structs.h"
#include <iostream>
#include "err.h"
#include "parser.h"

using namespace std;
int main()
{
    cout << fb_to_int(int_to_fb(15));
    server s(640, 640, 360, 2, 24444);
    s.start();
}