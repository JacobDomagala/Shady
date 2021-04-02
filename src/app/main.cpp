#include "app/shady.hpp"

int
main(int, char**)
{
   shady::app::Shady shady;

   shady.Init();
   shady.MainLoop();

   return 0;
}

