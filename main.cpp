#include <iostream>
#include <cmath>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

//Functions
void update_numbers();
void draw_line(RenderWindow& window);
void draw_graph(RenderWindow& window);

//Constants
constexpr float PIXELS_PER_METER = 5;
constexpr float GRAVITY = -9.81;
constexpr float FORCE = 400;
constexpr float MASS = 10;
constexpr float PI = 3.14159265f;
constexpr float THRUST_TIME = 1.5;
constexpr float ORIGIN = 50;

//Path points for graph
VertexArray path(PrimitiveType::LineStrip);

//Flight information
float x = 0, x_old, y = 0, y_old,
      vx = 0, vy = 0,
      a_thrust = FORCE/MASS, ax_tot, ay_tot,
      dt = 1.f/60.f, t = 0,
      launch_angle = 60 * PI / 180;

int main()
{
    //Initialize Window
    RenderWindow window(VideoMode({1920, 1080}), "RTS", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    window.clear(Color::Black);

    //Add origin to path line
    path.append(Vertex{{ORIGIN, 1080 - ORIGIN}, Color::Red});

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

        if (y >= 0)
        {
        //Update position
        update_numbers();

        //Draw Screen
        draw_graph(window);
        draw_line(window);

        //Display new
        window.display();
        }
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
    if (t < THRUST_TIME)
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
    y += vy * dt;
    x += vx * dt;
}

void draw_line(RenderWindow& window)
{
    if (t <= THRUST_TIME)
        {
            path.append(Vertex{{x * PIXELS_PER_METER + ORIGIN, 1080.f - y * PIXELS_PER_METER - ORIGIN}, Color::Green});
            window.draw(path);
        }
        if (t > THRUST_TIME)
        {
            path.append(Vertex{{x * PIXELS_PER_METER + ORIGIN, 1080.f - y * PIXELS_PER_METER - ORIGIN}, Color::Red});
            window.draw(path);
        }
}

void draw_graph(RenderWindow& window)
{
    RectangleShape rectangle({1820.f, 5.f});
    rectangle.setPosition({ORIGIN, 1030.f});
    window.draw(rectangle);
    rectangle.setSize({5.f, -980.f});
    rectangle.setPosition({ORIGIN - 5.f, 1035.f});
    window.draw(rectangle);

    for (int i = 1; i <37; i++)
    {
        rectangle.setPosition({ORIGIN + i * 50.f, 1070.f - ORIGIN});
        rectangle.setSize({5.f, 25.f});
        window.draw(rectangle);
    }
    for (int i = 1; i <20; i++)
    {
        rectangle.setPosition({ORIGIN - 15.f, 1080.f - ORIGIN - i * 50.f});
        rectangle.setSize({25.f, -5.f});
        window.draw(rectangle);
    }
}