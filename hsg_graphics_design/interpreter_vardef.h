#pragma once
#include <string>
#include <vector>
struct  var_unit
{
	std::string _type;
	void* _value_addr;
	int _cnt{0};
	var_unit(std::string& tp, void* vaddr,int cnt)
		:_type(tp), _value_addr(vaddr),_cnt(cnt)
	{}
};
using variable_list = std::vector<var_unit>;