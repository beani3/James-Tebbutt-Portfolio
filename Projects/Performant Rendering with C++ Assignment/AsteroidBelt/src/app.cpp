#include "app.hpp"
#include "gameLayer.hpp"
#include "uiLayer.hpp"
#include "LOD.hpp"

App::App(const WindowProperties& winProps) : Application(winProps)
{
	m_layerStack.emplace_top<AsteriodBelt>(m_window, m_data);
	m_layerStack.emplace_top<UI>(m_window, m_data);
	// m_layerStack.emplace_top<LOD>(m_window);
}

Application* startApplication()
{
	WindowProperties props("Asteroid Belt", 1422, 800);
	// WindowProperties props("LOD Demo", 1422, 800);
	props.isHostingImGui = true;
	props.isResizable = false;
	return new App(props);
}