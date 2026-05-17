#include <iostream>
#include <cmath>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

//Functions
void menu(RenderWindow& window);
void simulator(RenderWindow& window);
void update_numbers();
void draw_line(RenderWindow& window);
void draw_graph(RenderWindow& window);
void draw_state(RenderWindow& window, Text& distance, Text& height, Text& velocity, Text& acceleration);
void draw_vectors(RenderWindow& window);

//Constants
constexpr float PIXELS_PER_METER = 5;
constexpr float GRAVITY = -9.81;
constexpr float FORCE = 400;
constexpr float MASS = 10;
constexpr float PI = 3.14159265f;
constexpr float THRUST_TIME = 1.5;
constexpr float ORIGIN = 50;
constexpr float ARROW_LENGTH = 25;

//Path points for graph
VertexArray path(PrimitiveType::LineStrip);

//Flight information
float x = 0, x_old, y = 0.0000000000000000001, y_old,
      vx = 0, vy = 0, v,
      a_thrust = FORCE/MASS, ax, ay, a,
      dt = 1.f/60.f, t = 0,
      launch_angle = 60 * PI / 180;

Vertex v_arrow[2], a_arrow[2];

int main()
{
    //Initialize Window
    RenderWindow window(VideoMode({1920, 1080}), "RTS", Style::Titlebar | Style::Close);
    window.clear(Color::Black);
    window.display();
    window.setFramerateLimit(60);

    while(window.isOpen())
    menu(window);
    
    return 0;
}

void menu(RenderWindow& window)
{
    while (const std::optional event = window.pollEvent()) //Look for events
    {

        if (event->getIf<Event::Closed>())
        {
            window.close();
        }

        if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>())
        {
            if (mousePressed->button == Mouse::Button::Left)
            {
                simulator(window);
            }
        }
    }
}

void simulator(RenderWindow& window)
{
    //Initialize font and text specifications
    Font font;
        if (!font.openFromFile("fonts/Roboto-Regular.ttf"))
        {
            std::cout << "Failed to load font\n";
        }

    Text distance(font);
    distance.setCharacterSize(40);
    distance.setPosition({1500.f, 50.f});
    Text height(font);
    height.setCharacterSize(40);
    height.setPosition({1500.f, 110.f});
    Text velocity(font);
    velocity.setCharacterSize(40);
    velocity.setPosition({1500.f, 170.f});
    Text acceleration(font);
    acceleration.setCharacterSize(40);
    acceleration.setPosition({1500.f, 230.f});

    //Set v and a arrow colors
    v_arrow[0].color = v_arrow[1].color = Color::Cyan;
    a_arrow[0].color = a_arrow[1].color = Color::Magenta;


    //Add origin to path line
    //path.append(Vertex{{ORIGIN, 1080 - ORIGIN}, Color::Red});

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

        if (y > 0)
        {
        //Update position
        update_numbers();

        //Draw Screen
        window.clear(Color::Black); //Clear screen
        draw_graph(window);
        draw_state(window, distance, height, velocity, acceleration);
        draw_line(window);
        draw_vectors(window);

        //Display new
        window.display();
        }
        else //Keep updating screen
        {
            y = vx = vy = ax = ay = 0;
            //Draw Screen
            window.clear(Color::Black);
            draw_graph(window);
            draw_line(window);
            draw_state(window, distance, height, velocity, acceleration);
            draw_vectors(window);

            //Display new
            window.display();
        }
    }
}

void update_numbers()
{
    //New time
    t += dt;

    //Save old positions
    x_old = x;
    y_old = y;

    //Get current acceleration, velocity, and position
    if (t < THRUST_TIME)
    {
        ay = a_thrust * sin(launch_angle) + GRAVITY;
        ax = a_thrust * cos(launch_angle);
    }
    else
    {
        ay = GRAVITY;
        ax = 0;
    }

    vx += ax * dt;
    vy += ay * dt;

    y += vy * dt;
    x += vx * dt;

    //Velocity and acceleraion vector magnitudes
    v = sqrt(vx * vx + vy * vy);
    a = sqrt(ax * ax + ay * ay);
}

void draw_line(RenderWindow& window)
{
    Color pink(255, 192, 203);
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

void draw_state(RenderWindow& window, Text& distance, Text& height, Text& velocity, Text& acceleration)
{
    std::stringstream sx, sy, sv, sa;

    sx << std::fixed << std::setprecision(2) << x;
    sy << std::fixed << std::setprecision(2) << y;
    sv << std::fixed << std::setprecision(2) << v;
    sa << std::fixed << std::setprecision(2) << a;

    distance.setString("Distance: " + sx.str());
    height.setString("Height: " + sy.str());
    velocity.setString("Velocity: " + sv.str());
    acceleration.setString("Acceleration: " + sa.str());

    window.draw(distance);
    window.draw(height);
    window.draw(velocity);
    window.draw(acceleration);
}

void draw_vectors(RenderWindow& window)
{
    //Make arrow bounds
    v_arrow[0].position = {x * PIXELS_PER_METER + ORIGIN, 1080.f - y * PIXELS_PER_METER - ORIGIN};
    v_arrow[1].position = {(x + ARROW_LENGTH * vx / v) * PIXELS_PER_METER + ORIGIN,
                            1080.f - (y + ARROW_LENGTH * vy / v) * PIXELS_PER_METER - ORIGIN};

    a_arrow[0].position = {x * PIXELS_PER_METER + ORIGIN, 1080.f - y * PIXELS_PER_METER - ORIGIN};
    a_arrow[1].position = {(x + ARROW_LENGTH * 0.5f * ax / a) * PIXELS_PER_METER + ORIGIN,
                            1080.f - (y + ARROW_LENGTH * 0.5f * ay / a) * PIXELS_PER_METER - ORIGIN};

    //Draw vectors
    window.draw(v_arrow, 2, PrimitiveType::Lines);
    window.draw(a_arrow, 2, PrimitiveType::Lines);
}