#include "app.hpp"
#include "GAMR3521.hpp"

App::App(const WindowProperties& winProps) : Application(winProps)
{
	m_layerStack.push(std::make_unique<MainLayer>(MainLayer(m_window)));
}

Application* startApplication()
{
	WindowProperties props("GAMR3521 Assignment", 1600, 900);
	props.isHostingImGui = true;
	props.isResizable = false;
	return new App(props);
}