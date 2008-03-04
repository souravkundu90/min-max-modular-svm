#ifndef _DIVIDER_CPP
#define _DIVIDER_CPP

#include "divider.h"
#include <fstream>
using namespace std;

void Divider::parse(char* filename)
{
	try
	{
		ifstream inf(filename);
		char in,iin;
		while(inf>>in)
		{
			switch(in)
			{
				case '-':
				{
					inf>>iin;
					switch(iin)
					{
						case 'o':
						{
							inf>>precent;
							break;
						}
					}
					break;
				}
	
			}
		}
		inf.close();
	}
	catch(...)
	{
		precent = 0;
		return;
	}
}
#endif
