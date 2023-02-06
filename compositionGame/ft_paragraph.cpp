#include "ft_paragraph.h"
#include "ft_sentence.h"

namespace auto_future
{
	ft_paragraph::ft_paragraph()
	{

		_bdclr = { 1., 1., 1., 1. };
		
	}
	bool ft_paragraph::orignal_order() { 
		int ix = 0;
		for (auto& itc : _vchilds) {
			ft_sentence* pst = static_cast<ft_sentence*>(itc);
			if (pst->load_idx != ix) {
				return false;
			}
			ix++;
		}
		return true;
	}
	void ft_paragraph::shuffle() {
	  random_shuffle(_vchilds.begin(), _vchilds.end());
	}
	void ft_paragraph::draw()
	{
		ImVec2 abpos = absolute_coordinate_of_base_pos();
		ImVec2 winpos = ImGui::GetWindowPos();
		ImVec2 pos0 = { abpos.x + winpos.x, abpos.y + winpos.y };
		
		ImVec4 bkcl(_bdclr.x, _bdclr.y, _bdclr.z, _bdclr.w);
		ImU32 col = ImGui::ColorConvertFloat4ToU32(bkcl);
		ImVec2 pos1(pos0.x + _in_p._sizew, pos0.y + _weight);
		ImGui::RenderFrame(pos0, pos1, col, true);
		ImVec2 pos2(pos0.x+_weight, pos0.y + _in_p._sizeh);
        ImGui::RenderFrame(pos0, pos2, col, true);
        ImVec2 pos3(pos0.x+ _in_p._sizew, pos0.y + _in_p._sizeh);
		ImVec2 posa1(pos0.x + _in_p._sizew-_weight, pos0.y );
		ImGui::RenderFrame(posa1, pos3, col, true);
		ImVec2 posa2(pos0.x, pos0.y + _in_p._sizeh-_weight);
		ImGui::RenderFrame(posa2, pos3, col, true);
		//ft_base::draw();
	}

}