#include <iostream>
#include <myriad.h>

class MyriadTest : public Myriad::MyrApplication
{
    public:
        MyriadTest()
        {
            MYR_INFO("Hello this is an info message");
            std::cout << "Hello world from myr test" << std::endl;
        }

        ~MyriadTest()
        {
            std::cout << "myr test destructor" << std::endl;
        }

        void Run()
        {
            std::cout << "I ran." << std::endl;
            //Init the engine
            //Open a window
            Myriad::Window *win = new Myriad::Window();
            win->SetFPS(60);
            win->Init(800, 600, "Test Window");
            while (!win->ShouldClose())
            {
                //render
                MYR_WARN("Render");
            }
            
            delete win;

        }
};

Myriad::MyrApplication* Myriad::CreateApplication()
{
    return new MyriadTest();
}
