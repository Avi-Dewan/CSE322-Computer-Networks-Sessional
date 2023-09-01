#!/bin/bash

file_path="output/RenoVsWest"

dirToSave="mkdir -p $file_path"
dirToDel="rm -rf $file_path"

if ! $dirToDel; then
    exit 1
fi

if ! $dirToSave; then
    exit 1
fi

file_path="output/RenoVsHigh"

dirToSave="mkdir -p $file_path"
dirToDel="rm -rf $file_path"

if ! $dirToDel; then
    exit 1
fi

if ! $dirToSave; then
    exit 1
fi

file_path="output/RenoVsAdaptive"

dirToSave="mkdir -p $file_path"
dirToDel="rm -rf $file_path"

if ! $dirToDel; then
    exit 1
fi

if ! $dirToSave; then
    exit 1
fi

file_path="output/Plots"

dirToSave="mkdir -p $file_path"
dirToDel="rm -rf $file_path"

if ! $dirToDel; then
    exit 1
fi

if ! $dirToSave; then
    exit 1
fi



echo "Running simulation for Case 1: NewReno Vs WestwoodPlus"

for bottleneckDataRate in 1 50 100 150 200 150 300; do
  command="../ns3 run 'scratch/1905120.cc --bottleneckDataRate=${bottleneckDataRate} --whichParam=1 --whichCase=1'"
  echo "Running case-1 simulation for bottleneckDataRate=${bottleneckDataRate}"
  eval ${command}
  echo "case-1 simulation for bottleneckDataRate=${bottleneckDataRate} completed"
done

for plossRate in 2 3 4 5 6; do
  command="../ns3 run 'scratch/1905120.cc --plossRate=${plossRate} --whichParam=2 --whichCase=1'"
  echo "Running case-1 simulation for plossRate=${plossRate}"
  eval ${command}
  echo "case-1 simulation for plossRate=${plossRate} completed"
done


echo "Running simulation for Case 2: NewReno Vs HighSpeed"

for bottleneckDataRate in 1 50 100 150 200 150 300; do
  command="../ns3 run 'scratch/1905120.cc --bottleneckDataRate=${bottleneckDataRate} --whichParam=1 --whichCase=2'"
  echo "Running case-2 simulation for bottleneckDataRate=${bottleneckDataRate}"
  eval ${command}
  echo "case-2 simulation for bottleneckDataRate=${bottleneckDataRate} completed"
done

for plossRate in 2 3 4 5 6; do
  command="../ns3 run 'scratch/1905120.cc --plossRate=${plossRate} --whichParam=2 --whichCase=2'"
  echo "Running case-2 simulation for plossRate=${plossRate}"
  eval ${command}
  echo "case-2 simulation for plossRate=${plossRate} completed"
done


echo "Running simulation for Case 3: NewReno Vs AdaptiveReno"

for bottleneckDataRate in 1 50 100 150 200 150 300; do
  command="../ns3 run 'scratch/1905120.cc --bottleneckDataRate=${bottleneckDataRate} --whichParam=1 --whichCase=3'"
  echo "Running case-3 simulation for bottleneckDataRate=${bottleneckDataRate}"
  eval ${command}
  echo "case-3 simulation for bottleneckDataRate=${bottleneckDataRate} completed"
done

for plossRate in 2 3 4 5 6; do
  command="../ns3 run 'scratch/1905120.cc --plossRate=${plossRate} --whichParam=2 --whichCase=3'"
  echo "Running case-3 simulation for plossRate=${plossRate}"
  eval ${command}
  echo "case-3 simulation for plossRate=${plossRate} completed"
done


gnuplot -persist <<-EOFMarker

    set terminal png size 640, 480

    set output "output/Plots/DataRate_RW_.png"
    set title "Throughput vs Bottleneck Data Rate of NewReno and WestwoodPlus"
    set xlabel "Bottleneck Data Rate"
    set ylabel "Throughput"
    plot "output/RenoVsWest/thr_Vs_btlR.dat" using 1:2 with lines title "NewReno", \
            "output/RenoVsWest/thr_Vs_btlR.dat" using 1:3 with lines title "WestwoodPlus" 

    set output "output/Plots/PacketLoss_RW_.png"
    set title "Throughput vs Packet Loss Rate of NewReno & WestwoodPlus"
    set xlabel "Packet Loss Rate"
    set ylabel "Throughput"
    plot "output/RenoVsWest/thr_Vs_ploss.dat" using 1:2 with lines title "NewReno", \
         "output/RenoVsWest/thr_Vs_ploss.dat" using 1:3 with lines title "WestwoodPlus"

    set output "output/Plots/CongestionWindow_RW_.png"
    set title "Congestion Window VS Time of NewReno & WestwoodPlus"
    set xlabel "Time"
    set ylabel "Congestion Window"
    plot "output/RenoVsWest/cwndVsT1.cwnd" using 1:2 with lines title "NewReno", \
         "output/RenoVsWest/cwndVsT2.cwnd" using 1:2 with lines title "WestwoodPlus"



    set output "output/Plots/DataRate_RH_.png"
    set title "Throughput vs Bottleneck Data Rate of NewReno and HighSpeed"
    set xlabel "Bottleneck Data Rate"
    set ylabel "Throughput"
    plot "output/RenoVsHigh/thr_Vs_btlR.dat" using 1:2 with lines title "NewReno", \
            "output/RenoVsHigh/thr_Vs_btlR.dat" using 1:3 with lines title "HighSpeed" 

    set output "output/Plots/PacketLoss_RH_.png"
    set title "Throughput vs Packet Loss Rate of NewReno & HighSpeed"
    set xlabel "Packet Loss Rate"
    set ylabel "Throughput"
    plot "output/RenoVsHigh/thr_Vs_ploss.dat" using 1:2 with lines title "NewReno", \
         "output/RenoVsHigh/thr_Vs_ploss.dat" using 1:3 with lines title "HighSpeed"

    set output "output/Plots/CongestionWindow_RH_.png"
    set title "Congestion Window VS Time of NewReno & HighSpeed"
    set xlabel "Time"
    set ylabel "Congestion Window"
    plot "output/RenoVsHigh/cwndVsT1.cwnd" using 1:2 with lines title "NewReno", \
         "output/RenoVsHigh/cwndVsT2.cwnd" using 1:2 with lines title "HighSpeed"

    set output "output/Plots/JainIndex_Vs_DataRate_RH_.png"
    set title "JainIndex VS Bottleneck Data Rate of NewReno & HighSpeed"
    set xlabel "Bottleneck Data Rate"
    set ylabel "Jain Index"
    plot "output/RenoVsHigh/fairnessIdx_Vs_btlR.dat" using 1:2 with lines title "Jain Index"

    set output "output/Plots/JainIndex_Vs_PacketLoss_RH_.png"
    set title "JainIndex VS Packet Loss Rate of NewReno & HighSpeed"
    set xlabel "Packet Loss Rate"
    set ylabel "Jain Index"
    plot "output/RenoVsHigh/fairnessIdx_Vs_ploss.dat" using 1:2 with lines title "Jain Index"



    set output "output/Plots/DataRate_RA_.png"
    set title "Throughput vs Bottleneck Data Rate of NewReno and AdaptiveReno"
    set xlabel "Bottleneck Data Rate"
    set ylabel "Throughput"
    plot "output/RenoVsAdaptive/thr_Vs_btlR.dat" using 1:2 with lines title "NewReno", \
            "output/RenoVsAdaptive/thr_Vs_btlR.dat" using 1:3 with lines title "AdaptiveReno" 

    set output "output/Plots/PacketLoss_RA_.png"
    set title "Throughput vs Packet Loss Rate of NewReno & AdaptiveReno"
    set xlabel "Packet Loss Rate"
    set ylabel "Throughput"
    plot "output/RenoVsAdaptive/thr_Vs_ploss.dat" using 1:2 with lines title "NewReno", \
         "output/RenoVsAdaptive/thr_Vs_ploss.dat" using 1:3 with lines title "AdaptiveReno"

    set output "output/Plots/CongestionWindow_RA_.png"
    set title "Congestion Window VS Time of NewReno & AdaptiveReno"
    set xlabel "Time"
    set ylabel "Congestion Window"
    plot "output/RenoVsAdaptive/cwndVsT1.cwnd" using 1:2 with lines title "NewReno", \
         "output/RenoVsAdaptive/cwndVsT2.cwnd" using 1:2 with lines title "AdaptiveReno"

    set output "output/Plots/JainIndex_Vs_DataRate_RA_.png"
    set title "JainIndex VS Bottleneck Data Rate of NewReno & Adaptive Reno"
    set xlabel "Bottleneck Data Rate"
    set ylabel "Jain Index"
    plot "output/RenoVsAdaptive/fairnessIdx_Vs_btlR.dat" using 1:2 with lines title "Jain Index"

    set output "output/Plots/JainIndex_Vs_PacketLoss_RA_.png"
    set title "JainIndex VS Packet Loss Rate of NewReno & Adaptive Reno"
    set xlabel "Packet Loss Rate"
    set ylabel "Jain Index"
    plot "output/RenoVsAdaptive/fairnessIdx_Vs_ploss.dat" using 1:2 with lines title "Jain Index"

EOFMarker
