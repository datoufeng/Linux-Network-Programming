#include "Reactor.h"

using namespace std;

int main()
{
	EpollReactor er(128);
	er.listen(8080);

	er.Run();


	return 0;
}