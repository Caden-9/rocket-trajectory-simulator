#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

//Flight information
float x = 0, x_old, y = 0, y_old, v = 1000, a = -500, dt = 1.f/60.f;

int main()
{
    //Window
    RenderWindow window(VideoMode({1920, 1080}), "RTS", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    //Draw black screen
    window.clear(Color::Black); 

    //Simulator Loop
    while(window.isOpen())
    {
        //Check if closed
        while(const std::optional event = window.pollEvent())
        {
            //Close with button and esc
            if (event->is<Event::Closed>())
                window.close();
        }

        //Update position
        x_old = x;
        y_old = y;
        y += v*dt;
        v += a*dt;
        x += 250.f*dt;

            //Draw Screen
        //Draw new
        Vertex line[] = {
            Vertex{{x_old, 1080.f - y_old}},
            Vertex{{x, 1080.f - y}}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        //Display new
        window.display();
    }
    return 0;
}