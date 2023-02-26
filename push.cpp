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
  FILE *P;
	P = fopen("commits", "w");
  std::string commit_message;
  int i = 0;
  while (1)
  {

    if (commit_file.eof() || i== 2)
    {
        exit(0);
    }
    i++;
    std::getline(commit_file, commit_message);
    P = fopen("commits", "w");
    fwrite("good", 1 , 5 ,P);
    fclose(P);
    std::cout << commit_message << std::endl;
    std::string timestamp = commit_message.substr(0, 19);
    std::string message = "\"" + commit_message.substr(20) + "\"";
    std::string git_command = "git commit --date=\"" +  timestamp + "\" -m " + message;
    std::cout << git_command << std::endl;
      int result = std::system("git add .");
       result = std::system(git_command.c_str());
       
      result = std::system("git push");
      if (result != 0)
        std::cerr << "Error: Failed to commit changes\n";
    std::cout << "Commit successful: " << message << "\n";
 
  }
  
  commit_file.close();
  out.close();
  return 0;
}
