
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/callback.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-module.h"

using namespace ns3;


// ===========================================================================
//
//            s0----      -----r0
//  senders - s1---b0 --- b1---r1 - receivers
//
// ===========================================================================
//
class TutorialApp : public Application
{
public:
  TutorialApp ();
  virtual ~TutorialApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  uint32_t        m_simultime;
};

TutorialApp::TutorialApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0),
    m_simultime (0)
{
}

TutorialApp::~TutorialApp ()
{
  m_socket = 0;
}

/* static */
TypeId TutorialApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("TutorialApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<TutorialApp> ()
    ;
  return tid;
}

void
TutorialApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_simultime = simultime;
}

void
TutorialApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
    if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
TutorialApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
TutorialApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if(Simulator::Now().GetSeconds() < m_simultime) ScheduleTx();
}

void
TutorialApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &TutorialApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}



int main(int argc, char *argv[]){


    std::string tcpVariant1 = "ns3::TcpNewReno";
    std::string tcpVariant2 = "ns3::TcpWestwoodPlus"; 
    
    int nLeaf = 2;
    int nFlows = 2;


    std::string output = "scratch/output";

    int simulationTimeInSeconds = 10;
    int cleanupTime = 2;
    int bttlnckRate = 50;
    int packet_loss_exp = 6;
    uint32_t payloadSize = 1472;

    int whichParam = 1;
    int whichCase = 1;
    // input from CMD
    CommandLine cmd (__FILE__);
    cmd.AddValue ("whichCase","1 for newReno vs WestwoodPlus, 2 for new vs High, 3 for new vs Adaptive", whichCase);
    cmd.AddValue ("bottleneckDataRate","Max Packets allowed in the device queue", bttlnckRate);
    cmd.AddValue ("plossRate", "Packet loss rate", packet_loss_exp);
    cmd.AddValue ("whichParam","1 for bottleneckDataRate, 2 for packet loss rate", whichParam);
    cmd.Parse (argc,argv);

    if(whichCase == 2) {
        tcpVariant2 = "ns3::TcpHighSpeed"; 
        output += "/RenoVsHigh";
    } else if(whichCase == 3) {
        tcpVariant2 = "ns3::TcpAdaptiveReno"; 
        output += "/RenoVsAdaptive";
    } else {
        tcpVariant2 = "ns3::TcpWestwoodPlus";
        output += "/RenoVsWest";
    }

    std::cout << output << "\n";

    double packet_loss_rate = (1.0 / std::pow(10, packet_loss_exp));

    std::string bottleNeckDataRate = std::to_string(bttlnckRate) + "Mbps";
    std::string bottleNeckDelay = "100ms";
    std::string leafDataRate = "1Gbps";
    std::string leafDelay = "1ms";


    NS_LOG_UNCOND("USING TCP 1 = "<<tcpVariant1<<" ; TCP 2 = "<<tcpVariant2<<" ; nLeaf = "<<nLeaf<<
                  " ; bottleneck rate = "<<bottleNeckDataRate<<
                  " ; packet loss rate = "<<packet_loss_rate);


    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

    // SETUP NODE AND DEVICE
    // Create the point-to-point link helpers

    //bottleneck p2phelper
    PointToPointHelper bottleNeckLink;
    bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckDataRate));
    bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckDelay));

    //leadnodes p2phelper
    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute  ("DataRate", StringValue (leafDataRate));
    pointToPointLeaf.SetChannelAttribute ("Delay", StringValue (leafDelay));
    int bandwidth_delay_product = 100 * 100;
    // add router buffer capacity - equal to bandwidth delay product
    pointToPointLeaf.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue (std::to_string (bandwidth_delay_product) + "p"));

    // the network
    PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                  nLeaf, pointToPointLeaf,
                                  bottleNeckLink);

    // add error rate
    // need device container but its private: made public graph becomes weird
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (packet_loss_rate));
    // set receive error for bottleneck -> 0 dont work
    d.m_routerDevices.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); 

    
    // INSTALL STACK

    // tcp variant 1
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant1));
   
    InternetStackHelper stack1;
    stack1.Install (d.GetLeft (0)); // left leaves[0] -> sender1 with tcpvariant1
    stack1.Install (d.GetRight (0)); // right leaves[0] -> receiver1 with tcpvariant1

    stack1.Install (d.GetLeft()); //bottleneck uses tcpcariant1
    stack1.Install (d.GetRight()); //bottleneck uses tcpcariant1J

    // tcp variant 2
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant2));
    
    InternetStackHelper stack2;
    stack2.Install (d.GetLeft (1)); // left leaves
    stack2.Install (d.GetRight (1)); // right leaves
      

    // ASSIGN IP Addresses
    d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"), // left nodes
                          Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),  // right nodes
                          Ipv4AddressHelper ("10.3.1.0", "255.255.255.0")); // routers 
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables (); // populate routing table
    

    // install flow monitor
    FlowMonitorHelper flowmon;
    flowmon.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(cleanupTime)));

    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    uint16_t sp = 8080;

    for(int i=0;i<nFlows; i++){
      
      // sinkApp as receiver in rightSide

      Address sinkAddress (InetSocketAddress (d.GetRightIpv4Address (i), sp));
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sp));
      ApplicationContainer sinkApps = packetSinkHelper.Install (d.GetRight (i));

      sinkApps.Start (Seconds (0));
      sinkApps.Stop (Seconds (simulationTimeInSeconds+cleanupTime));

      // sender in leftside

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (d.GetLeft (i), TcpSocketFactory::GetTypeId ());
      Ptr<TutorialApp> app = CreateObject<TutorialApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, payloadSize, DataRate (leafDataRate), simulationTimeInSeconds);
      d.GetLeft (i)->AddApplication (app);
      app->SetStartTime (Seconds (1));
      app->SetStopTime (Seconds (simulationTimeInSeconds));

      std::ostringstream oss;
      oss << output << "/cwndVsT" << i+1 <<  ".cwnd";
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str());
      ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
    }
    
  
    Simulator::Stop (Seconds (simulationTimeInSeconds+cleanupTime));
    Simulator::Run ();


    // flow monitor statistics

    float totalThroughputTCP1 = 0;
    float totalThroughputTCP2 = 0;

    double jain_index_numerator = 0;
    double jain_index_denominator = 0;


    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    int iterCount = 0;

    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {

      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); // classifier returns FiveTuple in correspondance to a flowID

      float curThroughput = iter->second.rxBytes * 8.0/((simulationTimeInSeconds+cleanupTime)*1000);

      if(t.sourceAddress == "10.1.1.1" || t.sourceAddress == "10.2.1.1") {
          totalThroughputTCP1 += curThroughput;
      } else {
          totalThroughputTCP2 += curThroughput;
      }

      jain_index_numerator += curThroughput;
      jain_index_denominator += (curThroughput * curThroughput);

      iterCount += 1;

    }

    double jain_index = (jain_index_numerator * jain_index_numerator) / ( iterCount * jain_index_denominator);
    double avgThroughputTCP1 = totalThroughputTCP1 / 2;
    double avgThroughputTCP2 = totalThroughputTCP2 / 2;
    
    
    if(whichParam == 1) {
      std::ofstream ofs_thp (output +"/thr_Vs_btlR.dat", std::ios::out | std::ios::app);
      ofs_thp<<  bttlnckRate << " " << avgThroughputTCP1 << " " << avgThroughputTCP2 << std::endl; // throughput (kbp/s)

      if(whichCase == 2) {
          std::ofstream ofs_thp (output +"/fairnessIdx_Vs_btlR.dat", std::ios::out | std::ios::app);
          ofs_thp<<  bttlnckRate << " " << jain_index << std::endl; 
      } else if (whichCase == 3) {
          std::ofstream ofs_thp (output +"/fairnessIdx_Vs_btlR.dat", std::ios::out | std::ios::app);
          ofs_thp<<  bttlnckRate << " " << jain_index << std::endl; 
      }
      

    } else if (whichParam == 2) {
      std::ofstream ofs_thp (output +"/thr_Vs_ploss.dat", std::ios::out | std::ios::app);
      ofs_thp<<  packet_loss_rate << " " << avgThroughputTCP1 << " " << avgThroughputTCP2 << std::endl; // throughput (kbp/s)

      if(whichCase == 2) {
          std::ofstream ofs_thp (output +"/fairnessIdx_Vs_ploss.dat", std::ios::out | std::ios::app);
          ofs_thp<<  packet_loss_rate << " " << jain_index << std::endl; 
      } else if (whichCase == 3) {
          std::ofstream ofs_thp (output +"/fairnessIdx_Vs_ploss.dat", std::ios::out | std::ios::app);
          ofs_thp<<  packet_loss_rate << " " << jain_index << std::endl; 
      }
    }

    Simulator::Destroy ();

    return 0;
}