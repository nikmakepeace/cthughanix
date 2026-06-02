// Graphical executable entry point.

#include "Application.h"

int main(int argc, char* argv[]) {
    Application* application = createApplication(argc, argv);

    if (application->initialize())
        application->run();

    int exitStatus = application->exitStatus();
    if (!application->hasExitHandlers())
        destroyApplication();
    return exitStatus;
}
