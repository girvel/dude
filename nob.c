#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(".build")) return 1;

    Nob_Cmd cmd = {0};
    nob_cc(&cmd);
    nob_cmd_append(&cmd, "-c");
    nob_cc_flags(&cmd);
    nob_cc_inputs(&cmd, "main.c");
    nob_cc_output(&cmd, ".build/main.o");
    if (!nob_cmd_run(&cmd)) return 1;

    if (!nob_mkdir_if_not_exists(".build/src")) return 1;
    if (!nob_mkdir_if_not_exists(".build/src/systems")) return 1;

    nob_cc(&cmd);
    nob_cmd_append(&cmd, "-c");
    nob_cc_flags(&cmd);
    nob_cc_inputs(&cmd, "src/systems/display_grid.c");
    nob_cc_output(&cmd, ".build/src/systems/display_grid.o");
    if (!nob_cmd_run(&cmd)) return 1;

    nob_cc(&cmd);
    nob_cmd_append(&cmd, "-c", "-x", "c", "-DNOB_IMPLEMENTATION");
    nob_cc_flags(&cmd);
    nob_cc_inputs(&cmd, "nob.h");
    nob_cc_output(&cmd, ".build/nob.o");
    if (!nob_cmd_run(&cmd)) return 1;

    nob_cc(&cmd);
    nob_cc_inputs(&cmd, ".build/nob.o", ".build/main.o", ".build/src/systems/display_grid.o", "-lraylib", "-lm");
    nob_cc_output(&cmd, "dude");
    if (!nob_cmd_run(&cmd)) return 1;
}
