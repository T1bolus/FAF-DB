#pragma once

//Fast Help Functions

inline size_t findDelim(const string &input)
{
	for (size_t i = 2; i < input.size(); i++)
	{
		if (input[i] == ';')
			return i;
	}
	
	return string::npos;
}
