#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleNet");

int main(int argc, char *argv[]) {
    // Enable logging
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

    // Create 3 nodes
    NodeContainer nodes;
    nodes.Create(3);

    // Create links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // Connect nodes
    NetDeviceContainer link1 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer link2 = p2p.Install(nodes.Get(1), nodes.Get(2));

    // Install internet protocols
    InternetStackHelper net;
    net.Install(nodes);

    // Enable forwarding on middle node
    nodes.Get(1)->GetObject<Ipv4>()->SetAttribute("IpForward", BooleanValue(true));

    // Assign IP addresses
    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    ip.Assign(link1);
    
    ip.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ifs = ip.Assign(link2);

    // Setup routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Setup first server on node 2
    UdpServerHelper srv1(5000);
    ApplicationContainer srvApp1 = srv1.Install(nodes.Get(2));
    srvApp1.Start(Seconds(1.0));
    srvApp1.Stop(Seconds(10.0));

    // Setup first client on node 0
    UdpClientHelper cli1(ifs.GetAddress(1), 5000);
    cli1.SetAttribute("MaxPackets", UintegerValue(1000));
    cli1.SetAttribute("Interval", TimeValue(MilliSeconds(10)));
    cli1.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer cliApp1 = cli1.Install(nodes.Get(0));
    cliApp1.Start(Seconds(2.0));
    cliApp1.Stop(Seconds(9.0));

    // Setup second server on node 2
    UdpServerHelper srv2(5001);
    ApplicationContainer srvApp2 = srv2.Install(nodes.Get(2));
    srvApp2.Start(Seconds(1.0));
    srvApp2.Stop(Seconds(10.0));

    // Setup second client on node 0
    UdpClientHelper cli2(ifs.GetAddress(1), 5001);
    cli2.SetAttribute("MaxPackets", UintegerValue(1000));
    cli2.SetAttribute("Interval", TimeValue(MilliSeconds(10)));
    cli2.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer cliApp2 = cli2.Install(nodes.Get(0));
    cliApp2.Start(Seconds(2.0));
    cliApp2.Stop(Seconds(9.0));

    // Enable packet capture
    p2p.EnablePcap("scratch/net-n0", link1.Get(0), true);
    p2p.EnablePcap("scratch/net-n1", link2.Get(0), true);

    // Monitor network flows
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}