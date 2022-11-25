#pragma once
#include "application.h"
class my_application :
	public auto_future::application
{

public:
	my_application(int argc, char **argv);
	void resLoaded();
	void onUpdate();

	virtual ~my_application();

};

