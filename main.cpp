#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
using namespace sf;

int main()
{
    //Window
    RenderWindow window(VideoMode({2000, 1500}), "RTS", Style::Titlebar | Style::Close);

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

        //Update state
        

        //Draw Screen

        window.clear(Color::Black); //Clear old
        //Draw new


        //Display new
        window.display();
    }
    return 0;
}