#include "app/shady.hpp"

int
main(int /*argc*/, char** /*argv*/)
{
   shady::app::Shady shady;

   shady.Init();
   shady.MainLoop();

   return 0;
}
