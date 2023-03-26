function plotFile(filename, plot_line, plot_scatter)
plot_f = getLines(filename);
s_title = "none";
s_xlabel = "none";
s_ylabel = "none";
figure()
hold on
plot_arr = [];
for i=1:length(plot_f)
    disp(plot_f{i})
    if contains(plot_f{i}, "Title: ")
        s_title = extractAfter(plot_f{i}, "Title: ");
        %title(s_title)
    elseif contains(plot_f{i}, "xLabel: ")
        s_xlabel = extractAfter(plot_f{i}, "xLabel: ");
        %xlabel(s_xlabel)
        xlabel("Preference Cost")
    elseif contains(plot_f{i}, "yLabel: ")
        s_ylabel = extractAfter(plot_f{i}, "yLabel: ");
        ylabel(s_ylabel)
    else 
        plot_arr = [plot_arr; [str2double(extractBefore(plot_f{i}, ",")), str2double(extractAfter(plot_f{i}, ", "))]];
    end
end
if ~isempty(plot_arr)
    if plot_line
        plot(plot_arr(:,1), plot_arr(:,2));
    end
    if plot_scatter
        scatter(plot_arr(:,1), plot_arr(:,2), 'filled')
    end
end
