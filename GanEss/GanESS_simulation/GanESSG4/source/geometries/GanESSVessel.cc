// ----------------------------------------------------------------------------
// nexus | GanESSVessel.cc
//
// Vessel of the GanESS geometry.
//
// The GanESS Group
// ----------------------------------------------------------------------------

#include "GanESSVessel.hh"
#include "nexus/MaterialsList.h"
#include "nexus/Visibilities.h"
#include "nexus/OpticalMaterialProperties.h"
#include "nexus/CylinderPointSampler.h"
#include "nexus/SpherePointSampler.h"
#include "nexus/IonizationSD.h"
//#include "Next100Th228Source.h"
//#include "CalibrationSource.h"

#include <G4GenericMessenger.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4UnionSolid.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Trap.hh>
#include <G4Sphere.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <Randomize.hh>
#include <G4TransportationManager.hh>
#include <G4UnitsTable.hh>
#include <G4SubtractionSolid.hh>
#include <G4SDManager.hh>

#include <CLHEP/Units/SystemOfUnits.h>

#include <math.h>
#include <algorithm>

using namespace nexus;

  GanESSVessel::GanESSVessel():
    GeometryBase(),

    // General vessel dimensions
    vessel_length_     (850./2. * mm), // real length
    vessel_in_rad_     (640./2. * mm),
    vessel_out_rad_    (664./2. * mm),

    // Flange dimensions
    flange_length_     (50./2. * mm),
    flange_in_rad_     (640./2. * mm),
    flange_out_rad_    (820./2. * mm),
    flange_z_pos_      (vessel_length_ + flange_length_),

    // Cover Flange dimensions
    cover_flange_length_ (50./2. * mm),
    cover_flange_in_rad_(560./2. * mm),
    cover_flange_out_rad_(820./2. * mm),
    cover_flange_z_pos_  (flange_z_pos_ + flange_length_ + cover_flange_length_),
    //cover_flange_z_pos_  (2000 * mm),


    // Gas properties
    gas_("naturalXe"),
    pressure_   (13.5 * bar),
    temperature_(293. * kelvin)
    
{

      /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/GanESS/", "Control commands of GanESS geometry.");
    new G4UnitDefinition("1/MeV","1/MeV", "1/Energy", 1/MeV);

    msg_->DeclareProperty("gas", gas_, "Gas being used");

    G4GenericMessenger::Command& pressure_cmd =
     msg_->DeclareProperty("pressure", pressure_, "Xenon pressure");
    pressure_cmd.SetUnitCategory("Pressure");
    pressure_cmd.SetParameterName("pressure", false);
    pressure_cmd.SetRange("pressure>0.");

    G4GenericMessenger::Command& temperature_cmd =
     msg_->DeclareProperty("temperature", temperature_, "Xenon temperature");
    temperature_cmd.SetUnitCategory("Temperature");
    temperature_cmd.SetParameterName("temperature", false);

}
  void GanESSVessel::Construct()
  {

    // Body solid
    G4Tubs* vessel_body_solid = new G4Tubs("VESSEL_BODY", 0., vessel_out_rad_, vessel_length_, 0.*deg, 360.*deg);

    // Flanges
    G4Tubs* vessel_flange1_solid = new G4Tubs("VESSEL_FLANGE1", flange_in_rad_, flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union1_ = new G4UnionSolid("BODY_FLANGE1", vessel_body_solid, vessel_flange1_solid, nullptr, G4ThreeVector(0., 0., flange_z_pos_));
    
    G4Tubs* vessel_flange2_solid = new G4Tubs("VESSEL_FLANGE2", flange_in_rad_, flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union2_ = new G4UnionSolid("BODY_FLANGE2", vessel_union1_, vessel_flange2_solid, nullptr, G4ThreeVector(0., 0., -flange_z_pos_));

    // Cover Flanges
    G4Tubs* cover_flange1_solid = new G4Tubs("COVER_FLANGE1", 0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union3_ = new G4UnionSolid("BODY_COVER_FLANGE1", vessel_union2_, cover_flange1_solid, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_));
       
    G4Tubs* cover_flange2_solid = new G4Tubs("COVER_FLANGE2", cover_flange_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);

    // Holes in the cover flange2
    const G4int n_trap = 6;

    G4double rotateX[] = {0., 60., 120., 180., 180.+60., 180.+120.};
    G4double rotateY[] = {-90., -30., 30., 90., 180.-30., 180.+30.};

    G4double coordX[] = {302.5, 151.25, -151.25, -302.5, -151.25, 151.25};
    G4double coordY[] = {0., 261.973, 261.973, 0., -261.973, -261.973};

    const G4double d = 48.87/2.;

    G4VSolid* cover_flange2_final = cover_flange2_solid;

    for (G4int i = 0; i < n_trap; i++) {
        G4String name = "TRAP_COVER_FLANGE" + std::to_string(i);
        G4Trap* trap = new G4Trap(name, 40.5/2.*mm, 0.*deg, 0.*deg, cover_flange_length_, 118.476/2.*mm, 118.476/2.*mm, 0.*deg, cover_flange_length_, 63.764/2.*mm, 63.764/2.*mm, 0.*deg);
        G4RotationMatrix* rotation = new G4RotationMatrix();
        rotation->rotateX(90.*deg);
        rotation->rotateY(rotateY[i]*deg);
        G4double x = coordX[i] - d*std::cos(rotateX[i]*deg);
        G4double y = coordY[i] - d*std::sin(rotateX[i]*deg);
        G4ThreeVector position(x*mm, y*mm, 0.);
        cover_flange2_final = new G4SubtractionSolid("COVER_FLANGE2_SUB_" + std::to_string(i), cover_flange2_final, trap, rotation, position);
    }

    G4UnionSolid* vessel_union4_ = new G4UnionSolid("BODY_COVER_FLANGE2", vessel_union3_, cover_flange2_final, nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_));

    // Logical volume
    G4LogicalVolume* vessel_logic = new G4LogicalVolume(vessel_union4_, materials::Steel316Ti(),"VESSEL");
    this->SetLogicalVolume(vessel_logic); // Mother volume
    
    // Gas volume inside vessel mother volume
    G4Tubs* vessel_gas_body_solid = new G4Tubs("VESSEL_GAS_BODY", 0., vessel_in_rad_, vessel_length_, 0.*deg, 360.*deg);

    G4Material* vessel_gas_mat = nullptr;
    if (gas_ == "naturalXe") {
      vessel_gas_mat = materials::GXe(pressure_, temperature_);
    }

    G4LogicalVolume* vessel_gas_logic = new G4LogicalVolume(vessel_gas_body_solid, vessel_gas_mat, "VESSEL_GAS");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), vessel_gas_logic, "VESSEL_GAS", vessel_logic, false, 0, true);
    
    /// Set the gas volume as an ionization sensitive detector
    IonizationSD* ionisd = new IonizationSD("/GanESS/ACTIVE");
    vessel_gas_logic->SetSensitiveDetector(ionisd);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionisd);

    // Vertex generator in Vessel
    body_gen_  = new CylinderPointSampler(vessel_in_rad_, vessel_out_rad_, vessel_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
  }

    GanESSVessel::~GanESSVessel()
  {
    delete body_gen_;
  }

G4ThreeVector GanESSVessel::GenerateVertex(const G4String& region) const
{
  if (region == "VESSEL") {
    return body_gen_->GenerateVertex(VOLUME);
  }

  G4Exception(
      "[GanESSVessel]",
      "GenerateVertex()",
      FatalException,
      "Unknown vertex generation region!");

  return G4ThreeVector();
}


