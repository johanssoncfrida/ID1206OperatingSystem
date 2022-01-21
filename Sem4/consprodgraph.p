set terminal png
set termoption enhanced

set output "unicore.png"
set title "Execution time (ms) as a function of number of loops (1 core)"

set key left
set xlabel "Number of loops"
set ylabel "Time (ms)"

plot 'unicore.dat' using 1:3 w linespoints linecolor rgb "#0000FF" title "Pthreads", 'unicore.dat' using 1:2 w linespoints linecolor rgb "#32CD32" title "Green Threads"
