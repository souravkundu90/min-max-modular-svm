#include "util.h"
static const basic_string<char>::size_type npos = -1;
vector<string> split(const string& src, string delimit,
		string null_subst)
{
	if( src.empty() || delimit.empty() ) throw "split:empty string\0";

	vector<string> v;
	basic_string<char>::size_type deli_len = delimit.size();
	long index = npos, last_search_position = 0;
	while( (index=src.find(delimit,last_search_position))!=npos )
	{
		 if(index==last_search_position)
			   v.push_back(null_subst);
		  else
			    v.push_back( src.substr(last_search_position, 
							index-last_search_position) );
		   last_search_position = index + deli_len;
	}
	string last_one = src.substr(last_search_position);
	v.push_back( last_one.empty()? null_subst:last_one );
	return v;
}