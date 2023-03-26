function scatterPlotNamedTuples(filepath, tuple_name)
    data = manualParseTuples(filepath, tuple_name);
    figure()
    scatter(data(:,2), data(:,1), 'filled', 'd')
end

function data = manualParseTuples(filepath, tuple_name)
    %data = importdata(filepaths(j),':');
    data_f = getLines(filepath);
    data = [];
    ind = 1;
    for i=1:length(data_f)
        if contains(data_f{i}, "[Tup]") && contains(data_f{i}, tuple_name)
            data(ind,1) = str2double(extractBetween(data_f{i}, ": {", ","));
            data(ind,2) = str2double(extractBetween(data_f{i}, ",", "}"));
            ind = ind + 1;
        end
    end
end