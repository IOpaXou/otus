#pragma once

#include "ICommand.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

class LogCommand : public ICommand
{
public:
    static const std::string LogPath()
    {
        static const std::string logPath = std::filesystem::temp_directory_path() / "out.log";
        return logPath;
    }

    LogCommand(const std::exception& ex) : _ex(ex) {}
    void exec() override
    {
        std::ofstream fout;
        fout.open(LogPath(), std::ios::out);
        if (fout.is_open())
        {
            fout << _ex.what() << std::endl;
        }
    }

private:
    const std::exception& _ex;
};
