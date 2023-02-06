#include "ft_distortion.h"
#include "ft_trans.h"
#include <cmath>
#include "inverse-matrix.hpp"
namespace auto_future {
ps_shader ft_distortion::_pdis_sd = nullptr;
ps_primrive_object ft_distortion::_ps_prm = nullptr;
		  
const char *dis_vs_code = R"glsl(#version 300 es
	    precision mediump float;
	    layout(location=0) in vec2 position;
        layout(location=1) in vec2 textCoord;
        uniform mat2 customMtx;
        uniform vec2 customDelta;
        out vec2 outTxtCd;
        void main()
        {
             outTxtCd=textCoord;
             gl_Position=vec4(position.xy,0.0,1.0);
        }
		)glsl";
const char *dis_fs_code = R"glsl(#version 300 es
	    precision mediump float;
        in vec2 outTxtCd;
        uniform sampler2D text;
	    out vec4 o_clr;

        uniform float px[15];
        uniform float py[15];
        uniform int be_valid;
        void main()
        {
          vec2 tmpPos= outTxtCd;//vec2(1.f-outTxtCd.x,outTxtCd.y);
          vec2 Texcoord;
          if(be_valid<1){
             Texcoord=tmpPos;
          } else {
          Texcoord.x = px[0]
          +px[1] * tmpPos.x
          +px[2] * tmpPos.x  * tmpPos.x 
          +px[3] * tmpPos.x  * tmpPos.x  * tmpPos.x 
          +px[4] * tmpPos.x  * tmpPos.x  * tmpPos.x  * tmpPos.x 
          +px[5] * tmpPos.y 
          +px[6] * tmpPos.x  * tmpPos.y 
          +px[7] * tmpPos.x  * tmpPos.x  * tmpPos.y
          +px[8] * tmpPos.x  * tmpPos.x  * tmpPos.x  * tmpPos.y 
          +px[9] * tmpPos.y  * tmpPos.y 
          +px[10] * tmpPos.x  * tmpPos.y  * tmpPos.y 
          +px[11]  * tmpPos.x  * tmpPos.x  * tmpPos.y  * tmpPos.y 
          +px[12] * tmpPos.y  * tmpPos.y  * tmpPos.y 
          +px[13] * tmpPos.x  * tmpPos.y  * tmpPos.y  * tmpPos.y 
          +px[14] * tmpPos.y  * tmpPos.y  * tmpPos.y  * tmpPos.y ;
          Texcoord.y = py[0]
          +py[1] * tmpPos.x
          +py[2] * tmpPos.x  * tmpPos.x 
          +py[3] * tmpPos.x  * tmpPos.x  * tmpPos.x 
          +py[4] * tmpPos.x  * tmpPos.x  * tmpPos.x  * tmpPos.x 
          +py[5] * tmpPos.y 
          +py[6] * tmpPos.x  * tmpPos.y 
          +py[7] * tmpPos.x  * tmpPos.x  * tmpPos.y
          +py[8] * tmpPos.x  * tmpPos.x  * tmpPos.x  * tmpPos.y 
          +py[9] * tmpPos.y  * tmpPos.y 
          +py[10] * tmpPos.x  * tmpPos.y  * tmpPos.y 
          +py[11] * tmpPos.x  * tmpPos.x  * tmpPos.y  * tmpPos.y 
          +py[12] * tmpPos.y  * tmpPos.y  * tmpPos.y 
          +py[13] * tmpPos.x  * tmpPos.y  * tmpPos.y  * tmpPos.y 
          +py[14] * tmpPos.y  * tmpPos.y  * tmpPos.y  * tmpPos.y ;
          }
          o_clr = texture(text, Texcoord);
        }
		)glsl";
GLfloat _rect_vertices[] = {
    -1.f, -1.f,0.f,1.f,
    1.f, -1.f,1.f,1.f,
    -1.f, 1.f,0.f,0.f,
    1.f, 1.0f,1.f,0.f,
};
void init_pts(float(*ipt)[2], float(*opt)[2]) {
    float w = 1200, h = 800; 
    float pt_out[15][2] = {
        200,200,400,200,600,200,800,200,1000,200,
        200,400,400,400,600,400,800,400,1000,400,
        200,600,400,600,600,600,800,600,1000,600
    };
    float pt_in[15][2] = {
        200,199,400,196,601,212,801,196,1004,196,
        198,409,392,398,597,396,799,407,1005,393,
        203,603,399,602,599,601,798,608,997,604,
   };
    for (int i = 0; i < 15; i++) {
        opt[i][0] =  pt_out[i][0] / w;
        opt[i][1] =  pt_out[i][1] / h;
    }
    /*for (int i = 0; i < 15; i++) {
        pt_in[i][1] = h - pt_in[i][1];
    }*/
    for (int i = 0; i < 15; i++) {
        ipt[i][0]= pt_in[i][0] / w;
        ipt[i][1]= pt_in[i][1] / h;
    }
}
void calculate_params(float(*ipt)[2], float(*opt)[2],
    std::vector<float>&px, std::vector<float>& py) {
    using efmatrix = extendable_matrix::Matrix<float>;
    efmatrix matrix_cal(15), matrix_inverse(15);
    for (int ix = 0; ix < 15; ++ix) {
        matrix_cal[ix] = {
        1,
        ipt[ix][0],
        powf(ipt[ix][0],2),
        powf(ipt[ix][0],3),
        powf(ipt[ix][0],4),
        ipt[ix][1],
        ipt[ix][0] * ipt[ix][1],
        powf(ipt[ix][0],2) * ipt[ix][1],
        powf(ipt[ix][0],3) * ipt[ix][1],
        powf(ipt[ix][1],2),
        ipt[ix][0] * powf(ipt[ix][1],2),
        powf(ipt[ix][0],2) * powf(ipt[ix][1],2),
        powf(ipt[ix][1],3),
        ipt[ix][0] * powf(ipt[ix][1],3),
        powf(ipt[ix][1],4),
        };
    }
    matrix_cal.Inverse(matrix_inverse);
    std::vector<float> xvout = {
      opt[0][0],opt[1][0],opt[2][0],opt[3][0],opt[4][0],
      opt[5][0],opt[6][0],opt[7][0],opt[8][0],opt[9][0],
      opt[10][0],opt[11][0],opt[12][0],opt[13][0],opt[14][0],
    };
    matrix_inverse.multiply(xvout, px);
    std::vector<float> yvout = {
      opt[0][1],opt[1][1],opt[2][1],opt[3][1],opt[4][1],
      opt[5][1],opt[6][1],opt[7][1],opt[8][1],opt[9][1],
      opt[10][1],opt[11][1],opt[12][1],opt[13][1],opt[14][1],
    };
    matrix_inverse.multiply(yvout, py);
}

void calculate_params2(float(*ipt)[2], float(*opt)[2],
    std::vector<float>& px, std::vector<float>& py) {
    vector<vector<float>> matrix_cal;
    auto dm = px.size();
    matrix_cal.resize(dm, vector<float>(2 * dm, 0));
    auto chk0 = [](float fv) {
        if (fv == -0 ||
            (fv > -0.00000001) && (fv < 0.00000001)) {
           fv = 0;
        }
        return fv;
    };
    for (int ix = 0; ix < dm; ++ix) {
         
        matrix_cal[ix][0] = 1;
        matrix_cal[ix][1] = chk0(ipt[ix][0]);
        matrix_cal[ix][2] = chk0(powf(ipt[ix][0],2));
        matrix_cal[ix][3] = chk0(powf(ipt[ix][0],3));
        matrix_cal[ix][4] = chk0(powf(ipt[ix][0],4));
        matrix_cal[ix][5] = chk0(ipt[ix][1]);
        matrix_cal[ix][6] = chk0(ipt[ix][0] * ipt[ix][1]);
        matrix_cal[ix][7] = chk0(powf(ipt[ix][0],2) * ipt[ix][1]);
        matrix_cal[ix][8] = chk0(powf(ipt[ix][0],3) * ipt[ix][1]);
        matrix_cal[ix][9] = chk0(powf(ipt[ix][1],2));
        matrix_cal[ix][10] = chk0(ipt[ix][0] * powf(ipt[ix][1],2));
        matrix_cal[ix][11] = chk0(powf(ipt[ix][0],2) * powf(ipt[ix][1],2));
        matrix_cal[ix][12] = chk0(powf(ipt[ix][1],3));
        matrix_cal[ix][13] = chk0(ipt[ix][0] * powf(ipt[ix][1],3));
        matrix_cal[ix][14] = chk0(powf(ipt[ix][1],4));

        
        for (int j = dm; j < 2 * dm; j++) {
            if ((ix + dm) == j)
                matrix_cal[ix][j] = 1;
            else
                matrix_cal[ix][j] = 0;
        }
    }
    int zero = 0;
    for (int j = 0; j < dm; j++) {
        zero = 0;
        for (int i = 0; i < dm; i++) {
            if (matrix_cal[i][j] == 0) {
                ++zero;
                if (zero == dm) {
                    cout << "\nThis Matrix Can Not Be Inversed , ";
                    cout << "Because The row number (" << j + 1
                        << ") Is Full Of Zeros ";
                    return;
                }
            }
        }
    }
    for (int j = 0; j < dm; j++) {
        zero = 0;
        for (int i = 0; i < dm; i++) {
            if (matrix_cal[j][i] == 0) {
                ++zero;
                if (zero == dm) {
                    cout << "\nThis Matrix Can Not Be Inversed , ";
                    cout << "Because The Column Number (" << j + 1
                        << ") Is Full Of Zeros ";
                    return;
                }
            }
        }
    }
    auto chk_0 = [](float& fv) {
        if (fv == -0 ||
            (fv > -0.00000001) && (fv < 0.00000001)) {
            fv = 0;
        }
    };
    float a;
    for (int ii = 0; ii < dm - 1; ii++) // main code, (ii) is the looping item
    {
        // in case the matrix_cal[ii][ii]=0 , then we'll search for an elemnt !=0 from
         // the same column , then we'll add that row to the first row          
        if (matrix_cal[ii][ii] == 0)
        {
            for (int i = (ii + 1); i < dm; i++) {
                if (matrix_cal[i][ii] != 0) {
                    for (int j = 0; j < 2 * dm; j++) {
                        matrix_cal[ii][j] = matrix_cal[ii][j] + matrix_cal[i][j];
                        chk_0(matrix_cal[ii][j]);
                    }
                    break;
                }
            }
            if (matrix_cal[ii][ii] == 0) // This situation is for two columns with the same numbers , or proportion
            {
                cout << "\n\nThis matrix can not be inversed , ";
                cout << "Because the determinant = 0 ";
                return;
            }
        }

        if (matrix_cal[ii][ii] != 1) {
            a = matrix_cal[ii][ii];
            for (int j = 0; j < 2 * dm; j++) {

                matrix_cal[ii][j] = matrix_cal[ii][j] /
                    a; // to make matrix_cal[ii][ii] = 1 & divide all the row on matrix_cal[ii][ii]
                chk_0(matrix_cal[ii][j]);
            }
        }

        for (int i = ii + 1; i < dm;
            i++) // to make every element under matrix_cal[ii][ii] equal to zero
        {
            a = matrix_cal[i][ii];
            for (int j = ii; j < 2 * dm; j++) {
                matrix_cal[i][j] = -a * matrix_cal[ii][j] + matrix_cal[i][j];
                chk_0(matrix_cal[i][j]);
            }
        }

    }
    if (matrix_cal[dm - 1][dm - 1] == 0) {
        cout << "\n\nThis matrix can not be inversed , ";
        cout << "because there is a row full of zeros ( determinant = 0 )";
        return;
    }

    if (matrix_cal[dm - 1][dm - 1] != 1) {
        a = matrix_cal[dm - 1][dm - 1];

        for (int j = 0; j < 2 * dm; j++) {
            // to make matrix_cal[n-1][n-1] = 1 & divide all the row on matrix_cal[n-1][n-1]
            matrix_cal[dm - 1][j] = (matrix_cal[dm - 1][j] / a);
            chk_0(matrix_cal[dm - 1][j]);
        }
    }

    for (int ii = dm - 1; ii > 0; ii--) {
        for (int i = ii - 1; i >= 0;
            i--) // to make every element over matrix_cal[ii][ii] equal to zero
        {
            a = matrix_cal[i][ii];
            for (int j = 0; j < 2 * dm; j++) {
                matrix_cal[i][j] = -a * matrix_cal[ii][j] + matrix_cal[i][j];
                chk_0(matrix_cal[i][j]);
            }
        }
    }

    for (int i = 0; i < dm; i++) {
        for (int j = dm; j < 2 * dm; j++) {
            chk_0(matrix_cal[i][j]);
        }
    }
    auto mt_mul = [matrix_cal](std::vector<float>& vin, std::vector<float>& vout) {
        auto dm = vin.size();
        for (int ix = 0; ix < vout.size(); ix++) {
            float sm = 0;
            for (int ii = 0; ii < vin.size(); ii++) {
                sm += (vin[ii]*matrix_cal[ix][ii+dm]);
            }
            vout[ix] = sm;
        }
    };
    std::vector<float> xvout = {
      opt[0][0],opt[1][0],opt[2][0],opt[3][0],opt[4][0],
      opt[5][0],opt[6][0],opt[7][0],opt[8][0],opt[9][0],
      opt[10][0],opt[11][0],opt[12][0],opt[13][0],opt[14][0],
    };
    mt_mul(xvout, px);
    std::vector<float> yvout = {
      opt[0][1],opt[1][1],opt[2][1],opt[3][1],opt[4][1],
      opt[5][1],opt[6][1],opt[7][1],opt[8][1],opt[9][1],
      opt[10][1],opt[11][1],opt[12][1],opt[13][1],opt[14][1],
    };
    mt_mul(yvout, py);
}

ft_distortion::ft_distortion() {
  _pt._attached_txt[0] = '\0';
  if (!ft_distortion::_pdis_sd) {
    ft_distortion::_pdis_sd =
        make_shared<af_shader>(dis_vs_code, dis_fs_code);
  }
  if (!ft_distortion::_ps_prm) {
      _ps_prm = make_shared<primitive_object>();
      _ps_prm->set_ele_format({ 2,2 });
      _ps_prm->load_vertex_data(_rect_vertices,
          sizeof(_rect_vertices) / sizeof(float));
  }
  px.resize(15);
  py.resize(15);
  init_pts(input_point, output_point);
  //init_pts(output_point,input_point);
  calculate_params2(input_point, output_point, px, py);
#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

  reg_property_handle(&_pt, 0, [this](void *member_address) {
    if (_ps_txt) {
      ImGui::Text("Attached image:%s", _pt._attached_txt);
      ImGui::SameLine();
      if (ImGui::Button("Delink##txture")) {
        _ps_txt = nullptr;
      }
    } else {
      ImGui::InputText("Attached image:", _pt._attached_txt, FILE_NAME_LEN);
      if (ImGui::Button("Import##texture")) {
        auto itxt = g_mtexture_list.find(_pt._attached_txt);
        if (itxt != g_mtexture_list.end()) {
          _ps_txt = itxt->second;
        }
      }
    }
    bool recal = false;
    ImGui::Text("Input:");
    for (int ix = 0; ix < 15; ix++) {
        char str_idx[50] = { 0 };
        sprintf(str_idx, "in[%d]", ix);
        bool ch=ImGui::SliderFloat2(str_idx, input_point[ix], 0, 1.f, "%.5f", 0.00001f);
        if (!recal && ch) {
            recal = true;
        }
        
    }
    ImGui::Text("Output:");
    for (int ix = 0; ix < 15; ix++) {
        char str_idx[50] = { 0 };
        sprintf(str_idx, "out[%d]", ix);
        bool ch = ImGui::SliderFloat2(str_idx, output_point[ix], 0, 1.f, "%.5f", 0.00001f);
        if (!recal && ch) {
            recal = true;
        }
    }
    if (recal) {
        calculate_params2(input_point, output_point, px, py);
    }
    if (ImGui::Button("recal")) {
        init_pts(input_point, output_point);
        //init_pts(output_point,input_point);
        calculate_params2(input_point, output_point, px, py);
    }
    ImGui::Checkbox("cal_cul", (bool*) & be_valid);
  });
#endif
}

void ft_distortion::link() {

  auto iat = g_mtexture_list.find(_pt._attached_txt);
  if (iat != g_mtexture_list.end()) {
    _ps_txt = iat->second;
  }
}
void ft_distortion::draw() {
    if (!_ps_txt)
        return;
    _pdis_sd->use();
    _pdis_sd->uniform("px[0]", &px[0]);
    _pdis_sd->uniform("py[0]", &py[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _ps_txt->_atxt_id);
    _pdis_sd->uniform("text", 0);
    _pdis_sd->uniform("be_valid", (int*) & be_valid);
    glBindVertexArray(_ps_prm->_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
} // namespace auto_future