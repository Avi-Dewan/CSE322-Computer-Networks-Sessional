#!/bin/bash

file_path="output/static"

dirToSave="mkdir -p $file_path"
dirToDel="rm -rf $file_path"

if ! $dirToDel; then
    exit 1
fi

if ! $dirToSave; then
    exit 1
fi

# Create the 'output/static/plots' directory

plots_path="$file_path/plots"

dirToSavePlots="mkdir -p $plots_path"

if ! $dirToSavePlots; then
    exit 1
fi


echo "Running simulation for different number of Nodes"

for nNodes in 20 40 60 80 100; do
  command="../ns3 run 'scratch/1905120_1.cc --nNodes=${nNodes} --whichParam=1'"
  echo "Running simulation for nNodes=${nNodes}"
  eval ${command}
  echo "Simulation for nNodes=${nNodes} completed"
done



echo "Running simulation for different number of Flows"

for nFlows in 10 20 30 40 50; do
  command="../ns3 run 'scratch/1905120_1.cc --nFlows=${nFlows} --whichParam=2'"
  echo "Running simulation for nFlows=${nFlows}"
  eval ${command}
  echo "Simulation for nFlows=${nFlows} completed"
done


echo "Running simulation for different number of Flows"

for nPackets in 100 200 300 400 500; do
  command="../ns3 run 'scratch/1905120_1.cc --nPackets=${nPackets} --whichParam=3'"
  echo "Running simulation for nPackets=${nPackets}"
  eval ${command}
  echo "Simulation for nPackets=${nPackets} completed"
done

for coverageArea in 1 2 4 5; do
  command="../ns3 run 'scratch/1905120_1.cc --coverageArea=${coverageArea} --whichParam=4'"
  echo "Running simulation for coverageArea=${coverageArea}"
  eval ${command}
  echo "Simulation for coverageArea=${coverageArea} completed"
done


gnuplot -persist <<-EOFMarker

    set terminal png size 640,480

    set output "output/static/plots/throughput_vs_node.png"
    set xlabel "nodes"
    set ylabel "throughput"
    plot "output/static/throughput_nodes.dat" using 1:2 title 'Throughput vs nodes' with linespoints

    set output "output/static/plots/delivery_ratio_vs_node.png"
    set xlabel "nodes"
    set ylabel "delivery ratio"
    plot "output/static/deliveryR_nodes.dat" using 1:2 title 'Delivery ratio vs nodes' with linespoints

    set output "output/static/plots/throughput_vs_flow.png"
    set xlabel "flows"
    set ylabel "throughput"
    plot "output/static/throughput_flows.dat" using 1:2 title 'Throughput vs flows' with linespoints

    set output "output/static/plots/delivery_ratio_vs_flow.png"
    set xlabel "flows"
    set ylabel "delivery ratio"
    plot "output/static/deliveryR_flows.dat" using 1:2 title 'Delivery ratio vs flows' with linespoints

    set output "output/static/plots/throughput_vs_packet.png"
    set xlabel "packet"
    set ylabel "throughput"
    plot "output/static/throughput_packets.dat" using 1:2 title 'Throughput vs packets' with linespoints

    set output "output/static/plots/delivery_ratio_vs_packet.png"
    set xlabel "packet"
    set ylabel "delivery ratio"
    plot "output/static/deliveryR_packets.dat" using 1:2 title 'Delivery ratio vs packets' with linespoints

    set output "output/static/plots/throughput_vs_coverage.png"
    set xlabel "coverage"
    set ylabel "throughput"
    plot "output/static/throughput_areas.dat" using 1:2 title 'Throughput vs coverage' with linespoints

    set output "output/static/plots/delivery_ratio_vs_coverage.png"
    set xlabel "coverage"
    set ylabel "delivery ratio"
    plot "output/static/deliveryR_areas.dat" using 1:2 title 'Delivery ratio vs coverage' with linespoints

EOFMarker