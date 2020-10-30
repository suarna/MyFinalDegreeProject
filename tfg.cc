/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* 
 * File:   tfg_6lowpan.cc
 * Author: Miguel Angel Barrero Díaz (mikelone2002@hotmail.com)
 *
 * Created on 21 de abril de 2019, 11:13
 */
/*
 * Este programa ha sido elaborado como parte del Trabajo Fin de Grado de Miguel Angel Barrero Díaz
 * Se trata de la simulación de una red 6LoWPAN que pretende ser un complemento práctico de los conceptos abordados
 * a lo largo del proyecto
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-module.h"
#include "ns3/buildings-module.h"
#include "ns3/netanim-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/stats-module.h"
#include "ns3/gnuplot.h"
#include "ns3/gnuplot-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("tfg");

/*
 * 
 */
void PasarValorCanal(LrWpanPhyPibAttributes atributos,uint8_t canal, uint32_t page)
    {
        atributos.phyCurrentChannel=canal;
        atributos.phyCurrentPage=page; 
    }
 
static void 
CourseChange (std::string contexto, Ptr<const MobilityModel> mobilidad)
{
  Vector pos = mobilidad->GetPosition ();
  Vector vel = mobilidad->GetVelocity ();
  std::cout << Simulator::Now () << ", modelo=" << mobilidad << ", POSICION: x=" << pos.x << ", y=" << pos.y
            << ", z=" << pos.z << "; VELOCIDAD:" << vel.x << ", y=" << vel.y
            << ", z=" << vel.z << std::endl;
}
static void
PathLoss (std::string contexto ,Ptr< const SpectrumPhy > txPhy, Ptr< const SpectrumPhy > rxPhy, double lossDb)
{ 
    NS_LOG_UNCOND (Simulator::Now()<<"----------" <<"La pérdida en dB es:"<< lossDb);

     
}
static void 
EstadoTarjeta (std::string context ,ns3::Time time ,LrWpanPhyEnumeration oldValue, LrWpanPhyEnumeration newValue)
{
   NS_LOG_UNCOND ("Estado antiguo:" << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldValue) << 
           " Estado nuevo: " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newValue));

}

double contador=0;
static void
PaquetesPerdidos (std::string contexto ,Ptr< const Packet > packet)
{
    contador++;
    std::cout<<Simulator::Now()<<"Paquetes caídos en transmisión:"<<contador<<std::endl;

}    
int main(int argc, char** argv) {
    
    CommandLine cmd;
    bool verbose = true;
    uint32_t nNodos = 5; 
    bool tcp = false;
    bool disc = false;
    bool box = false;
    bool interiores = false;
    bool verbose_phy = false;
    bool verbose_mob = false;
    uint32_t semilla = 1;
    double experd = 3;
    
    
    cmd.AddValue("nNodos","Numero de nodos",nNodos);
    cmd.AddValue("verbose", "Activar verbose", verbose);
    cmd.AddValue("verbose_phy", "Activar verbose capa fisica", verbose_phy); 
    cmd.AddValue("verbose_mob", "Activar verbose mobilidad ", verbose_mob); 
    cmd.AddValue("tcp","Uso de TCP si true,false por defecto",tcp);
    cmd.AddValue("disc", "Position allocator disc", disc); 
    cmd.AddValue("box", "Position allocator box", box);
    cmd.AddValue("interiores", "Con true se genera un modelo de propagación en interiores", interiores);
    cmd.AddValue("semilla", "Si se introduce nuevo valor de semilla cambia la ejecución", semilla);
    cmd.AddValue("experd","Introducir el valor del expoente de perdidas para single spectrum channel",experd);

   
    cmd.Parse (argc, argv);
 
    if (verbose){
      LogComponentEnable ("tfg", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer",LOG_LEVEL_INFO);
      LogComponentEnable ("LrWpanPhy",LOG_LEVEL_INFO);
    }
    
    nNodos = nNodos == 0 ? 1:nNodos; //Si nnodos es igual a 0 nndos=1 else nnodos=4 (valor por defecto)
    RngSeedManager::SetSeed(semilla);
    
    /*-----------------Creamos nodos LrWPAN-------------*/
    
    NodeContainer nodoGateway;
    NodeContainer nodosLrwpanTer;
    nodoGateway.Create(1);
    nodosLrwpanTer.Create(nNodos);
    NodeContainer nodosLrwpan(nodoGateway,nodosLrwpanTer);
    
    /*-------------Creamos edificio e instalamos los nodos en el-----------*/
    double x_min = 0.0;
    double x_max = 15.0;
    double y_min = 0.0;
    double y_max = 40.0;
    double z_min = 0.0;
    double z_max = 4.0;
    Ptr<Building> edificio = CreateObject <Building> ();
    edificio->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
    edificio->SetBuildingType (Building::Residential);
    edificio->SetExtWallsType (Building::ConcreteWithWindows);
    edificio->SetNFloors (1);
    edificio->SetNRoomsX (2);
    edificio->SetNRoomsY (6);
    
   
       /*----------Generamos movilidad en los nodos---------------------------------*/
    
    if(disc){
    MobilityHelper movilidad_gateway;
    movilidad_gateway.Install(nodoGateway);
    
    MobilityHelper movilidad_nodos;
    movilidad_nodos.SetPositionAllocator ("ns3::RandomDiscPositionAllocator","X", StringValue ("15.0"),"Y", StringValue ("40.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=10]"));
    movilidad_nodos.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Mode", StringValue ("Time"),"Time", StringValue ("500ms"),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                              "Bounds", StringValue ("0|30|0|60"));
    
    movilidad_nodos.Install (nodosLrwpanTer);
    //
    }
    
    else  if(box){
 
    MobilityHelper movilidad_gateway;
    movilidad_gateway.Install(nodoGateway);
    
    MobilityHelper movilidad_nodos;
    movilidad_nodos.SetPositionAllocator ("ns3::RandomBoxPositionAllocator","X",StringValue (" ns3::UniformRandomVariable[Min=0.0|Max=15]"),
                              "Y", StringValue (" ns3::UniformRandomVariable[Min=0.0|Max=40.0]"));
    movilidad_nodos.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Mode", StringValue ("Time"),"Time", StringValue ("500ms"),
                              "Speed", StringValue ("ns3::NormalRandomVariable[Mean=1|Variance=4|Bound=10]"), 
                              "Bounds", StringValue ("0|15|0|40"));
    
    movilidad_nodos.Install (nodosLrwpanTer);
   
    }
    
    else{
    MobilityHelper movilidad_gateway;
    //movilidad_gateway.SetPositionAllocator("ns3::RandomRoomPositionAllocator"); //cambiar posicion de nodo gateway
    movilidad_gateway.Install(nodoGateway);
    
    
    MobilityHelper movilidad_nodos;

    movilidad_nodos.SetPositionAllocator ("ns3::RandomBuildingPositionAllocator");
    movilidad_nodos.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Mode", StringValue ("Time"),"Time", StringValue ("500ms"),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                              "Bounds", StringValue ("0|15|0|40"));
    
    
    movilidad_nodos.Install (nodosLrwpanTer);
    } 
    
    
    /*------------Instalamos nodos con mobilidad en edificio----------*/
    BuildingsHelper::Install (nodosLrwpan);
    BuildingsHelper::MakeMobilityModelConsistent();
    
    /*-----------------Creamos dispositivos LrWpan--------------------*/ 
     //Configuramos para canal 0 IEEE_802_15_4_868MHZ_BPSK (0 y 0) IEEE_802_15_4_868MHZ_ASK (0 y 1) IEEE_802_15_4_868MHZ_OQPSK (1 y 2)
    LrWpanPhyPibAttributes atributos;
    uint8_t canal =0;
    uint32_t page =0;
    PasarValorCanal(atributos,canal,page);
    
    LrWpanHelper lrWpanHelper;
    
    /*---------Con esta opción añadimos canal de propagacion en interiores------*/
    if(interiores){
    SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
    channelHelper.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel","ShadowSigmaOutdoor",DoubleValue(7),"ShadowSigmaIndoor",DoubleValue(7),
                            "ShadowSigmaExtWalls",DoubleValue(7),"InternalWallLoss",DoubleValue(2.5),"Frequency",DoubleValue(869e6));
    //channelHelper.AddSpectrumPropagationLoss("ns3::FriisSpectrumPropagationLossModel");
    channelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    lrWpanHelper.SetChannel(channelHelper.Create());
    }
    
    else{
    /*--------Configuramos canal single spectrum----------------------*/
    Config::Set("/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Channel/PropagationLossModel/$ns3::LogDistancePropagationLossModel/Exponent",DoubleValue(experd));
    }
    
    /*-----------Añadimos protocolo 6lowpan a la pila---------------------------*/
    
    NetDeviceContainer lrWpanDevices = lrWpanHelper.Install(nodosLrwpan); 
    
    //LrWpanPhyPibAttributes atributos;
    //atributos.phyCurrentChannel = 11;
    //atributos.phyCurrentPage = 1;
    //LrWpanPhyPibAttributes *p_atributos = &atributos;
    //LrWpanPibAttributeIdentifier idcanal = phyCurrentChannel;
    //LrWpanPibAttributeIdentifier idpage = phyCurrentPage;
    //Ptr<LrWpanPhy> phy= lrWpanDevices.Get(0)->GetObject<LrWpanNetDevice>()->GetPhy();
    //phy->PlmeSetAttributeRequest(idcanal, p_atributos);
    //phy->PlmeSetAttributeRequest(idpage, p_atributos);
    //uint8_t canal =0;
    //uint32_t page =0;
    //PasarValorCanal(atributos,canal,page);
    
    SixLowPanHelper sixLowPanHelper;
    NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install (lrWpanDevices);
    
    
    /*-----------Asociar a misma PAN,añadir stack internet y capa 6lowpan a los nodos lrwpan---------*/ 
    lrWpanHelper.AssociateToPan (lrWpanDevices, 0);   //Asociamos nodos a la misma PAN

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install (nodosLrwpan); 
   
    /*-------------Añadimos direcciones ipv6 a los dipsositivos lrwpan----------*/

    Ipv6AddressHelper ipv6AddressHelper;
    ipv6AddressHelper.SetBase (Ipv6Address ("2001::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer sixLowPanInterfaces = ipv6AddressHelper.Assign (sixLowPanDevices);

   

    for (uint32_t i = 0; i < sixLowPanInterfaces.GetN(); ++i){
     
            std::cout<<"Direccines de dispositvos lrwpan:"<<std::endl;
            std::cout<< sixLowPanInterfaces.GetAddress(i,1)<<std::endl;   
    }
    
   
    /*--------------------Arrancamos aplicaciones--------------------------------*/
    if(tcp){   //Intercambio TCP
        
     /*-----------Añadimos servidor http en nodo PAN Coordinador------------------*/
    
    ThreeGppHttpServerHelper serverhttpHelper (sixLowPanInterfaces.GetAddress(0,1));
    ApplicationContainer serverHttpApp = serverhttpHelper.Install(nodosLrwpan.Get(0));
    
    /*-----------Añadimos clientes http en los nodos End-Device------------------*/
    
    ThreeGppHttpClientHelper clienthttpHelper (sixLowPanInterfaces.GetAddress(0,1));
    ApplicationContainer clientHttpApp = clienthttpHelper.Install(nodosLrwpanTer);
    
    serverHttpApp.Start(Seconds (1.));
    serverHttpApp.Stop(Minutes (1.));

    clientHttpApp.Start(Seconds (1.));
    clientHttpApp.Stop(Minutes (1.));
    }
    
    else{   //Intercambio UDP
    
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
    UdpServerHelper udpServerHelper(5683);
    UdpClientHelper udpClientHelper(sixLowPanInterfaces.GetAddress(0,1),5683); 
   
    
    ApplicationContainer serverUdp =udpServerHelper.Install(nodosLrwpan.Get(0));
    ApplicationContainer clientUdp =udpClientHelper.Install(nodosLrwpanTer);
    
    serverUdp.Start(Seconds (1.));
    serverUdp.Stop(Seconds (60.));
    
    clientUdp.Start(Seconds (2.));
    clientUdp.Stop(Seconds (60.));
    }
       
    /*------------Habilitmos tracing---------------------------------*/
    internetStackHelper.EnablePcapIpv6("tfg",nodoGateway.Get(0)->GetId(),1,true);
    double tasa = lrWpanDevices.Get(0)->GetObject<LrWpanNetDevice>()->GetPhy()->GetDataOrSymbolRate(true);
    std::cout<<"La tasa de transmisión es:"<<tasa<<" bps"<<std::endl;
    
    /*---------------Simulación-----------------------*/
    Config::Connect ("/NodeList/0/DeviceList/0/$ns3::LrWpanNetDevice/Mac/MacTxDrop",MakeCallback(&PaquetesPerdidos));
    
    if(verbose_mob){
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",MakeCallback (&CourseChange));
    }
    if(verbose_phy){
    Config::Connect ("/NodeList/0/DeviceList/0/$ns3::LrWpanNetDevice/Channel/$ns3::SingleModelSpectrumChannel/PathLoss",MakeCallback (&PathLoss));
    Config::Connect ("/NodeList/0/DeviceList/0/$ns3::LrWpanNetDevice/Phy/MacTxDrop",MakeCallback(&EstadoTarjeta));
    }
    /*-----------Creamos animación-------------------------------*/
    AnimationInterface animacion ("animación_tfg.xml");
    
    Simulator::Stop(Minutes(1.0));
    Simulator::Run ();
    Simulator::Destroy ();
   
    NS_LOG_INFO ("Fin de ejecución");    
    
    return 0;
}

  
