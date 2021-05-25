#include <string>
using std::string;

string int_to_fb(const uint32_t n);
string long_to_eb(const uint64_t n);
uint32_t fb_to_int(const string n);
uint64_t eb_to_long(const string n);
uint32_t control_sum(const string s);