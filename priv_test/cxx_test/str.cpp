#include <iostream>
using namespace std;
int main(int argc ,char *argv[])
{
	std::string str = "uid" ;
	std::string out = "out" + str;
	cout << "out = " << out << endl;

	return 0;
}
