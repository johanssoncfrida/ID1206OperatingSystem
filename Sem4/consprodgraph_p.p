set terminal png
set termoption enhanced

set output "multicore.png"
set title "Execution time (ms) as a function of number of loops (multicore)"

set key left
set xlabel "Number of loops"
set ylabel "Time (ms)"


plot 'multicore.dat' using 1:3 w linespoints linecolor rgb "#0000FF" title "Pthreads", 'multicore.dat' using 1:2 w linespoints linecolor rgb "#32CD32" title "Green Threads"
