/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"


#include<iostream>
#include <iomanip>
#include <map>

// Default Network Topology
//
//
/*
            s0                      r0
            s1                      r1
            s2                      r1
            s1       x-----y        r1
            s1                      r1
            .                      .
            .                      .
            .                      .  
            sn                      rn
*/



using namespace ns3;
using std::vector;

std::string file_path = "scratch/output/mobile";

double delivery_ratio = 0;
double thorughput = 0;
double count = 0;
// calculate metrics
static void
TraceMetrics (Ptr<FlowMonitor> monitor)
{
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  Time curTime = Now ();
  
  // threshold
  double tot_thr = 0;
  // delay
  double tot_rx_packets = 0;
  // drop and delivery
  double tot_drop = 0;
  double tot_delivery = 0;
  int num_flows = 0;
  for(auto itr:stats)
  {
    // threshold
    tot_thr += (8 * itr.second.rxBytes ) / (1.0 * curTime.GetSeconds () );

    tot_rx_packets += itr.second.rxPackets;
    // drop and delivery

    tot_drop += itr.second.lostPackets;
    tot_delivery += itr.second.rxPackets;
    num_flows++;
  }
  // ofs_thp <<  curTime << " " << tot_thr << std::endl; // throughput (bit/s)
  // ofs_deliver <<  curTime << " " << (100.0 * tot_delivery)/(tot_rx_packets+tot_drop) << std::endl; // delivery ratio (%)

  thorughput += (tot_thr);
  delivery_ratio += ((100.0 * tot_delivery)/(tot_rx_packets+tot_drop));
  count++;

  Simulator::Schedule (Seconds (0.1), &TraceMetrics, monitor);
}

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int main (int argc, char *argv[])
{
  // Parse command line arguments

    const int tx_range = 5;
    const int packetSize = 1024;

    int nNodes = 20;
    int nFlows = 10;
    int nPackets = 100;
    int nodeSpeed = 10;
    int whichParam = -1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of wifi nodes", nNodes);
    cmd.AddValue("nFlows", "Number of flows", nFlows);
    cmd.AddValue("nPackets", "Number of packets per second", nPackets);
    cmd.AddValue("nodeSpeed", "speed of Nodes", nodeSpeed);
    cmd.AddValue("whichParam", "which Parameter we are varying in simulation", whichParam);
    cmd.Parse(argc, argv);


    Time::SetResolution(Time::NS);

    int nsNodes = nNodes/2; // no of sender
    int nrNodes = nNodes - nNodes/2; // no of reciever
    nFlows = std::max(nsNodes, nFlows); // no of flows min of sender and given


   

    std::cout << nNodes << " " << nFlows << " " << nPackets << " " << nodeSpeed << std::endl;  

    // LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    // Create nodes, channels and devices, and set the networks

    // bottleneck p2p connection
    // Nodes: x and y

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);



    // S wifi connection

    NodeContainer wifiStaNodes_S;
    wifiStaNodes_S.Create(nsNodes);
    NodeContainer wifiApNode_S = p2pNodes.Get(0); // x


    
    YansWifiChannelHelper channel_S = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy_S;
    phy_S.SetChannel(channel_S.Create()); 


    WifiMacHelper mac_S;
    Ssid ssidS = Ssid("ns-3-ssid"); // creates an 802.11 service set identifier (SSID) 

    WifiHelper wifi;

    // ActiveProbing false -  probe requests will not be sent by MACs created by this
    // helper, and stations will listen for AP beacons.
    NetDeviceContainer staDevices_S;
    mac_S.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidS), "ActiveProbing", BooleanValue(false));
    staDevices_S = wifi.Install(phy_S, mac_S, wifiStaNodes_S);

    NetDeviceContainer apDevices_S;
    mac_S.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidS));
    apDevices_S = wifi.Install(phy_S, mac_S, wifiApNode_S);


    // R wifi connection

    NodeContainer wifiStaNodes_R;
    wifiStaNodes_R.Create(nrNodes);
    NodeContainer wifiApNode_R = p2pNodes.Get(1); // y


    
    YansWifiChannelHelper channel_R = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy_R;
    phy_R.SetChannel(channel_R.Create()); 


    WifiMacHelper mac_R;
    Ssid ssid_R = Ssid("ns-3-ssid"); // creates an 802.11 service set identifier (SSID) 



    // ActiveProbing false -  probe requests will not be sent by MACs created by this
    // helper, and stations will listen for AP beacons.
    NetDeviceContainer staDevices_R;
    mac_R.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid_R), "ActiveProbing", BooleanValue(false));
    staDevices_R = wifi.Install(phy_R, mac_R, wifiStaNodes_R);

    NetDeviceContainer apDevices_R;
    mac_R.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid_R));
    apDevices_R = wifi.Install(phy_R, mac_R, wifiApNode_R);



    //Setting mobility(static) in S wifi and R wifi

    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(1.0), //coverageArea
                                  "DeltaY",
                                  DoubleValue(1.0),
                                  "GridWidth",
                                  UintegerValue(10),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant="+std::to_string(nodeSpeed)+"]"));

    mobility.Install(wifiStaNodes_S);
    mobility.Install(wifiStaNodes_R);


    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode_S);
    mobility.Install(wifiApNode_R);

  

  // Install Internet stack

    InternetStackHelper stack;
    stack.Install(p2pNodes);
    stack.Install(wifiStaNodes_S);
    stack.Install(wifiStaNodes_R);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer sNodesInterfaces, sApNodesInterfaces;
    sNodesInterfaces = address.Assign(staDevices_S);
    sApNodesInterfaces = address.Assign(apDevices_S);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer rNodesInterfaces, rApNodesInterfaces;
    rNodesInterfaces = address.Assign(staDevices_R);
    rApNodesInterfaces = address.Assign(apDevices_R);


    // Create applications


  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), 9));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < nrNodes; ++i){
    // create sink app on R side node
    sinkApps.Add (packetSinkHelper.Install (wifiStaNodes_R.Get(i)));
  }
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (6.5));

  uint64_t dataRate = (nPackets * packetSize * 8);

  // Install on/off app on all right side nodes
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  clientHelper.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
  
  ApplicationContainer clientApps;
  int cur_flow_count = 0;
  for (uint32_t i = 0; i < nrNodes; ++i)
    {
      // Create an on/off app on S side node which sends packets to the R side
      AddressValue remoteAddress (InetSocketAddress (rNodesInterfaces.GetAddress(i), 9));
      
      for(uint32_t j = 0; j < nsNodes; ++j)
      {
        clientHelper.SetAttribute ("Remote", remoteAddress);
        clientApps.Add (clientHelper.Install (wifiStaNodes_S.Get(j)));

        cur_flow_count++;
        if(cur_flow_count >= nFlows)
          break;
      }

      if(cur_flow_count >= nFlows)
          break;
    }
  clientApps.Start (Seconds (2.0)); // Start 1 second after sink
  clientApps.Stop (Seconds (6.0)); // Stop before the sink  


  //Dynamic Routing

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (6.5)); // force stop,



  // Set up tracing for metrics (throughput, packet delivery ratio)

  // Flow monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
  Simulator::Schedule (Seconds (2.2), &TraceMetrics, flowMonitor);


  // Run simulation

  // std::cout << "Running the simulation :( " <<file_path<< std::endl;
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile(file_path+"/flow.xml", true, true);
  
  // std::cout << "Destroying the simulation" << std::endl;

  Simulator::Destroy ();

  double avgThroughput = thorughput / count;
  double avgDelivery_ratio = delivery_ratio / count;

  if(whichParam == 1) {

    std::ofstream ofs_thp (file_path+"/throughput_nodes.dat", std::ios::out | std::ios::app);
    std::ofstream ofs_deliver (file_path+"/deliveryR_nodes.dat", std::ios::out | std::ios::app);
    ofs_thp<<  nNodes << " " << avgThroughput << std::endl; // throughput (bit/s)
    ofs_deliver <<  nNodes << " " << avgDelivery_ratio<< std::endl; // delivery ratio (%)

  } else if (whichParam == 2) {

    std::ofstream ofs_thp (file_path+"/throughput_flows.dat", std::ios::out | std::ios::app);
    std::ofstream ofs_deliver (file_path+"/deliveryR_flows.dat", std::ios::out | std::ios::app);
    ofs_thp <<  nFlows << " " << avgThroughput << std::endl; // throughput (bit/s)
    ofs_deliver <<  nFlows << " " << avgDelivery_ratio << std::endl; // delivery ratio (%)

  } else if(whichParam == 3) {

    std::ofstream ofs_thp (file_path+"/throughput_packets.dat", std::ios::out | std::ios::app);
    std::ofstream ofs_deliver (file_path+"/deliveryR_packets.dat", std::ios::out | std::ios::app);
    ofs_thp <<  nPackets << " " << avgThroughput << std::endl; // throughput (bit/s)
    ofs_deliver <<  nPackets << " " << avgDelivery_ratio << std::endl; // delivery ratio (%)

  } else if(whichParam == 4) {

    std::ofstream ofs_thp (file_path+"/throughput_speeds.dat", std::ios::out | std::ios::app);
    std::ofstream ofs_deliver (file_path+"/deliveryR_speeds.dat", std::ios::out | std::ios::app);
    ofs_thp <<  nodeSpeed << " " << avgThroughput << std::endl; // throughput (bit/s)
    ofs_deliver <<  nodeSpeed << " " << avgDelivery_ratio << std::endl; // delivery ratio (%)

  }



  return 0;
}