#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

//Functions
void menu(RenderWindow& window);
void simulator(RenderWindow& window);
void calculate_bounds();
void update_numbers();
void draw_path(RenderWindow& window);
void draw_graph(RenderWindow& window, RectangleShape& x_axis, RectangleShape& y_axis, RectangleShape& ticks);
void draw_state(RenderWindow& window, Text& time, Text& distance, Text& height, Text& velocity, Text& acceleration);
void draw_vectors(RenderWindow& window);

//Constants
constexpr float GRAVITY = -9.81,
                FORCE = 400, MASS = 5,
                PI = 3.14159265,
                THRUST_TIME = 1.5,
                ORIGIN = 50,
                ARROW_LENGTH = 25,
                X_AXIS_LENGTH = 1600, Y_AXIS_LENGTH = 900;

//Path points for graph
VertexArray path(PrimitiveType::LineStrip);

//Flight information
float x = 0, x_old, y = 0.0000000000000000001, y_old,
      vx = 0, vy = 0, v,
      a_thrust = FORCE/MASS, ax, ay, a,
      dt = 1.f/60.f, t = 0,
      launch_angle = 80 * PI / 180;

//Variables for calculate_bounds function
float height1, height_max, distance1, distance_max,
          velocity1y, velocity1x,
          time_peak, time_ground;

//Scale
float pixels_per_meter;

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

    Text time(font);
    time.setCharacterSize(40);
    time.setPosition({1500.f, 50.f});
    Text distance(font);
    distance.setCharacterSize(40);
    distance.setPosition({1500.f, 110.f});
    Text height(font);
    height.setCharacterSize(40);
    height.setPosition({1500.f, 170.f});
    Text velocity(font);
    velocity.setCharacterSize(40);
    velocity.setPosition({1500.f, 230.f});
    Text acceleration(font);
    acceleration.setCharacterSize(40);
    acceleration.setPosition({1500.f, 290.f});

    //Graph lines info
    RectangleShape x_axis({X_AXIS_LENGTH, 5.f});
    x_axis.setPosition({ORIGIN - 5.f, 1080.f - ORIGIN});
    RectangleShape y_axis({5.f, -Y_AXIS_LENGTH});
    y_axis.setPosition({ORIGIN - 5.f, 1085.f - ORIGIN});
    RectangleShape ticks({});

    //Arrow information
    v_arrow[0].color = v_arrow[1].color = Color::Cyan;
    a_arrow[0].color = a_arrow[1].color = Color::Magenta;

    //Find bounds to scale graph
    calculate_bounds();
    pixels_per_meter = std::min((X_AXIS_LENGTH / distance_max), (Y_AXIS_LENGTH / height_max));

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

        if (t <= time_ground + THRUST_TIME)
        {
        //Draw Screen
        window.clear(Color::Black); //Clear screen
        draw_graph(window, x_axis, y_axis, ticks);
        draw_state(window, time, distance, height, velocity, acceleration);
        draw_path(window);
        draw_vectors(window);

        //Update position
        update_numbers();

        //Display new
        window.display();
        }
        else //Keep updating screen
        {
            y = vx = vy = ax = ay = 0;
            //Draw Screen
            window.clear(Color::Black);
            draw_graph(window, x_axis, y_axis, ticks);
            draw_path(window);
            draw_state(window, time, distance, height, velocity, acceleration);

            //Display new
            window.display();
        }
    }
}

void calculate_bounds()
{
    //Time of peak for freefall portion (with t=0 when booster stops)
    time_peak = -(a_thrust * sin(launch_angle) * THRUST_TIME + GRAVITY * THRUST_TIME) / GRAVITY;

    //Height and velocity at booster off, then max height
    height1 = 0.5f * (a_thrust * sin(launch_angle) + GRAVITY) * THRUST_TIME * THRUST_TIME;
    velocity1y = a_thrust * sin(launch_angle) * THRUST_TIME + GRAVITY * THRUST_TIME;
    height_max = 0.5f * GRAVITY * time_peak * time_peak
                + velocity1y * time_peak
                + height1;

    //Time of ground for freefall portion (with t=0 when booster stops)
    time_ground = (-velocity1y - sqrt(velocity1y * velocity1y - 2.f * GRAVITY * height1)) / (2.f * 0.5f * GRAVITY);

    //Distance and velocity at booster off, then max distance
    distance1 = 0.5f * a_thrust * cos(launch_angle) * THRUST_TIME * THRUST_TIME;
    velocity1x = a_thrust * cos(launch_angle) * THRUST_TIME;
    distance_max = distance1 + velocity1x * time_ground;
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

    //Velocity and acceleration vector magnitudes
    v = sqrt(vx * vx + vy * vy);
    a = sqrt(ax * ax + ay * ay);
}

void draw_path(RenderWindow& window)
{
    if (t <= THRUST_TIME)
        {
            path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN}, Color::Green});
            window.draw(path);
        }
        if (t > THRUST_TIME)
        {
            path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN}, Color::Red});
            window.draw(path);
        }
}

void draw_graph(RenderWindow& window, RectangleShape& x_axis, RectangleShape& y_axis, RectangleShape& ticks)
{
    window.draw(x_axis);
    window.draw(y_axis);

    for (int i = 1; i <33; i++)
    {
        ticks.setPosition({ORIGIN + i * 50.f, 1070.f - ORIGIN});
        ticks.setSize({5.f, 25.f});
        window.draw(ticks);
    }
    for (int i = 1; i <19; i++)
    {
        ticks.setPosition({ORIGIN - 15.f, 1080.f - ORIGIN - i * 50.f});
        ticks.setSize({25.f, -5.f});
        window.draw(ticks);
    }
}

void draw_state(RenderWindow& window, Text& time, Text& distance, Text& height, Text& velocity, Text& acceleration)
{
    std::stringstream st, sx, sy, sv, sa;

    st << std::fixed << std::setprecision(2) << t;
    sx << std::fixed << std::setprecision(2) << x;
    sy << std::fixed << std::setprecision(2) << y;
    sv << std::fixed << std::setprecision(2) << v;
    sa << std::fixed << std::setprecision(2) << a;

    time.setString("Time: " + st.str());
    distance.setString("Distance: " + sx.str());
    height.setString("Height: " + sy.str());
    velocity.setString("Velocity: " + sv.str());
    acceleration.setString("Acceleration: " + sa.str());

    window.draw(time);
    window.draw(distance);
    window.draw(height);
    window.draw(velocity);
    window.draw(acceleration);
}

void draw_vectors(RenderWindow& window)
{
    //Make arrow bounds
    v_arrow[0].position = {x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN};
    v_arrow[1].position = {(x + ARROW_LENGTH * vx / v) * pixels_per_meter + ORIGIN,
                            1080.f - (y + ARROW_LENGTH * vy / v) * pixels_per_meter - ORIGIN};

    a_arrow[0].position = {x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN};
    a_arrow[1].position = {(x + ARROW_LENGTH * 0.5f * ax / a) * pixels_per_meter + ORIGIN,
                            1080.f - (y + ARROW_LENGTH * 0.5f * ay / a) * pixels_per_meter - ORIGIN};

    //Draw vectors
    window.draw(v_arrow, 2, PrimitiveType::Lines);
    window.draw(a_arrow, 2, PrimitiveType::Lines);
}