set logscale x 2
set xlabel "Buffer Size (KB)"
set ylabel "Throughput (GB/s)"
set label 1 at 2, 12.7 "Bandwidth" center
if (!exists("filename")) filename = "results.txt"
plot "read2.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "RDMA Reads with large local buffer", \
    "writes.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "RDMA Writes with large remote buffer", \
    "two_sided.txt" using ($1 / 1024):($2 * 1e-9) with linespoints title "Two sided communication", \
    12.5 with lines notitle

