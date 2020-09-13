#include "file_manager.hpp"
#include "trace/logger.hpp"

#include <fstream>

namespace shady::utils {

std::string
FileManager::ReadFile(const std::string& fileName, FileType type)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      trace::Logger::Fatal("FileManager::ReadFile -> {} can't be opened!", fileName);
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      trace::Logger::Fatal("FileManager::ReadFile -> {} is empty!", returnVal);
   }

   return returnVal;
}

void
FileManager::WriteToFile(const std::string& fileName, const std::string& content, FileType type)
{
   std::ofstream fileHandle;
   fileHandle.open(fileName);
   fileHandle << content;
}

} // namespace shady::utils