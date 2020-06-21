unset logscale x
set xtic 128
set tics font "Roboto,14"
set terminal png size 1200, 1000 font "Roboto,24"
set output "output/rdma_tcp_lat_high.png"
set xlabel "Buffer Size (KB)"
set ylabel "Latency (microseconds)"
unset label 1
unset label 2
set style line 1 lw 2.5
set style line 2 lw 2.5
set style line 3 lw 2.5
plot "rdma_write_latency.txt" every ::7 using ($1 / 1024):($2 * 1e-3) with linespoints title "RDMA Read" ls 1, \
    "tcp_eth_latency.txt" every ::7 using ($1 / 1024):($2 * 1e-3) with linespoints title "TCP over Ethernet" ls 2, \
    "tcp_latency.txt" every ::7 using ($1 / 1024):($2 * 1e-3) with linespoints title "TCP over Infiniband" ls 3
