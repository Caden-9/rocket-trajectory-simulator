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
void menu();
void simulator();

//Physics
void get_drag();
float acceleration_y(float t);
float velocity_y(float t);
float position_y(float t);
float acceleration_x(float t);
float velocity_x(float t);
float position_x(float t);

void calculate_bounds();
void get_scale();
void update_numbers();

/*Screen*/

//LHS
void draw_graph();
void draw_path();
void draw_arrows();

//RHS
void draw_UI();
void draw_status();
void draw_state();
void draw_controls();
void draw_options();
void draw_bottom_buttons();

void draw_plus_minus(float x_box, float y_box, float b_width, float b_height, float r_box);
void rounded_box(float x_box, float y_box, float b_width, float b_height, float r_box, const Color& middle, const Color& border);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Constants
constexpr float GRAVITY = -9.81f, PI = 3.14159265f,

                X_AXIS_LENGTH = 1000.f, Y_AXIS_LENGTH = 950.f,

                TEXT_SIZE = 31.f, HEADING_SIZE = 35.f,

                X_TEXT = 1200.f,
                X_STATUS_NUMBERS = 1830.f,

                GAP_STATUS = 50.f,
                GAP_CONTROL = 60.f,
                GAP_OPTIONS = 60.f,

                Y_STATUS = 100.f,
                Y_CONTROLS = Y_STATUS + GAP_STATUS * 4 + TEXT_SIZE + 52.f,
                Y_OPTIONS = Y_CONTROLS + GAP_CONTROL * 5 + TEXT_SIZE + 60.f,
                Y_BOTTOM_BUTTONS = Y_OPTIONS + GAP_OPTIONS + TEXT_SIZE + 70.f;

const Color UI_GREY(120, 120, 120),
            UI_BLUE(105, 145, 215),
            PLUS_MINUS_GREY(20, 20, 20);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Path points for graph and vertex points for arrows
VertexArray path(PrimitiveType::LineStrip);
Vertex v_arrow[2], a_arrow[2];

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*Variables*/

//Flight information
float dt = 1.f/60.f, t = 0.f, thrust_time = 1.5f,      //Time
      x = 0.f, y = 0.f,                                //Position
      vx = 0.f, vy = 0.f, v = 0.f,                     //Velocity
      ax, ay, a,                                       //Acceleration

      force = 400, mass_rocket = 10, mass_fuel = 10,   //Rocket
      m0 = mass_rocket + mass_fuel,
      m1 = mass_rocket + mass_fuel,
      fuel_per_second = mass_fuel / thrust_time,
      launch_angle = 80 * PI / 180;                    //Angle

//Constants used in calculations for after thrust time ends
float v_thrust_time_x, v_thrust_time_y,
      p_thrust_time_x, p_thrust_time_y;

//Drag
float a_drag_x = 0.f, a_drag_y = 0.f,
      v_drag_x = 0.f, v_drag_y = 0.f,
      p_drag_x = 0.f, p_drag_y = 0.f,
      rho = 1.225f, Cd = 0.4f, r = 0.05f, area;

//Variables for calculate_bounds function
float height_max, distance_max,
          velocity_scale_y, velocity_scale_x,
          time_peak, time_ground;

//Scale and graph
float origin_x = 110.f, origin_y = 86.f,
      pixels_per_meter,
      arrow_length_v, arrow_length_a,
      num_ticks_x, num_ticks_y;

bool in_air, end_thrust, drag_on;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Scale numbers

std::vector<int> scales = {1, 2, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
                           100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950,
                           1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
                           10000};
int num_scales;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*Screen*/

//Window
RenderWindow window(VideoMode({1920, 1080}), "RTS", Style::Titlebar | Style::Close);

//Text
Font roboto;
Text standard_text(roboto);
Text heading_text(roboto);
Text tick_numbers_text(roboto);
std::stringstream stf, srm, sfm, stt, sla;

//Graph
RectangleShape ticks({});
RectangleShape x_axis({X_AXIS_LENGTH + 5.f, 5.f});
RectangleShape y_axis({5.f, -Y_AXIS_LENGTH - 5.f});

//Rounded boxes function
RectangleShape box({});
CircleShape corner({});

//Other lines
RectangleShape seperator({740.f, 2.f});
RectangleShape pm_seperator({});
RectangleShape pm({});

//Bounds used for calculating wheretext should go
FloatRect bounds;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

int main()
{
    //Window
    window.setFramerateLimit(60);

    //Text
    if (!roboto.openFromFile("Fonts/Roboto-Regular.ttf"))
    {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    standard_text.setCharacterSize(TEXT_SIZE);
    heading_text.setCharacterSize(HEADING_SIZE);
    heading_text.setFillColor(UI_BLUE);
    seperator.setFillColor(UI_GREY);
    pm_seperator.setFillColor(UI_GREY);
    tick_numbers_text.setCharacterSize(20);

    //Axis positions
    x_axis.setPosition({origin_x - 5.f, 1080.f - origin_y});
    y_axis.setPosition({origin_x - 5.f, 1085.f - origin_y});

    //Start simulator
    while(window.isOpen())
    {
        menu();
    }
    
    return 0;
}

void menu()
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
                simulator();
            }
        }
    }
}

void simulator()
{
    //Make strings for control numbers
    stf << std::fixed << std::setprecision(0) << force;
    srm << std::fixed << std::setprecision(1) << mass_rocket;
    sfm << std::fixed << std::setprecision(1) << mass_fuel;
    stt << std::fixed << std::setprecision(1) << thrust_time;
    sla << std::fixed << std::setprecision(0) << launch_angle * 180.f / PI;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //Graph Ticks
    num_ticks_x = floor(X_AXIS_LENGTH / 50);
    num_ticks_y = floor(Y_AXIS_LENGTH / 50);
    
    //Find bounds to scale graph
    calculate_bounds();
    get_scale();

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------

    //Add origin to path line
    path.append(Vertex{{origin_x, 1080.f - origin_y}, Color::Green});

    //Arrow colors
    v_arrow[0].color = v_arrow[1].color = Color::Cyan;
    a_arrow[0].color = a_arrow[1].color = Color::Magenta;

    //Reset bools
    in_air = true;
    end_thrust = true;

    //Calculate frontal area for drag
    area = PI * r * r;

    //Simulator Loop
    while(window.isOpen())
    {
        //Check if closed
        while(const std::optional event = window.pollEvent())
        {
            //Close with button
            if (event->is<Event::Closed>())
                window.close();
        }

        if ((y >= 0) && in_air)
        {
        //Update position
        update_numbers();

        //Clear screen
        window.clear(Color::Black);
        
        //Draw Screen
        draw_UI();

        //Display new
        window.display();
        }
        else //Stop and keep updating screen
        {
            if (in_air)
            {
                y = 0.f;

                //Add final point to path instead of point with negative height
                path.resize(path.getVertexCount() - 1);
                path.append(Vertex{{x * pixels_per_meter + origin_x, 1080.f - origin_y}, Color::Red});
                
                //Don't do this again
                in_air = false;
            }
            
            //Clear screen
            window.clear(Color::Black);

            //Draw Screen
            draw_UI();

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

    //Time of max distance
    v_thrust_time_y = velocity_y(thrust_time);
    p_thrust_time_y = position_y(thrust_time);
    time_ground = (-v_thrust_time_y - sqrt(v_thrust_time_y * v_thrust_time_y - 2.f * GRAVITY * p_thrust_time_y)) / (GRAVITY) + thrust_time;

    //Max distance
    v_thrust_time_x = velocity_x(thrust_time);
    p_thrust_time_x = position_x(thrust_time);
    distance_max = position_x(time_ground);

    //Velocity after boosters stop for velocity arrow scale
    velocity_scale_x = velocity_x(thrust_time);
    velocity_scale_y = velocity_y(thrust_time);

    //Reset m1
    m1 = mass_fuel + mass_rocket;

    //Test
    //std::cout << time_peak << std::endl << height_max << std::endl << time_ground << std::endl << distance_max;
}

void get_scale()
{
    num_scales = scales.size();

    if (X_AXIS_LENGTH / distance_max < Y_AXIS_LENGTH / height_max)  
    {  
        for (int i = 0; i < num_scales; i++)
        {
            if (distance_max <= num_ticks_x * scales[i])
            {
                pixels_per_meter = 50.f / scales[i];
                break;
            }
            else
            {
                pixels_per_meter = 50.f / scales[i];
            }
        }
    }
    else
    {
        for (int i = 0; i < num_scales; i++)
        {
            if (height_max <= num_ticks_y * scales[i])
            {
                pixels_per_meter = 50.f / scales[i];
                break;
            }
            else
            {
                pixels_per_meter = 50.f / scales[i];
            }
        }
    }
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

void draw_path()
{
    if (in_air)
    {
        if (t <= thrust_time) //Green during boosters
        {
            path.append(Vertex{{x * pixels_per_meter + origin_x, 1080.f - y * pixels_per_meter - origin_y}, Color::Green});
        }
        else //Red during freefall
        {
            path.append(Vertex{{x * pixels_per_meter + origin_x, 1080.f - y * pixels_per_meter - origin_y}, Color::Red});
        }
    }
    window.draw(path);
}

void draw_arrows()
{
    //Calculate arrow lengths
    arrow_length_v = abs((v / sqrt(velocity_scale_x * velocity_scale_x + velocity_scale_y * velocity_scale_y))) * 100.f; //This number (50.f) is for max pixels long
    arrow_length_a = abs((a / GRAVITY)) * 50.f;
    
    //Make arrow bounds
    v_arrow[0].position = {x * pixels_per_meter + origin_x, 1080.f - y * pixels_per_meter - origin_y};
    v_arrow[1].position = {x * pixels_per_meter  + arrow_length_v * vx / v + origin_x,
                            1080.f - y * pixels_per_meter - arrow_length_v * vy / v - origin_y};

    a_arrow[0].position = {x * pixels_per_meter + origin_x, 1080.f - y * pixels_per_meter - origin_y};
    a_arrow[1].position = {x * pixels_per_meter + arrow_length_a * ax / a + origin_x,
                            1080.f - y * pixels_per_meter - arrow_length_a * ay / a - origin_y};

    //Draw vectors
    window.draw(v_arrow, 2, PrimitiveType::Lines);
    window.draw(a_arrow, 2, PrimitiveType::Lines);
}

void draw_graph()
{
    //Draw axis lines
    window.draw(x_axis);
    window.draw(y_axis);

    //Draw ticks
    for (int i = 1; i < num_ticks_x + 1; i++) //x_axis
    {
        ticks.setPosition({origin_x - 3 + i * 50.f, 1070.f - origin_y});
        ticks.setSize({3.f, 25.f});
        window.draw(ticks);
    }
    for (int i = 1; i < num_ticks_y + 1; i++) //y_axis
    {
        ticks.setPosition({origin_x - 15.f, 1083.f - origin_y - i * 50.f});
        ticks.setSize({25.f, -3.f});
        window.draw(ticks);
    }

    //Draw numbers
    //Create constant to skip every other number if they are too cluttered
    int i = 1;
    int c = 1;
    if (50.f / pixels_per_meter * num_ticks_x >= 1000)
    {
        c = 2;
    }
    
    for (i = 1 * c; i <= num_ticks_x; i += c) //x-axis
    {
        std::stringstream tick_numbers;

        //New number
        tick_numbers << std::fixed << std::setprecision(0) << 50.f / pixels_per_meter * i;
        tick_numbers_text.setString(tick_numbers.str());

        //Place in correct spot
        bounds = tick_numbers_text.getGlobalBounds();
        tick_numbers_text.setPosition({origin_x + i * 50.f - bounds.size.x / 2.f - 3.f, 1103 - origin_y});
        window.draw(tick_numbers_text);
    
    }

    c = 1;
    if (50.f / pixels_per_meter * num_ticks_y >= 1000)
    {
        c = 2;
    }

    for (i = 1 * c; i <= num_ticks_y; i += c) //y-axis
    {
        std::stringstream tick_numbers;

        //New number
        tick_numbers << std::fixed << std::setprecision(0) << 50.f / pixels_per_meter * i;
        tick_numbers_text.setString(tick_numbers.str());

        //Place in correct spot
        bounds = tick_numbers_text.getGlobalBounds();
        tick_numbers_text.setPosition({origin_x - 50.f - bounds.size.x / 2, 1077 - origin_y - bounds.size.y / 2.f - 50.f * i});
        window.draw(tick_numbers_text);
    }
}

void draw_UI()
{
    //Box left and right sides
    rounded_box(20.f, 20.f, 1120.f, 1040.f, 20.f, Color::Black, UI_GREY);
    rounded_box(1160.f, 20.f, 740.f, 1040.f, 20.f, Color::Black, UI_GREY);

    //Seperate right hand side sections
    seperator.setPosition({1160.f, Y_STATUS + GAP_STATUS * 4 + TEXT_SIZE + 30.f});
    window.draw(seperator);
    seperator.setPosition({1160.f, Y_CONTROLS + GAP_CONTROL * 5 + TEXT_SIZE + 40.f});
    window.draw(seperator);
    seperator.setPosition({1160.f, Y_OPTIONS + GAP_OPTIONS + TEXT_SIZE + 40.f});
    window.draw(seperator);

    //LHS
    draw_graph();
    draw_path();
    if ((y >= 0) && in_air)
    {
        draw_arrows();
    }

    //RHS
    draw_status();
    draw_controls();
    draw_options();
    draw_bottom_buttons();
}

void draw_status()
{
    //Title section
    heading_text.setString("STATUS");
    heading_text.setPosition({X_TEXT, Y_STATUS - 60.f});
    window.draw(heading_text);
    
    //Draw status labels
    standard_text.setString("Time:");
    standard_text.setPosition({X_TEXT, Y_STATUS});
    window.draw(standard_text);
    standard_text.setString("Distance:");
    standard_text.setPosition({X_TEXT, Y_STATUS + GAP_STATUS});
    window.draw(standard_text);
    standard_text.setString("Height:");
    standard_text.setPosition({X_TEXT, Y_STATUS + GAP_STATUS * 2});
    window.draw(standard_text);
    standard_text.setString("Velocity:");
    standard_text.setPosition({X_TEXT, Y_STATUS + GAP_STATUS * 3});
    window.draw(standard_text);
    standard_text.setString("Acceleration:");
    standard_text.setPosition({X_TEXT, Y_STATUS + GAP_STATUS * 4});
    window.draw(standard_text);

    draw_state();
}

void draw_controls()
{
    //Title section
    heading_text.setString("CONTROLS");
    heading_text.setPosition({X_TEXT, Y_CONTROLS});
    window.draw(heading_text);
    
    //Draw control labels
    standard_text.setString("Thrust Force:");
    standard_text.setPosition({X_TEXT, Y_CONTROLS + GAP_CONTROL});

            //Get bounds of text for reference to make boxes later
            bounds = standard_text.getGlobalBounds();
            float y_for_control_boxes = bounds.position.y,
                  height_for_control_boxes = bounds.size.y;

    window.draw(standard_text);
    standard_text.setString("Rocket Mass:");
    standard_text.setPosition({X_TEXT, Y_CONTROLS + GAP_CONTROL * 2});
    window.draw(standard_text);
    standard_text.setString("Fuel Mass:");
    standard_text.setPosition({X_TEXT, Y_CONTROLS + GAP_CONTROL * 3});
    window.draw(standard_text);
    standard_text.setString("Thrust Time:");
    standard_text.setPosition({X_TEXT, Y_CONTROLS + GAP_CONTROL * 4});
    window.draw(standard_text);
    standard_text.setString("Launch Angle:");
    standard_text.setPosition({X_TEXT, Y_CONTROLS + GAP_CONTROL * 5});
    window.draw(standard_text);

    //Draw boxes surrounding control numbers
    for (int i = 0; i < 5; i++)
    {
        rounded_box(1440.f, y_for_control_boxes - 13.f + i * GAP_CONTROL, 170.f, height_for_control_boxes + 26.f, 8.f, Color::Black, UI_GREY);
    }

    //Draw plus minus buttons
    for (int i = 0; i < 5; i++)
    {
        draw_plus_minus(1720.f, y_for_control_boxes - 13.f + i * GAP_CONTROL, 130, height_for_control_boxes + 26.f, 8.f);
    }

    //Draw control numbers
    standard_text.setString(stf.str());
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({1525.f - bounds.size.x / 2.f, Y_CONTROLS + GAP_CONTROL});
    window.draw(standard_text);
    standard_text.setString(srm.str());
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({1525.f - bounds.size.x / 2.f, Y_CONTROLS + GAP_CONTROL * 2});
    window.draw(standard_text);
    standard_text.setString(sfm.str());
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({1525.f - bounds.size.x / 2.f, Y_CONTROLS + GAP_CONTROL * 3});
    window.draw(standard_text);
    standard_text.setString(stt.str());
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({1525.f - bounds.size.x / 2.f, Y_CONTROLS + GAP_CONTROL * 4});
    window.draw(standard_text);
    standard_text.setString(sla.str());
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({1525.f - bounds.size.x / 2.f, Y_CONTROLS + GAP_CONTROL * 5});
    window.draw(standard_text);

    //Draw control number labels
    standard_text.setString("N");
    standard_text.setPosition({1630.f, Y_CONTROLS + GAP_CONTROL});
    window.draw(standard_text);
    standard_text.setString("kg");
    standard_text.setPosition({1630.f, Y_CONTROLS + GAP_CONTROL * 2});
    window.draw(standard_text);
    standard_text.setString("kg");
    standard_text.setPosition({1630.f, Y_CONTROLS + GAP_CONTROL * 3});
    window.draw(standard_text);
    standard_text.setString("s");
    standard_text.setPosition({1630.f, Y_CONTROLS + GAP_CONTROL * 4});
    window.draw(standard_text);
    standard_text.setString(U'°');
    standard_text.setPosition({1630.f, Y_CONTROLS + GAP_CONTROL * 5});
    window.draw(standard_text);
}

void draw_plus_minus(float x_box, float y_box, float b_width, float b_height, float r_box)
{
        //Outline
    //Boxes
    box.setOutlineThickness(2.f);
    box.setOutlineColor(UI_GREY);

    box.setSize({b_width - 2.f * r_box, b_height});
    box.setPosition({x_box + r_box, y_box});
    window.draw(box);

    box.setSize({b_width, b_height - 2.f * r_box});
    box.setPosition({x_box, y_box + r_box});
    window.draw(box);    
    
    //Corners
    corner.setOutlineThickness(2.f);
    corner.setOutlineColor(UI_GREY);
    corner.setRadius(r_box);

    corner.setPosition({x_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);

        //Middle
    //Boxes
    box.setOutlineColor(Color::Transparent);
    box.setFillColor(PLUS_MINUS_GREY);

    box.setSize({b_width - 2.f * r_box, b_height});
    box.setPosition({x_box + r_box, y_box});
    window.draw(box);

    box.setSize({b_width, b_height - 2.f * r_box});
    box.setPosition({x_box, y_box + r_box});
    window.draw(box);    

    //Corners
    corner.setOutlineColor(Color::Transparent);
    corner.setRadius(r_box);
    corner.setFillColor(PLUS_MINUS_GREY);

    corner.setPosition({x_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);

    //Seperator
    pm_seperator.setSize({2.f, b_height});
    pm_seperator.setPosition({x_box + b_width / 2.f - 1.f, y_box});
    window.draw(pm_seperator);

    //Symbols
    pm.setSize({22.f, 2.f});
    pm.setPosition({x_box + b_width / 4.f - 11.f, y_box + b_height / 2.f - 1.f});
    window.draw(pm);
    pm.setPosition({x_box + b_width / 4.f * 3.f - 11.f, y_box + b_height / 2.f - 1.f});
    window.draw(pm);
    pm.setSize({2.f, 22.f});
    pm.setPosition({x_box + b_width / 4.f * 3.f - 1.f, y_box + b_height / 2.f - 11.f});
    window.draw(pm);
}

void draw_options()
{
    //Title section
    heading_text.setString("OPTIONS");
    heading_text.setPosition({X_TEXT, Y_OPTIONS});
    window.draw(heading_text);

    //Check box
    rounded_box(X_TEXT + 5.f, Y_OPTIONS + GAP_OPTIONS + 3.f, 35.f, 35.f, 2.f, Color::Black, UI_BLUE);

    //Words
    standard_text.setString("Include Drag");
    standard_text.setPosition({X_TEXT + 60.f, Y_OPTIONS + GAP_OPTIONS});
    window.draw(standard_text);
}

void draw_bottom_buttons()
{
    //Draw box
    rounded_box(X_TEXT, Y_BOTTOM_BUTTONS, 660.f, 90.f, 5.f, UI_BLUE, UI_BLUE);

    //Draw launch
    standard_text.setCharacterSize(60);
    standard_text.setString("LAUNCH");
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_TEXT + 330.f - bounds.size.x / 2.f, Y_BOTTOM_BUTTONS + 5.f});
    window.draw(standard_text);
    standard_text.setCharacterSize(TEXT_SIZE);
}

void rounded_box(float x_box, float y_box, float b_width, float b_height, float r_box, const Color& middle, const Color& border)
{
        //Outline
    //Boxes
    box.setOutlineThickness(2.f);
    box.setOutlineColor(border);

    box.setSize({b_width - 2.f * r_box, b_height});
    box.setPosition({x_box + r_box, y_box});
    window.draw(box);

    box.setSize({b_width, b_height - 2.f * r_box});
    box.setPosition({x_box, y_box + r_box});
    window.draw(box);    
    
    //Corners
    corner.setOutlineThickness(2.f);
    corner.setOutlineColor(border);
    corner.setRadius(r_box);

    corner.setPosition({x_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);

        //Middle
    //Boxes
    box.setOutlineColor(Color::Transparent);
    box.setFillColor(middle);

    box.setSize({b_width - 2.f * r_box, b_height});
    box.setPosition({x_box + r_box, y_box});
    window.draw(box);

    box.setSize({b_width, b_height - 2.f * r_box});
    box.setPosition({x_box, y_box + r_box});
    window.draw(box);    

    //Corners
    corner.setOutlineColor(Color::Transparent);
    corner.setRadius(r_box);
    corner.setFillColor(middle);

    corner.setPosition({x_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box});
    window.draw(corner);
    corner.setPosition({x_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);
    corner.setPosition({x_box + b_width - 2.f * r_box, y_box + b_height - 2.f * r_box});
    window.draw(corner);
}

void draw_state()
{
    //Put current numbers into strings
    std::stringstream st, sx, sy, sv, sa;

    st << std::fixed << std::setprecision(2) << t;
    sx << std::fixed << std::setprecision(2) << x;
    sy << std::fixed << std::setprecision(2) << y;
    sv << std::fixed << std::setprecision(2) << v;
    sa << std::fixed << std::setprecision(2) << a;

    //Draw numbers
    standard_text.setString(st.str() + " s");
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_STATUS_NUMBERS - bounds.size.x, Y_STATUS});
    window.draw(standard_text);
    standard_text.setString(sx.str() + " m");
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_STATUS_NUMBERS - bounds.size.x, Y_STATUS + GAP_STATUS});
    window.draw(standard_text);
    standard_text.setString(sy.str() + " m");
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_STATUS_NUMBERS - bounds.size.x, Y_STATUS + GAP_STATUS * 2});
    window.draw(standard_text);
    standard_text.setString(sv.str() + " m/s");
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_STATUS_NUMBERS - bounds.size.x, Y_STATUS + GAP_STATUS * 3});
    window.draw(standard_text);
    standard_text.setString(String(sa.str() + " m/s") + U'²');
    bounds = standard_text.getGlobalBounds();
    standard_text.setPosition({X_STATUS_NUMBERS - bounds.size.x, Y_STATUS + GAP_STATUS * 4});
    window.draw(standard_text);
}