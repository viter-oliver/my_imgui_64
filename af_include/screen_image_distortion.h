#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "af_shader.h"
#include "res_output.h"
#include "af_primitive_object.h"
namespace auto_future
{
	class screen_image_distortion
	{
		//GLuint g_VboHandle;
		//GLuint g_VaoHandle;
		GLuint _fboId;
		GLint _last_fbo;
		GLint _prev_fbo;
		GLuint _rbo;
		GLuint _colorTextId;
		GLuint _distortionTextId;
		GLuint _depthStencilTextId;
		float _img_sz[4];
		GLuint _txtDistortion;
		ps_shader _pshader;
		ps_primrive_object _ps_prm;
		ps_af_texture _pat_image;
	public:
		screen_image_distortion(float win_width, float win_height, float screen_width, float screen_height, string path);
		~screen_image_distortion();
		void get_win_size(float &width, float &height) {
			width = _img_sz[0];
			height = _img_sz[1];
		}
		void get_screen_size(float &width, float &height)
		{
			width = _img_sz[2];
			height = _img_sz[3];
		}
		void bind_framebuffer();
		void draw();
		void disbind_framebuffer();
		void bind_texture();
		void create_test_dic(float width, float height, string path);
		void ScreenShot();
	};
}