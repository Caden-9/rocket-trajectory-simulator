#include <iostream>
#include <cmath>

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
constexpr float PI = 3.14159265f;

//Flight information
float x = 0, x_old, y = 0, y_old,
      vx = 0, vy = 0,
      a_thrust = FORCE/MASS, ax_tot, ay_tot,
      dt = 1.f/60.f, t = 0,
      launch_angle = 85 * PI / 180;

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
    {
        ay_tot = a_thrust * sin(launch_angle) + GRAVITY;
        ax_tot = a_thrust * cos(launch_angle);
    }
    else
    {
        ay_tot = GRAVITY;
        ax_tot = 0;
    }

    vx += ax_tot * dt;
    vy += ay_tot * dt;
    y += vy * dt * PIXELS_PER_METER;
    x += vx * dt * PIXELS_PER_METER;
}