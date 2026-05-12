// there are no special ini files for non-X versions

int open_ini_sys() {
    return 1;
}

int get_ini_str_sys(const char * /*name*/, char * /* value */) {
    return 1;
}
