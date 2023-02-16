#pragma once
#include <vector>
namespace auto_future {
	using ub =unsigned char;
	using dub=unsigned short;
	struct primitive_head {
          dub demension : 4;// bigger than 0 less than 7 
          dub format_index : 12;
          dub size;
    };
	using pm_format=std::vector<ub>;
    void get_pm_format(dub index,pm_format& pmf );//pmf.size()>1
    dub get_pm_format_index(pm_format& pmf);
}