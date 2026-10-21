#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
using namespace ns3;

int main(int argc, char *argv[])
{
  // 1. NODES                                        (cheat sheet 1) ✅
  NodeContainer nodes;
  nodes.Create(3);                                   // CHANGE: number of nodes

  // 2. LINKS                                        (cheat sheet 2) ✅
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));   // CHANGE: speed
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));       // CHANGE: delay
  // p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("50p")); // optional: queue size ❌
  NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
  NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
  // CHANGE: add one Install line per extra link

  // 3. INTERNET STACK                               (cheat sheet 4) ✅
  InternetStackHelper internet;
  internet.Install(nodes);

  // 4. IP ADDRESSES                                 (cheat sheet 5) ✅
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = address.Assign(d01);
  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = address.Assign(d12);

  // 5. ROUTING ❌
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // 6. RECEIVER (TCP sink) ❌
  PacketSinkHelper sink("ns3::TcpSocketFactory",              // TCP
                        InetSocketAddress(Ipv4Address::GetAny(), 5000));
  ApplicationContainer sinkApp = sink.Install(nodes.Get(2));  // CHANGE: receiver node
  sinkApp.Start(Seconds(1.0));
  sinkApp.Stop(Seconds(20.0));                                // CHANGE: stop time

  // 7. SENDER (TCP BulkSend) ❌
  BulkSendHelper source("ns3::TcpSocketFactory",
                        InetSocketAddress(i12.GetAddress(1), 5000)); // CHANGE: receiver's IP
  source.SetAttribute("SendSize", UintegerValue(1024));       // CHANGE: TCP packet size
  source.SetAttribute("MaxBytes", UintegerValue(0));          // 0 = send until stopped
  ApplicationContainer sourceApp = source.Install(nodes.Get(0)); // CHANGE: sender node
  sourceApp.Start(Seconds(2.0));
  sourceApp.Stop(Seconds(20.0));                              // CHANGE: stop time

  // 8. FLOWMONITOR                                  (cheat sheet 13) ✅
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // 9. RUN ❌
  Simulator::Stop(Seconds(20.0));                    // CHANGE: simulation time
  Simulator::Run();

  // 10. RESULTS ❌
  monitor->CheckForLostPackets();
  for (auto &f : monitor->GetFlowStats())
  {
    FlowMonitor::FlowStats s = f.second;
    double time = s.timeLastRxPacket.GetSeconds() - s.timeFirstTxPacket.GetSeconds();
    std::cout << "Flow " << f.first << std::endl;
    std::cout << "  Tx packets : " << s.txPackets << std::endl;
    std::cout << "  Rx packets : " << s.rxPackets << std::endl;
    std::cout << "  Packet loss: " << s.txPackets - s.rxPackets << std::endl;
    std::cout << "  Throughput : " << s.rxBytes * 8.0 / time / 1e6 << " Mbps" << std::endl;
    std::cout << "  Avg delay  : " << s.delaySum.GetSeconds() / s.rxPackets * 1000 << " ms" << std::endl;
  }

  Simulator::Destroy();
  return 0;
}