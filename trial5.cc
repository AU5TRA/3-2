/*
 * Simulation of RED, ARED, and SARED
 * [cite_start]Based on: "Markov Model Based Congestion Control for TCP" [cite: 1-3]
 * * [cite_start]Topology [cite: 176-186]:
 * 3 Sources --- (10Mb/s, 10ms) ---> Router 1
 * Router 1 --- (2Mb/s, 20ms) ---> Router 2 (Bottleneck)
 * Router 2 --- (10Mb/s, 10ms) ---> 2 Sinks
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SaredSimulation");

// --- SARED Global State ---
// [cite_start]// Paper Parameters [cite: 83-168]
double g_thMin = 40.0;
double g_thMax = 120.0;
double g_part = 0.0;           // Calculated as 0.4 * (thMax - thMin)
double g_currentMaxP = 0.125;  // Initial max_p

// Transition Counts (3x3 Matrix)
double g_n[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
double g_n_total[3] = {0,0,0};

// Transition Probabilities (Initial assumptions from paper)
double g_p[3][3] = {
    {0.9, 0.05, 0.05},
    {0.05, 0.9, 0.05},
    {0.05, 0.05, 0.9}
};
// Steady state probabilities p0, p1, p2
double g_pi[3] = {0.05, 0.9, 0.05}; 

int g_prevState = 0; 
double g_lastSetTime = 0.0;
double g_updateInterval = 0.5; // Update interval

// Global tracker for Average Queue Size (updated via Trace)
double g_monitorAvgQ = 0.0;

// Determine state based on Average Queue Size
int GetState(double avgQ) {
    if (avgQ <= g_thMin) return 0;
    if (avgQ >= g_thMax) return 2;
    return 1;
}

// Trace Hook: Updates the global average queue size whenever RED calculates it
void TraceAvgQ(double oldVal, double newVal) {
    g_monitorAvgQ = newVal;
}

// --- SARED Algorithm ---
// [cite_start]// Implements the adaptive logic from Section 3 [cite: 128-162]
void PeriodicSaredUpdate(Ptr<QueueDisc> queueDisc, Ptr<OutputStreamWrapper> logStream) {
    double now = Simulator::Now().GetSeconds();
    int newState = GetState(g_monitorAvgQ);

    // [cite_start]// 1. Update Transition Matrix Statistics [cite: 107-125]
    g_n[g_prevState][newState] += 1.0;
    g_n_total[g_prevState] += 1.0;
    
    if (g_n_total[g_prevState] > 0) {
        g_p[g_prevState][0] = g_n[g_prevState][0] / g_n_total[g_prevState];
        g_p[g_prevState][1] = g_n[g_prevState][1] / g_n_total[g_prevState];
        g_p[g_prevState][2] = g_n[g_prevState][2] / g_n_total[g_prevState];
    }
    
    g_prevState = newState;

    // 2. Adjust MaxP (if interval has passed)
    if (now > g_lastSetTime + g_updateInterval) {
        
        double alpha = 0.0;
        double beta = 1.0;
        double p0 = g_pi[0]; double p1 = g_pi[1]; double p2 = g_pi[2];

        // Case 1: Low Congestion (State 0-ish)
        if (g_monitorAvgQ <= g_thMin + g_part) {
            if (g_currentMaxP <= 0.01) {
                alpha = g_p[0][1] * (p0 + p2);
                beta = g_p[0][0] * p1;
                g_currentMaxP = g_currentMaxP * beta + alpha;
            } else if (g_currentMaxP >= 0.5) {
                beta = (g_p[2][2] - g_p[2][1] - g_p[1][0]) * p1;
                g_currentMaxP = g_currentMaxP * beta;
            } else {
                beta = (g_p[1][1] - g_p[1][0]) * p1;
                g_currentMaxP = g_currentMaxP * beta;
            }
        }
        // Case 2: Target Range
        else if (g_monitorAvgQ > g_thMin + g_part && g_monitorAvgQ < g_thMax - g_part) {
            if (g_currentMaxP <= 0.01) {
                alpha = g_p[0][1] * (p0 + p2);
                g_currentMaxP = g_currentMaxP + alpha;
            } else if (g_currentMaxP >= 0.5) {
                beta = (g_p[2][2] - g_p[2][1]) * p1;
                g_currentMaxP = g_currentMaxP * beta;
            } else {
                alpha = g_p[1][2] * (p0 + p2);
                beta = g_p[1][1] * p1;
                g_currentMaxP = g_currentMaxP * beta + alpha;
            }
        }
        // Case 3: High Congestion (State 2-ish)
        else if (g_monitorAvgQ >= g_thMax - g_part) {
            if (g_currentMaxP <= 0.01) {
                alpha = (g_p[0][1] + g_p[1][2]) * (p0 + p2);
                g_currentMaxP = g_currentMaxP + alpha;
            } else if (g_currentMaxP >= 0.5) {
                alpha = g_p[2][1] * (p0 + p2);
                beta = g_p[2][2] * p1;
                g_currentMaxP = g_currentMaxP * beta + alpha;
            } else {
                alpha = g_p[1][2] * (p0 + p2);
                g_currentMaxP = g_currentMaxP + alpha;
            }
        }

        // Clamp MaxP to valid range [0.01, 1.0]
        if (g_currentMaxP > 1.0) g_currentMaxP = 1.0;
        if (g_currentMaxP < 0.0001) g_currentMaxP = 0.0001; // Avoid divide by zero

        // Update QueueDisc
        // Note: NS-3 RedQueueDisc uses "LInterm" as the Denominator (1/MaxP)
        double lInterm = 1.0 / g_currentMaxP;
        queueDisc->SetAttribute("LInterm", DoubleValue(lInterm));
        
        g_lastSetTime = now;

        // Logging
        *logStream->GetStream() << now << "\t" << g_monitorAvgQ << "\t" << g_currentMaxP << "\t" << newState << std::endl;
    }

    Simulator::Schedule(Seconds(0.1), &PeriodicSaredUpdate, queueDisc, logStream);
}

int main (int argc, char *argv[])
{
    std::string mode = "SARED"; // Options: RED, ARED, SARED
    CommandLine cmd;
    cmd.AddValue("mode", "Simulation mode: RED, ARED, or SARED", mode);
    cmd.Parse(argc, argv);

    // --- Configuration ---
    // Parameters
    uint32_t packetSize = 1500;
    uint32_t qSize = 280; 
    g_part = 0.4 * (g_thMax - g_thMin); //

    // --- Topology Setup ---
    NodeContainer sources; sources.Create(3);
    NodeContainer routers; routers.Create(2);
    NodeContainer sinks; sinks.Create(2);

    PointToPointHelper p2pLeaf;
    p2pLeaf.SetDeviceAttribute("DataRate", DataRateValue(DataRate("10Mbps")));
    p2pLeaf.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

    PointToPointHelper p2pBottle;
    p2pBottle.SetDeviceAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
    p2pBottle.SetChannelAttribute("Delay", TimeValue(MilliSeconds(20)));

    // Install Links
    std::vector<NetDeviceContainer> srcDevs;
    for(uint32_t i=0; i<3; ++i) {
        srcDevs.push_back(p2pLeaf.Install(sources.Get(i), routers.Get(0)));
    }

    NetDeviceContainer bottleDevs = p2pBottle.Install(routers.Get(0), routers.Get(1));

    std::vector<NetDeviceContainer> sinkDevs;
    for(uint32_t i=0; i<2; ++i) {
        sinkDevs.push_back(p2pLeaf.Install(routers.Get(1), sinks.Get(i)));
    }

    InternetStackHelper stack;
    stack.Install(sources);
    stack.Install(routers);
    stack.Install(sinks);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    for(auto& dev : srcDevs) { address.Assign(dev); address.NewNetwork(); }
    
    address.SetBase("10.2.1.0", "255.255.255.0");
    address.Assign(bottleDevs);
    
    address.SetBase("10.3.1.0", "255.255.255.0");
    for(auto& dev : sinkDevs) { address.Assign(dev); address.NewNetwork(); }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // --- Traffic Control (QueueDisc) Setup ---
    TrafficControlHelper tch;
    
    // Set Root QueueDisc with common RED parameters
    // FIX 1: Use "MaxSize" instead of "QueueLimit" and "QW" instead of "Qw"
    tch.SetRootQueueDisc("ns3::RedQueueDisc",
                         "MinTh", DoubleValue(g_thMin),
                         "MaxTh", DoubleValue(g_thMax),
                         "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, qSize)), 
                         "LinkBandwidth", DataRateValue(DataRate("2Mbps")),
                         "LinkDelay", TimeValue(MilliSeconds(20)),
                         "QW", DoubleValue(0.002)); 

    // FIX 2: Uninstall the default QueueDisc before installing the new one
    tch.Uninstall(bottleDevs.Get(0));

    // Install QueueDisc
    QueueDiscContainer qDiscs = tch.Install(bottleDevs.Get(0));
    Ptr<QueueDisc> queueDisc = qDiscs.Get(0);

    // Apply Mode-Specific Attributes *After* Installation
    if (mode == "RED") {
        queueDisc->SetAttribute("UseAred", BooleanValue(false));
        queueDisc->SetAttribute("LInterm", DoubleValue(1.0/0.125)); // Fixed MaxP=0.125
    } 
    else if (mode == "ARED") {
        queueDisc->SetAttribute("UseAred", BooleanValue(true));
        // ARED auto-manages LInterm
    } 
    else if (mode == "SARED") {
        queueDisc->SetAttribute("UseAred", BooleanValue(false));
        // SARED manages LInterm via the PeriodicSaredUpdate function
        queueDisc->SetAttribute("LInterm", DoubleValue(1.0/g_currentMaxP));
        
        // Start SARED Loop
        AsciiTraceHelper ascii;
        Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("sared-log.txt");
        *stream->GetStream() << "Time\tAvgQ\tMaxP\tState" << std::endl;
        Simulator::Schedule(Seconds(0.1), &PeriodicSaredUpdate, queueDisc, stream);
    }

    // Connect Trace Source for Average Queue Size
    // This connects to the 'Average' trace source of the specific RedQueueDisc instance
    queueDisc->TraceConnectWithoutContext("Average", MakeCallback(&TraceAvgQ));

    // --- Traffic Generation ---
    uint16_t port = 5000;
    
    // Distribute 100 flows among 3 sources and 2 sinks
    auto CreateFlows = [&](int count, double start, double stop) {
        for(int i=0; i<count; ++i) {
            Ptr<Node> src = sources.Get(i % 3);
            Ptr<Node> dst = sinks.Get(i % 2);
            
            // FIX 3: GetAddress(1,0) returns InterfaceAddress. Added .GetLocal() to get Ipv4Address.
            Ipv4Address dstAddr = dst->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
            
            BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(dstAddr, port));
            source.SetAttribute("MaxBytes", UintegerValue(0));
            ApplicationContainer srcApp = source.Install(src);
            srcApp.Start(Seconds(start));
            srcApp.Stop(Seconds(stop));
            
            PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
            ApplicationContainer sinkApp = sink.Install(dst);
            sinkApp.Start(Seconds(start));
            sinkApp.Stop(Seconds(stop));
            port++;
        }
    };

    // 10 flows for entire duration
    CreateFlows(10, 0.0, 360.0);
    // 90 flows for sudden congestion (120s to 300s)
    CreateFlows(90, 120.0, 300.0);

    // --- Trace and Run ---
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(360.0));
    Simulator::Run();

    // Stats Output
    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    
    uint32_t totalTx = 0, totalLost = 0;
    for(auto const& i : stats) {
        totalTx += i.second.txPackets;
        totalLost += i.second.lostPackets;
    }

    std::cout << "--- Results for " << mode << " ---" << std::endl;
    std::cout << "Total Packets Sent: " << totalTx << std::endl;
    std::cout << "Total Packets Lost: " << totalLost << std::endl;
    std::cout << "Packet Loss Rate: " << (double)totalLost/totalTx * 100.0 << "%" << std::endl;

    Simulator::Destroy();
    return 0;
}