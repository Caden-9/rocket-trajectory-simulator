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
void get_drag();
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
      x = 0, y = 0.00000000000000001,                  //Position
      vx = 0, vy = 0, v = 0,                           //Velocity
      ax, ay, a,                                       //Acceleration

      force = 400, mass_rocket = 10, mass_fuel = 10,   //Rocket
      m0 = mass_rocket + mass_fuel,
      m1 = mass_rocket + mass_fuel,
      fuel_per_second = mass_fuel / thrust_time,
      launch_angle = 80 * PI / 180;                     //Angle

//Constants used in calculations for after thrust time ends
float v_thrust_time_x, v_thrust_time_y,
      p_thrust_time_x, p_thrust_time_y;

//Drag
float a_drag_x = 0, a_drag_y = 0,
      v_drag_x = 0, v_drag_y = 0,
      p_drag_x = 0, p_drag_y = 0,
      rho = 1.225, Cd = 0.4, r = 0.05, area;

//Variables for calculate_bounds function
float height_max, distance_max,
          velocity1y, velocity1x,
          time_peak, time_ground;

//Scale
float pixels_per_meter, arrow_length_v, arrow_length_a;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

bool end_flight, end_thrust;

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

    //Resest bools
    end_flight = true;
    end_thrust = true;

    //Find bounds to scale graph
    calculate_bounds();
    pixels_per_meter = std::min((X_AXIS_LENGTH / distance_max), (Y_AXIS_LENGTH / height_max));

    //Calculate frontal area for drag
    area = PI * r * r;

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
                y = 0.f;

                //Add final point to path
                path.append(Vertex{{x * pixels_per_meter + ORIGIN, 1080.f - ORIGIN}, Color::Red});
                
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

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void get_drag()
{  
    //Estimate current velocity

    //Calculate acceleration due to drag in x and y directions (currently uses previous velocity)
    a_drag_x = - 0.5f * rho * v * vx * Cd * area / m1;
    a_drag_y = - 0.5f * rho * v * vy * Cd * area / m1;

    //Reset position and velocity due to drag to zero at end of thrust (for calculations)
    if (t > thrust_time)
    {
        if (end_thrust)
        {
            //Remember position and velocity at end of thrust
            v_thrust_time_x = vx;
            v_thrust_time_y = vy;

            p_thrust_time_x = x;
            p_thrust_time_y = y;            
            
            //Reset drag position and velocity (we don't want to add p and v from during thrust twice)
            v_drag_x = 0.f;
            v_drag_y = 0.f;
            p_drag_x = 0.f;
            p_drag_y = 0.f;

            //Don't do this again
            end_thrust = false;
        }
    }

    //Change in velocity due to drag
    v_drag_x += a_drag_x * dt;
    v_drag_y += a_drag_y * dt;

    //Change in position due to velocity from drag
    p_drag_x += v_drag_x * dt;
    p_drag_y += v_drag_y * dt;

    //Test
    //std::cout << a_drag_x << std::endl << a_drag_y << std::endl << std::endl << v_drag_x << std::endl << v_drag_y << std::endl << std::endl << p_drag_x << std::endl << p_drag_y << std::endl << std::endl;
}

float acceleration_y(float t)
{
    if (t <= thrust_time)
    {
        return  GRAVITY                                                                                                             //Gravity

                + force / (m1) * sin(launch_angle)                                                                                  //Thrust
                
                + a_drag_y;                                                                                                         //Drag
    }
    else
    {
        return  GRAVITY                                                                                                             //Gravity
                
                + a_drag_y;                                                                                                         //Drag
    }

}

float velocity_y(float t)
{
    if (t <= thrust_time)
    {
        return  GRAVITY * t                                                                                                         //Gravity

                + (force / fuel_per_second) * std::log(m0 / m1) * sin(launch_angle)                                                 //Thrust

                + v_drag_y;                                                                                                         //Drag
    }
    else
    {
        return  GRAVITY * (t - thrust_time)                                                                                         //Gravity

                + v_drag_y                                                                                                          //Drag

                + v_thrust_time_y;                                                                                                  //Constant 1
    }

}

float position_y(float t)
{
    if (t <= thrust_time)
    {
        return  0.5f * GRAVITY * t * t                                                                                              //Gravity

                + (force / fuel_per_second) * sin(launch_angle)                                                                     //Thrust
                * (- std::log(m1 / m0) * t
                  + t
                  + (m0) / fuel_per_second * std::log(m1 / m0))

                + p_drag_y;                                                                                                         //Drag
    }
    else
    {
        return  0.5f * GRAVITY * (t - thrust_time) * (t - thrust_time)                                                              //Gravity

                + p_drag_y                                                                                                          //Drag

                + v_thrust_time_y * (t - thrust_time)                                                                               //Constant 1 * t

                + p_thrust_time_y;                                                                                                  //Constant 2
    }
}

float acceleration_x(float t)
{
    if (t <= thrust_time)
    {
        return  force / (m1) * cos(launch_angle)                                                                                    //Thrust
                
                + a_drag_x;                                                                                                         //Drag
    }
    else
    {
        return  a_drag_x;                                                                                                           //Drag
    }
}

float velocity_x(float t)
{
    if (t <= thrust_time)
    {
        return  + (force / fuel_per_second) * std::log(m0 / m1) * cos(launch_angle)                                                 //Thrust

                + v_drag_x;                                                                                                         //Drag
    }
    else
    {
        return  v_drag_x                                                                                                             //Drag
        
                + v_thrust_time_x;                                                                                                   //Constant 1
    } 
}

float position_x(float t)
{
    if (t <= thrust_time)
    {
        return  (force / fuel_per_second) * cos(launch_angle)                                                                       //Thrust
                * (- std::log(m1 / m0) * t
                  + t
                  + (m0) / fuel_per_second * std::log(m1 / m0))

                + p_drag_x;                                                                                                         //Drag
    }
    else
    {
        return  p_drag_x                                                                                                            //Drag
        
                + v_thrust_time_x * (t - thrust_time)                                                                               //Constant 1 * t

                + p_thrust_time_x;                                                                                                  //Constant 2
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void calculate_bounds()   //This function estimates the max height and distance of the rocket without considering drag
{
    //Set m1 to correct mass at peak height and distance
    m1 = mass_rocket;
    
    //Time of peak
    time_peak = - (force / fuel_per_second) * std::log(m0 / mass_rocket) * sin(launch_angle) / GRAVITY;

    //Max height
    v_thrust_time_y = velocity_y(thrust_time);
    p_thrust_time_y = position_y(thrust_time);
    height_max = position_y(time_peak);

    //Max distance; estimate about double the distance of that at the max height
    v_thrust_time_x = velocity_x(thrust_time);
    p_thrust_time_x = position_x(thrust_time);
    distance_max = 2 * position_x(time_peak);

    //Velocity after boosters stop for velocity arrow scale
    velocity1x = velocity_x(thrust_time);
    velocity1y = velocity_y(thrust_time);

    //Reset m1
    m1 = mass_fuel + mass_rocket;

    //Test
    //std::cout << time_peak << std::endl << height_max << std::endl << distance_max;
}

void update_numbers()
{
    //New time and mass
    t += dt;

    if (t <= thrust_time)
    {
        m1 = m0 - fuel_per_second * t;
    }
    else
    {
        m1 = mass_rocket;
    }

    //Drag
    get_drag();

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

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void draw_path(RenderWindow& window)
{
    if (end_flight)
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