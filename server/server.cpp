#include "misc_structs.h"
#include <iostream>
#include "err.h"

using namespace std;
int main()
{
    game g(1,2,5); 
    cout << g.NEW_GAME_msg().to_string() << endl;
    cout << g.get_height() << endl;
    event_msg msg;
    cout << msg.len << endl;
    //syserr("string(g.cos)");
}