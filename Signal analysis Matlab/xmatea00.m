%----------------------------------TASK #1--------------------------------------
[signal, fs] = audioread('xmatea00.wav');
signal = signal';
samples_length = length(signal);
binary_sym_representation = samples_length / 16;
fprintf("Vzorkovacia frekvencia signalu: %f [Hz]\n", fs);
fprintf("Dlzka signalu vo vzorkach: %f\n", samples_length);
fprintf("Dlzka signalu v sekundach: %f [s]\n", samples_length/fs);
fprintf("Pocet reprezentovanych symbolov: %f\n", binary_sym_representation);
%----------------------------------TASK #2--------------------------------------
tmp = 16;
i = 8;
a = 1;
while tmp <= 32000
  if(signal(i) > 0)
      vec(a) = 1;
  else
      vec(a) = 0;
  end
tmp = tmp + 16;
i = tmp - 8;
a = a + 1;
end
binary_idx = 1;
while binary_idx <= 320 
  if(signal(binary_idx) > 0)
      binary_plot_vector(binary_idx) = 1;
  else
      binary_plot_vector(binary_idx) = 0;
  end
  binary_idx = binary_idx + 1;
end
time_idx = 1;
time_cnt = 0;
while time_idx <= 320
  time_plot(time_idx) = time_cnt;
  time_idx = time_idx + 1;
  time_cnt = time_cnt + 0.0000625
end
x = 1:1:320;
x_bin = 8:16:320;
f = figure('Visible', 'off');
subplot(211);
ylabel('s[n], symbols');
xlabel('t');
grid;
hold on
plot(time_plot(x), signal(x));
stem(time_plot(x_bin), binary_plot_vector(x_bin));
hold off
print(f, "TASK_2.png", '-dpng');
close(f);
%----------------------------------TASK #3--------------------------------------
%do protokolu napisat ze je stabilny, pretoze vsetky poly su vo vnutri 1 kruznice
B = [0.0192 -0.0185 -0.0185 0.0192];
A = [1.0000 -2.8870 2.7997 -0.9113];
if abs(roots(A)) < 1
	fprintf('Filter je stabilny.\n');
else
	fprintf('Filter nie je stabilny.\n');
end
f = figure('Visible','off');
zplane(B,A);
grid;
xlabel('Re');
ylabel('Im');
print(f, "TASK_3.png", '-dpng');
close(f); 
%----------------------------------TASK #4--------------------------------------
f = figure('Visible', 'off');
step = (0 : 255) / 256 * fs / 2;
plot(step, abs(freqz(B,A,256)));
grid;
xlabel('f [Hz]');
ylabel('|H(f)|');
print(f, "TASK_4.png", '-dpng');
close(f);
%----------------------------------TASK #5--------------------------------------
filtered_signal = filter(B, A, signal); 
f = figure('Visible', 'off');
subplot(211);
ylabel('s[n], ss[n]');
xlabel('t');
grid;
hold on
plot(time_plot(x), signal(x));
plot(time_plot(x), filtered_signal(x));
hold off
print(f, "TASK_5.png", '-dpng');
close(f);
%----------------------------------TASK #6--------------------------------------
shift_idx = 1;
while shift_idx+16 <= 32000
  shifted_signal(shift_idx) = filtered_signal(shift_idx+16);
  shift_idx = shift_idx + 1;
end

signal_idx_task6 = 1;
while signal_idx_task6 <= 320
  if(shifted_signal(signal_idx_task6) > 0)
    symbols_of_shifted_signal(signal_idx_task6) = 1;
  else
    symbols_of_shifted_signal(signal_idx_task6) = 0;
  end
  signal_idx_task6 = signal_idx_task6 + 1;
end
f = figure('Visible', 'off');
subplot(211);
ylabel('s[n], ss[n], ss_s_h_i_f_t_e_d[n], symbols');
xlabel('t');
grid;
hold on
plot(time_plot(x), signal(x));
plot(time_plot(x), shifted_signal(x));
plot(time_plot(x), filtered_signal(x));
stem(time_plot(x_bin), symbols_of_shifted_signal(x_bin));
hold off
print(f, "TASK_6.png", '-dpng');
close(f);
%----------------------------------TASK #7--------------------------------------
tmp = 16;
i = 8;
a = 1;
while tmp <= 31984
  if(shifted_signal(i) > 0)
      vec_b(a) = 1;
  else
      vec_b(a) = 0;
  end
tmp = tmp + 16;
i = tmp - 8;
a = a + 1;
end
tmp = 1;
tmp_result = 0;
while tmp <= 1999
  c = xor(vec(tmp),vec_b(tmp))
  tmp_result = tmp_result + c;
  tmp = tmp + 1;
end
result = (tmp_result/1999)*100;
fprintf("Pocet chyb: %d\n", tmp_result);
fprintf("Chybovost: %f %%\n", result);
%----------------------------------TASK #8--------------------------------------
frequency = (0 : samples_length / 2 - 1) / samples_length * fs;
fft_signal = abs(fft(signal));
fft_signal = fft_signal(1 : samples_length / 2);
fft_filtered_signal = abs(fft(filtered_signal));
fft_filtered_signal = fft_filtered_signal(1 : samples_length / 2);
%treba popisat obrazky
f = figure('Visible', 'off');
xlabel('f [Hz]');
hold on
plot(frequency, fft_signal);
plot(frequency, fft_filtered_signal);
hold off
print(f, "TASK_8.png", '-dpng');
close(f);
%----------------------------------TASK #9--------------------------------------
N = length(signal);
gmin = min(min(signal)); gmax = max(max(signal));
L = 50;
g = linspace(gmin,gmax,L);
Fg = zeros(size(g));
for ii = 1:L,
thisg = g(ii);
Fg(ii) = sum(signal < thisg) / N;
end
deltax = g(2) - g(1);
pxn = hist(signal,g) / N / deltax;
f = figure('Visible', 'off');
subplot(211); 
plot (g,pxn);
legend("Probability density function");
print(f, "TASK_9.png", '-dpng');
close(f);
check_task9 = sum(sum(pxn)) * deltax;
fprintf("Hodnota integralu z ulohy 9: %d\n", check_task9);
%----------------------------------TASK #10-------------------------------------
k = (-50 : 50);
R = xcorr(signal) / samples_length;
R = R(k + samples_length);
f = figure('Visible', 'off');
plot(k, R);
xlabel('k');
print(f, "TASK_10.png", '-dpng');
close(f);

%----------------------------------TASK #11-------------------------------------
fprintf("Hodnota R[0]: %f\n", R(51));
fprintf("Hodnota R[1]: %f\n", R(52));
fprintf("Hodnota R[16]: %f\n", R(67));

%----------------------------------TASK #12-------------------------------------
x = linspace(min(signal), max(signal), 50);
h = zeros(L, L);
xcol = x(:); bigx = repmat(x(:), 1, N);
yr = signal(:)'; bigy = repmat(signal(:)', L, 1);
[dummy, ind1] = min(abs(bigy - bigx));
ind2 = ind1(1 + 1 : N);
for i = 1 : N - 1,
	d1 = ind1(i);
	d2 = ind2(i);
	h(d1, d2) = h(d1, d2) + 1;
end
surf = (x(2) - x(1)) ^ 2;
p = h / N / surf;
f = figure('Visible', 'off');
imagesc(x, x, p);
axis xy;
colorbar;
xlabel('x');
ylabel('y');
print(f, "TASK_12.png", '-dpng');
close(f);

%----------------------------------TASK #13-------------------------------------
check = sum(sum(p)) * surf;
fprintf("Integral z ulohy 12 by mal byt rovny 1 a je: %f.\n", check);

%----------------------------------TASK #14-------------------------------------
answer = sum(sum(repmat(x(:), 1, L) .* repmat(x(:)', L, 1) .* p)) * surf
fprintf('Hodnota koeficientu R[1] je %f.\n', answer);