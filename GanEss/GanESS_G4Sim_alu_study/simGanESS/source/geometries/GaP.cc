#include "GaP.hh"

#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4MultiUnion.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4UnitsTable.hh"
#include <G4UserLimits.hh>
#include <G4SDManager.hh>

#include "nexus/FactoryBase.h"
#include "nexus/CylinderPointSampler2020.h"
//#include "nexus/CylinderPointSampler.h"
#include "nexus/UniformElectricDriftField.h"
#include "nexus/OpticalMaterialProperties.h"
#include "nexus/XenonProperties.h"
#include "nexus/ArgonGasProperties.h"
#include "nexus/IonizationSD.h"

#include <iostream>
#include <cmath>
#include <vector>


using namespace nexus;

REGISTER_CLASS(GaP, GeometryBase)

GaP::GaP():
    GeometryBase(),
    msg_ (nullptr),
    gas_ (nullptr),
    mesh_mat_ (nullptr),

    gas_element_("Ar"),

    vessel_rad_        (276./2  *mm),
    vessel_length_     (38.599  *cm), // Adjusted length so that the gas volume is centered. Original length (38.639  *cm),

    mesh_rad_          (104./2  *mm),
    mesh_thickn_       (0.075   *mm),
    mesh_transparency_ (0.95),

    meshBracket_rad_      (180./2  *mm),
    meshBracket_thickn_   (6.      *mm),
    anodeBracket_rad_     (160./2  *mm),
    anodeBracket_thickn_  (6.975   *mm),
 
    pmt_rad_ (25.4/2  *mm),

    enclosure_pmt_rad_        (120./2  *mm),
    enclosure_pmt_thickn_     (8.5     *mm),
    enclosure_pmt_length_     (113.5   *mm),
    enclosurevac_pmt_length_  (110.5   *mm),

    plate_pmt_rad_        (105./2  *mm),
    plate_pmt_thickn_     (105./2  *mm),
    plate_pmt_length_     (10      *mm),
    plateUp_pmt_length_   (15      *mm),
    plateUp_pmt_thickn_   (21.5    *mm),

    pmtHolder_rad_        (115./2  *mm),
    pmtHolder_length_     (9.      *mm),

    tpb_coating_thickn_   (3       *micrometer),

    photoe_prob_       (0.),

    pressure_          (5 * bar),
    temperature_       (293.15 * kelvin),
    //sc_yield_          (2222 * 1/MeV), // Wsc = 45 eV, fr 
    sc_yield_          (0 * 1/MeV), // Wsc = 45 eV, fr
    elifetime_         (1e6* ms),
    //    elifetime_         (0* ms),

    drift_vel_         (1. * mm/microsecond),
    drift_transv_diff_ (1. * mm/sqrt(cm)),
    drift_long_diff_   (.3 * mm/sqrt(cm)),

    el_field_          (35.0 * kilovolt/cm),
    el_vel_            (3. * mm/microsecond),
    el_transv_diff_    (1. * mm/sqrt(cm)),
    el_long_diff_      (.3 * mm/sqrt(cm)),
    specific_vertex_{}
{
  // Messenger
  msg_ = new G4GenericMessenger(this, "/Geometry/GaP/",
                                "Control commands of the GaP geometry.");
  // Parametrized dimensions
  DefineConfigurationParameters();
}

GaP::~GaP()
{
  delete msg_;

  delete drift_gen_;
  delete el_gen_;

}

void GaP::Construct()
{
    // TPC gas
    if (gas_element_ == "Xe") {
      gas_ = materials::GXe(pressure_, temperature_);
      gas_->SetMaterialPropertiesTable(opticalprops::GXe(pressure_, temperature_, sc_yield_, elifetime_));
      yield_ = XenonELLightYield(el_field_, gas_->GetPressure());

    } 
    else if (gas_element_ == "Ar") {
      gas_ =  materials::GAr(pressure_, temperature_);
      //gas_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Ar");
      gas_->SetMaterialPropertiesTable(opticalprops::GAr(sc_yield_, elifetime_));
      G4cout << "gas_" << gas_ << G4endl;
      //gas_->SetMaterialPropertiesTable(nullptr);
     /// gas_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());
      yield_ = ArgonELLightYield(el_field_, gas_->GetPressure());
      //yield_ = 1;
      G4cout << "yield_" << yield_ << G4endl;
      

    }
    else {
      G4Exception("[NextNewVessel]", "Construct()", FatalException,
		  "Unknown kind of gas, valid options are: Xe, Ar");
    }

    // Mesh materials (cathode, anode and gate)
    mesh_mat_ = materials::FakeDielectric(gas_, "mesh_mat");
    mesh_mat_->SetMaterialPropertiesTable(opticalprops::FakeGrid(pressure_,
              temperature_, mesh_transparency_, mesh_thickn_,
              sc_yield_, elifetime_, photoe_prob_));

    steel_ = materials::Steel();
    steel_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

    //PEEK for the holders
    peek_ = materials::PEEK();
    peek_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

    // quartz (SiO2, crystalline) with evaporated TPB
    quartz_ = materials::FusedSilica();
    quartz_->SetMaterialPropertiesTable(opticalprops::FusedSilica());

    //Teflon
    teflon_mat_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    teflon_mat_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

    //Aluminium material for the foil on the top of the source
    aluminium_mat_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");
    aluminium_mat_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

    //Plexiglass material for source box
    plexiglass_mat_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_PLEXIGLASS");
    plexiglass_mat_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

    //TPB coating
    tpb_ = materials::TPB();
    tpb_->SetMaterialPropertiesTable(opticalprops::TPB());

    vacuum_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

    //Cylinder, acting as the vessel
    G4Tubs          *solid_vessel_steel = new G4Tubs("Vessel", 0, vessel_rad_, vessel_length_/2 , 0., 360.*deg);

    G4LogicalVolume *logic_vessel_steel = new G4LogicalVolume(solid_vessel_steel, steel_, "Vessel");
    this->SetLogicalVolume(logic_vessel_steel);

    //Build inside detector
    BuildTPC(gas_, mesh_mat_, steel_, peek_, vacuum_, quartz_, tpb_, logic_vessel_steel);
}

G4ThreeVector GaP::GenerateVertex(const G4String& region) const
{
  G4ThreeVector vertex;

  if (region == "AD_HOC") {
    vertex = specific_vertex_;
  }

  //// Gas regions
  else if (
    (region == "GasEL") ||
    (region == "GasDrift")) {
    //vertex = GenerateVertexGas(region);
    vertex = specific_vertex_; //in order to choose the position of the vertex in the macro
  }

  else {
    G4Exception("[GaP]", "GenerateVertex()", FatalException,
      "Unknown vertex generation region!");
  }

  return vertex;
}

G4ThreeVector GaP::GenerateVertexGas(const G4String& region) const
{
    G4ThreeVector vertex;

    if     (region == "GasEL")     {vertex = el_gen_->GenerateVertex("VOLUME");}
    else if(region == "GasDrift")  {vertex = drift_gen_->GenerateVertex("VOLUME");}
    else{G4Exception("[GaP]", "GenerateVertex()", FatalException,
                "Unknown vertex generation region!");}
    return vertex;
}


void GaP::DefineConfigurationParameters()
{
  // Gas element
  msg_->DeclareProperty("gas", gas_element_, "Gas element in TPC.");
  
    // Gas pressure
  G4GenericMessenger::Command& pressure_cmd =
    msg_->DeclareProperty("pressure", pressure_,
                          "Pressure of the gas.");
  pressure_cmd.SetUnitCategory("Pressure");
  pressure_cmd.SetParameterName("pressure", false);
  pressure_cmd.SetRange("pressure>0.");
  G4cout << "pressure is : " << pressure_ * bar << G4endl;

  // Gas temperature
  G4GenericMessenger::Command& temperature_cmd =
    msg_->DeclareProperty("temperature", temperature_,
                          "Temperature of the gas.");
  temperature_cmd.SetUnitCategory("Temperature");
  temperature_cmd.SetParameterName("temperature", false);
  temperature_cmd.SetRange("temperature>0.");

  // e- lifetime
  G4GenericMessenger::Command& e_lifetime_cmd =
    msg_->DeclareProperty("elifetime", elifetime_,
                          "Electron lifetime in gas.");
  e_lifetime_cmd.SetParameterName("elifetime", false);
  e_lifetime_cmd.SetUnitCategory("Time");
  e_lifetime_cmd.SetRange("elifetime>0.");

  // Drift velocity in drift region
  G4GenericMessenger::Command& drift_vel_cmd =
    msg_->DeclareProperty("drift_vel", drift_vel_,
                          "Electron drift velocity in the drift region.");
  drift_vel_cmd.SetParameterName("drift_vel", false);
  drift_vel_cmd.SetUnitCategory("Velocity");
  drift_vel_cmd.SetRange("drift_vel>0.");

  // Transverse diffusion in drift region
  new G4UnitDefinition("mm/sqrt(cm)", "mm/sqrt(cm)", "Diffusion", mm/sqrt(cm));
  G4GenericMessenger::Command& drift_transv_diff_cmd =
    msg_->DeclareProperty("drift_transv_diff", drift_transv_diff_,
                          "Tranvsersal diffusion in the drift region");
  drift_transv_diff_cmd.SetParameterName("drift_transv_diff", false);
  drift_transv_diff_cmd.SetUnitCategory("Diffusion");
  drift_transv_diff_cmd.SetRange("drift_transv_diff>0.");

  // Longitudinal diffusion in drift region
  G4GenericMessenger::Command& drift_long_diff_cmd =
    msg_->DeclareProperty("drift_long_diff", drift_long_diff_,
                          "Longitudinal diffusion in the drift region");
  drift_long_diff_cmd.SetParameterName("drift_long_diff", false);
  drift_long_diff_cmd.SetUnitCategory("Diffusion");
  drift_long_diff_cmd.SetRange("drift_long_diff>0.");

  // Scintillation yield (for S1)
  new G4UnitDefinition("1/MeV","1/MeV", "1/Energy", 1/MeV);
  G4GenericMessenger::Command& sc_yield_cmd =
    msg_->DeclareProperty("sc_yield", sc_yield_,
        "Set scintillation yield for gas. It is in photons/MeV");
  sc_yield_cmd.SetParameterName("sc_yield", true);
  sc_yield_cmd.SetUnitCategory("1/Energy");
 

  // Drift velocity in EL region
  G4GenericMessenger::Command& el_vel_cmd =
    msg_->DeclareProperty("el_vel", el_vel_,
                          "Electron drift velocity in the EL region.");
  el_vel_cmd.SetParameterName("el_vel", false);
  el_vel_cmd.SetUnitCategory("Velocity");
  el_vel_cmd.SetRange("el_vel>0.");

  // Transverse diffusion in EL region
  G4GenericMessenger::Command& el_transv_diff_cmd =
    msg_->DeclareProperty("el_transv_diff", el_transv_diff_,
                          "Tranvsersal diffusion in the EL region");
  el_transv_diff_cmd.SetParameterName("el_transv_diff", false);
  el_transv_diff_cmd.SetUnitCategory("Diffusion");
  el_transv_diff_cmd.SetRange("el_transv_diff>0.");

  // Longitudinal diffusion in EL region
  G4GenericMessenger::Command& el_long_diff_cmd =
    msg_->DeclareProperty("el_long_diff", el_long_diff_,
                          "Longitudinal diffusion in the EL region");
  el_long_diff_cmd.SetParameterName("el_long_diff", false);
  el_long_diff_cmd.SetUnitCategory("Diffusion");
  el_long_diff_cmd.SetRange("el_long_diff>0.");

  // EL field
  new G4UnitDefinition("kilovolt/cm", "kV/cm", "Electric field", kilovolt/cm);
  G4GenericMessenger::Command& el_field_cmd =
    msg_->DeclareProperty("el_field", el_field_,
                          "EL electric field intensity");
  el_field_cmd.SetParameterName("el_field", false);
  el_field_cmd.SetUnitCategory("Electric field");

  // Photoelectric probability
  msg_->DeclareProperty("photoe_prob", photoe_prob_,
                        "Probability of optical photon to ie- conversion");


  // Specific vertex in case region to shoot from is AD_HOC
  msg_->DeclarePropertyWithUnit("specific_vertex", "mm",  specific_vertex_, "Set generation vertex.");
}

void GaP::BuildTPC(G4Material* gas, G4Material* mesh_mat, G4Material* steel, G4Material* peek, G4Material* vacuum, G4Material* quartz, G4Material* tpb, G4LogicalVolume* logic_vessel_steel)
{   pmt_.SetSensorDepth(2);
    //// we declare the dimensions at the start because there are some dependencies between geometries
    pmt_.Construct();

    //PMT R11410

    G4double front_diam  = pmt_.FrontBodyDiameter();
    G4double front_len   = pmt_.FrontBodyLength();

    G4double rear_diam   = pmt_.RearBodyDiameter();
    G4double rear_len    = pmt_.RearBodyLength();

    G4double body_thick  = pmt_.BodyThickness();

    G4double win_diam    = pmt_.WindowDiameter();
    G4double win_thick   = pmt_.WindowThickness();

    G4double pc_diam     = pmt_.PhotocathodeDiameter();
    G4double pc_thick    = pmt_.PhotocathodeThickness();

    G4LogicalVolume* logic_pmt = pmt_.GetLogicalVolume();

    //PMT Length as to be calculated 

    G4double pmt_length_ = front_len + rear_len;

    //PMTR7378A

    //G4double pmt_length_ = pmt_.Length(); 

    G4double pmt_z  = 42.495*mm + pmt_length_/2;

    G4double tpb_coating_z = -pmt_z + pmt_length_/2 + tpb_coating_thickn_/2; // Z position of the surface of the PMTs

    G4double anodeBracket_z = -pmt_z + pmt_length_/2 + 2. * mm + meshBracket_thickn_/2; // 2mm between end of PMTs and anode
    G4double anode_z = anodeBracket_z;

    G4double gateBracket_z = anode_z + anodeBracket_thickn_/2 + 10.2 * mm  + meshBracket_thickn_/2;// Distance between the borders of the gate and the anode of 10.2mm
    G4double gate_z = gateBracket_z; //Position to have the gate position at the gateBracket center
    
    G4double cathBracket_z = gate_z + meshBracket_thickn_/2 + 87.0 * mm + meshBracket_thickn_/2; //87mm between Gate and Cathode
    G4double cathode_z = cathBracket_z;

    G4double cath_gate = cathode_z - gate_z - meshBracket_thickn_;

    G4double drift_z = cathode_z - (cathode_z - gate_z)/2 + meshBracket_thickn_/2; // Z position of Drift volume placed in the center of the gate and the cathode + meshBracket_thickn_/2 in order to not cover the gate (already covered by EL volume) and fully cover the cathode
    G4double drift_length_  = cathode_z - gate_z; // Length of the Drift volume to of 87mm + cathode volume

    G4double el_z = (gate_z + meshBracket_thickn_/2) - ((gate_z + meshBracket_thickn_/2) - (anode_z - anodeBracket_thickn_/2))/2; // Position of the center of the Gate - Anode distance 
    G4double el_length_     = (gate_z + meshBracket_thickn_/2) - (anode_z - anodeBracket_thickn_/2); //Distance between Gate - Anode (both entirely covered by EL volume)

    G4double lightTube_rad_    = 120./2 *mm;
    G4double lightTube_thickn_ = 5.     *mm;
    G4double lightTube_z       = cathode_z - (cathode_z - gate_z)/2; // Z position of light tube right in the middle of cathode and gate
    G4double lightTube_length_ = cathode_z - gate_z  - meshBracket_thickn_;// Light tube contained between cathode and gate

    G4double source_box_rad_ = 38. * mm;
    G4double source_box_thick_ = 3. * mm;

    G4double cath_plate_rad_int_ = source_box_rad_ + 0.003*mm; // Left plate of the light tube (hole in th center where we put the source box)
    G4double cath_plate_rad_ext_ = lightTube_rad_; // Left plate of the light tube 
    G4double cath_plate_length_ = lightTube_thickn_;
    G4double cath_plate_z = lightTube_z + lightTube_length_/2 - cath_plate_length_/2; // Position of the plate on the left part of the light tube

    // tpb for cath plate
    G4double tpb_cath_plate_thick = 0.003 * mm;
    G4double tpb_cath_plate_z = (cath_plate_z - cath_plate_length_/2 - tpb_cath_plate_thick/2);

    G4double source_plate_rad_ = source_box_rad_; // Small hole on the left plate to put the source box
    G4double source_plate_length_ = lightTube_thickn_/2 + 1.5*mm;
    G4double source_plate_z = cath_plate_z  + cath_plate_length_/2 - source_plate_length_/2; // Position of the Small hole on the left plate to put the source box

    // tpb for source plate
    G4double tpb_source_plate_thick = 0.003 * mm;
    G4double tpb_source_plate_z = (source_plate_z - tpb_source_plate_thick/2 - source_plate_length_/2);

    G4double tpb_lightTube_rad_int_   = lightTube_rad_ - 0.003 * mm;
    G4double tpb_lightTube_rad_ext_   = lightTube_rad_;
    G4double tpb_lightTube_length_ = lightTube_length_ - cath_plate_length_;
    G4double tpb_lightTube_z = lightTube_z - cath_plate_length_/2;

    G4double source_box_z_ = source_plate_z - source_plate_length_/2 - source_box_thick_/2 - tpb_source_plate_thick; // Source box placed on the small hole of the light tube
    
    G4double vsource_box_rad_ = source_box_rad_ - 1. * mm;
    G4double vsource_box_thick_ = source_box_thick_;
    G4double vsource_box_z_ = source_box_z_;

    G4double alu_foil_rad_ = source_box_rad_;
    G4double alu_foil_thick_ = 0.008 * mm; // Real thickness
    //G4double alu_foil_thick_ = 1. * mm;
    G4double alu_foil_z_ = source_box_z_ - source_box_thick_/2 - alu_foil_thick_/2; // Z position of the surface of the source box
    
    G4double rings_rad_int_ = 156*mm;
    G4double rings_rad_ext_ = 166*mm;
    G4double rings_length_ = 9*mm;
    G4double rings_z = 0; //Defined in the loop to generate rings

    /// Peek Mesh Holder (holds cathode and gate)
    G4double meshHolder_length_    = 36.75     *mm;
    G4double meshHolder_z = -13.005*mm + meshHolder_length_/2 ;
    G4double meshHolderBar_rad_     = 9./2   *mm;
    G4double meshHolderBar_length_  = 35.75  *mm;
    G4double meshHolderBar_xy = 73.769*mm;
    G4double meshHolderBar_z  = meshHolder_z + meshHolder_length_/2 + meshHolderBar_length_/2 ;

    G4double enclosure_pmt_z = vessel_length_/2 - enclosure_pmt_length_/2;
    G4double relative_pmt_z  = enclosure_pmt_z - pmt_z;

    G4double enclosurevac_pmt_z = vessel_length_/2 - enclosurevac_pmt_length_/2;
    G4double relativevac_pmt_z  = enclosurevac_pmt_z - pmt_z;

    G4double pmtHolder_z = enclosurevac_pmt_length_/2 - pmtHolder_length_/2;
    G4double plate1_pmt_z = enclosure_pmt_z - enclosure_pmt_length_/2 - plate_pmt_length_/2;
    G4double plate0_pmt_z = 64.495 *mm;
    G4double plateUp_pmt_rad_ = enclosure_pmt_rad_ + enclosure_pmt_thickn_;
    G4double plateUp_pmt_z = vessel_length_/2 - plateUp_pmt_length_/2 ;

    ////// Definition of the vessel (Cylinder that contains everything)
    G4Tubs          *solid_vessel = new G4Tubs("GasVessel", 0, vessel_rad_, vessel_length_/2 , 0., 360.*deg);
    G4LogicalVolume *logic_vessel = new G4LogicalVolume(solid_vessel, gas, "GasVessel");
    new G4PVPlacement(0, G4ThreeVector(), logic_vessel, "GasVessel", logic_vessel_steel, false, 0, true);

  // Geometries contained in GasVessel volume

    //Steel Bar joining Mesh holder and PMT clad (contained in GasVessel)
    G4Tubs          *solid_meshHolderBar = new G4Tubs("MeshHolderBar", 0., meshHolderBar_rad_, (meshHolderBar_length_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_meshHolderBar = new G4LogicalVolume(solid_meshHolderBar, steel, "MeshHolderBar");

    new G4PVPlacement(0, G4ThreeVector(meshHolderBar_xy, meshHolderBar_xy, -meshHolderBar_z), logic_meshHolderBar, "MeshHolderBarA", logic_vessel, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(-meshHolderBar_xy, meshHolderBar_xy, -meshHolderBar_z), logic_meshHolderBar, "MeshHolderBarB", logic_vessel, false, 1, true);
    new G4PVPlacement(0, G4ThreeVector(meshHolderBar_xy, -meshHolderBar_xy, -meshHolderBar_z), logic_meshHolderBar, "MeshHolderBarC", logic_vessel, false, 2, true);
    new G4PVPlacement(0, G4ThreeVector(-meshHolderBar_xy, -meshHolderBar_xy, -meshHolderBar_z), logic_meshHolderBar, "MeshHolderBarD", logic_vessel, false, 3, true);

    //// Drift (contained in GasVessel)
    G4Tubs          *solid_gas_drift = new G4Tubs("GasDrift", 0., meshBracket_rad_, (drift_length_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_gas_drift = new G4LogicalVolume(solid_gas_drift, gas, "GasDrift");
    G4VPhysicalVolume* drift_phys_ = new G4PVPlacement(0, G4ThreeVector(0., 0., drift_z), logic_gas_drift, "GasDrift", logic_vessel, false, 0, true);

    // Geomtries contained in GasDrift volume

     // Cathode Bracket (in "GasDrift" volume)
    G4Tubs          *solid_cathBracket = new G4Tubs("CathodeBracket", mesh_rad_, meshBracket_rad_, (meshBracket_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_cathBracket = new G4LogicalVolume(solid_cathBracket, steel, "CathodeBracket");
    new G4PVPlacement(0, G4ThreeVector(0., 0., cathBracket_z - drift_z), logic_cathBracket, "CathodeBracket", logic_gas_drift, false, 0, true);

    // Cathode (in "GasDrift" volume)
    G4Tubs          *solid_cathode = new G4Tubs("Cathode", 0., mesh_rad_, (mesh_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_cathode = new G4LogicalVolume(solid_cathode, mesh_mat, "Cathode");
    new G4PVPlacement(0, G4ThreeVector(0., 0., cathode_z - drift_z), logic_cathode, "Cathode", logic_gas_drift, false, 0, true);

    // Source Box (in "GasDrift" volume)
    G4Tubs          *solid_sourceBox = new G4Tubs("SourceBox", 0, source_box_rad_/2, (source_box_thick_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_sourceBox = new G4LogicalVolume(solid_sourceBox, plexiglass_mat_, "SourceBox");
    new G4PVPlacement(0, G4ThreeVector(0., 0., source_box_z_ - drift_z), logic_sourceBox, "SourceBox", logic_gas_drift, false, 0, true);

    // Vacuum inside Source Box (in "SourceBox" volume)
    G4Tubs          *solid_vSourceBox = new G4Tubs("VSourceBox", 0, source_box_rad_/2 - 0.5 * mm, (vsource_box_thick_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_vSourceBox = new G4LogicalVolume(solid_vSourceBox, vacuum, "VSourceBox");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0), logic_vSourceBox, "VSourceBox", logic_sourceBox, false, 0, true);

    // Aluminium foil on the top of the source box (in "GasDrift" volume)
    G4Tubs          *solid_aluFoil = new G4Tubs("AluFoil", 0, alu_foil_rad_/2, (alu_foil_thick_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_aluFoil = new G4LogicalVolume(solid_aluFoil, aluminium_mat_, "AluFoil");
    new G4PVPlacement(0, G4ThreeVector(0., 0., alu_foil_z_ - drift_z), logic_aluFoil, "AluFoil", logic_gas_drift, false, 0, true);

    // Light Tube (in GasDrift volume)
    G4Tubs          *solid_light_tube = new G4Tubs("light_tube_ext", lightTube_rad_ , lightTube_rad_ + lightTube_thickn_/2 , lightTube_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_light_tube = new G4LogicalVolume(solid_light_tube, teflon_mat_, "light_tube");
    new G4PVPlacement(0, G4ThreeVector(0., 0., lightTube_z - drift_z), logic_light_tube, "light_tube", logic_gas_drift, false, 0, true);

    // TPB coating inner part of the light tube (in vessel volume)
    G4Tubs          *solid_tpb_light_tube = new G4Tubs("tpb_light_tube", tpb_lightTube_rad_int_, tpb_lightTube_rad_ext_, tpb_lightTube_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_tpb_light_tube = new G4LogicalVolume(solid_tpb_light_tube,tpb, "tpb_light_tube");
    new G4PVPlacement(0, G4ThreeVector(0., 0., tpb_lightTube_z - drift_z), logic_tpb_light_tube, "tpb_light_tube", logic_gas_drift, false, 0, true);

    G4OpticalSurface* tpb_lightTube_coating_surf =
      new G4OpticalSurface("TPB_COATING_OPSURF", glisur, ground,
                           dielectric_dielectric, .01);
    new G4LogicalSkinSurface("TPB_COATING_OPSURF", logic_tpb_light_tube, tpb_lightTube_coating_surf);

    // cathode Plate (in GasDrift volume)
    G4Tubs          *solid_cath_plate = new G4Tubs("CathPlate", cath_plate_rad_int_/2, cath_plate_rad_ext_, cath_plate_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_cath_plate = new G4LogicalVolume(solid_cath_plate, teflon_mat_, "CathPlate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., cath_plate_z - drift_z), logic_cath_plate, "CathPlate", logic_gas_drift, false, 0, true);

    // tpb cathode Plate
    G4Tubs          *solid_tpb_cath_plate = new G4Tubs("tpb_cath_plate", cath_plate_rad_int_/2, cath_plate_rad_ext_, tpb_cath_plate_thick/2, 0, 360*deg);
    G4LogicalVolume *logic_tpb_cath_plate = new G4LogicalVolume(solid_tpb_cath_plate,tpb, "tpb_cath_plate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., tpb_cath_plate_z - drift_z), logic_tpb_cath_plate, "tpb_cath_plate", logic_gas_drift, false, 0, true);

    // Source plate (in GasDrift volume)
    G4Tubs          *solid_source_plate = new G4Tubs("sourcePlate", 0, source_plate_rad_/2, source_plate_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_source_plate = new G4LogicalVolume(solid_source_plate, teflon_mat_, "sourcePlate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., source_plate_z - drift_z), logic_source_plate, "sourcePlate", logic_gas_drift, false, 0, true);

    // tpb source plate
    G4Tubs          *solid_tpb_source_plate = new G4Tubs("tpb_sourcePlate", 0, source_plate_rad_/2, tpb_source_plate_thick/2, 0, 360*deg);
    G4LogicalVolume *logic_tpb_source_plate = new G4LogicalVolume(solid_tpb_source_plate,tpb, "tpb_sourcePlate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., tpb_source_plate_z - drift_z), logic_tpb_source_plate, "tpb_sourcePlate", logic_gas_drift, false, 0, true);

    // tpb sides source plate
    G4double tpb_sides_source_plate_z = cath_plate_z - source_plate_length_/2;
    G4double tpd_sides_source_plate_length = cath_plate_length_ - source_plate_length_;
    G4double tpb_sides_source_rad_int = (cath_plate_rad_int_ - 0.003*mm);
    G4double tpb_sides_source_rad_ext = cath_plate_rad_int_;

    G4Tubs          *solid_tpb_sides_source_plate = new G4Tubs("tpb_sides_sourcePlate", tpb_sides_source_rad_int/2, tpb_sides_source_rad_ext/2, tpd_sides_source_plate_length/2, 0, 360*deg);
    G4LogicalVolume *logic_tpb_sides_source_plate = new G4LogicalVolume(solid_tpb_sides_source_plate,tpb, "tpb_sides_sourcePlate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., tpb_sides_source_plate_z - drift_z), logic_tpb_sides_source_plate, "tpb_sides_sourcePlate", logic_gas_drift, false, 0, true);

    //Loop to generate the rings (Gas Drift volume)
    G4ThreeVector pos_rings = G4ThreeVector(0, 0, 0);

    G4Tubs          *solid_rings = new G4Tubs("rings", rings_rad_int_/2, rings_rad_ext_/2, rings_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_rings = new G4LogicalVolume(solid_rings, aluminium_mat_, "rings");
    
    for (G4int i = 1; i < 7; i++) {
      rings_z = cathode_z - drift_z - cath_gate * i/6 + rings_length_/2 ;
      pos_rings = G4ThreeVector(0, 0, rings_z);

      new G4PVPlacement(0, pos_rings, logic_rings, "rings", logic_gas_drift, false, i, true);
    }
   
    // Definition of Drift properties

    drift_gen_  = new CylinderPointSampler2020(drift_phys_); //Generator of random vertex in the Drift Volume (only if activated)

    // Define the drift field
    UniformElectricDriftField* drift_field = new UniformElectricDriftField();

    drift_field->SetCathodePosition(drift_z + drift_length_/2);
    drift_field->SetAnodePosition  (drift_z - drift_length_/2);
    drift_field->SetDriftVelocity(drift_vel_);
    drift_field->SetTransverseDiffusion(drift_transv_diff_);
    drift_field->SetLongitudinalDiffusion(drift_long_diff_);

    // Apply drift field properties to the drift region defined as the Drift Volume previously
    G4Region* drift_region = new G4Region("DRIFT");
    drift_region->SetUserInformation(drift_field);
    drift_region->AddRootLogicalVolume(logic_gas_drift);
   
    // Size of the steps in this volume
    logic_gas_drift->SetUserLimits(new G4UserLimits(1.*mm));

    // Set the DRIFT volume as an ionization sensitive detector to save what happens in this volume
    IonizationSD* active_sd = new IonizationSD("/GaP/DRIFT");
    logic_gas_drift->SetSensitiveDetector(active_sd);
    G4SDManager::GetSDMpointer()->AddNewDetector(active_sd);

    //// EL gap volume (in vessel volume)
    G4Tubs          *solid_gas_el = new G4Tubs("GasEL", 0., meshBracket_rad_, (el_length_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_gas_el = new G4LogicalVolume(solid_gas_el, gas, "GasEL");
    G4VPhysicalVolume* el_phys_ = new G4PVPlacement(0, G4ThreeVector(0., 0., el_z), logic_gas_el, "GasEL", logic_vessel, false, 0, true);
    
    el_gen_  = new CylinderPointSampler2020(el_phys_);
    
    //// Gate
    G4Tubs          *solid_gate = new G4Tubs("Gate", 0., mesh_rad_, (mesh_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_gate = new G4LogicalVolume(solid_gate, mesh_mat, "Gate");
    new G4PVPlacement(0, G4ThreeVector(0., 0., gate_z - el_z), logic_gate, "Gate", logic_gas_el, false, 0, true);

    // Gate Bracket
    G4Tubs          *solid_gateBracket = new G4Tubs("GateBracket", mesh_rad_, meshBracket_rad_, (meshBracket_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_gateBracket = new G4LogicalVolume(solid_gateBracket, steel, "GateBracket");
    new G4PVPlacement(0, G4ThreeVector(0., 0., gateBracket_z - el_z), logic_gateBracket, "GateBracket", logic_gas_el, false, 0, true);

    //// Anode
    G4Tubs          *solid_anode = new G4Tubs("Anode", 0., mesh_rad_, (mesh_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_anode = new G4LogicalVolume(solid_anode, mesh_mat, "Anode");
    new G4PVPlacement(0, G4ThreeVector(0., 0., anode_z - el_z), logic_anode, "Anode", logic_gas_el, false, 0, true);

    // Anode Bracket
    G4Tubs          *solid_anodeBracket = new G4Tubs("AnodeBracket", mesh_rad_, anodeBracket_rad_, (anodeBracket_thickn_)/2, 0., 360.*deg);
    G4LogicalVolume *logic_anodeBracket = new G4LogicalVolume(solid_anodeBracket, steel, "AnodeBracket");
    new G4PVPlacement(0, G4ThreeVector(0., 0., anodeBracket_z - el_z), logic_anodeBracket, "AnodeBracket", logic_gas_el, false, 0, true);

    /// Define EL electric field    
    UniformElectricDriftField* el_field = new UniformElectricDriftField();
    el_field->SetCathodePosition(el_z + (el_length_)/2.);
    el_field->SetAnodePosition  (el_z - (el_length_)/2.);
    el_field->SetDriftVelocity        (el_vel_);
    el_field->SetTransverseDiffusion  (el_transv_diff_);
    el_field->SetLongitudinalDiffusion(el_long_diff_);
    el_field->SetLightYield(yield_);
    G4Region* el_region = new G4Region("EL");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(logic_gas_el);

    G4cout << "* GATE Z position: " << el_z + gate_z << G4endl; 
    G4cout << "* GATE Volt position: " << el_z + el_length_/2. << G4endl;
    G4cout << "* ANODE Z position: " << el_z + anode_z << G4endl;
    G4cout << "* ANODE Volt position: " << el_z - el_length_/2. << G4endl;
    G4cout << "* EL_GAP Z positions: " << el_z - el_length_/2. <<
              " to " << el_z + el_length_/2. << G4endl;

    G4cout << "* EL field intensity (kV/cm): " << el_field_ / (kilovolt/cm)
            << "  ->  EL Light yield (photons/ie-/cm): " << yield_ / (1/cm) << G4endl;
    G4cout << "* EL Light yield (photons/ie-): " << yield_ * el_length_ << G4endl;
    
    /// Build PMT (in vessel volume)
    G4Tubs *solid_pmt = new G4Tubs("SolidPMT", 0., pmt_rad_, pmt_length_/2, 0., 360.*deg); // Hamamatsu pmt length: 43*mm | STEP pmt gap length: 57.5*mm

    // Position pairs (x,Y) for PMTs for PMTR7378A
    //std::vector <float> pmt_PsX={-15.573, 20.68, -36.253, 0., 36.253, -20.68, 15.573};
    //std::vector <float> pmt_PsY={-32.871, -29.922, -2.949, 0., 2.949, 29.922, 32.871};

    // Position pairs (x,Y) for PMTs for PMTR11410
    std::vector <float> pmt_PsX={0.};
    std::vector <float> pmt_PsY={0.};

    /// Evaporated TPB
    G4Tubs          *solid_tpb_coating = new G4Tubs("CoatingTPB", 0, pmt_rad_ , tpb_coating_thickn_/2, 0, 360*deg);
    G4LogicalVolume *logic_tpb_coating = new G4LogicalVolume(solid_tpb_coating, tpb, "CoatingTPB");
    G4ThreeVector pos_pmt = G4ThreeVector(0, 0, 0);
    G4ThreeVector pos_tpb_coating = G4ThreeVector(0., 0., 0.);

    //loop to coat TPB on the surface of each PMTs
    for (G4int i = 0; i < G4int(pmt_PsX.size()); i++) {
      pos_tpb_coating = G4ThreeVector(pmt_PsX[i]*mm, pmt_PsY[i]*mm, tpb_coating_z);

      new G4PVPlacement(0, pos_tpb_coating, logic_tpb_coating, "CoatingTPB", logic_vessel, false, 0, true);
      G4cout << "pos_pmt is : " << pos_pmt << G4endl;
    }

    // Optical surface between gas and TPB to model the latter's roughness
    G4OpticalSurface* tpb_coating_surf =
      new G4OpticalSurface("TPB_COATING_OPSURF", glisur, ground,
                           dielectric_dielectric, .01);
    new G4LogicalSkinSurface("TPB_COATING_OPSURF", logic_tpb_coating, tpb_coating_surf);

    // PMT clad
    G4VSolid *solid_enclosure_pmt = new G4Tubs("EnclosurePMT", 0, enclosure_pmt_rad_+ enclosure_pmt_thickn_ , (enclosure_pmt_length_)/2, 0., 360.*deg);
    
    // Vacuum inside the pmt enclosure
    
    G4VSolid *solid_enclosurevac_pmt = new G4Tubs("EnclosureVacPMT", 0, enclosure_pmt_rad_, (enclosurevac_pmt_length_)/2, 0., 360.*deg);
    // PMT Holder
    
    G4VSolid *solid_pmtHolder = new G4Tubs("PMTHolder", 0, pmtHolder_rad_, (pmtHolder_length_)/2, 0., 360.*deg);
    // Steel plate enclosing the pmt tube
    
    G4VSolid *solid_plate1_pmt = new G4Tubs("PMTplateBottom1", 0, plate_pmt_rad_+plate_pmt_thickn_, plate_pmt_length_/2, 0, 360*deg);

    G4ThreeVector pos_enclosure_pmt = G4ThreeVector(0, 0, 0);
    G4ThreeVector pos_enclosurevac_pmt = G4ThreeVector(0, 0, 0);
    G4ThreeVector pos = G4ThreeVector(0, 0, 0);

    // Loop to generate the PMTs
    for (G4int i = 0; i < G4int(pmt_PsX.size()); i++) {
      pos_pmt = G4ThreeVector(pmt_PsX[i]*mm, pmt_PsY[i]*mm, -pmt_z);
      pos_enclosure_pmt = G4ThreeVector(pmt_PsX[i]*mm, pmt_PsY[i]*mm, relative_pmt_z);
      pos_enclosurevac_pmt = G4ThreeVector(pmt_PsX[i]*mm, pmt_PsY[i]*mm, relativevac_pmt_z);
      pos = G4ThreeVector(pmt_PsX[i]*mm, pmt_PsY[i]*mm, 0.);

      solid_enclosure_pmt = new G4SubtractionSolid("EnclosurePMT_Sub", solid_enclosure_pmt, solid_pmt,  0, pos_enclosure_pmt);
      solid_enclosurevac_pmt = new G4SubtractionSolid("EnclosureVacPMT_Sub", solid_enclosurevac_pmt, solid_pmt,  0, pos_enclosurevac_pmt);
      solid_pmtHolder = new G4SubtractionSolid("PMTHolder_Sub", solid_pmtHolder, solid_pmt,  0, pos);
      solid_plate1_pmt = new G4SubtractionSolid("PMTplateBottom1_Sub", solid_plate1_pmt, solid_pmt,  0, pos);

      new G4PVPlacement(0, pos_pmt, logic_pmt, "PMT", logic_vessel, false, i, true);
    }

    G4LogicalVolume *logic_enclosure_pmt = new G4LogicalVolume(solid_enclosure_pmt, steel, "EnclosurePMT");
    new G4PVPlacement(0, G4ThreeVector(0., 0., -enclosure_pmt_z), logic_enclosure_pmt, "EnclosurePMT", logic_vessel, false, 0, true);

    G4LogicalVolume *logic_enclosurevac_pmt = new G4LogicalVolume(solid_enclosurevac_pmt, vacuum, "EnclosureVacPMT");
    new G4PVPlacement(0, G4ThreeVector(0., 0, enclosure_pmt_z - enclosurevac_pmt_z), logic_enclosurevac_pmt, "EnclosureVacPMT", logic_enclosure_pmt, false, 0, true);

    G4LogicalVolume *logic_pmtHolder = new G4LogicalVolume(solid_pmtHolder, steel, "PMTHolder");
    new G4PVPlacement(0, G4ThreeVector(0., 0, pmtHolder_z), logic_pmtHolder, "PMTHolder", logic_enclosurevac_pmt, false, 0, true);

    G4LogicalVolume *logic_plate1_pmt = new G4LogicalVolume(solid_plate1_pmt, steel, "PMTplateBottom1");
    new G4PVPlacement(0, G4ThreeVector(0., 0., -plate1_pmt_z), logic_plate1_pmt, "PMTplateBottom1", logic_vessel, false, 0, true);

    // Steel plate attached where the peek holders are attached
    G4Tubs          *solid_plate0_pmt = new G4Tubs("PMTplateBottom0", plate_pmt_rad_, plate_pmt_rad_+plate_pmt_thickn_, plate_pmt_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_plate0_pmt = new G4LogicalVolume(solid_plate0_pmt, steel, "PMTplateBottom0");

    //G4double plate0_pmt_z = plate1_pmt_z - plate_pmt_length_;
    
    new G4PVPlacement(0, G4ThreeVector(0., 0., -plate0_pmt_z), logic_plate0_pmt, "PMTplateBottom0", logic_vessel, false, 0, true);

    // Upper steel plate at the pmt clad
    G4Tubs          *solid_plateUp_pmt = new G4Tubs("PMTplateUp", plateUp_pmt_rad_, plateUp_pmt_rad_+plateUp_pmt_thickn_, plateUp_pmt_length_/2, 0, 360*deg);
    G4LogicalVolume *logic_plateUp_pmt = new G4LogicalVolume(solid_plateUp_pmt, steel, "PMTplateUp");

    
    new G4PVPlacement(0, G4ThreeVector(0., 0., -plateUp_pmt_z), logic_plateUp_pmt, "PMTplateUp", logic_vessel, false, 0, true);

}
