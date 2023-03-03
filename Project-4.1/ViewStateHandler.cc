#include "ViewStateHandler.h"

char* ViewStateHandler::GetCurrentFilePath()
{
    //std::cout << "Path returning from ViewStateHandler::GetCurrentFilePath() is = " << strdup(filePath) << std::endl;
    return filePath;
}

void ViewStateHandler::SetCurrentFilePath(char *path)
{
    filePath = path;
}

void ViewStateHandler::SetCurrentFilePath(const char *path)
{
    filePath = (char*)path;
    //std::cout << "Path set to ViewStateHandler::SetCurrentFilePath() is = " << strdup(filePath) << std::endl;
}