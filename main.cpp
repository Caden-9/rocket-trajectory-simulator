#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

//Functions
void update_numbers();

//Constants
constexpr float PIXELS_PER_METER = 5;
constexpr float GRAVITY = -9.81;
constexpr float FORCE = 400;
constexpr float MASS = 10;

//Flight information
float x = 0, x_old, y = 0, y_old,
      v = 0, a_thrust = FORCE/MASS, a_tot,
      dt = 1.f/60.f, t = 0;

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
        update_numbers();

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

void update_numbers()
{
    //New time
    t += dt;

    //Save old positions
    x_old = x;
    y_old = y;

    //Get current acceleration and velocity
    if (t < 1.5)
        a_tot = a_thrust + GRAVITY;
    else
        a_tot = GRAVITY;

    v += a_tot * dt;
    y += v * dt * PIXELS_PER_METER;
    x += 10.f * dt * PIXELS_PER_METER;
}