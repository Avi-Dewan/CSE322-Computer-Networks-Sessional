#!/bin/bash

file_path="output/mobile"

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
  command="../ns3 run 'scratch/1905120_2.cc --nNodes=${nNodes} --whichParam=1'"
  echo "Running simulation for nNodes=${nNodes}"
  eval ${command}
  echo "Simulation for nNodes=${nNodes} completed"
done



echo "Running simulation for different number of Flows"

for nFlows in 10 20 30 40 50; do
  command="../ns3 run 'scratch/1905120_2.cc --nFlows=${nFlows} --whichParam=2'"
  echo "Running simulation for nFlows=${nFlows}"
  eval ${command}
  echo "Simulation for nFlows=${nFlows} completed"
done


echo "Running simulation for different number of Flows"

for nPackets in 100 200 300 400 500; do
  command="../ns3 run 'scratch/1905120_2.cc --nPackets=${nPackets} --whichParam=3'"
  echo "Running simulation for nPackets=${nPackets}"
  eval ${command}
  echo "Simulation for nPackets=${nPackets} completed"
done

echo "Running simulation for different  node speeds"

for nodeSpeed in 5 10 15 20 25; do
  command="../ns3 run 'scratch/1905120_2.cc --nodeSpeed=${nodeSpeed} --whichParam=4'"
  echo "Running simulation for nodeSpeed=${nodeSpeed}"
  eval ${command}
  echo "Simulation for nodeSpeed=${nodeSpeed} completed"
done


gnuplot -persist <<-EOFMarker

    set terminal png size 640,480

    set output "output/mobile/plots/throughput_vs_node.png"
    set xlabel "nodes"
    set ylabel "throughput"
    plot "output/mobile/throughput_nodes.dat" using 1:2 title 'Throughput vs nodes' with linespoints

    set output "output/mobile/plots/delivery_ratio_vs_node.png"
    set xlabel "nodes"
    set ylabel "delivery ratio"
    plot "output/mobile/deliveryR_nodes.dat" using 1:2 title 'Delivery ratio vs nodes' with linespoints

    set output "output/mobile/plots/throughput_vs_flow.png"
    set xlabel "flows"
    set ylabel "throughput"
    plot "output/mobile/throughput_flows.dat" using 1:2 title 'Throughput vs flows' with linespoints

    set output "output/mobile/plots/delivery_ratio_vs_flow.png"
    set xlabel "flows"
    set ylabel "delivery ratio"
    plot "output/mobile/deliveryR_flows.dat" using 1:2 title 'Delivery ratio vs flows' with linespoints

    set output "output/mobile/plots/throughput_vs_packet.png"
    set xlabel "packet"
    set ylabel "throughput"
    plot "output/mobile/throughput_packets.dat" using 1:2 title 'Throughput vs packets' with linespoints

    set output "output/mobile/plots/delivery_ratio_vs_packet.png"
    set xlabel "packet"
    set ylabel "delivery ratio"
    plot "output/mobile/deliveryR_packets.dat" using 1:2 title 'Delivery ratio vs packets' with linespoints

    set output "output/mobile/plots/throughput_vs_nodeSpeed.png"
    set xlabel "node speed"
    set ylabel "throughput"
    plot "output/mobile/throughput_speeds.dat" using 1:2 title 'Throughput vs nodeSpeed' with linespoints

    set output "output/mobile/plots/delivery_ratio_vs_nodeSpeed.png"
    set xlabel "node speed"
    set ylabel "delivery ratio"
    plot "output/mobile/deliveryR_speeds.dat" using 1:2 title 'Delivery ratio vs nodeSpeed' with linespoints

EOFMarker