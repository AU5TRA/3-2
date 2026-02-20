/*
 * Simulation of RED (Random Early Detection) Queue
 * Final Working Version - No Compilation Errors
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RedSimulationScript");

// --- Trace Callbacks ---

static void
OnDrop (Ptr<const QueueDiscItem> item)
{
  std::cout << Simulator::Now ().GetSeconds () << "s: *** PACKET DROPPED (RED) ***" << std::endl;
}

static void
OnAvgQueueSizeChange (double oldVal, double newVal)
{
  std::cout << Simulator::Now ().GetSeconds () << "s: Avg Queue Size: " << newVal << std::endl;
}

int
main (int argc, char *argv[])
{
  // --- 1. AGGRESSIVE RED SETTINGS (Guaranteed Drops) ---
  double thMin = 0.0;   // Drop starts immediately
  double thMax = 10.0;  // Force drops very early
  uint32_t bufferSize = 100; 
  double wq = 0.002;
  double maxP = 1.0;    // 100% drop probability
  double lInterm = 1.0 / maxP; 
  
  uint32_t packetSize = 1000; 
  double simDuration = 10.0; 

  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  // --- 2. Topology ---
  NodeContainer sources, sinks, routers;
  sources.Create (3);
  sinks.Create (3);
  routers.Create (2); 

  // --- 3. Link Config ---
  // High speed input
  PointToPointHelper edgeLink;
  edgeLink.SetDeviceAttribute ("DataRate", StringValue ("100Mbps")); 
  edgeLink.SetChannelAttribute ("Delay", StringValue ("1ms"));
  edgeLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("10000p")); 

  NetDeviceContainer sourceDevices, sinkDevices;
  for (uint32_t i = 0; i < 3; ++i) {
      sourceDevices.Add (edgeLink.Install (sources.Get (i), routers.Get (0)));
      sinkDevices.Add (edgeLink.Install (routers.Get (1), sinks.Get (i)));
  }

  // Low speed bottleneck
  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue ("500Kbps")); 
  bottleneckLink.SetChannelAttribute ("Delay", StringValue ("50ms"));
  bottleneckLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  NetDeviceContainer bottleneckDevices;
  bottleneckDevices = bottleneckLink.Install (routers.Get (0), routers.Get (1));

  InternetStackHelper stack;
  stack.Install (sources);
  stack.Install (routers);
  stack.Install (sinks);

  // --- 4. Install RED & Connect Traces Safely ---
  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::RedQueueDisc",
                        "MinTh", DoubleValue (thMin),
                        "MaxTh", DoubleValue (thMax),
                        "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, bufferSize)),
                        "QW", DoubleValue (wq),
                        "LInterm", DoubleValue (lInterm));
  
  QueueDiscContainer qDiscs = tch.Install (bottleneckDevices);

  // Iterate using index 'i' to match Queues to Devices
  for (uint32_t i = 0; i < qDiscs.GetN (); ++i) {
      Ptr<QueueDisc> q = qDiscs.Get (i);
      
      // FIX: Get the Node ID from the device container, not the queue
      uint32_t nodeId = bottleneckDevices.Get (i)->GetNode ()->GetId ();
      
      std::cout << "Setup: RED Queue found on Node " 
                << nodeId
                << ". Hooking traces..." << std::endl;

      q->TraceConnectWithoutContext ("Drop", MakeCallback (&OnDrop));
      q->TraceConnectWithoutContext ("AverageQueueSize", MakeCallback (&OnAvgQueueSizeChange));
  }

  // --- 5. IP Addressing ---
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address.Assign (sourceDevices);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  address.Assign (bottleneckDevices);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (sinkDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // --- 6. Traffic Generation ---
  uint16_t port = 9;
  for (uint32_t i = 0; i < 3; ++i)
    {
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApps = sinkHelper.Install (sinks.Get (i));
      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (simDuration));

      Ptr<Ipv4> ipv4 = sinks.Get (i)->GetObject<Ipv4> ();
      Ipv4Address sinkAddress = ipv4->GetAddress (1, 0).GetLocal ();

      BulkSendHelper sourceHelper ("ns3::TcpSocketFactory",
                                   InetSocketAddress (sinkAddress, port));
      sourceHelper.SetAttribute ("MaxBytes", UintegerValue (0));
      sourceHelper.SetAttribute ("SendSize", UintegerValue (packetSize));
      
      ApplicationContainer sourceApps = sourceHelper.Install (sources.Get (i));
      sourceApps.Start (Seconds (1.0)); 
      sourceApps.Stop (Seconds (simDuration));
    }

  bottleneckLink.EnablePcap ("red-bottleneck", bottleneckDevices);

  std::cout << "Running simulation (Check console for Drops)..." << std::endl;
  
  Simulator::Stop (Seconds (simDuration + 1.0));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}