
#include <ncurses.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
// Add these includes at the top
#include <cstdlib>    // For system()
#include <unistd.h>   // For fork(), exec()
#include <sys/wait.h> // For waitpid()

namespace fs = std::filesystem;


void show_loading_message(int row, const std::string& message) {
    move(row, 0);
    clrtoeol();
    attron(A_BOLD | A_BLINK);
    mvprintw(row, 0, "%s", message.c_str());
    attroff(A_BOLD | A_BLINK);
    refresh();
}

void clear_loading_message(int row) {
    move(row, 0);
    clrtoeol();
    refresh();
}

  void draw_search(int rows){
    mvprintw(rows-1,0,"search:");
               clrtoeol(); // clear rest of line
               refresh();
  }

   void draw_paths(std::vector<std::string>& paths,std::string query){


       int y = 0;

                for (const auto& path : paths) {
                        if (!query.empty() && path.find(query) != std::string::npos) {
                                mvprintw(y++, 0, "%s", path.c_str());
                }
                    }
   }

   void draw_paths_with_selection(const std::vector<std::string>& filtered_paths, int selected) {

    for (int i = 0; i < filtered_paths.size(); ++i) {
        move(i, 0);
        clrtoeol();
        if (i == selected) {
            attron(COLOR_PAIR(1));
            mvprintw(i, 0, "%s", filtered_paths[i].c_str());
            attroff(COLOR_PAIR(1));
        } else {
            mvprintw(i, 0, "%s", filtered_paths[i].c_str());
        }
    }
}



void redraw_paths(std::vector<std::string>& paths, const std::string& selected_path)
{
  paths.clear();
  int rows,cols;
   getmaxyx(stdscr, rows, cols);

   show_loading_message(rows-1,"loading full paths .....");
   size_t entry_length = 3;
  for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(selected_path)) {
                 std::string path_str = dir_entry.path().string();
                  paths.push_back(path_str);          // store the path
                 // std::cout << path_str << '\n';      // optional print
                  if (entry_length < path_str.length())
                    entry_length = path_str.length();
                     }



  clear_loading_message(rows);
}


void open_file(const std::string& file_path) {
    // Check if it's an executable file
    if (access(file_path.c_str(), X_OK) == 0) {
        // It's executable - run it directly
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            endwin(); // Exit ncurses mode
            execl(file_path.c_str(), file_path.c_str(), NULL);
            exit(1); // If execl fails
        } else if (pid > 0) {
            // Parent process - don't wait, continue with file manager
            // (detach the process so it runs in background)
        }
    } else {
        // It's a regular file - try to open with default application
        std::string command = "xdg-open '" + file_path + "' &"; // & to run in background
        system(command.c_str());
    }
}


   int main(){

            initscr();            // start ncurses
            noecho();             // don’t print typed characters automatically
            cbreak();             // disable line buffering
            keypad(stdscr, TRUE); // allow arrow keys
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLUE); // call once at start
               std::vector<std::string> paths;
               std::vector<std::string> filtered_paths;
               std:: vector<std::string> all_paths;


               size_t entry_length = 3;
               std::string actual_path = fs::current_path();
               for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(actual_path)) {
                 std::string path_str = dir_entry.path().string();
                  paths.push_back(path_str);          // store the path
                 // std::cout << path_str << '\n';      // optional print
                  if (entry_length < path_str.length())
                    entry_length = path_str.length();
                     }

                    all_paths = paths; // save paths

               std::string query;
               int rows,cols;
                int window_state;

              // ??  rows will be equal to the maximum number of rows in terminal

               int ch;
               goto search;
               search:
                  getmaxyx(stdscr, rows, cols) ;
               mvprintw(rows-1,0,"search:");
               clrtoeol(); // clear rest of line
               refresh();



               move(rows-1,7);// moves the cursor to the y coordinates 8  ater : exactly
               while((ch=getch()) !='\n'){
                 if(ch == KEY_RESIZE){


                   mvprintw(rows-1,0,"");
                   clrtoeol();

                   goto search;
                 }
                 if(ch == KEY_BACKSPACE || ch == 127){
                      if (!query.empty()) query.pop_back();
                }else if(ch == (int)('A' & 0x1F)){

                        if(actual_path == fs::path(actual_path).root_path()){
                          goto search;
                        }else{
                        actual_path = fs::path(actual_path).parent_path();
                         clear();
                        redraw_paths(paths,actual_path);
                        all_paths = paths;
                        goto search;
                        }

                    }
                else{

                    query.push_back(ch);

                    }
                     for(int y = 0; y < rows - 1; ++y) {
                      move(y, 0);
                      clrtoeol();


                     }
                    std::thread drawing_paths_lag([&](){
                      draw_paths(paths,query);
                    });
                    drawing_paths_lag.join();
                    filtered_paths.clear();
                    for (const auto& path : paths) {
                      if (path.find(query) != std::string::npos) {
                       filtered_paths.push_back(path);
                          }
                      }




                 mvprintw(rows-1, 0, "Search: %s", query.c_str());

                 clrtoeol();
                 move(rows-1, 8 + query.length()); // cursor after input
                 refresh();



               }
               /////// end of search ///////
               goto select;
               select:

               int selected = 0;
               move(selected,0);
               while((ch=getch()) !='\n'){



                   if(ch == KEY_DOWN)
                   {
                        if(selected>=0 && (selected <= filtered_paths.size()))selected++;
                        move(selected,0);
                        draw_paths_with_selection(filtered_paths,selected);



                  }else if(ch == KEY_UP){
                    if(selected>0)
                    {selected--;};
                    move(selected,0);
                     draw_paths_with_selection(filtered_paths,selected);
                  }else
                  {
                    if(ch == (int)('X' & 0x1F))
                    {
                      query = "";
                      draw_search(rows);
                      paths = all_paths;


                      goto search;
                    }else if(ch == (int)('A' & 0x1F)){

                        if(actual_path == fs::path(actual_path).root_path()){
                          goto search;
                        }else{
                          actual_path = fs::path(actual_path).parent_path();
                          clear();
                        redraw_paths(paths,actual_path);
                        std::thread assignment_thread([&](){
                           all_paths = paths;
                        });
                        assignment_thread.join();
                        goto search;

                        }


                  }

               }}

               if(ch == '\n'){
                 std::string selected_path = filtered_paths[selected];
                if (fs::is_directory(selected_path)) {
                  redraw_paths(paths, selected_path);
                  filtered_paths = paths; // reset filtered list
                  selected = 0;
                  query.clear();          // optional
                  goto select;
                    }
                    else if(fs::is_regular_file(selected_path))
                     {
                                open_file(selected_path);
                                goto select;
                    }
              }



      return 0;
   }
