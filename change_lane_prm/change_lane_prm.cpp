#include <fstream>
#include <iostream>
#include "af_type.h"
#include "primitive_format.h"

using namespace auto_future;
using namespace std;
struct prm_unit {
  af_vec3 pos;
  af_vec3 normal;
  af_vec2 txt;
};
int main(int argc,char** argv){
  float xmin = -1.5f, xmax = 1.8f;
  float w = 0.05;
  auto fun = [](float x) { return x * x * x; };
  auto next = [] { 
	static float x0 = 4.5;
    x0 += 0.5;
    return 1/x0;
  };
  vector<prm_unit> v_prm;
  float x0=0.0f;
  auto ng = [](float& x) { x = -x; };
  while (x0 < xmax) {
    x0 += next();
    auto z = x0 * x0 * x0;
    prm_unit u0 = {{x0-w, 0, z}, {0, 1, 0}, {0,0}};
    prm_unit u1 = {{x0+w, 0, z}, {0, 1, 0}, {0,0}};

    v_prm.push_back(u0);
    v_prm.push_back(u1);
    ng(u0.pos.x);
    ng(u0.pos.z);
    ng(u1.pos.x);
    ng(u1.pos.z);
    v_prm.insert(v_prm.begin(), u1);
    v_prm.insert(v_prm.begin(), u0);
  }
  //calculate  coordination of texture
  auto cnt = v_prm.size();
  auto zmin = v_prm[0].pos.z;
  auto zspan = v_prm[cnt - 1].pos.z - zmin;
  auto pr_cnt = cnt / 2;
  for (int ix = 0; ix < pr_cnt; ix++) {
    auto id = ix * 2;
    auto& pl = v_prm[id];
    pl.txt.x = 0;
    auto zd = pl.pos.z - zmin;
    pl.txt.y = zd / zspan;
    auto& pr = v_prm[id+1];
    pr.txt.x = 1;
    pr.txt.y = pl.txt.y;
  }
  ofstream _fout("change_lane.prm",ios::binary);
  pm_format pf_cl = {3, 3, 2};
  primitive_head phead = {3, get_pm_format_index(pf_cl), cnt};
  _fout.write((char*)&phead, sizeof(primitive_head));
  _fout.write((char*)&v_prm[0], cnt * sizeof(prm_unit));
  return 0;
}