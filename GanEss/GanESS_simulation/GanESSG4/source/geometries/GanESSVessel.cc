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
    vessel_in_rad_    (68.0  * cm),
    vessel_thickness_ (1.  * cm),

    // Body
    body_length_ (198.6 * cm),

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
    G4double vessel_out_rad = vessel_in_rad_ + vessel_thickness_;
    G4Tubs* vessel_body_solid =
      new G4Tubs("VESSEL_BODY", 0., vessel_out_rad, body_length_/2., 0.*deg, 360.*deg);

    G4LogicalVolume* vessel_logic = new G4LogicalVolume(vessel_body_solid, materials::Steel316Ti(), "VESSEL");
    this->SetLogicalVolume(vessel_logic); // Mother volume

    // Gas volume
    G4Tubs* vessel_gas_body_solid = new G4Tubs("VESSEL_GAS_BODY", 0., vessel_in_rad_, body_length_/2., 0.*deg, 360.*deg);

    G4Material* vessel_gas_mat = nullptr;
    if (gas_ == "naturalXe") {
      vessel_gas_mat = materials::GXe(pressure_, temperature_);
    }

    G4LogicalVolume* vessel_gas_logic = new G4LogicalVolume(vessel_gas_body_solid, vessel_gas_mat, "VESSEL_GAS");
    
    /// Set the gas volume as an ionization sensitive detector
    IonizationSD* ionisd = new IonizationSD("/GanESS/ACTIVE");
    vessel_gas_logic->SetSensitiveDetector(ionisd);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionisd);

    // Placed in the vessel mother volume
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), vessel_gas_logic, "VESSEL_GAS", vessel_logic, false, 0, false);

    // Vertex generator in Vessel
    body_gen_  = new CylinderPointSampler(vessel_in_rad_, vessel_out_rad, body_length_/2.,0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
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


