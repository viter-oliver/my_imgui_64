#pragma once

#include <Windows.h>
#include <dshow.h>

#pragma comment(lib, "strmiids")

#include <vector>
#include <string>

struct Device {
	std::string devicePath;
	std::string deviceName; // This can be used to show the devices to the user
};

class DeviceEnumerator {

public:

	DeviceEnumerator() = default;
	std::vector<Device> getDevicesMap(const GUID deviceClass);
	std::vector<Device> getVideoDevicesMap();
	std::vector<Device> getAudioDevicesMap();

private:

	std::string ConvertBSTRToMBS(BSTR bstr);
	std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);

};
