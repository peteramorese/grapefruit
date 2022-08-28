function binPlotNamedTuples(filepaths, tuple_names, bins)
    if length(tuple_names) > 1 && length(filepaths) ~= length(tuple_names)
        error("Number of tuple names must be single or match the number of filepaths")
    end
    figure()
    hold on
    % data(:,2) is the mu values that need to be discretized
    for j=1:length(filepaths)
        if length(tuple_names) > 1
            data = manualParseTuples(filepaths(j), tuple_names(j));
        else
            data = manualParseTuples(filepaths(j), tuple_names(1));
        end
        if isscalar(bins)
            [bin_numbers, bin_edges] = discretize(data(:,2), bins);
        elseif isvector(bins)
            bin_numbers = discretize(data(:,2), bins);
        else
            error("Incorrect bins type")
        end
        binned_data = cell(1,max(bin_numbers));
        for i=1:length(bin_numbers)
            binned_data{bin_numbers(i)} = [binned_data{bin_numbers(i)} data(bin_numbers(i),1)];
        end
        avg_data = zeros(1,max(bin_numbers));
        keep_pt = zeros(1, max(bin_numbers));
        for i=1:length(binned_data)
            if ~isempty(binned_data{i})
                avg_data(bin_numbers(i)) = mean(binned_data{i});
                keep_pt(bin_numbers(i)) = 1;
            end
        end
        plot(avg_data(logical(keep_pt)))
    end
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