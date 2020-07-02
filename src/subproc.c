char * yed_run_subproc(char *cmd, int *output_len, int *status) {
    FILE    *stream;
    array_t  out;
    char     c;

    stream = popen(cmd, "r");

    if (stream == NULL)    { return NULL; }

    out = array_make(char);

    c = 0;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\r') {
            continue;
        }
        array_push(out, c);
    }

    if (array_len(out)
    &&  *(char*)array_last(out) == '\n') {
        array_pop(out);
    }

    *output_len = array_len(out);
    *status     = pclose(stream);

    array_zero_term(out);

    return array_data(out);
}
