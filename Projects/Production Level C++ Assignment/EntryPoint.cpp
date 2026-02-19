/**
 * @file main.cpp
 * @author [Jethro Shell]
 * @date [01/03/2025]
 */

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>

 /**
  * The main function, which sets up and runs the SFML application.
  *
  * @return 0 if the program finishes successfully
  */
int entryPoint() {
    /**
     * Create a new window with a title of "GAMR2541 SFML Assignment!" 
     *
     * @dimensions of 800x600 pixels.
     */
    sf::RenderWindow window(sf::VideoMode(800, 600), "GAMR2541 SFML Assignment!");

    /**
     * Create a new rectangle shape with dimensions of 100x100 pixels.
     */
    sf::RectangleShape rectangle(sf::Vector2f(100, 100));

    /**
     * Set the fill color of the rectangle to green.
     */
    rectangle.setFillColor(sf::Color::Green);

    /**
     * Position the rectangle at (100, 100) on the screen.
     */
    rectangle.setPosition(100, 100);

    /**
     * Set the target frame rate for the application.
     */
    float f_fps = 60.0f;

    /**
     * Create a clock object to measure time.
     */
    sf::Clock clock;

    /**
     * Get the initial time since the last frame was drawn.
     */
    sf::Time time = clock.restart();

    /**
     * Calculate the target time per frame based on the desired frame rate.
     */
    sf::Time timePerFrame = sf::seconds(1.0f / f_fps);

    while (window.isOpen()) {

        // Check if the W key is pressed and move the rectangle up accordingly
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            rectangle.move(0, -1);
        }
        // Check if the S key is pressed and move the rectangle down accordingly
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            rectangle.move(0, 1);
        }
        // Check if the A key is pressed and move the rectangle left accordingly
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            rectangle.move(-1, 0);
        }
        // Check if the D key is pressed and move the rectangle right accordingly
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            rectangle.move(1, 0);
        }

        // Poll for events in the window
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close the window if the user clicks on the close button
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Clear the window and draw the rectangle
        window.clear();
        window.draw(rectangle);
        window.display();
    }

    return 0;
}