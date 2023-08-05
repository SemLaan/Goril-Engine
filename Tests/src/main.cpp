#include <goril.h>

int main()
{
	gr::Timer timer = gr::Timer();

	GRTRACE("time: {}", timer.SecondsSinceStart());

	GRERROR("test");

	GRTRACE("time: {}", timer.SecondsSinceStart());


	return 0;
}