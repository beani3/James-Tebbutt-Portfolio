#pragma once

// Entry point -  extern in DemonRenderer application.hpp
#include "core/entryPoint.hpp"
#include "sharedData.hpp"

class App : public Application
{
public:
	App(const WindowProperties& winProps);
private:
	SharedData m_data; //!< Data shared between UI and GAMR3531 layers
};
