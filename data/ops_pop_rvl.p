unset logscale x
set xtics 2048
set tics font "Roboto,14"
set terminal png size 1200, 1000 font "Roboto,24"
set output "output/ops_pop_rvl.png"
set xlabel "Buffer Size (Bytes)"
set ylabel "Throughput (KOps/s)"
unset label 1
set style line 1 lw 2.5
set style line 2 lw 2.5
set style line 3 lw 2.5
plot "gas_remote_pop.txt" using ($4):($13 * 1e-3) with linespoints title "Remote" ls 1, \
     "gas_local_pop.txt" using ($4 ):($13 * 1e-3) with linespoints title "Local" ls 2, \

