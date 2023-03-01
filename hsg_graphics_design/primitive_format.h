#pragma once
#include <vector>
namespace auto_future {
	using ub =unsigned char;
	using dub=unsigned short;
    enum enum_usage{
        gl_stream_draw,
        gl_stream_read,
        gl_stream_copy,
        gl_static_draw,
        gl_static_read,
        gl_static_copy,
        gl_dynamic_draw,
        gl_dynamic_read,
        gl_dynamic_copy,
        gl_usage_cnt
    };
	struct primitive_head {
          dub demension : 4;// bigger than 0 less than 7 
          dub format_index : 12;
          dub size;
    };
    using u32_t=unsigned int;
    struct primitive_file_head {
          dub demension : 4;// bigger than 0 less than 7 
          dub format_index : 12;
          dub tpye:1;//0 interlaced,1 progressive
          dub usage:4;//enum_usage
          dub reserverd:11;
          u32_t size;
    };
	using pm_format=std::vector<ub>;
    void get_pm_format(dub index,pm_format& pmf );//pmf.size()>1
    dub get_pm_format_index(pm_format& pmf);
}