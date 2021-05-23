#include "parser.h"

string int_to_fb(const int n){
    string result = "";
    result += (unsigned char) ((n>>24) & 0xFF);
    result += (unsigned char)((n >> 16) & 0xFF);
    result += (unsigned char)((n >> 8) & 0xFF);
    result += (unsigned char)(n & 0xFF);
    return result;
}

int control_sum(const string s){
    //TODO
    return 0;
}