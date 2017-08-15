#include "gba.h"
#include "petitlib.h"

int main()
{
	int count = 123;
	char str[256];
	init();
	print("petit lib\n");
	print("---------\n");
	print("now booting...\n");
	
	while(1)
	{	
		linput(str);
		print("> %s", str);
	}

	finish();
}