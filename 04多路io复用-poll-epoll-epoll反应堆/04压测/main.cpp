#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	if (1 > argc)
	{
		cout << "please input N.." << endl;
		return 1;
	}
	pid_t pid;
	for (int i = 0; i < atoi(argv[1]); i++)
	{
		pid = vfork();
		if (0 == pid) break;
	}

	switch (pid)
	{
	case 0:
		execlp("nc", "nc", "192.168.233.233", "8080", NULL);
		cout << "execl is failed.." << endl;
		break;
	default:
		pause();
		break;
	}


	return 0;
}