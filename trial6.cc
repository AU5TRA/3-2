/*
 * SARED: Stochastically Adaptive Random Early Detection
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"  

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SaredSimulation");

// =================================
// SARED Queue Disc Implementation
// =================================

class SaredQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId (void);
  SaredQueueDisc ();
  virtual ~SaredQueueDisc ();

protected:
  virtual void DoDispose (void);
  virtual void InitializeParams (void);
  virtual bool CheckConfig (void);
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);

private:
  void UpdateSared (void);
  void CalculateTransitionProbabilities (double newAve);
  double CalculateP (void);

  // RED Parameters
  double m_minTh;
  double m_maxTh;
  double m_wq;
  double m_curMaxP;
  uint32_t m_meanPktSize;

  // State Variables
  double m_qAvg;
  Time m_qTime;
  int32_t m_count;

  // SARED Specific
  Time m_interval;
  Time m_lastSet;
  double m_oldAve;

  // Markov Transition Data
  double m_n[3][3];   // Transition counts
  double m_p[3][3];   // Transition probabilities
  double m_targetP[3]; // Target stationary probabilities

  Ptr<UniformRandomVariable> m_uv; // random generator for dropping
};

NS_OBJECT_ENSURE_REGISTERED (SaredQueueDisc);

TypeId SaredQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SaredQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<SaredQueueDisc> ()
    .AddAttribute ("MinTh", "Minimum average queue threshold",
                   DoubleValue (40), MakeDoubleAccessor (&SaredQueueDisc::m_minTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxTh", "Maximum average queue threshold",
                   DoubleValue (120), MakeDoubleAccessor (&SaredQueueDisc::m_maxTh),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MeanPktSize", "Average packet size in bytes",
                   UintegerValue (1500), MakeUintegerAccessor (&SaredQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QW", "Queue weight for EWMA",
                   DoubleValue (0.002), MakeDoubleAccessor (&SaredQueueDisc::m_wq),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Interval", "Interval for updating transition probabilities",
                   StringValue ("0.5s"), MakeTimeAccessor (&SaredQueueDisc::m_interval),
                   MakeTimeChecker ());
  return tid;
}

SaredQueueDisc::SaredQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  m_curMaxP = 0.125;
  m_qAvg    = 0.0;
  m_count   = -1;  //not in drop zone yet
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

  // Target distro
  m_targetP[0] = 0.05;
  m_targetP[1] = 0.90;
  m_targetP[2] = 0.05;
}

SaredQueueDisc::~SaredQueueDisc () {}

void SaredQueueDisc::DoDispose (void)
{
  QueueDisc::DoDispose ();
}

void SaredQueueDisc::InitializeParams (void)
{
  m_lastSet = Simulator::Now ();
}

bool SaredQueueDisc::CheckConfig (void)
{
  if (m_minTh >= m_maxTh)
    {
      NS_LOG_ERROR ("MinTh must be less than MaxTh");
      return false;
    }
  if (GetNInternalQueues() == 0)
  {
    uint32_t bufPkts = static_cast<uint32_t>(m_maxTh * 3);
    auto q = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
        "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, bufPkts)));
    AddInternalQueue(q);
  }

  return true;
}

Ptr<QueueDiscItem> SaredQueueDisc::DoDequeue (void)
{
  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
  if (!item)
    return nullptr;
  //track when queue goes idle
  if (GetInternalQueue (0)->GetNPackets () == 0)
    m_qTime = Simulator::Now ();
  return item;
}

Ptr<const QueueDiscItem> SaredQueueDisc::DoPeek (void)
{
  return GetInternalQueue (0)->Peek ();
}
bool SaredQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  uint32_t nQueued = GetInternalQueue (0)->GetNPackets ();

  if (nQueued == 0)
    {
      double idleSecs = (Simulator::Now () - m_qTime).GetSeconds ();
      // paper decay: avg = avg * (1-wq)^m
      double pktTime = (double)m_meanPktSize * 8.0 / (2e6); // seconds per pkt at 2Mbps bottleneck
      double m_idle  = idleSecs / pktTime;
      m_qAvg = m_qAvg * std::pow (1.0 - m_wq, m_idle);
    }
  else
    {
      m_qAvg = (1.0 - m_wq) * m_qAvg + m_wq * (double)nQueued;
    }

  if (Simulator::Now () > m_lastSet + m_interval)
    {
      UpdateSared ();
    }

  // drop decision using current m_curMaxP
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
          double denom = 1.0 - (double)m_count * pb;
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
      // Q avg above maxTh = drop
      drop = true;
      m_count = 0;
    }

  if (drop)
    {
      DropBeforeEnqueue (item, "SARED Drop");
      return false;
    }

  bool enqueued = GetInternalQueue (0)->Enqueue (item);
  if (!enqueued)
    DropBeforeEnqueue (item, "Queue Full");
  return enqueued;
}

double SaredQueueDisc::CalculateP (void)
{
  return m_curMaxP * (m_qAvg - m_minTh) / (m_maxTh - m_minTh);
}
// ---------------------------------------------------------------------------
void SaredQueueDisc::UpdateSared (void)
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
          //beta = (p22 - p21 - p10)*p1
          beta = (m_p[2][2] - m_p[2][1] - m_p[1][0]) * m_targetP[1];
          if (beta <= 0.0) beta = 0.1;
          m_curMaxP = m_curMaxP * beta;
        }
      else
        {
          // beta = (p11 - p10)*p1
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
          // alpha=p12*(p0+p2); beta=p11*p1
          alpha     = m_p[1][2] * (m_targetP[0] + m_targetP[2]);
          beta      = m_p[1][1] * m_targetP[1];
          m_curMaxP = m_curMaxP * beta + alpha;
        }
    }
  else  // ave >= th_max - part 
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

  // max_p has to be valid 
  if (m_curMaxP > 1.0) m_curMaxP = 1.0;
  if (m_curMaxP < 1e-5) m_curMaxP = 1e-5;

  m_lastSet = Simulator::Now ();
  m_oldAve  = m_qAvg;
}

void SaredQueueDisc::CalculateTransitionProbabilities (double newAve)
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


std::ofstream g_queueSizeTrace;

void QueueSizeTrace (Ptr<QueueDisc> qd)
{
  g_queueSizeTrace << Simulator::Now ().GetSeconds () << " "
                   << qd->GetNPackets () << std::endl;
  Simulator::Schedule (Seconds (0.1), &QueueSizeTrace, qd);
}

int main (int argc, char *argv[])
{
  LogComponentEnable ("SaredSimulation", LOG_LEVEL_INFO);

  uint32_t thMin     = 40;
  uint32_t thMax     = 120;
  uint32_t bufferSize = 280;
  std::string linkRate  = "2Mbps";
  std::string linkDelay = "20ms";

  CommandLine cmd;
  cmd.AddValue ("thMin",      "RED min threshold",  thMin);
  cmd.AddValue ("thMax",      "RED max threshold",  thMax);
  cmd.AddValue ("bufferSize", "Buffer size (pkts)", bufferSize);
  cmd.Parse (argc, argv);

  NodeContainer sources, sinks, routers;
  sources.Create (3);
  sinks.Create (2);
  routers.Create (2);

  PointToPointHelper leafLink;
  leafLink.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  leafLink.SetChannelAttribute ("Delay",    StringValue ("10ms"));

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute  ("DataRate", StringValue (linkRate));
  bottleneckLink.SetChannelAttribute ("Delay",    StringValue (linkDelay));

  std::vector<NetDeviceContainer> srcLinks;
  for (uint32_t i = 0; i < sources.GetN (); ++i)
    srcLinks.push_back (leafLink.Install (sources.Get (i), routers.Get (0)));

  NetDeviceContainer routerDevs = bottleneckLink.Install (routers.Get (0), routers.Get (1));

  std::vector<NetDeviceContainer> sinkLinks;
  for (uint32_t i = 0; i < sinks.GetN (); ++i)
    sinkLinks.push_back (leafLink.Install (routers.Get (1), sinks.Get (i)));

  InternetStackHelper stack;
  stack.Install (sources);
  stack.Install (routers);
  stack.Install (sinks);

  // SARED router 0 -> router 1
  TrafficControlHelper tch;

  tch.SetRootQueueDisc ("ns3::SaredQueueDisc", "MinTh", DoubleValue (thMin), "MaxTh", DoubleValue (thMax));

  QueueDiscContainer qDiscs = tch.Install (routerDevs.Get (0));

  Ipv4AddressHelper address;
  for (uint32_t i = 0; i < srcLinks.size (); ++i)
    {
      std::ostringstream subnet;
      subnet << "10.1." << i + 1 << ".0";
      address.SetBase (subnet.str ().c_str (), "255.255.255.0");
      address.Assign (srcLinks[i]);
    }

  address.SetBase ("10.2.1.0", "255.255.255.0");
  address.Assign (routerDevs);

  for (uint32_t i = 0; i < sinkLinks.size (); ++i)
    {
      std::ostringstream subnet;
      subnet << "10.3." << i + 1 << ".0";
      address.SetBase (subnet.str ().c_str (), "255.255.255.0");
      address.Assign (sinkLinks[i]);
    }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 50000;

  // 6 steady background flows → sink 0 (runs full 360s)
  for (uint32_t i = 0; i < 6; ++i)
    {
      Address sinkAddr (InetSocketAddress (Ipv4Address ("10.3.1.2"), port + i));
      BulkSendHelper bsrc ("ns3::TcpSocketFactory", sinkAddr);
      bsrc.SetAttribute ("MaxBytes", UintegerValue (0));
      ApplicationContainer app = bsrc.Install (sources.Get (i % 3));
      app.Start (Seconds (0.0));
      app.Stop  (Seconds (360.0));

      PacketSinkHelper sink ("ns3::TcpSocketFactory",
                             InetSocketAddress (Ipv4Address::GetAny (), port + i));
      sink.Install (sinks.Get (0));
    }

  // 20 burst flows -> sink 1 (starts at 120s, ends at 300s)
  for (uint32_t i = 0; i < 20; ++i)
    {
      Address sinkAddr (InetSocketAddress (Ipv4Address ("10.3.2.2"), port + 100 + i));
      BulkSendHelper bsrc ("ns3::TcpSocketFactory", sinkAddr);
      bsrc.SetAttribute ("MaxBytes", UintegerValue (0));
      ApplicationContainer app = bsrc.Install (sources.Get (i % 3));
      app.Start (Seconds (120.0));
      app.Stop  (Seconds (300.0));

      PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port + 100 + i));
      sink.Install (sinks.Get (1));
    }




  g_queueSizeTrace.open ("sared-queue.dat");
  Simulator::Schedule (Seconds (0.1), &QueueSizeTrace, qDiscs.Get (0));

  std::ofstream plotFile ("sared-queue.plt");
  plotFile << "set terminal png size 800,600\n"
           << "set output 'sared-queue.png'\n"
           << "set title 'SARED Queue Size Evolution'\n"
           << "set xlabel 'Time (s)'\n"
           << "set ylabel 'Queue Size (Packets)'\n"
           << "set yrange [0:300]\n"
           << "plot 'sared-queue.dat' using 1:2 with lines title 'Queue Size'\n";
  plotFile.close ();

  NS_LOG_INFO ("Starting Simulation...");
  Simulator::Stop (Seconds (360.0));
  Simulator::Run ();
  Simulator::Destroy ();

  g_queueSizeTrace.close ();
  // NS_LOG_INFO ("Done. Run: gnuplot sared-queue.plt");

  return 0;
}