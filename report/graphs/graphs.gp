#!/usr/bin/gnuplot

set terminal pdf
set output "time-ib.pdf"
set xlabel "počet procesorů"
set ylabel "čas [s]"
set key center top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g1ib.txt" using 1:2 title "n=200 k=4 i=4" with linespoints ls 1 lc rgb "#B22222", "g2ib.txt" using 1:2 title "n=32 k=6 i=1" with linespoints ls 1 lc rgb "#1E90FF", "g3ib.txt" using 1:2 title "n=50 k=3 i=2" with linespoints ls 1 lc rgb "#228B22"

set terminal pdf
set output "speedup-ib.pdf"
set xlabel "počet procesorů"
set ylabel "zrychlení"
set key left top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g1ib.txt" using 1:4 title "n=200 k=4 i=4" with linespoints ls 1 lc rgb "#B22222", "g2ib.txt" using 1:4 title "n=32 k=6 i=1" with linespoints ls 1 lc rgb "#1E90FF", "g3ib.txt" using 1:4 title "n=50 k=3 i=2" with linespoints ls 1 lc rgb "#228B22"

set terminal pdf
set output "time-eth.pdf"
set xlabel "počet procesorů"
set ylabel "čas [s]"
set key right top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g1eth.txt" using 1:2 title "n=200 k=4 i=4" with linespoints ls 1 lc rgb "#B22222", "g2eth.txt" using 1:2 title "n=32 k=6 i=1" with linespoints ls 1 lc rgb "#1E90FF", "g3eth.txt" using 1:2 title "n=50 k=3 i=2" with linespoints ls 1 lc rgb "#228B22"

set terminal pdf
set output "speedup-eth.pdf"
set xlabel "počet procesorů"
set ylabel "zrychlení"
set key left top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g1eth.txt" using 1:4 title "n=200 k=4 i=4" with linespoints ls 1 lc rgb "#B22222", "g2eth.txt" using 1:4 title "n=32 k=6 i=1" with linespoints ls 1 lc rgb "#1E90FF", "g3eth.txt" using 1:4 title "n=50 k=3 i=2" with linespoints ls 1 lc rgb "#228B22"

set terminal pdf
set output "graph1-speedup.pdf"
set xlabel "počet procesorů"
set ylabel "zrychlení"
set key left top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g1ib.txt" using 1:4 title "InfiniBand" with linespoints ls 1 lc rgb "#B22222", "g1eth.txt" using 1:4 title "Ethernet" with linespoints ls 1 lc rgb "#1E90FF"

set terminal pdf
set output "graph2-speedup.pdf"
set xlabel "počet procesorů"
set ylabel "zrychlení"
set key left top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g2ib.txt" using 1:4 title "InfiniBand" with linespoints ls 1 lc rgb "#B22222", "g2eth.txt" using 1:4 title "Ethernet" with linespoints ls 1 lc rgb "#1E90FF"

set terminal pdf
set output "graph3-speedup.pdf"
set xlabel "počet procesorů"
set ylabel "zrychlení"
set key left top
set style line 1 lt 1 lw 4 pt 7 ps 0.5

plot "g3ib.txt" using 1:4 title "InfiniBand" with linespoints ls 1 lc rgb "#B22222", "g3eth.txt" using 1:4 title "Ethernet" with linespoints ls 1 lc rgb "#1E90FF"
