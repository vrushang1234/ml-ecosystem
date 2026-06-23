#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main() {
  std::ifstream file("../test/test.nn");
  std::string s;
  while (std::getline(file, s)) {
    std::stringstream ss(s);
    std::string word;
    while (ss >> word) {
      if (word == "network") {
        ss >> word;
        std::cout << "Network: " << word << std::endl;
      }
    }
  }
  if (file.eof())
    std::cout << "Reached end of file." << std::endl;
  else
    std::cerr << "Error: File reading failed!" << std::endl;
  file.close();
  return 0;
}
