#pragma once


class OS {
public:
	static int GetTime() {
		return std::chrono::system_clock::now().time_since_epoch().count();
	}
};