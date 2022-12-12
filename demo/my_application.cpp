#include "my_application.h"
#include "af_timer.h"
#include "main.h"
#include <chrono>
#include "lcm_data_listener.h"
#include <thread>
using namespace std;
using namespace auto_future;
int scene_width = 1800;
int scene_height = 1500;
af_timer g_timer;
my_application::my_application(int argc, char **argv)
	:application(argc,argv)
{
	_win_width = _screen_width = scene_width = 1800;
	_win_height = _screen_height = scene_height = 1500;
	_wposx = _wposy = 0;
	if (argc >= 4) {
		_win_width =_screen_width= scene_width =  atoi(argv[2]);
		_win_height = _screen_height= scene_height = atoi(argv[3]);
	}
}

bool receiving_lcm=false;
void my_application::resLoaded()
{
    lcm_data_listener::init();
	thread thd_lcm([&]{
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
	adas_data::execute_data();
	g_timer.execute();
}
AFGUI_APP(my_application)


