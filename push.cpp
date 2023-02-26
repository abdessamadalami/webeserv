#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <filename> <repository_path>\n";
    return 1;
  }

  std::ifstream commit_file(argv[1]);
  std::ofstream out("commits");

  if (!commit_file.is_open()) {
    std::cerr << "Error: Could not open file " << argv[1] << "\n";
    return 1;
  }

  std::string commit_message;
  while (1)
  {

    if (commit_file.eof())
    {
        exit(0);
    }

    std::getline(commit_file, commit_message);
    out << commit_message ;
    std::cout << commit_message << std::endl;
    std::string timestamp = commit_message.substr(0, 19);
    std::string message = "\"" + commit_message.substr(20) + "\"";
    std::string git_command = "git commit --date=\"" +  timestamp + "\" -m " + message;
    std::cout << git_command << std::endl;
      int result = std::system("git add .");
    result = std::system(git_command.c_str());
      if (result != 0) {
        std::cerr << "Error: Failed to commit changes\n";
    std::cout << "Commit successful: " << message << "\n";
   }
  }
  commit_file.close();
  out.close();
  return 0;
}
