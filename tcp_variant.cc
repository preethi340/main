#include "ns3/core-module.h"                  // [§1]
#include "ns3/network-module.h"               // [§1]
#include "ns3/internet-module.h"              // [§1]
#include "ns3/point-to-point-module.h"        // [§1]
#include "ns3/applications-module.h"          // [§1]
#include "ns3/flow-monitor-module.h"          // [§1]
#include "ns3/traffic-control-module.h"       // [§1]
#include <fstream>                            // ❌
using namespace ns3;                          // [§1]

// ---- congestion window trace (for the cwnd graph) ---- ❌ all of this
std::ofstream cwndFile("cwnd.txt");
void CwndChange(uint32_t oldW, uint32_t newW)
{
  cwndFile << Simulator::Now().GetSeconds() << " " << newW << std::endl;
}
void ConnectCwnd()
{
  Config::ConnectWithoutContext(
    "/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
    MakeCallback(&CwndChange));
}

int main(int argc, char *argv[])              // ❌
{
  std::string variant = "ns3::TcpNewReno";    // CHANGE: TcpNewReno / TcpCubic / TcpBbr

  NodeContainer vehicles;  vehicles.Create(5);                            // [§2]
  NodeContainer router;    router.Create(1);                              // [§2]
  NodeContainer server;    server.Create(1);                              // [§2]

  // ---------- TCP settings ----------
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(variant)); // [§5]
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024)); // [§5]
  Config::SetDefault("ns3::TcpSocketState::EnablePacing",
                     BooleanValue(variant == "ns3::TcpBbr"));             // [§5] BBR needs pacing

  // ---------- Links ----------
  PointToPointHelper access;                                              // [§3]
  access.SetDeviceAttribute("DataRate", StringValue("10Mbps"));           // [§3]
  access.SetChannelAttribute("Delay", StringValue("2ms"));                // [§3]
  PointToPointHelper bottleneck;                                          // [§3]
  bottleneck.SetDeviceAttribute("DataRate", StringValue("5Mbps"));       // [§3]
  bottleneck.SetChannelAttribute("Delay", StringValue("20ms"));           // [§3]
  bottleneck.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("50p")); // [§3]

  InternetStackHelper internet;                                           // [§4]
  internet.Install(vehicles);  internet.Install(router);  internet.Install(server); // [§4]

  Ipv4AddressHelper ipv4;                                                 // [§4]
  for (int i = 0; i < 5; i++)                       // vehicle i <-> router, own subnet each
  {
    NetDeviceContainer d = access.Install(vehicles.Get(i), router.Get(0)); // [§3]
    std::string net = "10.1." + std::to_string(i + 1) + ".0";
    ipv4.SetBase(net.c_str(), "255.255.255.0");                           // [§4]
    ipv4.Assign(d);                                                       // [§4]
  }
  NetDeviceContainer devBtl = bottleneck.Install(router.Get(0), server.Get(0)); // [§3]
  ipv4.SetBase("10.1.10.0", "255.255.255.0");                             // [§4]
  Ipv4InterfaceContainer ifBtl = ipv4.Assign(devBtl);                     // [§4]

  TrafficControlHelper tch;                                               // [§3 gotcha]
  tch.Uninstall(devBtl);                                                  // [§3]

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();                       // [§4]

  // ---------- Applications: 5 TCP senders -> 1 sink ----------
  PacketSinkHelper tcpSink("ns3::TcpSocketFactory",                       // [§6 BulkSend part]
        InetSocketAddress(Ipv4Address::GetAny(), 8080));
  ApplicationContainer sinkApp = tcpSink.Install(server.Get(0));          // [§6]
  sinkApp.Start(Seconds(0.0));  sinkApp.Stop(Seconds(20.0));              // [§6]

  for (int i = 0; i < 5; i++)
  {
    BulkSendHelper src("ns3::TcpSocketFactory",
          InetSocketAddress(ifBtl.GetAddress(1), 8080));                  // [§6] server's IP
    src.SetAttribute("SendSize", UintegerValue(1024));                    // [§6]
    src.SetAttribute("MaxBytes", UintegerValue(0));                       // [§6]
    ApplicationContainer app = src.Install(vehicles.Get(i));              // [§6]
    app.Start(Seconds(1.0));  app.Stop(Seconds(20.0));                    // [§6]
  }

  Simulator::Schedule(Seconds(1.1), &ConnectCwnd);                        // ❌ trace vehicle 0's cwnd

  // ---------- FlowMonitor ----------
  FlowMonitorHelper fmh;                                                  // [§11 install part]
  Ptr<FlowMonitor> mon = fmh.InstallAll();                                // [§11]
  Simulator::Stop(Seconds(20.0));                                         // [§11]
  Simulator::Run();                                                       // [§11]
  mon->CheckForLostPackets();                                             // [§11]

  // ---------- Results: all ❌ mug ----------
  Ptr<Ipv4FlowClassifier> cl = DynamicCast<Ipv4FlowClassifier>(fmh.GetClassifier());
  double sum = 0, sumsq = 0;  int n = 0;            // for Jain's fairness
  std::cout << "Variant: " << variant << std::endl;
  for (auto &kv : mon->GetFlowStats())
  {
    Ipv4FlowClassifier::FiveTuple t = cl->FindFlow(kv.first);
    if (t.destinationPort != 8080) continue;        // skip the ACK flows
    FlowMonitor::FlowStats s = kv.second;
    double dur   = (s.timeLastRxPacket - s.timeFirstTxPacket).GetSeconds();
    double thr   = s.rxBytes * 8.0 / dur / 1e6;
    double delay = s.delaySum.GetSeconds() / s.rxPackets * 1000;
    double loss  = 100.0 * (s.txPackets - s.rxPackets) / s.txPackets;
    std::cout << t.sourceAddress << "  Thr=" << thr << " Mbps  Delay=" << delay
              << " ms  Loss=" << loss << " %" << std::endl;
    sum += thr;  sumsq += thr * thr;  n++;
  }
  std::cout << "Jain's fairness = " << (sum * sum) / (n * sumsq) << std::endl;

  Simulator::Destroy();                       // ❌
  return 0;                                   // ❌
}