#pragma once
#include <utility>

class WindowContext
{
public:
	static WindowContext* context;
	WindowContext() {};
	~WindowContext() {};
	virtual std::pair<int, int> getSize() = 0;
	virtual std::pair<double, double> getCursorPos() = 0;
};

