#include "my_application.h"
#include "af_timer.h"
#include "main.h"
#include <chrono>
#include "lcm_data_listener.h"
#include <thread>
using namespace std;
using namespace auto_future;
af_timer g_timer;
my_application::my_application(int argc, char **argv)
	:application(argc,argv)
{
	_screen_width =_win_width=1800;
	_screen_height = _win_height = 1500;
	_wposx = _wposy = 0;
}

bool receiving_lcm=false;
void my_application::resLoaded()
{
	thread thd_lcm([&]{
		lcm_data_listener::init();
		receiving_lcm=true;
		while(receiving_lcm){
			lcm_data_listener::hanlde();
		}
	});
	thd_lcm.detach();
}


my_application::~my_application()
{
}
void my_application::onUpdate(){
	//
	g_timer.execute();
}
AFGUI_APP(my_application)


