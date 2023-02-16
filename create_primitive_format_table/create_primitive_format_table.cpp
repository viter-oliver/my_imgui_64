#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>

using namespace std;
using byte=unsigned char;
using vbyte=vector<byte>;
int main(int argc,char** argv){
  const int demesion_min = 1;
  const int demesion_max = 6;
  const int sub_dem_min = 1;
  const int sub_dem_max = 4;
  const char n_str[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8','9'};
  ofstream _fout("primitive_format_table.h");
  function<void(vbyte&)> pt_vb = [&](vbyte& vb) {
    for (int ix = 0; ix < vb.size(); ++ix) {
      _fout << n_str[vb[ix]] << ",";
    }
    _fout << " ";
  };

  _fout << "unsigned char ptm_table[]={" << endl;
  for (int dm = demesion_min; dm <= demesion_max; dm++) {
    vbyte v_sm(dm);
    function<void(vbyte&,int dm)> creat_vb = [&](vbyte& vb,int d) { 
        if (d== dm) {
            pt_vb(vb);
        } else {
          
          for (int ix = sub_dem_min; ix <= sub_dem_max; ix++) {
            vb[d] = ix;
            creat_vb(vb,d+1);
          }
        }
    };
    creat_vb(v_sm,0);
    _fout << endl;
  }
  _fout << "};" << endl;
  _fout << "int pm_sz = sizeof ptm_table;" << endl;
  return 0;
}