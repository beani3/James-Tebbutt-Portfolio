#include "GameEngine.h"
#include "SplashScreen.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
	sf::RenderWindow window(sf::VideoMode(1280, 720), "GAMR2541 SFML Assignment!");
	GameEngine gameEngine(window);

    // Processes File   
    std::vector<std::string> v_contents;
    SplashScreen ss(v_contents);
    std::thread readFile(&SplashScreen::LoadingFile, &ss);
    // Generates Splashscreen with static text
    std::thread genSS(&SplashScreen::GenerateSplashScreen, &ss);

    readFile.join(); 
    genSS.join();
	gameEngine.run(); 
}