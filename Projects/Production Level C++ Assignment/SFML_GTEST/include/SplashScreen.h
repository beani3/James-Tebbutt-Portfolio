#include <SFML/Graphics.hpp>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#pragma once

class SplashScreen {
public:
	SplashScreen() = default;
	SplashScreen(std::vector<std::string>& contents);
	~SplashScreen() = default;
	void LoadingFile();
	void GenerateSplashScreen();
private:
	std::thread m_loadingThread;
	std::ifstream m_fileToRead;
	std::ofstream m_output;
	std::vector<std::string> v_contents;
	std::mutex m_mtx;
	std::condition_variable m_condVar;
	bool b_loaded = false;
	sf::Text m_text;
	sf::Font m_font;
	sf::RectangleShape m_loadingOutline;
	sf::RectangleShape m_loadingRect;
	sf::View m_camera;
};