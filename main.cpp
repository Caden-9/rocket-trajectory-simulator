/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*Functions*/

//Menu and Simulator
void menu(RenderWindow& window);
void simulator(RenderWindow& window);

//Physics
float acceleration_y(float t);
float velocity_y(float t);
float position_y(float t);
float acceleration_x(float t);
float velocity_x(float t);
float position_x(float t);

void calculate_bounds();
void update_numbers();

/*Screen*/
void draw_path(RenderWindow& window);
void draw_graph(RenderWindow& window, RectangleShape& x_axis, RectangleShape& y_axis, RectangleShape& ticks);
void draw_state(RenderWindow& window, Text& time, Text& distance, Text& height, Text& velocity, Text& acceleration);
void draw_arrows(RenderWindow& window);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Constants
constexpr float GRAVITY = -9.81,
                PI = 3.14159265,
                ORIGIN = 50,
                X_AXIS_LENGTH = 1600, Y_AXIS_LENGTH = 900;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Path points for graph and vertex points for arrows
VertexArray path(PrimitiveType::LineStrip);
Vertex v_arrow[2], a_arrow[2];

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*Variables*/

//Flight information
float dt = 1.f/60.f, t = 0, thrust_time = 1.5,         //Time
      x = 0, y = 0.0000000000000000001,                //Position
      vx = 0, vy = 0, v,                               //Velocity
      a_thrust, ax, ay, a,                             //Acceleration

      force = 400, mass_rocket = 10, mass_fuel = 10,   //Rocket
      fuel_per_second = mass_fuel / thrust_time,
      launch_angle = 60 * PI / 180;                     //Angle

//Variables for calculate_bounds function
float height_max, distance_max,
          velocity1y, velocity1x,
          time_peak, time_ground;

//Scale
float pixels_per_meter, arrow_length_v, arrow_length_a;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

bool end_flight;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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

    //Flight bool for final correction
    end_flight = true;

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

        if (t < time_ground)
        {
        //Update position
        update_numbers();

        //Clear screen
        window.clear(Color::Black);
        
        //Draw Screen
        draw_graph(window, x_axis, y_axis, ticks);
        draw_state(window, time, distance, height, velocity, acceleration);
        draw_path(window);
        draw_arrows(window);

        //Display new
        window.display();
        }
        else //Stop and keep updating screen
        {
            if (end_flight)
            {
                //Update to actual final numbers
                t = time_ground;
                x = position_x(t);
                y = abs(position_y(t)); //absolute value to not have -0.00
                v = sqrt(velocity_x(t) * velocity_x(t) + velocity_y(t) * velocity_y(t));
                a = sqrt(acceleration_x(t) * acceleration_x(t) + acceleration_y(t) * acceleration_y(t));

                //Add final point to path
                path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN}, Color::Red});
                
                //Don't do this again
                end_flight = false;
            }
            
            //Clear screen
            window.clear(Color::Black);

            //Draw Screen
            draw_graph(window, x_axis, y_axis, ticks);
            draw_path(window);
            draw_state(window, time, distance, height, velocity, acceleration);

            //Display new
            window.display();
        }
    }
}

float acceleration_y(float t)
{
    if (t <= thrust_time)
    {
        return GRAVITY + force / (mass_rocket + mass_fuel - fuel_per_second * t) * sin(launch_angle);
    }
    else
    {
        return GRAVITY;
    }

}

float velocity_y(float t)
{
    if (t <= thrust_time)
    {
        return GRAVITY * t - (force / fuel_per_second) * std::log(mass_rocket + mass_fuel - fuel_per_second * t) * sin(launch_angle)
                + (force / fuel_per_second) * std::log(mass_rocket + mass_fuel) * sin(launch_angle);                                               //Constant
    }
    else
    {
        return velocity_y(thrust_time) + GRAVITY * (t - thrust_time);
    }

}

float position_y(float t)
{
    if (t <= thrust_time)
    {
        return 0.5f * GRAVITY * t * t - (force / fuel_per_second) * sin(launch_angle) * 
                 (std::log(mass_rocket + mass_fuel - fuel_per_second * t) * t
                  - t - (mass_rocket + mass_fuel) / fuel_per_second * std::log(mass_rocket + mass_fuel - fuel_per_second * t))
                 + (force / fuel_per_second) * std::log(mass_rocket + mass_fuel) * sin(launch_angle) * t                                            //Constant * t
                 - (force / fuel_per_second) * sin(launch_angle) * (mass_rocket + mass_fuel) / fuel_per_second * std::log(mass_rocket + mass_fuel); //Constant
    }
    else
    {
        return position_y(thrust_time)
            + 0.5f * GRAVITY * (t - thrust_time) * (t - thrust_time)
            + velocity_y(thrust_time) * (t - thrust_time);
    }
}

float acceleration_x(float t)
{
    if (t <= thrust_time)
    {
        return force / (mass_rocket + mass_fuel - fuel_per_second * t) * cos(launch_angle);
    }
    else
    {
        return 0.f;
    }
}

float velocity_x(float t)
{
    if (t <= thrust_time)
    {
        return  - (force / fuel_per_second) * std::log(mass_rocket + mass_fuel - fuel_per_second * t) * cos(launch_angle)
                + (force / fuel_per_second) * std::log(mass_rocket + mass_fuel) * cos(launch_angle);                                               //Constant
    }
    else
    {
        return velocity_x(thrust_time);
    } 
}

float position_x(float t)
{
    if (t <= thrust_time)
    {
        return  - (force / fuel_per_second) * cos(launch_angle) * 
                 (std::log(mass_rocket + mass_fuel - fuel_per_second * t) * t
                  - t - (mass_rocket + mass_fuel) / fuel_per_second * std::log(mass_rocket + mass_fuel - fuel_per_second * t))
                 + (force / fuel_per_second) * std::log(mass_rocket + mass_fuel) * cos(launch_angle) * t                                            //Constant * t
                 - (force / fuel_per_second) * cos(launch_angle) * (mass_rocket + mass_fuel) / fuel_per_second * std::log(mass_rocket + mass_fuel); //Constant
    }
    else
    {
        return position_x(thrust_time)
            + velocity_x(thrust_time) * (t - thrust_time);
    }
}

void calculate_bounds()
{
    //Time of peak
    time_peak = - velocity_y(thrust_time) / GRAVITY + thrust_time;

    //Max height
    height_max = position_y(time_peak);

    //Time rocket hits ground
    time_ground = (-velocity_y(thrust_time) -
                sqrt(velocity_y(thrust_time) * velocity_y(thrust_time) - 2.f * GRAVITY * position_y(thrust_time)))
                / (GRAVITY) + thrust_time;

    //Max distance
    distance_max = position_x(time_ground);

    //Velocity after boosters stop for velocity arrow scale
    velocity1x = velocity_x(thrust_time);
    velocity1y = velocity_y(thrust_time);
}

void update_numbers()
{
    //New time, mass, and acceleration
    t += dt;

    //Get current acceleration, velocity, and position
    ax = acceleration_x(t);
    ay = acceleration_y(t);

    vx = velocity_x(t);
    vy = velocity_y(t);

    x = position_x(t);
    y = position_y(t);

    //Velocity and acceleration vector magnitudes
    v = sqrt(vx * vx + vy * vy);
    a = sqrt(ax * ax + ay * ay);
}

void draw_path(RenderWindow& window)
{
    if (t <= time_ground)
    {
        if (t <= thrust_time) //Green during boosters
        {
            path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN}, Color::Green});
        }
        else //Red during freefall
        {
            path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN}, Color::Red});
        }
    }
    window.draw(path);
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

void draw_arrows(RenderWindow& window)
{
    //Calculate arrow lengths
    arrow_length_v = abs((v / sqrt(velocity1x * velocity1x + velocity1y * velocity1y))) * 100.f; //This number (50.f) is for max pixels long
    arrow_length_a = abs((a / GRAVITY)) * 50.f;
    
    //Make arrow bounds
    v_arrow[0].position = {x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN};
    v_arrow[1].position = {x * pixels_per_meter  + arrow_length_v * vx / v + ORIGIN,
                            1080.f - y * pixels_per_meter - arrow_length_v * vy / v - ORIGIN};

    a_arrow[0].position = {x * pixels_per_meter + ORIGIN, 1080.f - y * pixels_per_meter - ORIGIN};
    a_arrow[1].position = {x * pixels_per_meter + arrow_length_a * ax / a + ORIGIN,
                            1080.f - y * pixels_per_meter - arrow_length_a * ay / a - ORIGIN};

    //Draw vectors
    window.draw(v_arrow, 2, PrimitiveType::Lines);
    window.draw(a_arrow, 2, PrimitiveType::Lines);
}