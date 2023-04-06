function stochastic_sims
% Script file for designing and testing various random processes
% for the CAN simulator.

% Parameters
N = 1000; % Number of points to simulate
x_min = 0; % Minimum bound of process (reduce uncertainty, avoid UB)
x_max = 4; % Maximum bound of process
x0 = 2; % Initial condition
a = .7; % Increment amount for floating-point operations

% Generate realization of random process
% x = rw_int(N, x_min, x_max, x0);
% x = rw_slint(N, x_min, x_max, x0);
% x = rp_inc_can(N);
% x = rw_uslint(N, 4, x_min, x_max, x0);
% x = dq_saw(N, 8, x_min, x_max);
% x = dc_saw(N, a, x_min, x_max);
x = rp_uslfl(N, a, 4, x_min, x_max, x0);

% Analyze qualitatively the sample realization, including its distribution
stoch_analysis(x, x_min, x_max, 1);

end

function stoch_analysis(x, x_min, x_max, x_diff)
% Gathers basic insights of realization x to determine its concordance
% with the specifications and identify stability issues
% 
% x      = stochastic realization
% x_min  = minimum bound
% x_max  = maximum bound
% x_diff = difference between bins, used for histogram

% Histogram for values
% Look for unusual or uneven spikes suggesting unwanted convergence
b_vals = x_min:x_diff:x_max; % Bin locations (discrete)
b_freq = zeros(1, length(b_vals)); % Count per bin
b_out = 0; % Count of values outside of bin (usually this is erroneous)

ii = 1; jj = 1; % Initialize outside of scope to check values outside loop
for ii = 1:length(x)
    for jj = 1:length(b_vals)
        % Check if value is in bin; conditional expression varies
        % depending on whether or not the process is quantized
        if (x(ii) >= b_vals(jj)) && (x(ii) < b_vals(jj) + x_diff);
        % if x(ii) == b_vals(jj)
            b_freq(jj) = b_freq(jj) + 1;
            break % No other bins need to be checked
        end
    end

    % Determine if the value lies outside of the bins
    if jj > length(b_vals)
        b_out = b_out + 1;
    end
end

% Plot histogram
figure(1)
plot(b_vals, b_freq)
xlabel("Sequence values")
ylabel("Frequency")
title("Distribution of discretized numbers in x")

% Relay if any erroneous values were discovered
fprintf("Number of values not in expected distribution: %d\n", b_out);

% Histogram of differences
% Check if stochastic differentials are as defined
% as well as their robustness if the random process is bounded
% Take difference
dx = x(2:end) - x(1:end-1);

% Plot
figure(2)
histogram(dx)
xlabel("Bins")
ylabel("Frequency")
title("Distribution of differences x'[n] = x[n] - x[n - 1]")

% Time-evolution plots
% Look for intervals of unusual or undefined behavior, instability, etc.
figure(3)
subplot(2, 1, 1); % Values
plot(1:length(x), x);
xlabel("Time (n)")
ylabel("x[n]")
title("Time-evolution plots of values x[n] and differences x'[n]")
subplot(2, 1, 2); % Differences
plot(1:length(x)-1, dx);
xlabel("Time (n)")
ylabel("x'[n]")

end

function x = rp_uslfl(N, a, k, x_min, x_max, x0)
% Generates an floating-point random walk within the given bounds and using
% 
%   dW ~ a * (Unif(0, 2) - 1)
% 
% as the noise distribution, updated every kth interval.
% 
% N     = number of points for realization
% k     = time interval for update to (potentially) occur
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound
% x0    = initial condition to start the process

% Storage for state
x = [x0; zeros(N-1,1)];

% Turn noise data into random process
for ii = 2:N
    % Prevent update until interval reached
    if mod(ii, k) == 0
        % Check bounds
        x(ii) = x(ii - 1) + a * (randi([0 2]) - 1);
        if x(ii) < x_min
            x(ii) = x_min;
        elseif x(ii) > x_max
            x(ii) = x_max;
        end
    else % No update
        x(ii) = x(ii - 1);
    end
end

end

function x = rp_inc_can(N)
% Random process generated as close to the one used in the original 
% CAN bus simulations as can be approximated
%
% N = number of points

% Allocate data and set IC
x = zeros(1, N);
x(1) = 1.0;

% Generate process
b = [0 0];
for ii = 2:N
    % Update bounds
    b = [max(1, x(ii - 1) - 1), min(90, x(ii - 1) + 1)] * 1000;

    % Update data at point
    x(ii) = (b(1) + (b(2) - b(1)) * rand) / 1000;
end

end

function x = dc_saw(N, a, x_min, x_max)
% Generates a floating-point signal following the saw waveform
% within the bounds and with slope a.
% 
% N     = number of points for realization
% a     = update increment
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound, also starting value

% Storage for state
x = [x_min; zeros(N-1,1)];

% Generate saw
for ii = 2:N
    % Using traditional loop-based design for smoother transition to C
    x(ii) = x(ii - 1) + a;
    if x(ii) > x_max
        x(ii) = x(ii) - x_max + x_min;
    end
end

end

function x = dq_saw(N, k, x_min, x_max)
% Generates an integral signal following the saw waveform within the bounds,
% where the slope is equal to 1/k, or incrementing every kth interval.
% Resembles a discrete saw upsampled by k in a sample-hold fashion.
% 
% N     = number of points for realization
% k     = time interval for increment to occur
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound, also starting value

% Storage for state
x = [x_min; zeros(N-1,1)];

% Generate saw
for ii = 2:N
    % Prevent update until interval reached
    if mod(ii, k) == 0
        % Cycle back of maximum reached
        if x(ii - 1) == x_max
            x(ii) = x_min;
        else % Increment uniformly
            x(ii) = x(ii - 1) + 1;
        end
    else % No update
        x(ii) = x(ii - 1);
    end
end

end

function x = rw_uslint(N, k, x_min, x_max, x0)
% Generates an integral random walk within the given bounds and using
% 
%   dW ~ Unif(0, 2) - 1
% 
% as the noise distribution, updated every kth interval.
% 
% N     = number of points for realization
% k     = time interval for update to (potentially) occur
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound
% x0    = initial condition to start the process

% Storage for state
x = [x0; zeros(N-1,1)];

% Turn noise data into random process
for ii = 2:N
    if mod(ii, k) == 0
        % Check the bounds first to avoid unsigned overflow
        if x(ii - 1) == x_min
            x(ii) = x(ii - 1) + randi([0 1]);
        elseif x(ii - 1) == x_max
            x(ii) = x(ii - 1) - randi([0 1]);
        else % Update as usual, using the noise value
            x(ii) = x(ii - 1) + randi([0 2]) - 1;
        end
    else % No update
        x(ii) = x(ii - 1);
    end
end

end

function x = rw_slint(N, x_min, x_max, x0)
% Generates an integral random walk within the given bounds and using
% 
%   dW ~ Unif(0, 2) - 1
% 
% as the noise distribution. Biases appear to vary per realization.
% 
% N     = number of points for realization
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound
% x0    = initial condition to start the process

% Store noise realization in-place for convenience
x = randi(3, N, 1) - 2;

% Set IC
x(1) = x0;

% Turn noise data into random process
for ii = 2:N
    % Check the bounds first to avoid unsigned overflow
    if (x(ii - 1) == x_min) && (x(ii) == -1) % Avoid underpassing lower bound
        x(ii) = x(ii - 1);
    elseif (x(ii - 1) == x_max) && (x(ii) == 1) % Avoid surpassing upper bound
        x(ii) = x(ii - 1);
    else % Update as usual, using the noise value
        x(ii) = x(ii - 1) + x(ii);
    end
end

end

function x = rw_int(N, x_min, x_max, x0)
% Generates an integral random walk within the given bounds and using
% 
%   dW ~ 2 * Bern(0.5) - 1
% 
% as the noise distribution. Realizations are always unique.
% 
% N     = number of points for realization
% x_min = minimum bound (advised to be nonnegative)
% x_max = maximum bound
% x0    = initial condition to start the process

% Store noise realization in-place for convenience
x = randi(2, N, 1) * 2 - 3;

% Set IC
x(1) = x0;

% Turn noise data into random process
for ii = 2:N
    % Check the bounds first to avoid unsigned overflow
    if x(ii - 1) == x_min % Always increase from lower bound
        x(ii) = x(ii - 1) + 1;
    elseif x(ii - 1) == x_max % Always decrease from upper bound
        x(ii) = x(ii - 1) - 1;
    else % Update as usual, using the noise value
        x(ii) = x(ii - 1) + x(ii);
    end
end

end