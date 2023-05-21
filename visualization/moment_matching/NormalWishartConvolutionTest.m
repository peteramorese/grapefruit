clear; close all; clc;

dim = 2;
m = 10;
mc_samples = 10000;

mu_low = 3.0;
mu_hi = 10.0;
sig_low = 0.5;
sig_hi = 1.5;
samp_low = 4;
samp_hi = 6;

% True distributions
mu_true = zeros(dim, 1, m);
Sigma_true = zeros(dim, dim, m);
samples = zeros(dim, 1);
for k = 1:m
    for r = 1:dim
        mu_true(r, 1, k) = (mu_hi - mu_low) * rand() + mu_low;
    end
    Sigma_samples = (sig_hi - sig_low) * rand(10, dim) + sig_low;
    Sigma_true(:,:,k) = cov(Sigma_samples);
    
    samples(k) = randi(samp_hi - samp_low) + samp_hi;
end

% Initial hyper parameters
kappa_0 = 1.0;
mu_0 = zeros(dim, 1);
nu_0 = 3;
Lambda_0 = eye(dim);

x = cell(m);
kappa_n = zeros(1, m);
mu_n = zeros(dim, 1, m);
nu_n = zeros(1, m);
Lambda_n = zeros(dim, dim, m);
for k = 1:m
    n = samples(k);
    x{k} = mvnrnd(mu_true(:,:,k), Sigma_true(:,:,k), n)';
    x_bar = mean(x{k}, 2);
    mu_n(:, :, k) = kappa_0 / (kappa_0 + n) * mu_0 + n / (kappa_0 + n) * x_bar;
    nu_n(k) = nu_0 + n;
    kappa_n(k) = kappa_0 + n;
    
    Sum = zeros(dim, dim);
    for s = 1:n
        Sum = Sum + (x{k}(:,s) - x_bar) * (x{k}(:,s) - x_bar)';
    end
    Lambda_n(:, :, k) = Lambda_0 + Sum + n * kappa_0 / (n + kappa_0) * (x_bar - mu_0) * (x_bar - mu_0)';
end

% Sample
mu = zeros(dim, mc_samples);
Sigma = zeros(dim, dim, mc_samples);
for s = 1:mc_samples
    mu(:,s) = zeros(dim, 1);
    Sigma(:, :, s) = zeros(dim, dim);
    for k = 1:m
        [mu_nw, Sigma_nw] = sampleNormalIWishart(kappa_n(:,k), mu_n(:,:,k), nu_n(:,k), Lambda_n(:,:,k));
        mu(:,s) = mu(:, s) + mu_nw;
        Sigma(:, :, s) = Sigma(:, :, s) + Sigma_nw;
    end
end

% Visualize
figure()
n_figures = dim + uniqueCovParams(dim);
for d = 1:dim
    subplot(n_figures, 1, d);
    histogram(mu(d,:));
    title(sprintf("mean %d", d));
end
f = 1;
for r = 1:dim
    for c = r:dim
        disp(f+dim)
        subplot(n_figures, 1, f+dim);
        histogram(Sigma(r, c, :));
        title(sprintf("covariance %d", f));
        f = f + 1;
    end
end

% Compare
mu_mvn = zeros(dim + dim^2, 1);
Sigma_mvn = zeros(dim + dim^2, dim + dim^2);
for k = 1:m
    [mu_mvn_k, Sigma_mvn_k] = normalIWishartToMVN(kappa_n(:,k), mu_n(:,:,k), nu_n(:,k), Lambda_n(:,:,k));
    disp(mu_mvn_k);
    mu_mvn = mu_mvn + mu_mvn_k;
    Sigma_mvn = Sigma_mvn + Sigma_mvn_k;
end

% Mean (2D)
figure()
hold on
scatter(mu(1,:), mu(2, :));
scatter(mu_mvn(1), mu_mvn(2), "rD");

x1 = linspace(50,70,1000);
x2 = linspace(50,70,1000);
[X1, X2] = meshgrid(x1, x2);
X = [X1(:) X2(:)];
y = mvnpdf(X, mu_mvn(1:2)', Sigma_mvn(1:2, 1:2));
y = reshape(y,length(x2),length(x1));
contour(x1, x2, y, [0.0001 0.001 0.01 0.05 0.15 0.25 0.35]);

% Covariances (2D)
figure()
hold on
scatter(Sigma(1,1,:), Sigma(1,2, :), "filled");
scatter(mu_mvn(3), mu_mvn(4), "rD");

x1 = linspace(-6,6,1000);
x2 = linspace(-15,15,1000);
[X1, X2] = meshgrid(x1, x2);
X = [X1(:) X2(:)];
y = mvnpdf(X, mu_mvn([3, 4])', Sigma_mvn([3, 4], [3, 4]));
y = reshape(y,length(x2),length(x1));
contour(x1, x2, y, [0.0001 0.001 0.01 0.05 0.15 0.25 0.35]);


function [mu_params, Sigma_params] = normalIWishartToMVN(kappa_n, mu_n, nu_n, Lambda_n)
    dim = length(mu_n);
    params_dim = dim + dim^2;
    mu_params = zeros(params_dim, 1);
    Sigma_params = zeros(params_dim, params_dim);
    Psi = inv(Lambda_n);
    
    % Mean of student-t marginal (mu_n)
    mu_params(1:dim) = mu_n;
    
    % Mean of Wishart marginal
    mean_wishart = Psi / (nu_n - dim - 1);
    f = 1;
    for r = 1:dim
        for c = 1:dim
            mu_params(dim + f) = mean_wishart(r, c);
            fprintf("i: %d corresponds to sig(%d, %d)\n",f, r, c);
            f = f + 1;
            
        end
    end
    
    % Variance of student-t marginal
    st_nu = nu_n - dim + 1;
    st_Sigma = Psi / (kappa_n * (nu_n - dim + 1));
    Sigma_params(1:dim, 1:dim) = st_nu / (st_nu - 2) * st_Sigma;
    
    % Variance of Wishart marginal
    ws_Sigma = zeros(dim^2, dim^2);
    fr = 1;
    for i = 1:dim
        for j = 1:dim
            fc = 1;
            for k = 1:dim
                for l = 1:dim
                    if (i == j && k == l && i == k)
                        ws_Sigma(fr, fc) = 2 * Psi(i, i)^2 / ((nu_n - dim - 1)^2 * (nu_n - dim - 3));
                    elseif (i == k && j == l) 
                        ws_Sigma(fr, fc) = ((nu_n - dim + 1) * Psi(i, j)^2 + ...
                            (nu_n - dim - 1) * Psi(i, i) * Psi(j, j)) ...
                            / ((nu_n - dim) * (nu_n - dim - 1)^2 * (nu_n - dim - 3));    
                    else
                        ws_Sigma(fr, fc) = (2 * Psi(i, j) * Psi(k, l) + ...
                            (nu_n - dim - 1) * (Psi(i, k) * Psi(j, l) + Psi(i, l) * Psi(k, j))) ...
                            / ((nu_n - dim) * (nu_n - dim - 1)^2 * (nu_n - dim - 3));
                    end
                    fprintf("r: %d, c: %d corresponds to sig(%d, %d)*sig(%d, %d)\n",fr, fc, i, j, k, l);
                    fc = fc + 1;
                end
            end
            fr = fr + 1;
        end
    end
    Sigma_params(dim+1:params_dim, dim+1:params_dim) = ws_Sigma;
end

function [mu, Sigma] = sampleNormalIWishart(kappa_n, mu_n, nu_n, Lambda_n)
    Sigma = iwishrnd(inv(Lambda_n), nu_n);
    mu = mvnrnd(mu_n, 1/kappa_n * Sigma)';
end

function n = uniqueCovParams(d)
    n = sum(1:d);
end
