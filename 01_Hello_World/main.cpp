#include "MediaInterface/MediaInterface.h"
#include "Program.h"
#include <stdexcept>
#include <iostream>
#include <memory>

using mi::MediaInterface;
using std::exception;
using std::cout;
using std::endl;

int main(int argc, char* args[])
{
    try
    {
        //Initialise the graphics library, create a window, and run
        //the program defined by the Program class.
        MediaInterface([](MediaInterface& mi){ return std::make_unique<Program>(&mi); });
    }
    catch(exception& e)
    {
        cout << "\nERROR: Exception occurred: " << e.what() << endl;
    }
    catch(...)
    {
        cout << "\nERROR: Unknown exception occurred." << endl;
    }

    return 0;
}




