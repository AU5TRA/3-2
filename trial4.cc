/*
 * Simulation of RED (Random Early Detection) Queue
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RedSimulationScript");

std::ofstream dropLog;
std::ofstream queueLog;
std::ofstream statsLog;


uint64_t totalDrops = 0;
uint64_t totalPackets = 0;


static void
OnDrop (Ptr<const QueueDiscItem> item)
{
  totalDrops++;
  dropLog << Simulator::Now ().GetSeconds () 
          << "\t" << item->GetSize () 
          << std::endl;
  
  // NS_LOG_INFO ("Time: " << Simulator::Now ().GetSeconds () 
  //              << "s - Packet Dropped - Size: " << item->GetSize () 
  //              << " bytes - Total Drops: " << totalDrops);
}

static void
OnEnqueue (Ptr<const QueueDiscItem> item)
{
  totalPackets++;
  // NS_LOG_DEBUG ("Time: " << Simulator::Now ().GetSeconds () 
  //               << "s - Packet Enqueued - Size: " << item->GetSize () 
  //               << " bytes - Total Packets: " << totalPackets);
}

static void
OnDequeue (Ptr<const QueueDiscItem> item)
{
  // NS_LOG_DEBUG ("Time: " << Simulator::Now ().GetSeconds () 
  //               << "s - Packet Dequeued - Size: " << item->GetSize () 
  //               << " bytes");
}

static void
OnAvgQueueSizeChange (double oldVal, double newVal)
{
  queueLog << Simulator::Now ().GetSeconds () 
           << "\t" << oldVal 
           << "\t" << newVal 
           << std::endl;
  
  NS_LOG_DEBUG ("Time: " << Simulator::Now ().GetSeconds () 
                << "s - Avg Queue Size changed from " << oldVal 
                << " to " << newVal);
}

static void
OnQueueSizeChange (uint32_t oldVal, uint32_t newVal)
{
  NS_LOG_DEBUG ("Time: " << Simulator::Now ().GetSeconds () 
                << "s - Queue Size: " << newVal << " packets");
}


static void
LogStatistics (Ptr<QueueDisc> qDisc)
{
  Ptr<RedQueueDisc> redQueue = DynamicCast<RedQueueDisc> (qDisc);
  
  if (redQueue)
    {
      QueueDisc::Stats stats = redQueue->GetStats ();
      
      statsLog << Simulator::Now ().GetSeconds () 
               << "\t" << stats.nTotalEnqueuedPackets
               << "\t" << stats.nTotalDequeuedPackets
               << "\t" << stats.nTotalDroppedPackets
               << "\t" << redQueue->GetCurrentSize ().GetValue ()
               << std::endl;
      
      // NS_LOG_INFO ("Time: " << Simulator::Now ().GetSeconds () 
      //              << "s - Stats: Enqueued=" << stats.nTotalEnqueuedPackets
      //              << ", Dequeued=" << stats.nTotalDequeuedPackets
      //              << ", Dropped=" << stats.nTotalDroppedPackets
      //              << ", QueueSize=" << redQueue->GetCurrentSize ().GetValue ());
    }
  

  Simulator::Schedule (Seconds (0.5), &LogStatistics, qDisc);
}

int
main (int argc, char *argv[])
{
  double thMin = 20.0;   // start dropping at 10 pckt
  double thMax = 60.0;   // MaxP at 30 pckt
  uint32_t bufferSize = 150;  // Buffer for 100 packets
  double wq = 0.002;
  double maxP = 0.1;     // 30% max drop probability
  double lInterm = 50.0;
  
  uint32_t packetSize = 1024; 
  double simDuration = 30.0;  
  uint32_t numSources = 5;   
 
  std::string edgeRate = "10Mbps";
  std::string bottleneckRate = "500Kbps";  
  std::string sourceRate = "200Kbps";      
  
  std::string logLevel = "INFO";
  bool enablePcap = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("thMin", "RED minimum threshold", thMin);
  cmd.AddValue ("thMax", "RED maximum threshold", thMax);
  cmd.AddValue ("bufferSize", "Queue buffer size in packets", bufferSize);
  cmd.AddValue ("wq", "Queue weight", wq);
  cmd.AddValue ("maxP", "Maximum drop probability", maxP);
  cmd.AddValue ("simDuration", "Simulation duration in seconds", simDuration);
  cmd.AddValue ("numSources", "Number of traffic sources", numSources);
  cmd.AddValue ("bottleneckRate", "Bottleneck link data rate", bottleneckRate);
  cmd.AddValue ("sourceRate", "Per-source data rate", sourceRate);
  cmd.AddValue ("logLevel", "Log level (ERROR|WARN|INFO|DEBUG|ALL)", logLevel);
  cmd.AddValue ("enablePcap", "Enable PCAP tracing", enablePcap);
  cmd.Parse (argc, argv);

  if (logLevel == "ERROR")
    LogComponentEnable ("RedSimulationScript", LOG_LEVEL_ERROR);
  else if (logLevel == "WARN")
    LogComponentEnable ("RedSimulationScript", LOG_LEVEL_WARN);
  else if (logLevel == "INFO")
    LogComponentEnable ("RedSimulationScript", LOG_LEVEL_INFO);
  else if (logLevel == "DEBUG")
    LogComponentEnable ("RedSimulationScript", LOG_LEVEL_DEBUG);
  else if (logLevel == "ALL")
    LogComponentEnable ("RedSimulationScript", LOG_LEVEL_ALL);

  LogComponentEnable ("RedQueueDisc", LOG_LEVEL_WARN);
  
  std::string bottleneckNumStr = bottleneckRate.substr(0, bottleneckRate.find("Kbps"));
  std::string sourceNumStr = sourceRate.substr(0, sourceRate.find("Kbps"));
  
  dropLog.open ("red-drops.dat");
  dropLog << "# Time(s)\tPacketSize(bytes)" << std::endl;
  
  queueLog.open ("red-queue-avg.dat");
  queueLog << "# Time(s)\tOldAvgSize\tNewAvgSize" << std::endl;
  
  statsLog.open ("red-statistics.dat");
  statsLog << "# Time(s)\tEnqueued\tDequeued\tDropped\tQueueSize" << std::endl;

  NS_LOG_INFO ("Configuration: MinTh=" << thMin << ", MaxTh=" << thMax 
               << ", BufferSize=" << bufferSize << ", QW=" << wq 
               << ", MaxP=" << maxP);
  

  NodeContainer sources, sinks, routers;
  sources.Create (numSources);
  sinks.Create (numSources);
  routers.Create (2); 

  NS_LOG_INFO ("Topology: " << numSources << " sources, " << numSources << " sinks, 2 routers");

  PointToPointHelper edgeLink;
  edgeLink.SetDeviceAttribute ("DataRate", StringValue (edgeRate)); 
  edgeLink.SetChannelAttribute ("Delay", StringValue ("1ms"));
  edgeLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("10000p")); 

  NetDeviceContainer sourceDevices, sinkDevices;
  for (uint32_t i = 0; i < numSources; ++i) {
      sourceDevices.Add (edgeLink.Install (sources.Get (i), routers.Get (0)));
      sinkDevices.Add (edgeLink.Install (routers.Get (1), sinks.Get (i)));
  }
  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bottleneckRate));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue ("20ms"));
  bottleneckLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  NetDeviceContainer bottleneckDevices;
  bottleneckDevices = bottleneckLink.Install (routers.Get (0), routers.Get (1));

  InternetStackHelper stack;
  stack.Install (sources);
  stack.Install (routers);
  stack.Install (sinks);


  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::RedQueueDisc",
                        "MinTh", DoubleValue (thMin),
                        "MaxTh", DoubleValue (thMax),
                        "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, bufferSize)),
                        "QW", DoubleValue (wq),
                        "LInterm", DoubleValue (lInterm));
  
  QueueDiscContainer qDiscs = tch.Install (bottleneckDevices);

  for (uint32_t i = 0; i < qDiscs.GetN (); ++i) {
      Ptr<QueueDisc> q = qDiscs.Get (i);
      // uint32_t nodeId = bottleneckDevices.Get (i)->GetNode ()->GetId ();
      

      q->TraceConnectWithoutContext ("Drop", MakeCallback (&OnDrop));
      q->TraceConnectWithoutContext ("Enqueue", MakeCallback (&OnEnqueue));
      q->TraceConnectWithoutContext ("Dequeue", MakeCallback (&OnDequeue));
      q->TraceConnectWithoutContext ("AverageQueueSize", MakeCallback (&OnAvgQueueSizeChange));
      q->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&OnQueueSizeChange));
      
      Simulator::Schedule (Seconds (0.5), &LogStatistics, q);
  }

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address.Assign (sourceDevices);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  address.Assign (bottleneckDevices);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (sinkDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 9;
  
  
  for (uint32_t i = 0; i < numSources; ++i)
    {
      // UDP Sink
      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApps = sinkHelper.Install (sinks.Get (i));
      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (simDuration));

      Ptr<Ipv4> ipv4 = sinks.Get (i)->GetObject<Ipv4> ();
      Ipv4Address sinkAddress = ipv4->GetAddress (1, 0).GetLocal ();

      OnOffHelper sourceHelper ("ns3::UdpSocketFactory",
                                InetSocketAddress (sinkAddress, port));
      sourceHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      sourceHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      sourceHelper.SetAttribute ("DataRate", StringValue (sourceRate));
      sourceHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
      
      ApplicationContainer sourceApps = sourceHelper.Install (sources.Get (i));
     
      sourceApps.Start (Seconds (1.0 + i * 0.05)); 
      sourceApps.Stop (Seconds (simDuration));

    }

  std::string totalBwStr = sourceRate.substr(0, sourceRate.find("Kbps"));
  std::string bottleneckBwStr = bottleneckRate.substr(0, bottleneckRate.find("Kbps"));
  
  double totalBw = numSources * atof(totalBwStr.c_str());
  double bottleneckBw = atof(bottleneckBwStr.c_str());
  
  NS_LOG_INFO ("Total offered load: " << totalBw << " Kbps");
  NS_LOG_INFO ("Bottleneck capacity: " << bottleneckBw << " Kbps");
  NS_LOG_INFO ("Oversubscription: " << (totalBw/bottleneckBw) << "x");
  
  if (totalBw <= bottleneckBw) {
    NS_LOG_ERROR ("ERROR: No congestion! Offered load <= bottleneck capacity!");
    NS_LOG_ERROR ("Increase --numSources or --sourceRate, or decrease --bottleneckRate");
  }

  if (enablePcap)
    {
      bottleneckLink.EnablePcap ("red-bottleneck", bottleneckDevices);
      edgeLink.EnablePcap ("red-edge", sourceDevices.Get (0), true);
    }

  AsciiTraceHelper ascii;
  bottleneckLink.EnableAsciiAll (ascii.CreateFileStream ("red-bottleneck.tr"));
  
  
  Simulator::Stop (Seconds (simDuration + 1.0));
  Simulator::Run ();
  
  NS_LOG_INFO ("=== Simulation Complete ===");
  
  uint64_t totalEnqueued = 0;
  uint64_t totalDequeued = 0;
  uint64_t totalDroppedByQueue = 0;
  
  for (uint32_t i = 0; i < qDiscs.GetN (); ++i)
    {
      Ptr<QueueDisc> q = qDiscs.Get (i);
      QueueDisc::Stats stats = q->GetStats ();
      totalEnqueued += stats.nTotalEnqueuedPackets;
      totalDequeued += stats.nTotalDequeuedPackets;
      totalDroppedByQueue += stats.nTotalDroppedPackets;
    }
  
  uint64_t totalAttempted = totalEnqueued + totalDroppedByQueue;
  
  NS_LOG_INFO ("Total Packets Attempted: " << totalAttempted);
  NS_LOG_INFO ("Total Packets Enqueued: " << totalEnqueued);
  NS_LOG_INFO ("Total Packets Dequeued: " << totalDequeued);
  NS_LOG_INFO ("Total Packets Dropped: " << totalDroppedByQueue);
  
  if (totalAttempted > 0)
    {
      double dropRate = (double)totalDroppedByQueue / (double)totalAttempted * 100.0;
      NS_LOG_INFO ("Drop Rate: " << dropRate << "%");
      
      double enqueueRate = (double)totalEnqueued / (double)totalAttempted * 100.0;
      NS_LOG_INFO ("Enqueue Success Rate: " << enqueueRate << "%");
    }

  for (uint32_t i = 0; i < qDiscs.GetN (); ++i)
    {
      Ptr<QueueDisc> q = qDiscs.Get (i);
      QueueDisc::Stats stats = q->GetStats ();
      
      NS_LOG_INFO ("Queue " << i << " Statistics:");
      NS_LOG_INFO ("  Enqueued: " << stats.nTotalEnqueuedPackets);
      NS_LOG_INFO ("  Dequeued: " << stats.nTotalDequeuedPackets);
      NS_LOG_INFO ("  Dropped: " << stats.nTotalDroppedPackets);
      
      if (stats.nTotalEnqueuedPackets + stats.nTotalDroppedPackets > 0)
        {
          double qDropRate = (double)stats.nTotalDroppedPackets / (double)(stats.nTotalEnqueuedPackets + stats.nTotalDroppedPackets) * 100.0;
          NS_LOG_INFO ("  Queue " << i << " Drop Rate: " << qDropRate << "%");
        }
    }

  dropLog.close ();
  queueLog.close ();
  statsLog.close ();


  Simulator::Destroy ();
  
  return 0;
}