#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Sared-Ared");

class SaredQueueDisc : public QueueDisc
{
  public:
    static TypeId GetTypeId();
    SaredQueueDisc();
    ~SaredQueueDisc();

  protected:
    bool DoEnqueue(Ptr<QueueDiscItem> item) override;
    Ptr<QueueDiscItem> DoDequeue() override;
    Ptr<const QueueDiscItem> DoPeek() override;

    bool CheckConfig() override;
    void DoDispose() override;
    void InitializeParams() override;

  private:
    void UpdateSared();
    void CalculateTransitionProbabilities(double new_ave);
    double CalculateP();

    // RED Parameters
    double _th_min;
    double _th_max;
    double _Wq;   // EWMA weight of queue
    double _curMaxP;  // static for RED
    uint32_t _meanPktSize;

    uint32_t _bufferSizePkts;
    DataRate _linkBandwidth;

    // State Variables
    double _qAvg; // EWMA of q size = (1 - Wq) x avg + Wq x q
    Time _qTime; // time of last empty q
    int32_t _count; // packets since last drop

    // SARED
    Time _interval;  // interval of recalculating m_p
    Time _lastSet;
    double _old_ave;

    // Markov
    double m_n[3][3];  // count
    double m_p[3][3]; // probability
    double m_targetP[3];

    Ptr<UniformRandomVariable> m_uv;
};

NS_OBJECT_ENSURE_REGISTERED(SaredQueueDisc);

TypeId
SaredQueueDisc::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SaredQueueDisc")
            .SetParent<QueueDisc>()
            .SetGroupName("TrafficControl")
            .AddConstructor<SaredQueueDisc>()
            .AddAttribute("MinTh",
                          "Minimum average queue threshold (packets)",
                          DoubleValue(40),   //20
                          MakeDoubleAccessor(&SaredQueueDisc::_th_min),
                          MakeDoubleChecker<double>())
            .AddAttribute("MaxTh",
                          "Maximum average queue threshold (packets)",
                          DoubleValue(120),  //60
                          MakeDoubleAccessor(&SaredQueueDisc::_th_max),
                          MakeDoubleChecker<double>())
            .AddAttribute("MeanPktSize",
                          "Average packet size in bytes",
                          UintegerValue(1500),
                          MakeUintegerAccessor(&SaredQueueDisc::_meanPktSize),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("QW",
                          "Queue weight for EWMA",
                          DoubleValue(0.002),
                          MakeDoubleAccessor(&SaredQueueDisc::_Wq),
                          MakeDoubleChecker<double>())
            .AddAttribute("Interval",
                          "Interval for updating transition probabilities",
                          StringValue("0.5s"),
                          MakeTimeAccessor(&SaredQueueDisc::_interval),
                          MakeTimeChecker())
            .AddAttribute("BufferSize",
                          "Internal queue limit (packets)",
                          UintegerValue(280),  //120
                          MakeUintegerAccessor(&SaredQueueDisc::_bufferSizePkts),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("LinkBandwidth",
                          "Bottleneck link bandwidth (for idle EWMA decay)",
                          DataRateValue(DataRate("2Mbps")),   //5
                          MakeDataRateAccessor(&SaredQueueDisc::_linkBandwidth),
                          MakeDataRateChecker())
            .AddAttribute("MaxSize",
                          "Maximum size of the internal queue(packets)",
                          QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, 280)),  //120
                          MakeQueueSizeAccessor(&QueueDisc::SetMaxSize, &QueueDisc::GetMaxSize),
                          MakeQueueSizeChecker());
    return tid;
}

SaredQueueDisc::SaredQueueDisc()
    : QueueDisc(QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
    _curMaxP = 0.125;
    _qAvg = 0.0;
    _count = -1;
    _lastSet = Seconds(0.0);
    _old_ave = 0.0;
    _qTime = Seconds(0.0);

    m_uv = CreateObject<UniformRandomVariable>();

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_n[i][j] = (i == j) ? 18 : 1; //keeping consistency with m_p
            m_p[i][j] = (i == j) ? 0.9 : 0.05;
        }
    }

    m_targetP[0] = 0.05;
    m_targetP[1] = 0.90;
    m_targetP[2] = 0.05;
}

SaredQueueDisc::~SaredQueueDisc()
{
}

void
SaredQueueDisc::DoDispose()
{
    QueueDisc::DoDispose();
}

void
SaredQueueDisc::InitializeParams()
{
    SetMaxSize(QueueSize(QueueSizeUnit::PACKETS, _bufferSizePkts));

    if (GetNInternalQueues() > 0)
    {
        GetInternalQueue(0)->SetMaxSize(GetMaxSize());
    }

    _lastSet = Simulator::Now();
    _qTime = Simulator::Now();
    _qAvg = 0.0;
    _count = -1;
    _old_ave = 0.0;
}

bool
SaredQueueDisc::CheckConfig()
{
    if (_th_min >= _th_max)
    {
        return false;
    }

    if (GetNInternalQueues() == 0)
    {
        QueueSize qs = GetMaxSize();
        if (qs.GetUnit() != QueueSizeUnit::PACKETS)
        {
            qs = QueueSize(QueueSizeUnit::PACKETS, 120);
        }

        auto q =
            CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>("MaxSize", QueueSizeValue(qs)); // packet buffer
        AddInternalQueue(q);
    }
    else
    {
        GetInternalQueue(0)->SetMaxSize(GetMaxSize());
    }

    return true;
}

Ptr<QueueDiscItem>
SaredQueueDisc::DoDequeue()
{
    Ptr<QueueDiscItem> item = GetInternalQueue(0)->Dequeue();
    if (!item)
    {
        return nullptr;
    }

    if (GetInternalQueue(0)->GetNPackets() == 0)
    {
        _qTime = Simulator::Now();
    }

    return item;
}

Ptr<const QueueDiscItem>
SaredQueueDisc::DoPeek()
{
    return GetInternalQueue(0)->Peek();
}

bool
SaredQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item)
{
    uint32_t nQueued = GetInternalQueue(0)->GetNPackets();

    if (nQueued == 0)  // no pckts
    {
        double idleSecs = (Simulator::Now() - _qTime).GetSeconds();
        double bitrate = (double)_linkBandwidth.GetBitRate();
        double pktTime =
            (double)_meanPktSize * 8.0 / bitrate; // seconds per packet at bottleneck rate
        if (pktTime <= 0)
        {
            pktTime = 1e-6;
        }
        double m_idle = idleSecs / pktTime;
        _qAvg = _qAvg * std::pow(1.0 - _Wq, m_idle);
    }
    else
    {
        _qAvg = (1.0 - _Wq) * _qAvg + _Wq * (double)nQueued;
    }

    if (Simulator::Now() > _lastSet + _interval)
    {
        UpdateSared();
    }

    bool drop = false;

    if (_qAvg < _th_min)
    {
        _count = -1;
    }
    else if (_qAvg < _th_max)
    {
        if (_count < 0)
        {
            _count = 0;
        }
        _count++;

        double pb = CalculateP();

        if (pb > 0.0)
        {
            double denom = 1.0 - (double)_count * pb;
            if (denom <= 0.0)   // pb is very high/ too many packets
            {
                drop = true;    // force drop
                _count = 0;
            }
            else
            {
                double pa = pb / denom;  
                if (m_uv->GetValue() < pa)   // randomized
                {
                    drop = true;
                    _count = 0;
                }
            }
        }
    }
    else   // over th_max, must drop
    {
        drop = true;
        _count = 0;
    }

    if (drop)
    {
        DropBeforeEnqueue(item, "SARED Drop");   // we judge dropping before enqueue
        return false;
    }

    bool enqueued = GetInternalQueue(0)->Enqueue(item);
    if (!enqueued)
    {
        return false;  // failed to enqueue
    }
    return true;
}

double
SaredQueueDisc::CalculateP()
{
    return _curMaxP * (_qAvg - _th_min) / (_th_max - _th_min);
}

void
SaredQueueDisc::UpdateSared(void)
{
    CalculateTransitionProbabilities(_qAvg);

    double part = 0.4 * (_th_max - _th_min);
    double alpha = 0.0;
    double beta = 1.0;   // beta can't be < 0, if happens we enforce a min value of 0.1

    if (_qAvg <= _th_min + part)
    {
        if (_curMaxP <= 0.01)   
        {
            alpha = m_p[0][1] * (m_targetP[0] + m_targetP[2]);
            beta = m_p[0][0] * m_targetP[1];
            _curMaxP = _curMaxP * beta + alpha;
        }
        else if (_curMaxP >= 0.5)
        {
            beta = (m_p[2][2] - m_p[2][1] - m_p[1][0]) * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1;
            }
            _curMaxP = _curMaxP * beta;
        }
        else
        {
            beta = (m_p[1][1] - m_p[1][0]) * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1;
            }
            _curMaxP = _curMaxP * beta;
        }
    }
    else if (_qAvg < _th_max - part)
    {
        if (_curMaxP <= 0.01)
        {
            alpha = m_p[0][1] * (m_targetP[0] + m_targetP[2]);
            _curMaxP = _curMaxP + alpha;
        }
        else if (_curMaxP >= 0.5)
        {
            beta = (m_p[2][2] - m_p[2][1]) * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1;    /////////////////
            }
            _curMaxP = _curMaxP * beta;
        }
        else
        {
            beta = (m_p[1][1] - m_p[1][0]) * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1; /////////////
            }
            _curMaxP = _curMaxP * beta;
        }
    }
    else if (_qAvg > _th_min + part && _qAvg < _th_max - part)
    {
        if (_curMaxP <= 0.01)
        {
            alpha = (m_p[0][1] + m_p[1][2]) * (m_targetP[0] + m_targetP[2]);
            _curMaxP = _curMaxP + alpha;
        }
        else if (_curMaxP >= 0.5)
        {
            beta = (m_p[2][2] - m_p[2][1]) * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1; /////////////
            }
            _curMaxP = _curMaxP * beta ;
        }
        else
        {
            alpha = m_p[1][2] * (m_targetP[0] + m_targetP[2]);
            beta = m_p[1][1] * m_targetP[1];
            if (beta <= 0.0)
            {   
                beta = 0.1; /////////////
            }
            _curMaxP = _curMaxP * beta + alpha;
        }
    }
    else if (_qAvg >= _th_max - part)
    {
        if (_curMaxP <= 0.01)
        {
            alpha = m_p[0][2] * (m_targetP[0] + m_targetP[2]);
            _curMaxP = _curMaxP + alpha;
        }
        else if (_curMaxP >= 0.5)
        {
            alpha = m_p[2][1] * (m_targetP[0] + m_targetP[2]);
            beta = m_p[2][2] * m_targetP[1];
            if (beta <= 0.0)
            {
                beta = 0.1; /////////////
            }
            _curMaxP = _curMaxP * beta + alpha;
        }
        else
        {
            alpha = m_p[1][2] * (m_targetP[0] + m_targetP[2]);
            _curMaxP = _curMaxP  + alpha;
        }
    }

    // Enforcing bounds
    if (_curMaxP > 1.0)
    {
        _curMaxP = 1.0;
    }
    if (_curMaxP < 1e-5)
    {
        _curMaxP = 1e-5;
    }

    _lastSet = Simulator::Now();
    _old_ave = _qAvg;
}

void
SaredQueueDisc::CalculateTransitionProbabilities(double new_ave)
{

    if (_old_ave < _th_min && new_ave < _th_min)
    {
        m_n[0][0] += 1.0; // n00
    }

    if (_old_ave < _th_min && (new_ave >= _th_min && new_ave < _th_max))
    {
        m_n[0][1] += 1.0; // n01
    }

    if (_old_ave < _th_min && new_ave >= _th_max)
    {
        m_n[0][2] += 1.0; // n02
    }

    if ((_old_ave >= _th_min && _old_ave < _th_max) && new_ave < _th_min)
    {
        m_n[1][0] += 1.0; // n10
    }

    if ((_old_ave >= _th_min && _old_ave < _th_max) &&
        (new_ave >= _th_min && new_ave < _th_max))
    {
        m_n[1][1] += 1.0; // n11
    }

    if ((_old_ave >= _th_min && _old_ave < _th_max) && new_ave >= _th_max)
    {
        m_n[1][2] += 1.0; // n12
    }

    if (_old_ave >= _th_max && new_ave < _th_min)
    {
        m_n[2][0] += 1.0; // n20
    }

    if (_old_ave >= _th_max && (new_ave >= _th_min && new_ave < _th_max))
    {
        m_n[2][1] += 1.0; // n21
    }

    if (_old_ave >= _th_max && new_ave >= _th_max)
    {
        m_n[2][2] += 1.0; // n22
    }

    for (int i = 0; i < 3; i++)
    {
        double n_i = m_n[i][0] + m_n[i][1] + m_n[i][2];
        if (n_i > 0)
        {
            m_p[i][0] = m_n[i][0] / n_i;
            m_p[i][1] = m_n[i][1] / n_i;
            m_p[i][2] = m_n[i][2] / n_i;
        }
    }
}
  
static double
RunOnce(uint32_t nFlows,
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
    NodeContainer srcs, sinks, routers;
    srcs.Create(3);
    sinks.Create(2);
    routers.Create(2);

    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue(accessRate));
    access.SetChannelAttribute("Delay", StringValue(accessDelay));

    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue(bottleneckRate));
    bottleneck.SetChannelAttribute("Delay", StringValue(bottleneckDelay));

    // 3 sources -> router 1
    NetDeviceContainer Src_0 = access.Install(srcs.Get(0), routers.Get(0));
    NetDeviceContainer Src_1 = access.Install(srcs.Get(1), routers.Get(0));
    NetDeviceContainer Src_2 = access.Install(srcs.Get(2), routers.Get(0));

    //router 1 -> router 2
    NetDeviceContainer R1_R2 = bottleneck.Install(routers.Get(0), routers.Get(1));

    // router 2 -> 2 sinks
    NetDeviceContainer Sink_0 = access.Install(routers.Get(1), sinks.Get(0));
    NetDeviceContainer Sink_1 = access.Install(routers.Get(1), sinks.Get(1));

    InternetStackHelper stack;
    stack.Install(srcs);
    stack.Install(routers);
    stack.Install(sinks);

    TrafficControlHelper tch;

    if (useSared)
    {
        tch.SetRootQueueDisc("ns3::SaredQueueDisc",
                             "MinTh", DoubleValue(thMin),
                             "MaxTh", DoubleValue(thMax),
                             "QW", DoubleValue(0.002),
                             "MeanPktSize", UintegerValue(1500),
                             "Interval", StringValue("0.5s"),
                             "BufferSize", UintegerValue(bufferSizePkts),
                             "LinkBandwidth", DataRateValue(DataRate(bottleneckRate)));
    }
    else
    {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
                             "MinTh", DoubleValue(thMin),
                             "MaxTh", DoubleValue(thMax),
                             "QW", DoubleValue(0.002),
                             "MeanPktSize", UintegerValue(1500),
                             "LinkBandwidth", StringValue(bottleneckRate),
                             "LinkDelay", StringValue(bottleneckDelay),
                             "ARED", BooleanValue(true),
                             "LInterm", DoubleValue(8.0),
                             "MaxSize",
                             QueueSizeValue(
                                 QueueSize(QueueSizeUnit::PACKETS, bufferSizePkts)));
    }

    // queue disc on outgoing side of router_1
    QueueDiscContainer qdiscs = tch.Install(R1_R2.Get(0));
    (void)qdiscs;

    Ipv4AddressHelper address;

    address.SetBase("10.0.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iSrc_0 = address.Assign(Src_0);

    address.SetBase("10.0.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iSrc_1 = address.Assign(Src_1);

    address.SetBase("10.0.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iSrc_2 = address.Assign(Src_2);

    address.SetBase("10.0.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iR1_R2 = address.Assign(R1_R2);

    address.SetBase("10.0.5.0", "255.255.255.0");
    Ipv4InterfaceContainer iSink_0 = address.Assign(Sink_0);

    address.SetBase("10.0.6.0", "255.255.255.0");
    Ipv4InterfaceContainer iSink_1 = address.Assign(Sink_1);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    const uint16_t basePort = 50000;

    ApplicationContainer sinkApps;
    ApplicationContainer srcApps;

    for (uint32_t i = 0; i < nFlows; ++i)
    {
        uint16_t port = basePort + i;

        uint32_t sinkIndex = i % 2;
        uint32_t srcIndex = i % 3;

        Ipv4Address dstAddr =
            (sinkIndex == 0) ? iSink_0.GetAddress(1) : iSink_1.GetAddress(1);

        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
        sinkApps.Add(sinkHelper.Install(sinks.Get(sinkIndex)));

        Address dst(InetSocketAddress(dstAddr, port));
        BulkSendHelper srcHelper("ns3::TcpSocketFactory", dst);
        srcHelper.SetAttribute("MaxBytes", UintegerValue(0));

        ApplicationContainer a = srcHelper.Install(srcs.Get(srcIndex));
        a.Start(Seconds(0.0));
        a.Stop(Seconds(simTimeSec));
        srcApps.Add(a);
    }

    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(simTimeSec));

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTimeSec));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());

    uint64_t totalTx = 0;
    uint64_t totalLost = 0;

    auto stats = monitor->GetFlowStats();
    for (const auto& kv : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        if (t.protocol == 6 &&
            t.destinationPort >= basePort &&
            t.destinationPort < static_cast<uint16_t>(basePort + nFlows))
        {
            totalTx += kv.second.txPackets;
            totalLost += kv.second.lostPackets;
        }
    }

    Simulator::Destroy();

    if (totalTx == 0)
    {
        return 0.0;  // no flow
    }

    return 100.0 * static_cast<double>(totalLost) / static_cast<double>(totalTx);
}


int
main(int argc, char* argv[])
{
    uint32_t thMin = 40;  //20
    uint32_t thMax = 120;  //60
    uint32_t bufferSize = 280;     //120

    uint32_t minFlows = 5;
    uint32_t maxFlows = 105;
    uint32_t stepFlows = 10;

    std::string accessRate = "10Mbps";
    std::string accessDelay = "10ms";
    std::string bottleneckRate = "2Mbps";
    std::string bottleneckDelay = "20ms";

    double simTimeSec = 360.0;

    CommandLine cmd;
    cmd.AddValue("thMin", "min threshold (pkts)", thMin);
    cmd.AddValue("thMax", "max threshold (pkts)", thMax);
    cmd.AddValue("bufferSize", "buffer size (pkts)", bufferSize);
    cmd.AddValue("minFlows", "min number of flows", minFlows);
    cmd.AddValue("maxFlows", "max number of flows", maxFlows);
    cmd.AddValue("stepFlows", "step for flow sweep", stepFlows);
    cmd.AddValue("simTime", "simulation time (s)", simTimeSec);
    cmd.AddValue("accessRate", "access link data rate", accessRate);
    cmd.AddValue("accessDelay", "access link delay", accessDelay);
    cmd.AddValue("bottleneckRate", "bottleneck data rate", bottleneckRate);
    cmd.AddValue("bottleneckDelay", "bottleneck delay", bottleneckDelay);
    cmd.Parse(argc, argv);

    std::ofstream out("loss-vs-flows.dat");
    out << "# flows  ared_loss_pct  sared_loss_pct\n";

    std::cout << "Sweeping flows: [" << minFlows << ", " << maxFlows << "] step " << stepFlows
              << "\n";

    for (uint32_t n = minFlows; n <= maxFlows; n += stepFlows)
    {
        double aredLoss = RunOnce(n,
                                  false,
                                  thMin,
                                  thMax,
                                  bufferSize,
                                  accessRate,
                                  accessDelay,
                                  bottleneckRate,
                                  bottleneckDelay,
                                  simTimeSec);

        double saredLoss = RunOnce(n,
                                   true,
                                   thMin,
                                   thMax,
                                   bufferSize,
                                   accessRate,
                                   accessDelay,
                                   bottleneckRate,
                                   bottleneckDelay,
                                   simTimeSec);

        out << n << " " << aredLoss << " " << saredLoss << "\n";

        std::cout << "nFlows=" << n << " | ARED loss%=" << aredLoss << " | SARED loss%=" << saredLoss << "\n";
    }

    out.close();

    std::ofstream plt("loss-vs-flows.plt");
    plt << "set terminal png size 900,650\n"
        << "set output 'loss-vs-flows.png'\n"
        << "set title 'Percentage Packet Loss vs Number of Flows'\n"
        << "set xlabel 'Number of TCP flows'\n"
        << "set ylabel 'Percentage Packet Loss (%)'\n"
        << "set grid\n"
        << "plot \\\n"
        << "  'loss-vs-flows.dat' using 1:2 with linespoints title 'ARED', \\\n"
        << "  'loss-vs-flows.dat' using 1:3 with linespoints title 'SARED'\n";
    plt.close();

    // std::cout << "\nWrote: loss-vs-flows.dat, loss-vs-flows.plt\n"
    //           << "Run:   gnuplot loss-vs-flows.plt\n"
    //           << "Output loss-vs-flows.png\n";

    return 0;
}
