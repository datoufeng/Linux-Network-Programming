#include "Reactor.h"

using namespace std;

int main()
{
	EpollReactor er(128);
	er.listen(8080);

	er.run();

	return 0;
}