#include <iostream>
#include <event.h>

using namespace std;

int main()
{
	event_base* eb = event_base_new();
	
	const char** p = event_get_supported_methods();

	cout << "all supported methods: ";
	for  (int i = 0;  NULL != p[i]; i++)
	{
		cout << p[i] << " ";
	}
	cout << endl;
	free(p);

	cout << "system used methods: " << event_base_get_method(eb) << endl;

	event_base_free(eb);

	return 0;
}
