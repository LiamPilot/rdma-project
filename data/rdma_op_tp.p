set logscale x 2
set xtics 2
set tics font "Roboto,14"
set terminal png size 1200, 1000 font "Roboto,24"
set output "output/rdma_op_tp.png"
set xlabel "Buffer Size (KB)"
set ylabel "Throughput (GB/s)"
set label 1 at 1.5, 12.7 "Infiniband EDR Bandwidth" left
set style line 1 lw 2.5
set style line 2 lw 2.5
set style line 3 lw 2.5
plot "rdma_write_throughput.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "RDMA Read" ls 1, \
    "rdma_read_throughput.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "RDMA Write" ls 2, \
    "rdma_two_sided_throughput.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "RDMA Two Sided" ls 3, \
    12.5 with lines notitle lt rgb "red" lw 2
