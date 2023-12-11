#include "screen_image_distortion.h"
#include "common_functions.h"
#include "res_output.h"
#include "SOIL.h"
#include <fstream>
namespace auto_future
{
    const char *vs_code = R"glsl(#version 320 es
    precision highp float;
    layout (location = 0) in vec2 position;
    layout (location = 1) in vec2 txtCoor;
    smooth out vec2 outTxtCd;
    void main()
    {
        outTxtCd = txtCoor;
        gl_Position=vec4(position.x, position.y,0.0,1.0);
    }
)glsl";
#if 0
const char *fs_code = R"glsl(#version 320 es
precision highp float;
in vec2 outTxtCd;
//in vec2 xxTxtCd;
out vec4 o_clr;
uniform vec2 dt;
uniform sampler2D text;
uniform sampler2D txtDistortion;
void main()
{
    vec2 txtMap = outTxtCd;
    if(dt.x>0.0 && dt.y>0.0){
        vec4 uvraw = texture(txtDistortion, outTxtCd);
        //float cx = uvraw.y  / 256.0  +  uvraw.x;
        //float cy = uvraw.w  / 256.0  + uvraw.z;
        //txtMap=vec2(cx, cy);
        vec2 coor = vec2(uvraw.y * 65536.0 +  uvraw.x * 256.0, uvraw.w * 65536.0 + uvraw.z * 256.0);
        txtMap = coor / dt;
        txtMap.y = 1.0 - txtMap.y;
    }
    if(txtMap.x > 0.0 || txtMap.y > 0.0){
        o_clr = texture(text, txtMap);
    }else{
        o_clr = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
)glsl";
#else
    const char *fs_code = R"glsl(#version 320 es
    precision highp float;
    precision highp usampler2D;
    in vec2 outTxtCd;
    out vec4 o_clr;
    uniform vec2 dt;
    uniform float scale_y;
    uniform sampler2D text;
    uniform usampler2D txtDistortion;
    void main()
    {
    vec2 txtMap = outTxtCd;
    if(dt.x>0.0 && dt.y>0.0) {
        uvec4 uvraw = texture(txtDistortion, txtMap);
        vec2 coor = vec2(uvraw.yw * 256u + uvraw.xz);
        txtMap = coor / (dt + 1.0);
        txtMap.y = 1.0 - txtMap.y * scale_y;
    }
    if(txtMap.x > 0.0 || txtMap.y > 0.0) {
        o_clr = texture(text, txtMap);
    }else{
        o_clr = vec4(0.0, 0.0, 1.0, 1.0);
    }
    }
)glsl";
#endif

    static GLfloat _plain_vertices[] = {
    -1.f,-1.f, 0.f,1.f,
    1.f,-1.f, 1.f,1.f,
    -1.f,1.f, 0.f,0.f,
    1.f,1.f, 1.f,0.f
    };
    const auto sz_ff = sizeof(_plain_vertices);
    const char *dic_res_name = "distorted_dic.png";
    screen_image_distortion::screen_image_distortion(float win_width,
        float win_height,
        float screen_width,
        float screen_height,
        string path) {
        _img_sz[0] = win_width;
        _img_sz[1] = win_height;
        _img_sz[2] = screen_width;
        _img_sz[3] = screen_height;
        _pshader = make_shared<af_shader>(vs_code, fs_code);
        _pshader->use();
        _ps_prm = make_shared<primitive_object>();
        _ps_prm->set_ele_format({ 2,2 });
        _ps_prm->load_vertex_data(_plain_vertices, sizeof(_plain_vertices) / sizeof(float));
        prepareFBO1(_colorTextId, _depthStencilTextId, _fboId, _img_sz[2], _img_sz[3]);
        auto iat = g_mtexture_list.find(dic_res_name);
        if (iat != g_mtexture_list.end()) {
            _pat_image = iat->second;
        }
        create_test_dic(_img_sz[0], _img_sz[1], path);
    }

    screen_image_distortion::~screen_image_distortion()
    {
        //glDeleteBuffers(1, &g_VboHandle);
        //glDeleteVertexArrays(1, &g_VaoHandle);
        glDeleteTextures(1, &_colorTextId);
        glDeleteTextures(1, &_depthStencilTextId);
        glDeleteFramebuffers(1, &_fboId);
    }
    void screen_image_distortion::bind_framebuffer()
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_prev_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fboId);

        //glViewport(0, 0, _width, _height);
        //glClearColor(1, 1,1, 1);
        //glClear(GL_COLOR_BUFFER_BIT);
    }
    void screen_image_distortion::draw()
    {
        if (!_pat_image) {
            return;
        }
        GLint win_x = (_img_sz[2] - _img_sz[0]) / 2.0;
        GLint win_y = (_img_sz[3] - _img_sz[1]) / 2.0;
        GLfloat scale_y = _img_sz[1] / _img_sz[3];
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glViewport(win_x, win_y, _img_sz[0], _img_sz[1]);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _pshader->use();
        bind_texture();
        _pshader->uniform("dt", _img_sz);
        _pshader->uniform("scale_y", &scale_y);
        glBindVertexArray(_ps_prm->_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    void screen_image_distortion::disbind_framebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _prev_fbo);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            GLenum err_code = glGetError();
            printf("%s::%d glerror:%d\n", __FUNCTION__, __LINE__, err_code);
            printf("at distortion !\n");
        }
    }
    void screen_image_distortion::bind_texture() {
        glActiveTexture(GL_TEXTURE0);
        GLuint txt_idx = 0;
        txt_idx = _colorTextId;
        glBindTexture(GL_TEXTURE_2D, txt_idx);
        _pshader->uniform("text", 0);
        glActiveTexture(GL_TEXTURE1);
        txt_idx = _distortionTextId;
        glBindTexture(GL_TEXTURE_2D, txt_idx);
        _pshader->uniform("txtDistortion", 1);
    }
    void screen_image_distortion::create_test_dic(float width, float height, string path) {
        ifstream if_dic_pos;
        if_dic_pos.open(path, std::ios::binary);

        GLuint tpos;
        GLuint sz = (GLuint)(width * height * 4);
        //GLubyte *data;
        //data = new GLubyte[sz * 4];
        GLubyte *i_data = new GLubyte[sz];
        GLuint *o_data = new GLuint[sz / 4];

        GLuint idx = 0;
        if (if_dic_pos.is_open()) {
            if_dic_pos.read((char *)i_data, sz);
        }

        //if (of_dic_pos.is_open()) {
        //    for (GLushort row = 0; row < (GLushort)height; row++) {
        //        for (GLushort col = 0; col < (GLushort)width; col++) {
        //            tpos = col;
        //            tpos = tpos << 2 * 8;
        //            tpos &= 0xFFFF0000;
        //            tpos |= row;
        //            o_data[idx] = tpos;
        //            of_dic_pos.write((const char *)&tpos, sizeof(GLuint));
        //            //idx = idx + 4;
        //            idx++;
        //        }
        //    }
        //}
        if_dic_pos.close();
        //of_dic_pos.close();

        glGenTextures(1, &_distortionTextId);
        glBindTexture(GL_TEXTURE_2D, _distortionTextId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //               target    level  internalFormat          border  format            type              data
        glTexImage2D(GL_TEXTURE_2D, 0,    GL_RGBA8UI, width, height, 0,  GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, i_data);
        glBindTexture(GL_TEXTURE_2D, 0);
        delete[] i_data;
        delete[] o_data;
    }
    void screen_image_distortion::ScreenShot()
    {

        int WindowSizeX = _img_sz[2], WindowSizeY = _img_sz[3];
        int i, j, save_result;
        GLubyte *ColorBuffer = new GLubyte[WindowSizeX * WindowSizeY * 3];
        glReadPixels(0, 0, WindowSizeX, WindowSizeY, GL_RGB, GL_UNSIGNED_BYTE, ColorBuffer);
        /*	invert the image	*/
        for (j = 0; j * 2 < WindowSizeY; ++j) {
            int index1 = j * WindowSizeX * 3;
            int index2 = (WindowSizeY - 1 - j) * WindowSizeX * 3;
            for (i = WindowSizeX * 3; i > 0; --i) {
                unsigned char temp = ColorBuffer[index1];
                ColorBuffer[index1] = ColorBuffer[index2];
                ColorBuffer[index2] = temp;
                ++index1;
                ++index2;
            }
        }
        save_result = SOIL_save_image("out_image.bmp", SOIL_SAVE_TYPE_BMP, WindowSizeX, WindowSizeY, 3, ColorBuffer);
        /*  And free the memory	*/
        SOIL_free_image_data(ColorBuffer);
    }

}
