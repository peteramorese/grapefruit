function line_cell = getLines(filename)
    file_id = fopen(filename);
    line_cell = {};
    tline = fgetl(file_id);
    l = 1;
    while ischar(tline)
        line_cell{l} = tline;
        tline = fgetl(file_id);
        l = l + 1;
    end
    fclose(file_id);
end
