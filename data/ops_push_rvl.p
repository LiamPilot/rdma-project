unset logscale x
set xtics 2048
set tics font "Roboto,14"
set terminal png size 1200, 1000 font "Roboto,24"
set output "remote_push_ops.png"
set xlabel "Buffer Size (Bytes)"
set ylabel "Throughput (KOps/s)"
unset label 1
set style line 1 lw 2.5
set style line 2 lw 2.5
set style line 3 lw 2.5
plot "gas_remote_push.txt" using ($4):($13 * 1e-3) with linespoints title "GASNet-Ex" ls 1, \
     "mpi_eth_remote_push.txt" using ($4 ):($13 * 1e-3) with linespoints title "MPI over Ethernet" ls 2, \
     "mpi_ib_remote_push.txt" using ($4 ):($13 * 1e-3) with linespoints title "MPI over Infiniband" ls 3
