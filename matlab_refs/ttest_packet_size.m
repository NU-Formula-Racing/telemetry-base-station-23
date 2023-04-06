function [a, c] = ttest_packet_size
% Fits the data from Test 1 of the base station.
% a = coefficients for linear regrression of transformed data
% c = correlation coefficient between transformed P, R

% Packet size (unit, number of bytes in a LoRa data packet)
P = [251 128 64 32 16 8 4];

% Transmission rate (number of packets per second)
R = [2.491 4.511 8.071 12.988 17.511 23.968 27.316];

% Determine coefficients using the transformed linear regression model
% 1 / R = a(1) * P + a(2)
R_inv = 1 ./ R;
a = polyfit(P, R_inv, 1);

% Plot regression
figure(1)
plot(P, R, "*", "DisplayName", "Measured")
hold on
plot(P, 1 ./ (a(1) * P + a(2)), "DisplayName", "Regression")

% Find correlation of regression
Phi = cov(P, R_inv) / std(P) / std(R_inv);
c = Phi(1, 2);

end