/**  Brief: Application Main Window
 *   Author: Caio Rodrigues - caiorss [at] rodrigues [at] gmail [dot] com
 *
 *
 ************************************************************/
#include <iostream>
#include <string>
#include <vector>

#include <QApplication>
#include "appmainwindow.hpp"

/// ---- Unix-only --------//
#if defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#endif 

constexpr const char* APP_NAME = "qqemu";

void detach_terminal(std::vector<std::string> args = {});

int main(int argc, char** argv)
{

    QSharedMemory shmem;
    shmem.setKey("qemu-launcher-key");

    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);   

    // If application is running on a PTY (terminal)
    // fork another process of this executable and  detach 
    // terminate this process.
    if( isatty(STDIN_FILENO) )
    {
        detach_terminal();
    }

    // Ensure that only a single application runs.
    #if 0 
    if( !shmem.create(1) )
    {
        QMessageBox::warning( nullptr
                             ,"Error report"
                             ,"Only one application instance is allowed.");

        std::cerr << " [ERROR] Only one application instance is allowed." << std::endl;
        return -1;
    }
    #endif 


    AppMainWindow maingui;
    maingui.setWindowIcon(QIcon(":/assets/appicon.png"));
    maingui.showNormal();


    return app.exec();
}

  /// --- Additional functions ------------------------//

/// @brief => Detach current application from terminal by forking its process 
/// and running its executable again in the child process. 
/// Note: This function only works on Linux, but it can be adapated for other
/// Unix-like operating systems.
void detach_terminal(std::vector<std::string> args)
{
    #if defined(__unix__) 
    std::printf(" [TRACE] <BEFORE FORK> PID of parent process = %d \n", getpid());

    auto read_symlink = [](std::string const& path) -> std::string 
    {
        // Create a buffer with size PATH_MAX + 1 filled with 0 ('\0'), null characters
        std::string buffer(PATH_MAX, 0);
        // ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
        ssize_t nread = ::readlink(path.c_str(), &buffer[0], PATH_MAX);
        if(nread == -1){
            fprintf(stderr, " Error: %s \n", strerror(errno));
            throw std::runtime_error("Error: unable to read symlink. Check 'errno' variable");
        }
        buffer.resize(nread);
        return buffer;
    }; 

    auto app = read_symlink("/proc/self/exe");

    // PID of child process (copy of this process)
    pid_t pid = fork();

    if (pid == -1)
    {
        std::fprintf(stderr, "Error: unable to launch process");
        throw std::runtime_error("Error: unable to launch process");
    }
    if (pid == 0)
    {
        // std::printf(" [TRACE] Running on child process => PID_CHILD = %d \n", getpid());

        // Close file descriptors, in order to disconnect the process from the terminal.
        // This procedure allows the process to be launched as a daemon (aka service).
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(STDIN_FILENO);

        // Execvp system call, replace the image of this process with a new one
        // from an executable.

        std::vector<const char *> pargs;
        pargs.reserve(args.size() + 1);
        pargs.push_back(app.c_str());
        for (auto const &a : args)
        {
            pargs.push_back(a.c_str());
        }
        pargs.push_back(nullptr);

        // Signature: int execvp(const char *file, char *const argv[]);

        // execvp(app.c_str(), execvp(app.c_str(), (char* const *) pargs.data() )
        int status = execvp(app.c_str(), (char *const *)pargs.data());
        if (status == -1)
        {
            std::fprintf(stderr, " Error: unable to launch process");
            throw std::runtime_error("Error: failed to launch process");
        }
        return;
    }

    // Terminate parent application
    exit(0);
    // -------- Parent process ----------------//
    #endif
}
