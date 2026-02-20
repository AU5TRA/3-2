/*
 * Simulation of RED (Random Early Detection) Queue
 * Corrected for ns-3.46+ using TrafficControlHelper
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h" 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RedSimulationScript");

int
main (int argc, char *argv[])
{
  double thMin = 40.0;
  double thMax = 120.0;
  uint32_t bufferSize = 280; 
  double wq = 0.002;
  double maxP = 0.125;
  double lInterm = 1.0 / maxP; 
  
  uint32_t packetSize = 1500;
  double simDuration = 360.0; 

  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  // Enable Logging
  LogComponentEnable ("RedQueueDisc", LOG_LEVEL_INFO); // Uncomment to see RED specifics
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
  // --- 1. Topology Creation ---

  NodeContainer sources, sinks, routers;
  sources.Create (3);
  sinks.Create (3);
  routers.Create (2); // Node 0 is R1, Node 1 is R2

  // --- 2. Link Configuration ---
  
  // A. Edge Links (Source -> R1 and R2 -> Sink)
  PointToPointHelper edgeLink;
  edgeLink.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  edgeLink.SetChannelAttribute ("Delay", StringValue ("10ms"));
  edgeLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  NetDeviceContainer sourceDevices, sinkDevices;
  for (uint32_t i = 0; i < 3; ++i) {
      sourceDevices.Add (edgeLink.Install (sources.Get (i), routers.Get (0)));
      sinkDevices.Add (edgeLink.Install (routers.Get (1), sinks.Get (i)));
  }

  // B. Bottleneck Link (R1 -> R2)
  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  // IMPORTANT: For the underlying device, we use a simple DropTailQueue.
  // We will install RED on top of this using TrafficControlHelper later.
  bottleneckLink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  NetDeviceContainer bottleneckDevices;
  bottleneckDevices = bottleneckLink.Install (routers.Get (0), routers.Get (1));

  // --- 3. Install Internet Stack ---
  InternetStackHelper stack;
  stack.Install (sources);
  stack.Install (routers);
  stack.Install (sinks);

  // --- 4. Traffic Control (RED) ---
  // This is the FIX: Install RED QueueDisc on the bottleneck devices
  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::RedQueueDisc",
                        "MinTh", DoubleValue (thMin),
                        "MaxTh", DoubleValue (thMax),
                        "MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, bufferSize)),
                        "QW", DoubleValue (wq),
                        "LInterm", DoubleValue (lInterm));
  
  // Install RED only on the bottleneck devices
  tch.Install (bottleneckDevices);

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
      // Sink
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApps = sinkHelper.Install (sinks.Get (i));
      sinkApps.Start (Seconds (0.0));
      sinkApps.Stop (Seconds (simDuration));

      // Source
      Ptr<Ipv4> ipv4 = sinks.Get (i)->GetObject<Ipv4> ();
      Ipv4Address sinkAddress = ipv4->GetAddress (1, 0).GetLocal ();

      BulkSendHelper sourceHelper ("ns3::TcpSocketFactory",
                                   InetSocketAddress (sinkAddress, port));
      sourceHelper.SetAttribute ("MaxBytes", UintegerValue (0));
      sourceHelper.SetAttribute ("SendSize", UintegerValue (packetSize));
      
      ApplicationContainer sourceApps = sourceHelper.Install (sources.Get (i));
      sourceApps.Start (Seconds (1.0 + (i * 0.5))); 
      sourceApps.Stop (Seconds (simDuration));
    }

  // --- 7. Run Simulation ---
  Simulator::Stop (Seconds (simDuration + 1.0));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}