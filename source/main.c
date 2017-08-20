#include <string.h>
#include "gba.h"
#include "petitlib.h"

int main()
{
	char str[256];
	int h_count = 0;
	init();
	print("petit lib\n");
	print("---------\n");
	print("now booting...\n");
	
	while(1)
	{	
		linput(str);
		print("> %s\n", str);
		if (strcmp(str, "hage") == 0) {
			h_count++;
			for (int i = 0; i < h_count; i++)
				print("HAGE! ");
			print("\n");
		}
	}

	finish();
}