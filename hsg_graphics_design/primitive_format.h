#pragma once
#include <vector>
namespace auto_future {
	using ub =unsigned char;
	using dub=unsigned short;
    enum enum_usage{
        gl_stream_draw,//GL_STREAM_DRAW
        gl_stream_read,//GL_STREAM_READ
        gl_stream_copy,//GL_STREAM_COPY
        gl_static_draw,//GL_STATIC_DRAW
        gl_static_read,//GL_STATIC_READ
        gl_static_copy,//GL_STATIC_COPY
        gl_dynamic_draw,//GL_DYNAMIC_DRAW
        gl_dynamic_read,//GL_DYNAMIC_READ
        gl_dynamic_copy,//GL_DYNAMIC_COPY
        gl_usage_cnt
    };
    enum enum_element_type{
        gl_no_element,//0
        gl_unsigned_byte,//GL_UNSIGNED_BYTE
        gl_unsigned_short,//GL_UNSIGNED_SHORT
        gl_unsigned_int,//GL_UNSIGNED_INT
        gl_element_type_cnt
    };
	struct primitive_head {//will be deprecated in next version
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
          dub element_type:2;//enum_element_type
          dub reserverd:9;
          u32_t size;
    };
    constexpr auto fsz = sizeof primitive_file_head;
	using pm_format=std::vector<ub>;
    void get_pm_format(dub index,pm_format& pmf );//pmf.size()>1
    dub get_pm_format_index(pm_format& pmf);
}