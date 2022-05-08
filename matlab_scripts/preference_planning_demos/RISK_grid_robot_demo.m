clear; close all; clc;


%%%%%%%%%%%%%%%
animate = true;
N_ANIMATE = 25;
robot_size = 200;

arrow_length = .9;
arrow_grid_offset = 0.0;
text_offset_x = .1;
text_offset_y = .3;
no_animate_line_width = 6;
%%%%%%%%%%%%%%%

plan = importdata("plan_files/strat_traj_execution.txt");

GRID_SIZE = str2double(extractAfter(plan{end}, "GRID_SIZE_"));


end_plan_found = false;
end_plan_ind = -1;
LOI = [];
LOI_lbl = string([]);
act_clr = [];
for i=1:length(plan)
    if contains(plan{i}, "ap")
        if (~end_plan_found)
            end_plan_ind = i;
            end_plan_found = true;
            LOI = zeros(length(plan)-i, 2);
        end
        x_str = extractBetween(plan{i}, "x", "_");
        y_str = extractBetween(plan{i}, "y", "_");
        if contains(plan{i}, "safety")
            AP_type = "Bad";
        elseif contains(plan{i}, "liveness")
            AP_type = "Goal";
        end
%         AP_type = extractAfter(plan{i}, "prio");
        LOI(i-end_plan_ind+1, :) = [str2double(x_str), str2double(y_str)];
        LOI_lbl(i-end_plan_ind+1) = AP_type;
    elseif contains(plan{i}, "SYS")
        act_clr(i,:) = [0 0.4470 0.7410];
    elseif contains(plan{i}, "ENV")
        act_clr(i,:) = .7*[1 0 1];
    end
end
% act_clr(end+1,:) = [0 0 1];


states = zeros(end_plan_ind, 2);
directions = zeros(end_plan_ind, 4); % x, y, u, v

states(1,:) = [0, 0]; % Init state



for i = 1:end_plan_ind
    if (contains(plan{i}, "move_up"))
        states(i+1, :) = states(i, :) + [0, 1];
        directions(i+1, 1:2) = states(i, :) + [-arrow_grid_offset, 0];
        directions(i+1, 3:4) = arrow_length * [0, 1];
    elseif (contains(plan{i}, "move_down"))
        states(i+1, :) = states(i, :) + [0, -1];
        directions(i+1, 1:2) = states(i, :) + [arrow_grid_offset, 0];
        directions(i+1, 3:4) = arrow_length * [0, -1];
    elseif (contains(plan{i}, "move_right"))
        states(i+1, :) = states(i, :) + [1, 0];
        directions(i+1, 1:2) = states(i, :) + [0, arrow_grid_offset];
        directions(i+1, 3:4) = arrow_length * [1, 0];
    elseif (contains(plan{i}, "move_left"))
        states(i+1, :) = states(i, :) + [-1, 0];
        directions(i+1, 1:2) = states(i, :) + [0, -arrow_grid_offset];
        directions(i+1, 3:4) = arrow_length * [-1, 0];
    elseif (contains(plan{i}, "stay_put"))
        states(i+1, :) = states(i, :);
        directions(i+1, 1:2) = states(i, :);
        directions(i+1, 3:4) = arrow_length * [0, 0];
    end

end
states = states + .5;
directions(:,1:2) = directions(:,1:2) + .5;
LOI(:,1:2) = LOI(:,1:2) + .5;
figure(1)

hold on
% x_vec = 0:GRID_SIZE-1;
% y_vec = 0:GRID_SIZE-1;



text_array = {[0, 0] , ""};
for i=1:length(LOI(:,1))
    I = 0;
    if i~=1
        [~, I] = ismember([LOI(i,1) + text_offset_x, LOI(i,2) + text_offset_y], text_array{1,1}(1:end, 1:2), 'rows');
    end
    if (I == 0) || (i == 1)
%         temp_str = sprintf("Prio: %d", LOI(i,3));
        text_array{1,1} = [text_array{1,1}; [LOI(i,1)+text_offset_x, LOI(i,2)+text_offset_y]];
        text_array{1,2} = [text_array{1,2}; LOI_lbl(i)];
    else
%         temp_str = sprintf(" & %d", LOI(i,3));
        if (LOI_lbl(i) == "safety")
            temp_str = "BAD";
        elseif (LOI_lbl(i) == "liveness")
            temp_str = "GOAL";
        else
            temp_str = "unk";
        end
        text_array{1,2}(I) = text_array{1,2}(I) + temp_str;
    end
end
axis([0 GRID_SIZE 0 GRID_SIZE])
for i=2:length(text_array{1,1}(:,1))
    
    if (text_array{2}(i) == "Bad")
        scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 400, [1 .6 .6],"filled")
    else
        scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 400, [.5 1 .5],"filled")
    end
    text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
end
% scatter(LOI(:,1), LOI(:,2), 80, "filled", "color", 'r')
% scatter(states(:,1), states(:,2),40,'filled', "Color",'k')
% directions(end,:) = [];
grid on
xticks(0:GRID_SIZE)
yticks(0:GRID_SIZE)
title("Grid Robot Trajectory")
xlabel("X")
ylabel("Y")
scatter(states(1,1), states(1,2), 160, 'r', "filled",'d')
text(states(1,1) + text_offset_x, states(1,2) + text_offset_y, "Init State")

if (~animate)
%     plot(states(:,1), states(:,2),"LineWidth", 8, 'Color',act_clr(i,:))
    %scatter(states(:,1), states(:,2),"LineWidth", 8, "Color",'c')
    for i=1:size(directions,1)
        if i == 1
            H = quiver(directions(i,1),directions(i,2),directions(i,3),directions(i,4),'off','Color', act_clr(1,:),'LineWidth',no_animate_line_width);
        else
            H = quiver(directions(i,1),directions(i,2),directions(i,3),directions(i,4),'off','Color', act_clr(i-1,:),'LineWidth',no_animate_line_width);
        end
    end
else
    hold on
    erf_mat = linspace(0,1.4,N_ANIMATE);
    for i=1:length(states(:,1))
        if i~=1
            for j=1:N_ANIMATE
                delete(hprev)
                scale = erf(erf_mat(j));
                %             hprev = scatter(states(i,1), states(i,2), 80, act_clr(i,:), 'filled');
                currstate = [states(i-1,1) + scale*(states(i,1)-states(i-1,1)), states(i-1,2) + scale*(states(i,2)-states(i-1,2))];
                hprev = scatter(currstate(1),currstate(2), robot_size, act_clr(i-1,:),'s', 'filled');
                H = quiver(directions(i,1),directions(i,2),scale*directions(i,3),scale*directions(i,4), 'off','Color', act_clr(i-1,:),'LineWidth',scale*3+.0001);
                pause(1/(2*N_ANIMATE))
                delete(H)
                
            end

        else
            hprev = scatter(states(i,1), states(i,2), robot_size, act_clr(i,:), 's', 'filled');
            disp("Press any button to start animation")
            pause()
%             delete(hprev)
        end


        
    end 
end
% for i=2:length(text_array{1,1}(:,1))
%     text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
%     scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 80, "filled", "color",'r')
% end


% for i=2:length(text_array{1,1}(:,1))
%     text(text_array{1,1}(i,1), text_array{1,1}(i,2), text_array{1,2}(i), "FontSize",12);
%     scatter(text_array{1,1}(i,1) - text_offset_x, text_array{1,1}(i,2) - text_offset_y, 80, "filled", "color",'r')
% end
% scatter(states(1,1), states(1,2), 160, 'r', "filled",'d')
% text(states(1,1), states(1,2), "Init State")


