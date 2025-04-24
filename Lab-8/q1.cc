#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoNodeTopology");

int main() {
    LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
    LogComponentEnable("TcpL4Protocol", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer dev0_1 = p2p.Install(nodes.Get(0), nodes.Get(1));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(dev0_1);

    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 5001));
    ApplicationContainer tcpServerApp = tcpSink.Install(nodes.Get(1));
    tcpServerApp.Start(Seconds(1.0));
    tcpServerApp.Stop(Seconds(10.0));

    BulkSendHelper tcpClient("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), 5001));
    tcpClient.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer tcpClientApp = tcpClient.Install(nodes.Get(1));
    tcpClientApp.Start(Seconds(1.0));
    tcpClientApp.Stop(Seconds(10.0));

    // Flow Monitor
    FlowMonitorHelper flowMon;
    Ptr<FlowMonitor> monitor = flowMon.InstallAll();

    p2p.EnablePcapAll("scratch/q1");

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    
    monitor->CheckForLostPackets();
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    for(auto& flow : stats) {
        std::cout << flow.first << "\n";
        std::cout << flow.second.rxPackets << "\n";
        std::cout << flow.second.txPackets << "\n";
        "\n";
    }

    Simulator::Destroy();
}