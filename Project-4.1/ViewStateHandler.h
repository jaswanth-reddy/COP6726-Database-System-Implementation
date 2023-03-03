#pragma once

#include <string.h>
#include <bits/stdc++.h>

class ViewStateHandler
{
private:
    inline static char *filePath = "bin/nation.bin";
    ViewStateHandler() {};
    ~ViewStateHandler() {};

public:
    static char *GetCurrentFilePath();
    static void SetCurrentFilePath(char *path);
    static void SetCurrentFilePath(const char *path);
};