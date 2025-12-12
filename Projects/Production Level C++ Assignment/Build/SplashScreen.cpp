#include "SplashScreen.h"
#include <iostream>

SplashScreen::SplashScreen(std::vector<std::string>& contents) :
	v_contents(contents)
{
	m_font.loadFromFile("../Assets/Fonts/calibri.ttf");
	m_text = sf::Text("Press Space to Fill Bar", m_font, 40);
	m_text.setColor(sf::Color::White);
	m_text.setOrigin(m_text.getGlobalBounds().width / 2, m_text.getGlobalBounds().height / 2);
	m_loadingOutline.setSize(sf::Vector2f(400, 60));
	m_loadingOutline.setOutlineColor(sf::Color::White);
	m_loadingOutline.setOutlineThickness(3.0f);
	m_loadingOutline.setFillColor(sf::Color::Transparent);
	m_loadingOutline.setOrigin(m_loadingOutline.getSize().x / 2, m_loadingOutline.getSize().y / 2);
	m_loadingRect.setSize(sf::Vector2f(5, 60));
	m_loadingRect.setFillColor(sf::Color::Red);
	m_loadingRect.setOrigin(m_loadingRect.getSize().x / 2, m_loadingRect.getSize().y / 2);
}

void SplashScreen::LoadingFile() {
	m_fileToRead = std::ifstream("../Users/UserDetails.csv");
	m_output = std::ofstream("../Users/Output.csv");

	if (m_fileToRead.is_open() && m_output.is_open()) {
		m_output << "Line Number, Content\n";

		std::string s_line;
		int i_lineNum = 0;

		/* Pushed the data back into a vector that is used
		 * to read said data and output it onto the command
		 * line.
		 */
		while (std::getline(m_fileToRead, s_line)) {
			std::lock_guard<std::mutex> lock(m_mtx);
			v_contents.push_back(s_line);
			b_loaded = false;

			for (int i = 0; i < v_contents.size() > 0; i++) {
				m_output << v_contents.at(i) << "\n";
			}
		}

		b_loaded = true;
		m_fileToRead.close();
		m_output.close();
	}
	
	for (const auto& content : v_contents) {
		std::cout << "Contents: " << content << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}

void SplashScreen::GenerateSplashScreen() {
	sf::Event event;
	sf::RenderWindow window(sf::VideoMode(1280, 720), "GAMR2541 SFML Assignment!");
	m_loadingOutline.setPosition(window.getSize().x / 2, window.getSize().y / 2);
	m_text.setPosition(window.getSize().x / 2, (window.getSize().y / 2) - 75);
	m_loadingRect.setPosition((window.getSize().x / 2) - 197, window.getSize().y / 2);

	int fps = 60;
	int delay = 1000 / fps;
	/* Creates loop for duration of  the splashscreen so that inputs are being registed per frame
	 * rather than once which is how it was before
	 */
	for (int i = 0; i < (fps * 3); i++) {
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
				m_loadingRect.setSize(sf::Vector2f(m_loadingRect.getSize().x + 15, 60));
			}
		}
		// Shakes bar if it goes outside the bounds of the outline
		if (m_loadingRect.getSize().x > m_loadingOutline.getSize().x) {
			m_loadingRect.setRotation(m_loadingRect.getRotation() + rand() % 2);
			m_loadingRect.setRotation(0);
			m_loadingRect.setRotation(m_loadingRect.getRotation() - rand() % 2);
		}

		std::lock_guard<std::mutex> lock(m_mtx);
		window.clear();
		window.draw(m_text);
		window.draw(m_loadingOutline);
		window.draw(m_loadingRect);
		window.display();
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}
}