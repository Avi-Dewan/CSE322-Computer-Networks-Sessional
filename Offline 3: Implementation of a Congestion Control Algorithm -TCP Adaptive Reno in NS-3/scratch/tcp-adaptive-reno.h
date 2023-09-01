#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"
#include "tcp-westwood-plus.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"
#include "ns3/event-id.h"

namespace ns3 {

class Packet;
class TcpHeader;
class Time;
class EventId;


class TcpAdaptiveReno : public TcpWestwoodPlus
{
public:
 
  static TypeId GetTypeId (void);

  TcpAdaptiveReno (void);
  
  TcpAdaptiveReno (const TcpAdaptiveReno& sock);
  virtual ~TcpAdaptiveReno (void);

  enum FilterType 
  {
    NONE,
    TUSTIN
  };

  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                          const Time& rtt);

  virtual Ptr<TcpCongestionOps> Fork ();

private:

  void EstimateBW (const Time& rtt, Ptr<TcpSocketState> tcb);

  double EstimateCongestionLevel();

  void EstimateIncWnd(Ptr<TcpSocketState> tcb);

protected:
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  Time                   minRtt;                 //!< Minimum RTT
  Time                   currentRtt;             //!< Current RTT
  Time                   jPacketLossRtt;            //!< RTT of j packet loss
  Time                   con_j_Rtt;                //!< Conjestion RTT (j th event)
  Time                   prevCon_j_Rtt;            //!< Previous Conjestion RTT (j-1 th event)

  // Window calculations
  int32_t                incWnd;                 //!< Increment Window
  uint32_t               Wbase;                //!< Base Window
  int32_t                Wprobe;               //!< Probe Window 
};

} // namespace ns3

#endif /* TCP_ADAPTIVE_RENO_H */
