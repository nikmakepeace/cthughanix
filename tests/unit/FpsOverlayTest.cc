#include "FpsOverlay.h"

#include <assert.h>
#include <string.h>

static void testDisabledFpsOverlayAddsNoCommands() {
    OverlayCommands commands;

    FpsOverlay::append(commands, 25.0, 0);

    assert(commands.empty());
}

static void testEnabledFpsOverlayAddsTopRightCommand() {
    OverlayCommands commands;

    FpsOverlay::append(commands, 25.5, 1);

    assert(commands.count() == 1);
    assert(strcmp(commands.at(0).text.c_str(), "fps: 25.50") == 0);
    assert(commands.at(0).y == 0.0);
    assert(commands.at(0).justification == 'r');
    assert(commands.at(0).color == 0);
    assert(commands.at(0).noDarken == 1);
}

int main() {
    testDisabledFpsOverlayAddsNoCommands();
    testEnabledFpsOverlayAddsTopRightCommand();
    return 0;
}
