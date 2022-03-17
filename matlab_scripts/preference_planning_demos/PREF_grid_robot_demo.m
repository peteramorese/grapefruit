clear; clc; %close all;

%%%%%%%%%%%%%%%
GRID_SIZE = 10;
animate = false;
%%%%%%%%%%%%%%%

plan = importdata("plan_files/plan.txt");

end_plan_found = false;
end_plan_ind = -1;
LOI = [];
for i=1:length(plan)
    if contains(plan{i}, "ap")
        if (~end_plan_found)
            end_plan_ind = i;
            end_plan_found = true;
            LOI = zeros(length(plan)-i, 3);
        end
        x_str = extractBetween(plan{i}, "x", "_");
        y_str = extractBetween(plan{i}, "y", "_");
        prio = extractAfter(plan{i}, "prio");
        LOI(i-end_plan_ind+1, :) = [str2double(x_str), str2double(y_str), str2double(prio)];
    end
end


states = zeros(end_plan_ind, 2);
directions = zeros(end_plan_ind, 4); % x, y, u, v
arrow_length = .5;
arrow_grid_offset = .05;
text_offset_x = .1;
text_offset_y = .3;
states(1,:) = [0, 0]; % Init state



for i = 1:end_plan_ind
    if (contains(plan{i}, "move_up"))
        states(i+1, :) = states(i, :) + [0, 1];
        directions(i+1, 1:2) = states(i, :) + [-arrow_grid_offset, 0];
        directions(i+1, 3:4) = arrow_length * [0, 1];

    end
    if (contains(plan{i}, "move_down"))
        states(i+1, :) = states(i, :) + [0, -1];
        directions(i+1, 1:2) = states(i, :) + [arrow_grid_offset, 0];
        directions(i+1, 3:4) = arrow_length * [0, -1];

    end
    if (contains(plan{i}, "move_right"))
        states(i+1, :) = states(i, :) + [1, 0];
        directions(i+1, 1:2) = states(i, :) + [0, arrow_grid_offset];
        directions(i+1, 3:4) = arrow_length * [1, 0];

    end
    if (contains(plan{i}, "move_left"))
        states(i+1, :) = states(i, :) + [-1, 0];
        directions(i+1, 1:2) = states(i, :) + [0, -arrow_grid_offset];
        directions(i+1, 3:4) = arrow_length * [-1, 0];

    end
end
states = states + .5;
directions(:,1:2) = directions(:,1:2) + .5;
LOI(:,1:2) = LOI(:,1:2) + .5;
figure()

hold on
% x_vec = 0:GRID_SIZE-1;
% y_vec = 0:GRID_SIZE-1;

scatter(states(1,1), states(1,2), 160, 'r', "filled",'d')
text(states(1,1), states(1,2), "Init State")

text_array = {[0, 0] , ""};
for i=1:length(LOI(:,1))
    I = 0;
    if i~=1
        [~, I] = ismember([LOI(i,1) + text_offset_x, LOI(i,2) + text_offset_y], text_array{1,1}(1:end, 1:2), 'rows');
    end


    if (I == 0) || (i == 1)
        temp_str = sprintf("Prio: %d", LOI(i,3));
        text_array{1,1} = [text_array{1,1}; [LOI(i,1)+text_offset_x, LOI(i,2)+text_offset_y]];
        text_array{1,2} = [text_array{1,2}; temp_str];
    else
        temp_str = sprintf(" & %d", LOI(i,3));
        text_array{1,2}(I) = text_array{1,2}(I) + temp_str;
    end

end
axis([0 GRID_SIZE 0 GRID_SIZE])
for i=2:length(text_array{1,1}(:,1))
    text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
    scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 80, "filled", "color",'r')
end
% scatter(LOI(:,1), LOI(:,2), 80, "filled", "color", 'r')
% scatter(states(:,1), states(:,2),40,'filled', "Color",'k')
if (~animate)
    plot(states(:,1), states(:,2),"LineWidth", 8, "Color",'c')
    H = quiver(directions(:,1),directions(:,2),directions(:,3),directions(:,4), 0, 'r');
else

    hold on
    for i=1:length(states(:,1))
        plot(states(1:i,1), states(1:i,2),"LineWidth", 8, "Color",'c')
        H = quiver(directions(1:i,1),directions(1:i,2),directions(1:i,3),directions(1:i,4), 0, 'r');
        pause(.07)
    end
    for i=2:length(text_array{1,1}(:,1))
    text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
    scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 80, "filled", "color",'r')
end
end
for i=2:length(text_array{1,1}(:,1))
    text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
    scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 80, "filled", "color",'r')
end

grid on
xticks(0:GRID_SIZE)
yticks(0:GRID_SIZE)
title("Grid Robot Trajectory")
xlabel("X")
ylabel("Y")

