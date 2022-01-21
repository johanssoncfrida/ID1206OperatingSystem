set terminal png
set termoption enhanced

set output "graph0.png"
set title "Average length of blocks"

set key left
set xlabel "Rounds"
set ylabel "Size (bytes)"
set style fill solid
plot [0.5:][0:3000] 'nomergesizeofblock.dat' u 2: xtic(1) with histogram title 'no merge', 'mergesizeofblock.dat' u 2: xtic(1) with histogram title 'merge'



