#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
using namespace ns3;

int main(int argc, char *argv[])
{
  // 1. NODES                                        (cheat sheet 1) ✅
  NodeContainer nodes;
  nodes.Create(3);                                   // CHANGE: number of nodes

  // 2. WI-FI (replaces the p2p links)               (cheat sheet 3) ✅
  WifiHelper wifi;
  WifiMacHelper mac;
  YansWifiPhyHelper phy;
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  phy.SetChannel(channel.Create());
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager");
  mac.SetType("ns3::AdhocWifiMac");                  // ❌ NOT in cheat sheet: add this line
  NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

  // 3. MOBILITY                                     (cheat sheet 7, 8) ✅
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)),
                            "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // CHANGE: speed (m/s) ❌
  // For fixed nodes, use this instead:
  // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // 4. INTERNET STACK                               (cheat sheet 4) ✅
  InternetStackHelper internet;
  // For AODV (multi-hop), uncomment these two lines (cheat sheet 9) ✅
  // AodvHelper aodv;
  // internet.SetRoutingHelper(aodv);
  internet.Install(nodes);

  // 5. IP ADDRESSES                                 (cheat sheet 5) ✅
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);
  // all nodes are on ONE network, so no PopulateRoutingTables needed

  // 6. RECEIVER ❌
  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), 5000));
  ApplicationContainer sinkApp = sink.Install(nodes.Get(2));   // CHANGE: receiver node
  sinkApp.Start(Seconds(1.0));
  sinkApp.Stop(Seconds(20.0));

  // 7. SENDER ❌
  OnOffHelper source("ns3::UdpSocketFactory",
                     InetSocketAddress(interfaces.GetAddress(2), 5000)); // CHANGE: receiver's index
  source.SetAttribute("DataRate", StringValue("1Mbps"));      // CHANGE: rate
  source.SetAttribute("PacketSize", UintegerValue(512));      // CHANGE: packet size
  ApplicationContainer sourceApp = source.Install(nodes.Get(0)); // CHANGE: sender node
  sourceApp.Start(Seconds(2.0));
  sourceApp.Stop(Seconds(20.0));

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