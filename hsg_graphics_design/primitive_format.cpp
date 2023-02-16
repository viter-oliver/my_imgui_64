#include "primitive_format.h"
namespace auto_future {
#include "primitive_format_table.h"
const dub ptm_indx_tab[] = {
	4,
	32,
	192,
	1024,
	5120,
	24576,
};
void get_pm_format(dub index, pm_format& pmf) { 
  auto demension = pmf.size();
  if (demension < 1) return;
  ub* phead = ptm_table;
  auto span_index = demension - 1;
  dub span = 0;
  for (int ix = 0; ix < span_index; ix++) {
    span += ptm_indx_tab[ix];
  }
  phead += span;
  auto cur_span = index * demension;
  phead += cur_span;
  for (auto& pm : pmf) {
    pm = *phead++;
  }
}
dub get_pm_format_index(pm_format& pmf) {
  auto ix = pmf.size() - 1;
  dub index = 0;
  auto pow_v = [](ub b, ub cnt) {
    dub vb = 1;
    for (dub i = 0; i < cnt; i++) {
      vb *= b;
    }
    return vb;
  };
  for (dub i = 0; i < pmf.size(); ix--, i++) {
    auto pu = pmf[ix] - 1;
    auto pv = pu * pow_v(4, i);
    index += pv;
  }
  return index;
}
}