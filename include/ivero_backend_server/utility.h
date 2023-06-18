
#ifndef UTILITY_H
#define UTILITY_H

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
namespace ivero_backend_server {

extern int IP_PORT;
extern std::string STREAM_PORT;
extern std::string PCL_PORT;
extern std::string WORKSPACE;
extern int IMAGE_WIDTH;
extern int IMAGE_HEIGHT;
extern std::string RGB_PROCESS;
extern std::string PCL_PROCESS;

// ANSI escape codes for text colors

// Foreground text colors:

//     Black: \033[30m
//     Red: \033[31m
//     Green: \033[32m
//     Yellow: \033[33m
//     Blue: \033[34m
//     Magenta: \033[35m
//     Cyan: \033[36m
//     White: \033[37m

// Background colors:

//     Black: \033[40m
//     Red: \033[41m
//     Green: \033[42m
//     Yellow: \033[43m
//     Blue: \033[44m
//     Magenta: \033[45m
//     Cyan: \033[46m
//     White: \033[47m

// Additionally, you can use the following codes to modify text style:

//     Bold: \033[1m
//     Underline: \033[4m
//     Reverse: \033[7m

const std::string RED_COLOR = "\033[31m";
const std::string YELLOW_COLOR = "\033[33m";
const std::string GREEN_COLOR = "\033[32m";
const std::string RESET_COLOR = "\033[0m";
const std::string RESET_CYAN = "\033[36m";

#define IVERO_SERVER_PRINT_INFO(message)                                       \
  std::cout << GREEN_COLOR << "[IVERO_SERVER_INFO] " << message << RESET_COLOR \
            << std::endl;
#define IVERO_SERVER_PRINT_WARNING(message)                                    \
  std::cout << YELLOW_COLOR << "[IVERO_SERVER_WARNING] " << message            \
            << RESET_COLOR << std::endl;
#define IVERO_SERVER_PRINT_ERROR(message)                                      \
  std::cerr << RED_COLOR << "[IVERO_SERVER_ERROR] " << message << RESET_COLOR  \
            << std::endl;
#define IVERO_SERVER_PRINT_DATACHANNEL(message)                                \
  std::cout << RESET_CYAN << "[IVERO_SERVER_PCL] " << message << RESET_COLOR   \
            << std::endl;
#define IVERO_SERVER_PRINT_RGB(message)                                        \
  std::cout << RESET_CYAN << "[IVERO_SERVER_RGB] " << message << RESET_COLOR   \
            << std::endl;

void RUN();
void printPattern(const std::string &pattern);
void exitHandler();
void signalHandler(int signal);
std::string getParentPath(const std::string &filePath, int levels);
std::string getValueFromFile(const std::string &content, int levels);
bool IsProcessRunning(const std::string &processName);

} // namespace ivero_backend_server
#endif // UTILITY_H