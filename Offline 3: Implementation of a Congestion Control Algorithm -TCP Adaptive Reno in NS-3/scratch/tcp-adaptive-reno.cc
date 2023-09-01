#include "tcp-adaptive-reno.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "tcp-socket-base.h"

NS_LOG_COMPONENT_DEFINE ("TcpAdaptiveReno");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::TcpAdaptiveReno")
    .SetParent<TcpNewReno>()
    .SetGroupName ("Internet")
    .AddConstructor<TcpAdaptiveReno>()
    .AddAttribute("FilterType", "Use this to choose no filter or Tustin's approximation filter",
                  EnumValue(TcpAdaptiveReno::TUSTIN), MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                  MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
    .AddTraceSource("EstimatedBW", "The estimated bandwidth",
                    MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                    "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno (void) :
  TcpWestwoodPlus(),
  minRtt (Time (0)),
  currentRtt (Time (0)),
  jPacketLossRtt (Time (0)),
  con_j_Rtt (Time (0)),
  prevCon_j_Rtt(Time(0)),
  incWnd (0),
  Wbase (0),
  Wprobe (0)
{
  NS_LOG_FUNCTION (this);
}

TcpAdaptiveReno::TcpAdaptiveReno (const TcpAdaptiveReno& sock) :
  TcpWestwoodPlus (sock),
  minRtt (Time (0)),
  currentRtt (Time (0)),
  jPacketLossRtt (Time (0)),
  con_j_Rtt (Time (0)),
  prevCon_j_Rtt(Time(0)),
  incWnd (0),
  Wbase (0),
  Wprobe (0)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpAdaptiveReno::~TcpAdaptiveReno (void)
{
}


// This function is called each time a packet is Acked. It will increase the
// count of acked segments and update the current estimated bandwidth.

void
TcpAdaptiveReno::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                        const Time& rtt)
{


  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }

  m_ackedSegments += packetsAcked;


  // INITIALIZE AND SET VALUES FOR  m_minRtt, m_currentRtt


  // calculate min rtt 
  if(minRtt.IsZero()) { 
    minRtt = rtt; 
  }
  else if(rtt <= minRtt) { 
    minRtt = rtt; 
  }

  currentRtt = rtt;

  TcpWestwoodPlus::EstimateBW (rtt, tcb);
}

// EstimateCongestionLevel: Estimates the current congestion level using Round Trip Time (RTT). 
double
TcpAdaptiveReno::EstimateCongestionLevel()
{

  float a = 0.85; // exponential smoothing factor, as said in spec

  if(prevCon_j_Rtt< minRtt) a = 0; // the initial value should take the full current Jth loss Rtt
  
  double Rtt_cong_j = a*prevCon_j_Rtt.GetSeconds() + (1-a)*jPacketLossRtt.GetSeconds(); 

  con_j_Rtt = Seconds(Rtt_cong_j); 


  double c = std::min(
    (currentRtt.GetSeconds() - minRtt.GetSeconds()) / (Rtt_cong_j - minRtt.GetSeconds()),
    1.0
  );

  return c;
}


void 
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{

  double c = EstimateCongestionLevel(); //c is the estimated congestion level calculated in the EstimateCongestionLevel() function.

  int M = 1000; // Scalling factor, M = 10 mbps in paper 

  
  double B = m_currentBW.Get().GetBitRate();  // B is the current bandwidth calculated in 
                                              // TcpWestWoodPlus’s EstimateBW() function which is called in PktsAcked()

  double MSS = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize); //MSS is maximum segment size that is assumed 
                                                                              //as the square of the original SegmentSize
 
  double m_maxIncWnd =  B / M* MSS ; 

  double alpha = 10; 
  double beta = 2 * m_maxIncWnd * ((1/alpha) - ((1/alpha + 1)/(std::exp(alpha))));
  double gamma = 1 - (2 * m_maxIncWnd * ((1/alpha) - ((1/alpha + 0.5)/(std::exp(alpha)))));

  incWnd = (int)((m_maxIncWnd / std::exp(alpha * c)) + (beta * c) + gamma);
}

// This function increases congestion window in congestion
// avoidance phase
void
TcpAdaptiveReno::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{

  if (segmentsAcked > 0) {

      EstimateIncWnd(tcb);

      // In TCP-AReno, congestion window W is managed in two parts;
      // a base part, Wbase, and a fast probing part, Wprobe

      // base_window = USE NEW RENO IMPLEMENTATION

      // The base part maintains a congestion window size equivalent to TCP-Reno 

      // The base part is always increased like
      // TCP-Reno, i.e. 1MSS/RTT, while the increase of the probing part, Winc, is dynamically
      // adjusted

      double MSS1 = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize);

      double increase = MSS1 / tcb->m_cWnd.Get ();

      increase = std::max (1.0, increase);

      Wbase += static_cast<uint32_t> (increase); // Wbase = Wbase + 1MSS/W
      

      // change probe window
      // The probing part is
      // introduced to quickly fill the bottleneck link

      Wprobe = std::max(
        (double) (Wprobe + incWnd / (int)tcb->m_cWnd.Get()), 
        (double) 0
      );   // Wprobe = max(Wprobe + Winc/W, 0)
      
     
      tcb->m_cWnd = Wbase + Wprobe; // W = Wbase + Wprobe
    }

}

uint32_t
TcpAdaptiveReno::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)  // This function is called when a loss event has occured
{
  
  // Firstly, here we need to update the RT Tj−1cong & RT Tj

  prevCon_j_Rtt= con_j_Rtt; // a loss event has occured. so set the previous conjestion RTT
  jPacketLossRtt = currentRtt; // this will now contain the RTT of previous packet or jth loss event
  
  // we need to estimate the new reduced
  // congestion window size according to the congestion level

  double c = EstimateCongestionLevel(); // estiamted congestion level

  uint32_t tempSsthresh = (uint32_t) (tcb->m_cWnd / (1.0+c)); // Wbase = W ∗ Wdec = W / (1 + c)

  uint32_t ssthresh = std::max (
    2*tcb->m_segmentSize, 
    tempSsthresh
  );  //  the minimum value the ssthresh can take is 2*segmentSize similar to the other algorithms.
 
  // reset calculations
  Wbase = ssthresh; 
 
  Wprobe = 0; // Wprobe = 0
  
  return ssthresh;

}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork ()
{
  return CreateObject<TcpAdaptiveReno> (*this);
}

} 
