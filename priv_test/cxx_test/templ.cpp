#include <iostream>

using namespace std;
template<class T> 
T fun(T a, T b)
{
	return (a > b )? a : b;
}

//定义主函数
int main()
{
	cout << fun(3, 2) << endl;
	cout << fun(2.3, 4.6) << endl;
	return 0;
}



