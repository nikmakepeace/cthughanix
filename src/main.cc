// Graphical executable entry point.

#include "Application.h"

int main(int argc, char* argv[]) {
    // Keep process-level policy inside Application so startup, loop, and
    // shutdown order are documented and tested in one place.
    Application application(argc, argv);

    if (application.initialize())
        application.run();

    return application.exitStatus();
}
