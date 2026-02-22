#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Sared-Ared");

class SaredQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId (void);
  SaredQueueDisc ();
  ~SaredQueueDisc () override;

protected:
  bool DoEnqueue (Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue (void) override;
  Ptr<const QueueDiscItem> DoPeek () override;

  bool CheckConfig (void) override;
  void DoDispose (void) override;
  void InitializeParams (void) override;
  
private:
  void UpdateSared (void);
  void CalculateTransitionProbabilities (double newAve);
  double CalculateP (void);

  // RED Parameters
  double   m_minTh;
  double   m_maxTh;
  double   m_wq;
  double   m_curMaxP;
  uint32_t m_meanPktSize;

  // Buffer + link parameters (for idle decay)
  uint32_t m_bufferSizePkts;
  DataRate m_linkBandwidth;

  // State Variables
  double m_qAvg;
  Time   m_qTime;
  int32_t m_count;

  // SARED Specific
  Time   m_interval;
  Time   m_lastSet;
  double m_oldAve;

  // Markov Transition Data
  double m_n[3][3];
  double m_p[3][3];
  double m_targetP[3];

  Ptr<UniformRandomVariable> m_uv;
};

NS_OBJECT_ENSURE_REGISTERED (SaredQueueDisc);

TypeId
SaredQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SaredQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<SaredQueueDisc> ()
    .AddAttribute ("MinTh", "Minimum average queue threshold (packets)",
                   DoubleValue (20), MakeDoubleAccessor (&SaredQueueDisc::m_minTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh", "Maximum average queue threshold (packets)",
                   DoubleValue (60), MakeDoubleAccessor (&SaredQueueDisc::m_maxTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MeanPktSize", "Average packet size in bytes",
                   UintegerValue (1500), MakeUintegerAccessor (&SaredQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QW", "Queue weight for EWMA",
                   DoubleValue (0.002), MakeDoubleAccessor (&SaredQueueDisc::m_wq),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Interval", "Interval for updating transition probabilities",
                   StringValue ("0.5s"), MakeTimeAccessor (&SaredQueueDisc::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("BufferSize", "Internal queue limit (packets)",
                   UintegerValue (120), MakeUintegerAccessor (&SaredQueueDisc::m_bufferSizePkts),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("LinkBandwidth", "Bottleneck link bandwidth (for idle EWMA decay)",
                   DataRateValue (DataRate ("5Mbps")),
                   MakeDataRateAccessor (&SaredQueueDisc::m_linkBandwidth),
                   MakeDataRateChecker ())
    .AddAttribute ("MaxSize",
                  "Maximum size of the internal queue",
                  QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, 120)),
                  MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                        &QueueDisc::GetMaxSize),
                  MakeQueueSizeChecker ());
  return tid;
}

SaredQueueDisc::SaredQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  m_curMaxP = 0.125; // initial max_p
  m_qAvg    = 0.0;
  m_count   = -1;
  m_lastSet = Seconds (0.0);
  m_oldAve  = 0.0;
  m_qTime   = Seconds (0.0);

  m_uv = CreateObject<UniformRandomVariable> ();

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      {
        m_n[i][j] = 1.0;
        m_p[i][j] = 1.0 / 3.0;
      }

  // Target stationary distribution (paper-style)
  m_targetP[0] = 0.05;
  m_targetP[1] = 0.90;
  m_targetP[2] = 0.05;
}

SaredQueueDisc::~SaredQueueDisc () {}

void
SaredQueueDisc::DoDispose (void)
{
  QueueDisc::DoDispose ();
}

// void
// SaredQueueDisc::InitializeParams (void)
// {
//   m_lastSet = Simulator::Now ();
// }

// void SaredQueueDisc::InitializeParams (void)
// {
//   // keep QueueDisc stats consistent with the internal queue limit
//   SetMaxSize (QueueSize (QueueSizeUnit::PACKETS, m_bufferSizePkts));
//   m_lastSet = Simulator::Now ();
// }
void
SaredQueueDisc::InitializeParams (void)
{
  SetMaxSize (QueueSize (QueueSizeUnit::PACKETS, m_bufferSizePkts));

  // IMPORTANT: CheckConfig() runs before InitializeParams(), so resync here
  if (GetNInternalQueues () > 0)
    {
      GetInternalQueue (0)->SetMaxSize (GetMaxSize ());
    }

  m_lastSet = Simulator::Now ();
  m_qTime   = Simulator::Now ();   // avoids huge “idle” on first enqueue
  m_qAvg    = 0.0;
  m_count   = -1;
  m_oldAve  = 0.0;
}

// bool
// SaredQueueDisc::CheckConfig (void)
// {
//   if (m_minTh >= m_maxTh)
//     {
//       NS_LOG_ERROR ("MinTh must be less than MaxTh");
//       return false;
//     }

//   if (GetNInternalQueues () == 0)
//     {
//       auto q = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
//         "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, m_bufferSizePkts)));
//       AddInternalQueue (q);
//     }
//   return true;
// }

bool SaredQueueDisc::CheckConfig (void)
{
  if (m_minTh >= m_maxTh)
    {
      NS_LOG_ERROR ("MinTh must be less than MaxTh");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      QueueSize qs = GetMaxSize ();
      if (qs.GetUnit () != QueueSizeUnit::PACKETS)
        {
          // force packets for this experiment
          qs = QueueSize (QueueSizeUnit::PACKETS, 120);
        }

      auto q = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
        "MaxSize", QueueSizeValue (qs));
      AddInternalQueue (q);
    }
  else
    {
      GetInternalQueue (0)->SetMaxSize (GetMaxSize ());
    }

  return true;
}   

Ptr<QueueDiscItem>
SaredQueueDisc::DoDequeue (void)
{
  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
  if (!item)
    return nullptr;

  if (GetInternalQueue (0)->GetNPackets () == 0)
    m_qTime = Simulator::Now ();

  return item;
}

Ptr<const QueueDiscItem>
SaredQueueDisc::DoPeek (void)
{
  return GetInternalQueue (0)->Peek ();
}

bool
SaredQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  uint32_t nQueued = GetInternalQueue (0)->GetNPackets ();

  if (nQueued == 0)
    {
      double idleSecs = (Simulator::Now () - m_qTime).GetSeconds ();
      double bitrate  = (double) m_linkBandwidth.GetBitRate ();
      double pktTime  = (double) m_meanPktSize * 8.0 / bitrate; // seconds per packet at bottleneck rate
      if (pktTime <= 0) pktTime = 1e-6;
      double m_idle   = idleSecs / pktTime;
      m_qAvg = m_qAvg * std::pow (1.0 - m_wq, m_idle);
    }
  else
    {
      m_qAvg = (1.0 - m_wq) * m_qAvg + m_wq * (double) nQueued;
    }

  if (Simulator::Now () > m_lastSet + m_interval)
    {
      UpdateSared ();
    }

  bool drop = false;

  if (m_qAvg < m_minTh)
    {
      m_count = -1;
    }
  else if (m_qAvg < m_maxTh)
    {
      if (m_count < 0) m_count = 0;
      m_count++;

      double pb = CalculateP ();

      if (pb > 0.0)
        {
          double denom = 1.0 - (double) m_count * pb;
          if (denom <= 0.0)
            {
              drop = true;
              m_count = 0;
            }
          else
            {
              double pa = pb / denom;
              if (m_uv->GetValue () < pa)
                {
                  drop = true;
                  m_count = 0;
                }
            }
        }
    }
  else
    {
      drop = true;
      m_count = 0;
    }

  if (drop)
    {
      DropBeforeEnqueue (item, "SARED Drop");
      return false;
    }

  // bool enqueued = GetInternalQueue (0)->Enqueue (item);
  // if (!enqueued)
  //   DropBeforeEnqueue (item, "Queue Full");
  // return enqueued;

  bool enqueued = GetInternalQueue (0)->Enqueue (item);
  if (!enqueued)
  {
    // DropBeforeEnqueue (item, "Internal queue full"); 
    return false;
  }
  return true;
}

double
SaredQueueDisc::CalculateP (void)
{
  return m_curMaxP * (m_qAvg - m_minTh) / (m_maxTh - m_minTh);
}

void
SaredQueueDisc::UpdateSared (void)
{
  CalculateTransitionProbabilities (m_qAvg);

  double part  = 0.4 * (m_maxTh - m_minTh);
  double alpha = 0.0;
  double beta  = 1.0;

  if (m_qAvg <= m_minTh + part)
    {
      if (m_curMaxP <= 0.01)
        {
          alpha = m_p[0][1] * (m_targetP[0] + m_targetP[2]);
          beta  = m_p[0][0] * m_targetP[1];
          m_curMaxP = m_curMaxP * beta + alpha;
        }
      else if (m_curMaxP >= 0.5)
        {
          beta = (m_p[2][2] - m_p[2][1] - m_p[1][0]) * m_targetP[1];
          if (beta <= 0.0) beta = 0.1;
          m_curMaxP = m_curMaxP * beta;
        }
      else
        {
          beta = (m_p[1][1] - m_p[1][0]) * m_targetP[1];
          if (beta <= 0.0) beta = 0.1;
          m_curMaxP = m_curMaxP * beta;
        }
    }
  else if (m_qAvg < m_maxTh - part)
    {
      if (m_curMaxP <= 0.01)
        {
          alpha     = m_p[0][1] * (m_targetP[0] + m_targetP[2]);
          m_curMaxP = m_curMaxP + alpha;
        }
      else if (m_curMaxP >= 0.5)
        {
          beta = (m_p[2][2] - m_p[2][1]) * m_targetP[1];
          if (beta <= 0.0) beta = 0.1;
          m_curMaxP = m_curMaxP * beta;
        }
      else
        {
          alpha     = m_p[1][2] * (m_targetP[0] + m_targetP[2]);
          beta      = m_p[1][1] * m_targetP[1];
          m_curMaxP = m_curMaxP * beta + alpha;
        }
    }
  else
    {
      if (m_curMaxP <= 0.01)
        {
          alpha     = (m_p[0][1] + m_p[1][2]) * (m_targetP[0] + m_targetP[2]);
          m_curMaxP = m_curMaxP + alpha;
        }
      else if (m_curMaxP >= 0.5)
        {
          alpha     = m_p[2][1] * (m_targetP[0] + m_targetP[2]);
          beta      = m_p[2][2] * m_targetP[1];
          m_curMaxP = m_curMaxP * beta + alpha;
        }
      else
        {
          alpha     = m_p[1][2] * (m_targetP[0] + m_targetP[2]);
          m_curMaxP = m_curMaxP + alpha;
        }
    }

  if (m_curMaxP > 1.0)  m_curMaxP = 1.0;
  if (m_curMaxP < 1e-5) m_curMaxP = 1e-5;

  m_lastSet = Simulator::Now ();
  m_oldAve  = m_qAvg;
}

void
SaredQueueDisc::CalculateTransitionProbabilities (double newAve)
{
  int stateOld = (m_oldAve < m_minTh) ? 0 : (m_oldAve < m_maxTh ? 1 : 2);
  int stateNew = (newAve   < m_minTh) ? 0 : (newAve   < m_maxTh ? 1 : 2);

  m_n[stateOld][stateNew] += 1.0;

  for (int i = 0; i < 3; i++)
    {
      double sum = m_n[i][0] + m_n[i][1] + m_n[i][2];
      if (sum > 0)
        {
          m_p[i][0] = m_n[i][0] / sum;
          m_p[i][1] = m_n[i][1] / sum;
          m_p[i][2] = m_n[i][2] / sum;
        }
    }
}

// =========================================
// Experiment runner (build topology each run)
// =========================================
static double
RunOnce (uint32_t nFlows,
         bool useSared,
         uint32_t thMin,
         uint32_t thMax,
         uint32_t bufferSizePkts,
         std::string accessRate,
         std::string accessDelay,
         std::string bottleneckRate,
         std::string bottleneckDelay,
         double simTimeSec)
{
  // Topology: Source --(access)-- R1 --(bottleneck)-- R2 --(access)-- Sink
  NodeContainer src, sink, routers;
  src.Create (1);
  sink.Create (1);
  routers.Create (2);

  PointToPointHelper access;
  access.SetDeviceAttribute  ("DataRate", StringValue (accessRate));
  access.SetChannelAttribute ("Delay",    StringValue (accessDelay));

  PointToPointHelper bottleneck;
  bottleneck.SetDeviceAttribute  ("DataRate", StringValue (bottleneckRate));
  bottleneck.SetChannelAttribute ("Delay",    StringValue (bottleneckDelay));

  NetDeviceContainer dSrcR1 = access.Install (src.Get (0), routers.Get (0));
  NetDeviceContainer dR2Snk = access.Install (routers.Get (1), sink.Get (0));
  NetDeviceContainer dR1R2  = bottleneck.Install (routers.Get (0), routers.Get (1));

  InternetStackHelper stack;
  stack.Install (src);
  stack.Install (routers);
  stack.Install (sink);

  // Install queue disc on R1 -> R2 device (bottleneck egress from R1)
  TrafficControlHelper tch;

  if (useSared)
    {
      tch.SetRootQueueDisc ("ns3::SaredQueueDisc",
                            "MinTh", DoubleValue (thMin),
                            "MaxTh", DoubleValue (thMax),
                            "QW", DoubleValue (0.002),
                            "MeanPktSize", UintegerValue (1500),
                            "Interval", StringValue ("0.5s"),
                            "BufferSize", UintegerValue (bufferSizePkts),
                            "LinkBandwidth", DataRateValue (DataRate (bottleneckRate)));
                            // "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, bufferSizePkts)));
    }
  else
    {
      // Built-in RED with ARED enabled (traffic-control module)
      // NOTE: ns-3 uses LInterm ~ 1/max_p in classic ns2-compatible parameterization.
      tch.SetRootQueueDisc ("ns3::RedQueueDisc",
                            "MinTh", DoubleValue (thMin),
                            "MaxTh", DoubleValue (thMax),
                            "QW", DoubleValue (0.002),
                            "MeanPktSize", UintegerValue (1500),
                            "LinkBandwidth", StringValue (bottleneckRate),
                            "LinkDelay", StringValue (bottleneckDelay),
                            "ARED", BooleanValue (true),
                            "LInterm", DoubleValue (8.0), // ~ max_p = 1/8 = 0.125
                            "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, bufferSizePkts)));
    }

  QueueDiscContainer qdiscs = tch.Install (dR1R2.Get (0));
  (void) qdiscs;

  // Addressing
  Ipv4AddressHelper address;
  address.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifSrcR1 = address.Assign (dSrcR1);

  address.SetBase ("10.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ifR1R2  = address.Assign (dR1R2);

  address.SetBase ("10.0.3.0", "255.255.255.0");
  Ipv4InterfaceContainer ifR2Snk = address.Assign (dR2Snk);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Applications: nFlows BulkSend -> PacketSink (unique ports)
  const uint16_t basePort = 50000;

  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < nFlows; ++i)
    {
      uint16_t port = basePort + i;

      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), port));
      sinkApps.Add (sinkHelper.Install (sink.Get (0)));

      Address dst (InetSocketAddress (ifR2Snk.GetAddress (1), port));
      BulkSendHelper srcHelper ("ns3::TcpSocketFactory", dst);
      srcHelper.SetAttribute ("MaxBytes", UintegerValue (0));
      ApplicationContainer a = srcHelper.Install (src.Get (0));
      a.Start (Seconds (0.0));
      a.Stop  (Seconds (simTimeSec));
    }
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop  (Seconds (simTimeSec));

  // FlowMonitor
  FlowMonitorHelper flowmonHelper;
  Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll ();

  Simulator::Stop (Seconds (simTimeSec));
  Simulator::Run ();

  monitor->CheckForLostPackets ();

  // Compute loss% ONLY over forward data flows (dest port in [basePort, basePort+nFlows))
  Ptr<Ipv4FlowClassifier> classifier =
    DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());

  uint64_t totalTx = 0;
  uint64_t totalLost = 0;

  auto stats = monitor->GetFlowStats ();
  for (const auto &kv : stats)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (kv.first);
      if (t.protocol == 6 && // TCP
          t.destinationPort >= basePort &&
          t.destinationPort < (uint16_t)(basePort + nFlows))
        {
          totalTx   += kv.second.txPackets;
          totalLost += kv.second.lostPackets;
        }
    }

  Simulator::Destroy ();

  if (totalTx == 0) return 0.0;
  return 100.0 * (double) totalLost / (double) totalTx;
}

int
main (int argc, char *argv[])
{
  uint32_t thMin = 20;
  uint32_t thMax = 60;
  uint32_t bufferSize = 120;

  uint32_t minFlows = 5;
  uint32_t maxFlows = 100;
  uint32_t stepFlows = 5;

  std::string accessRate  = "10Mbps";
  std::string accessDelay = "10ms";
  std::string bottleneckRate  = "5Mbps";
  std::string bottleneckDelay = "20ms";

  double simTimeSec = 60.0;

  CommandLine cmd;
  cmd.AddValue ("thMin", "min threshold (pkts)", thMin);
  cmd.AddValue ("thMax", "max threshold (pkts)", thMax);
  cmd.AddValue ("bufferSize", "buffer size (pkts)", bufferSize);
  cmd.AddValue ("minFlows", "min number of flows", minFlows);
  cmd.AddValue ("maxFlows", "max number of flows", maxFlows);
  cmd.AddValue ("stepFlows", "step for flow sweep", stepFlows);
  cmd.AddValue ("simTime", "simulation time (s)", simTimeSec);
  cmd.AddValue ("accessRate", "access link data rate", accessRate);
  cmd.AddValue ("accessDelay", "access link delay", accessDelay);
  cmd.AddValue ("bottleneckRate", "bottleneck data rate", bottleneckRate);
  cmd.AddValue ("bottleneckDelay", "bottleneck delay", bottleneckDelay);
  cmd.Parse (argc, argv);

  std::ofstream out ("loss-vs-flows.dat");
  out << "# flows  ared_loss_pct  sared_loss_pct\n";

  std::cout << "Sweeping flows: [" << minFlows << ", " << maxFlows
            << "] step " << stepFlows << "\n";

  for (uint32_t n = minFlows; n <= maxFlows; n += stepFlows)
    {
      double aredLoss  = RunOnce (n, false, thMin, thMax, bufferSize,
                                 accessRate, accessDelay,
                                 bottleneckRate, bottleneckDelay,
                                 simTimeSec);

      double saredLoss = RunOnce (n, true, thMin, thMax, bufferSize,
                                 accessRate, accessDelay,
                                 bottleneckRate, bottleneckDelay,
                                 simTimeSec);

      out << n << " " << aredLoss << " " << saredLoss << "\n";

      std::cout << "nFlows=" << n
                << " | ARED loss%=" << aredLoss
                << " | SARED loss%=" << saredLoss << "\n";
    }

  out.close ();

  // gnuplot script (Fig. 9(b)-style)
  std::ofstream plt ("loss-vs-flows.plt");
  plt << "set terminal png size 900,650\n"
      << "set output 'loss-vs-flows.png'\n"
      << "set title 'Percentage Packet Loss vs Number of Flows'\n"
      << "set xlabel 'Number of TCP flows'\n"
      << "set ylabel 'Percentage Packet Loss (%)'\n"
      << "set grid\n"
      << "plot \\\n"
      << "  'loss-vs-flows.dat' using 1:2 with linespoints title 'ARED', \\\n"
      << "  'loss-vs-flows.dat' using 1:3 with linespoints title 'SARED'\n";
  plt.close ();

  std::cout << "\nWrote: loss-vs-flows.dat, loss-vs-flows.plt\n"
            << "Run:   gnuplot loss-vs-flows.plt\n"
            << "Output loss-vs-flows.png\n";

  return 0;
}
