set terminal png
set termoption enhanced

set output "graph1.png"
set title "Length freelist as a function of rounds"

set key left center
set xlabel "Rounds"
set ylabel "Length freelist"

plot 'noMerge.dat' using 1:2 w points linecolor rgb "#9400d3" title "No merge", 'merge.dat' using 1:2 w points linecolor rgb "#009e73" title "with merge"

