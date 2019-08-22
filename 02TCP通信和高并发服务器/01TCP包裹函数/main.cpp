#include "wrap.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
	char buf[100] = { 0 };

	int n = Readline(STDIN_FILENO, buf, sizeof(buf));

	cout << n << "[" << buf << "]" << endl;

	return 0;
}