unset logscale x
set xtics 128
set tics font "Roboto,14"
set terminal png size 1200, 1000 font "Roboto,24"
set output "output/rdma_op_lat_low.png"
set xlabel "Buffer Size (Bytes)"
set ylabel "Latency (microseconds)"
unset label 1
set style line 1 lw 2.5
set style line 2 lw 2.5
set style line 3 lw 2.5
plot "rdma_write_latency.txt" every ::::8 using ($1):($2 * 1e-3) with linespoints title "RDMA Read" ls 1, \
    "rdma_read_latency.txt" every ::::8 using ($1 ):($2 * 1e-3) with linespoints title "RDMA Write" ls 2, \
    "rdma_two_sided_latency.txt" every ::::8 using ($1 ):($2 * 1e-3) with linespoints title "RDMA Two Sided" ls 3, \
