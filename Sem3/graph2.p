set terminal png
set termoption enhanced

set output "graph2.png"
set title "Performance"

set key left
set xlabel "Rounds"
set ylabel "Time (ms)"

plot 'notimproved.dat' using 1:2 w linespoint linecolor rgb "#9400d3" title "head 24 bytes header", 'improved.dat' using 1:2 w linespoint linecolor rgb "#009e73" title "taken 8 bytes header"

