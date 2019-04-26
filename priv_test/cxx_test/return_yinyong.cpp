#include <iostream>
int &get(int * arry, int index) {return arry[index]; }

using namespace std;
int main()
{
	int ia[10];
	for( int i = 0; i != 10; ++i){
		get(ia, i) = i; 
		cout << "ia[" << i << "]="  << ia[i] << endl;
	}
	return 0;
}

