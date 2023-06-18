#include <ivero_backend_server/utility.h>

namespace ivero_backend_server {

void RUN() {
  while (true) {
  }
}

void printPattern(const std::string &pattern) {
  int length = pattern.length();
  std::cout << std::string(length + 4, '-') << std::endl;
  std::cout << "- " << pattern << " -" << std::endl;
  std::cout << std::string(length + 4, '-') << std::endl;
}

void exitHandler() { printPattern("Ivero Backend Server Stop"); }

void signalHandler(int signal) {
  if (signal == SIGINT) {
    exitHandler();
    std::exit(0); // Terminate the program gracefully
  }
}

std::string getParentPath(const std::string &filePath, int levels) {
  if (levels <= 0) {
    return filePath;
  }

  size_t found = filePath.find_last_of("/\\");
  if (found != std::string::npos) {
    std::string parentPath = filePath.substr(0, found);
    return getParentPath(parentPath, levels - 1);
  }

  return "";
}

std::string getValueFromFile(const std::string &content, int levels) {

  std::string currentFilePath = __FILE__;
  std::string parentPath = getParentPath(currentFilePath, levels);
  std::string configFolderPath = parentPath + "/config/config.txt";

  std::ifstream file(configFolderPath);
  if (!file.is_open()) {
    std::cout << "Failed to open file: " << configFolderPath << std::endl;
    return "";
  }

  std::string line;

  while (std::getline(file, line)) {
    if (line.find(content + "=") != std::string::npos) {
      size_t pos = line.find("="); // Find the position of the equal sign
      if (pos != std::string::npos && pos < line.length() - 1) {
        return line.substr(pos + 1); // Extract the value after the equal sign
      }
    }
  }

  std::cout << "Could not find " << content << " in file: " << configFolderPath
            << std::endl;
  return "";
}

bool IsProcessRunning(const std::string &processName) {
  std::string command = "pgrep -c " + processName;
  FILE *pipe = popen(command.c_str(), "r");

  if (pipe != nullptr) {
    char result[128];
    if (fgets(result, sizeof(result), pipe) != nullptr) {
      int count = std::atoi(result);
      pclose(pipe);
      return (count > 0);
    }

    pclose(pipe);
  }

  return false;
}

int IP_PORT = std::stoi(getValueFromFile("IP_PORT", 2));

std::string STREAM_PORT = getValueFromFile("STREAM_PORT", 2);

std::string PCL_PORT = getValueFromFile("PCL_PORT", 2);

std::string WORKSPACE = getValueFromFile("WORKSPACE", 2);

int IMAGE_WIDTH = std::stoi(getValueFromFile("IMAGE_WIDTH", 2));

int IMAGE_HEIGHT = std::stoi(getValueFromFile("IMAGE_HEIGHT", 2));

std::string RGB_PROCESS = getValueFromFile("RGB_PROCESS", 2);

std::string PCL_PROCESS = getValueFromFile("PCL_PROCESS", 2);

} // namespace ivero_backend_server