// ----------------------------------------------------------------------------
// nexus | GanESSPmtPlate.hh
//
// Geometry of the GanESS Pmt Plate.
//
// The GanESS group
// ----------------------------------------------------------------------------

#include "GanESSPmtPlate.hh"
#include "nexus/Next100Stave.h"
#include "nexus/MaterialsList.h"
#include "nexus/Visibilities.h"
#include "nexus/IonizationSD.h"
#include "nexus/OpticalMaterialProperties.h"
#include "nexus/UniformElectricDriftField.h"
#include "nexus/XenonProperties.h"
#include "nexus/CylinderPointSampler.h"
#include "nexus/ConePointSampler.hh"
#include "nexus/BoxPointSampler.h"
#include "nexus/HexagonMeshTools.h"

#include <G4Navigator.hh>
#include <G4SystemOfUnits.hh>
#include <G4PhysicalConstants.hh>
#include <G4GenericMessenger.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4Material.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Box.hh>
#include <G4ExtrudedSolid.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4NistManager.hh>
#include <G4UserLimits.hh>
#include <G4SDManager.hh>
#include <G4UnitsTable.hh>
#include <G4TransportationManager.hh>

#include <cassert>

using namespace nexus;


GanESSPmtPlate::GanESSPmtPlate():
  GeometryBase(),

  // Plate dimensions
  plate_out_rad_ (630./2.*mm),
  //plate_out_rad_ (615./2.*mm),
  plate_length_  (38./2.*mm), 
  plate_z_pos_    (442.*mm + plate_length_),

  // PMTs
  n_pmts_ (19),

  // Holes for pmts
  hole_pmt_out_rad_ (80./2.*mm),
  hole_pmt_length_ (plate_length_),

  // Steel pmt tubes
  steel_tube1_length_ (8./2.*mm),
  steel_tube1_in_rad_ (62./2.*mm),
  steel_tube1_out_rad_ (116./2.*mm),
  //steel_tube1_out_rad_ (110./2.*mm),
  steel_tube1_z_pos_ (plate_z_pos_ + plate_length_ + steel_tube1_length_),

  // Steel pmt tubes
  steel_tube2_length_ (114./2.*mm),
  steel_tube2_in_rad_ (steel_tube1_in_rad_),
  steel_tube2_out_rad_ (77.963/2.*mm),
  steel_tube2_z_pos_ (steel_tube1_z_pos_ + steel_tube1_length_ + steel_tube2_length_),

  // Steel cone
  steel_cone_length_ (5./2.*mm),
  steel_cone_out_rad_z_neg_ (steel_tube2_out_rad_),
  steel_cone_out_rad_z_pos_ (68./2.*mm),
  steel_cone_z_pos_ (steel_tube2_z_pos_ + steel_tube2_length_ + steel_cone_length_),

  // Steel inside tube
  steel_inside_tube_length_ (11./2.*mm),
  steel_inside_tube_out_rad_ (steel_tube2_in_rad_),
  steel_inside_tube_z_pos_ (steel_tube2_z_pos_ + steel_tube2_length_ - steel_inside_tube_length_),
  //steel_inside_tube_z_pos_ (0.),

  // Steel tube end
  steel_tube_end_length_ (10./2.*mm),
  steel_tube_end_out_rad_ (18./2.*mm),
  steel_tube_end_z_pos_ (steel_cone_z_pos_ + steel_cone_length_ + steel_tube_end_length_),

  //PMT window
  steel_window_tube1_length_ (10./2.*mm),
  steel_window_tube1_in_rad_ (hole_pmt_out_rad_), //80./2.*mm
  steel_window_tube1_out_rad_ (118./2.*mm),
  steel_window_tube1_z_pos_ (plate_z_pos_ - plate_length_ - steel_window_tube1_length_),

  steel_window_tube2_length_ (4.25/2.*mm),
  steel_window_tube2_in_rad_ (87.5/2.*mm),
  steel_window_tube2_out_rad_ (92./2.*mm),
  steel_window_tube2_z_pos_ (steel_window_tube1_z_pos_ - steel_window_tube1_length_ - steel_window_tube2_length_),

  steel_window_tube3_length_ (5.25/2.*mm),
  steel_window_tube3_in_rad_ (84./2.*mm),
  steel_window_tube3_out_rad_ (85.4/2.*mm),
  steel_window_tube3_z_pos_ (steel_window_tube2_z_pos_ - steel_window_tube2_length_ - steel_window_tube3_length_),

  //Inner volumes tube2
  steel_window_tube2_inner1_length_ (2./2.*mm),
  steel_window_tube2_inner1_in_rad_ (steel_window_tube1_in_rad_),
  steel_window_tube2_inner1_out_rad_ (steel_window_tube2_in_rad_),
  steel_window_tube2_inner1_z_pos_ (steel_window_tube1_z_pos_ - steel_window_tube1_length_ - steel_window_tube2_inner1_length_),

  steel_window_tube2_inner2_length_ (1./2.*mm),
  steel_window_tube2_inner2_in_rad_ (steel_window_tube3_in_rad_),
  steel_window_tube2_inner2_out_rad_ (steel_window_tube2_in_rad_),
  steel_window_tube2_inner2_z_pos_ (steel_window_tube3_z_pos_ + steel_window_tube3_length_ + steel_window_tube2_inner2_length_),

  // Sapphire pmt windows
  sapphire_window_length_ (5.7/2.*mm),
  sapphire_window_out_rad_ (steel_window_tube3_in_rad_),
  sapphire_window_z_pos_ (steel_window_tube3_z_pos_)

{

  /// Define new categories
  new G4UnitDefinition("kilovolt/cm","kV/cm","Electric field", kilovolt/cm);
  new G4UnitDefinition("mm/sqrt(cm)","mm/sqrt(cm)","Diffusion", mm/sqrt(cm));
  new G4UnitDefinition("mm/microsecond","mm/microsecond","drift velocity", mm/microsecond);

}


void GanESSPmtPlate::SetMotherLogicalVolume(G4LogicalVolume* mother_logic)
{
  mother_logic_ = mother_logic;
}


void GanESSPmtPlate::SetMotherPhysicalVolume(G4VPhysicalVolume* mother_phys)
{
  mother_phys_ = mother_phys;
}


void GanESSPmtPlate::DefineMaterials()
{
  /// Read gas properties from mother volume
  gas_         = mother_logic_->GetMaterial();
  pressure_    = gas_->GetPressure();
  temperature_ = gas_->GetTemperature();
  //sc_yield_    = gas_->GetMaterialPropertiesTable()->GetConstProperty("SCINTILLATIONYIELD");
  //e_lifetime_  = gas_->GetMaterialPropertiesTable()->GetConstProperty("ATTACHMENT");


  /// TPB coating
  tpb_ = materials::TPB();
  tpb_->SetMaterialPropertiesTable(opticalprops::TPB());

  /// Steel
  //steel_ = materials::Steel316Ti();
  // In Geant4 11.0.0, a bug in treating the OpBoundaryProcess produced in the
  // surface makes the code fail. This is avoided by setting
  // an empty G4MaterialPropertiesTable of the G4Material.
  //steel_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());


  // AISI-304L (https://www.metalcor.de/en/datenblatt/7/)
    G4double density = 7.9 * g/cm3;

    G4Element* Fe = G4NistManager::Instance()->FindOrBuildElement("Fe");
    G4Element* Cr = G4NistManager::Instance()->FindOrBuildElement("Cr");
    G4Element* Ni = G4NistManager::Instance()->FindOrBuildElement("Ni");
    G4Element* Mn = G4NistManager::Instance()->FindOrBuildElement("Mn");
    G4Element* Si = G4NistManager::Instance()->FindOrBuildElement("Si");
    G4Element* C  = G4NistManager::Instance()->FindOrBuildElement("C");
    G4Element* P  = G4NistManager::Instance()->FindOrBuildElement("P");
    G4Element* S  = G4NistManager::Instance()->FindOrBuildElement("S");

    G4Material* steel_ = new G4Material(
        "AISI304L", density, 8
    );

    steel_->AddElement(Fe, 0.6966);
    steel_->AddElement(Cr, 0.180);
    steel_->AddElement(Ni, 0.0925);
    steel_->AddElement(Mn, 0.020);
    steel_->AddElement(Si, 0.010);
    steel_->AddElement(C,  0.0003);
    steel_->AddElement(P,  0.00045);
    steel_->AddElement(S,  0.00015);

}

void GanESSPmtPlate::Construct()
{

  // AISI-304L (https://www.metalcor.de/en/datenblatt/7/)
    G4double density = 7.9 * g/cm3;

    G4Element* Fe = G4NistManager::Instance()->FindOrBuildElement("Fe");
    G4Element* Cr = G4NistManager::Instance()->FindOrBuildElement("Cr");
    G4Element* Ni = G4NistManager::Instance()->FindOrBuildElement("Ni");
    G4Element* Mn = G4NistManager::Instance()->FindOrBuildElement("Mn");
    G4Element* Si = G4NistManager::Instance()->FindOrBuildElement("Si");
    G4Element* C  = G4NistManager::Instance()->FindOrBuildElement("C");
    G4Element* P  = G4NistManager::Instance()->FindOrBuildElement("P");
    G4Element* S  = G4NistManager::Instance()->FindOrBuildElement("S");

    G4Material* steel_ = new G4Material(
        "AISI304L", density, 8
    );

    steel_->AddElement(Fe, 0.6966);
    steel_->AddElement(Cr, 0.180);
    steel_->AddElement(Ni, 0.0925);
    steel_->AddElement(Mn, 0.020);
    steel_->AddElement(Si, 0.010);
    steel_->AddElement(C,  0.0003);
    steel_->AddElement(P,  0.00045);
    steel_->AddElement(S,  0.00015);

  //PMT R11410 importation
  pmt_.SetSensorDepth(2);
  pmt_.Construct();
  G4LogicalVolume* logic_pmt = pmt_.GetLogicalVolume();

  G4double front_diam  = pmt_.FrontBodyDiameter();
  G4double front_len   = pmt_.FrontBodyLength();

  G4double rear_diam   = pmt_.RearBodyDiameter();
  G4double rear_len    = pmt_.RearBodyLength();

  G4double body_thick  = pmt_.BodyThickness();

  G4double win_diam    = pmt_.WindowDiameter();
  G4double win_thick   = pmt_.WindowThickness();

  G4double pc_diam     = pmt_.PhotocathodeDiameter();
  G4double pc_thick    = pmt_.PhotocathodeThickness();

  //PMT Length as to be calculated 

  G4double pmt_length_ = front_len + rear_len;

  // PMT Z pos (placed just behind the sapphire window)
  G4double pmt_z_pos_ (steel_window_tube2_inner1_z_pos_ + steel_window_tube2_inner1_length_ + front_len/2.);

  // Create the plate volume
  G4Tubs* plate_solid = new G4Tubs("PLATE", 0, plate_out_rad_, plate_length_, 0, 360*deg);
  
  G4double plate_gen_z_pos_ = plate_z_pos_;
    
  for (G4int i=0; i<2.; i++)
    {
      plate_gen_ = new CylinderPointSampler(0., plate_out_rad_, plate_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., plate_gen_z_pos_));
      plate_gen_vector_.push_back(plate_gen_);
      plate_gen_z_pos_ = -plate_gen_z_pos_;
    }

  G4VSolid* plate_solid_cp1 = plate_solid;

  // Create the holes for the PMTs
  G4Tubs* pmt_hole_solid = new G4Tubs("PMT_HOLE", 0, hole_pmt_out_rad_, hole_pmt_length_, 0, 360*deg);
  
  G4double X_pmt_hole[] = {0., 59.25, 118.5, 177.75, 237., 177.75, 118.5, 0., -118.5, -177.75, -237., -177.75, -118.5, 0., -59.25, -118.5, -59.25, 59.25, 118.5};
  G4double Y_pmt_hole[] = {0., -102.624, -205.248, -102.624, 0., 102.624, 205.248, 205.248, 205.248, 102.624, 0., -102.624, -205.248, -205.248, -102.624, 0., 102.624, 102.624, 0.};

  // Steel tubes for PMTs
  G4Tubs* steel_tube1_solid = new G4Tubs("STEEL_TUBE1", steel_tube1_in_rad_, steel_tube1_out_rad_, steel_tube1_length_, 0, 360*deg);
  G4LogicalVolume* steel_tube1_logic = new G4LogicalVolume(steel_tube1_solid, steel_, "STEEL_TUBE1");



  G4Tubs* steel_tube2_solid = new G4Tubs("STEEL_TUBE2", steel_tube2_in_rad_, steel_tube2_out_rad_, steel_tube2_length_, 0, 360*deg);
  G4LogicalVolume* steel_tube2_logic = new G4LogicalVolume(steel_tube2_solid, steel_, "STEEL_TUBE2");



  G4Cons* steel_cone_solid = new G4Cons("STEEL_CONE", 0., steel_cone_out_rad_z_neg_, 0., steel_cone_out_rad_z_pos_, steel_cone_length_, 0.*deg, 360.*deg);
  G4LogicalVolume* steel_cone_logic = new G4LogicalVolume(steel_cone_solid, steel_, "STEEL_CONE");

  G4Tubs* steel_inside_tube_solid = new G4Tubs("STEEL_INSIDE_TUBE", 0, steel_inside_tube_out_rad_, steel_inside_tube_length_, 0, 360*deg);
  G4LogicalVolume* steel_inside_tube_logic = new G4LogicalVolume(steel_inside_tube_solid, steel_, "STEEL_INSIDE_TUBE");

  G4Tubs* steel_tube_end_solid = new G4Tubs("STEEL_TUBE_END", 0, steel_tube_end_out_rad_, steel_tube_end_length_, 0, 360*deg);
  G4LogicalVolume* steel_tube_end_logic = new G4LogicalVolume(steel_tube_end_solid, steel_, "STEEL_TUBE_END");

  // Steel pmt window tubes
  G4Tubs* steel_window_tube1_solid = new G4Tubs("STEEL_WINDOW_TUBE1", steel_window_tube1_in_rad_, steel_window_tube1_out_rad_, steel_window_tube1_length_, 0, 360*deg);
  G4LogicalVolume* steel_window_tube1_logic = new G4LogicalVolume(steel_window_tube1_solid, steel_, "STEEL_WINDOW_TUBE1");

  G4Tubs* steel_window_tube2_solid = new G4Tubs("STEEL_WINDOW_TUBE2", steel_window_tube2_in_rad_, steel_window_tube2_out_rad_, steel_window_tube2_length_, 0, 360*deg);
  G4LogicalVolume* steel_window_tube2_logic = new G4LogicalVolume(steel_window_tube2_solid, steel_, "STEEL_WINDOW_TUBE2");

  G4Tubs* steel_window_tube3_solid = new G4Tubs("STEEL_WINDOW_TUBE3", steel_window_tube3_in_rad_, steel_window_tube3_out_rad_, steel_window_tube3_length_, 0, 360*deg);
  G4LogicalVolume* steel_window_tube3_logic = new G4LogicalVolume(steel_window_tube3_solid, steel_, "STEEL_WINDOW_TUBE3");
  
  G4Tubs* steel_window_tube2_inner1_solid = new G4Tubs("STEEL_WINDOW_TUBE2_INNER1", steel_window_tube2_inner1_in_rad_, steel_window_tube2_inner1_out_rad_, steel_window_tube2_inner1_length_, 0, 360*deg);
  G4LogicalVolume* steel_window_tube2_inner1_logic = new G4LogicalVolume(steel_window_tube2_inner1_solid, steel_, "STEEL_WINDOW_TUBE2_INNER1");
  
  G4Tubs* steel_window_tube2_inner2_solid = new G4Tubs("STEEL_WINDOW_TUBE2_INNER2", steel_window_tube2_inner2_in_rad_, steel_window_tube2_inner2_out_rad_, steel_window_tube2_inner2_length_, 0, 360*deg);
  G4LogicalVolume* steel_window_tube2_inner2_logic = new G4LogicalVolume(steel_window_tube2_inner2_solid, steel_, "STEEL_WINDOW_TUBE2_INNER2");
  
  // Sapphire pmt windows
  G4Tubs* sapphire_window_solid = new G4Tubs("SAPPHIRE_WINDOW", 0, sapphire_window_out_rad_, sapphire_window_length_, 0, 360*deg);
  G4LogicalVolume* sapphire_window_logic = new G4LogicalVolume(sapphire_window_solid, materials::Sapphire(), "SAPPHIRE_WINDOW");
  
  // Array of PMT elements to place them all at the same (x,y) positions and proper z positions
  G4String pmt_tubs_names[] = {"STEEL_TUBE1", "STEEL_TUBE2", "STEEL_CONE", "STEEL_INSIDE_TUBE", "STEEL_TUBE_END", "STEEL_WINDOW_TUBE1", "STEEL_WINDOW_TUBE2", "STEEL_WINDOW_TUBE3", "STEEL_WINDOW_TUBE2_INNER1", "STEEL_WINDOW_TUBE2_INNER2", "SAPPHIRE_WINDOW"};
  G4double pmt_tubs_z_pos[] = {steel_tube1_z_pos_, steel_tube2_z_pos_, steel_cone_z_pos_, steel_inside_tube_z_pos_, steel_tube_end_z_pos_, steel_window_tube1_z_pos_, steel_window_tube2_z_pos_, steel_window_tube3_z_pos_, steel_window_tube2_inner1_z_pos_, steel_window_tube2_inner2_z_pos_, sapphire_window_z_pos_};
  G4LogicalVolume* pmt_tubs_logical_volumes[] = {steel_tube1_logic, steel_tube2_logic, steel_cone_logic, steel_inside_tube_logic, steel_tube_end_logic, steel_window_tube1_logic, steel_window_tube2_logic, steel_window_tube3_logic, steel_window_tube2_inner1_logic, steel_window_tube2_inner2_logic, sapphire_window_logic};
  
  G4RotationMatrix* rotation = new G4RotationMatrix();
  rotation->rotateX(180.*deg);
  // Loop for placing PMT volumes related
  for (G4int i = 0; i < n_pmts_; i++) 
  {
    // Subatract the PMT holes from the plate
    plate_solid_cp1 = new G4SubtractionSolid("PLATE_SUB", plate_solid_cp1, pmt_hole_solid, nullptr, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], 0.));

    // Placement of the pmts
    new G4PVPlacement(rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_z_pos_), logic_pmt, "PMT", mother_logic_, false, i, true);
    new G4PVPlacement(nullptr, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_z_pos_), logic_pmt, "PMT", mother_logic_, false, n_pmts_ + i, true);

    // PMT Tubes (behind the pmt)
    // Loop on each pieces to place for 1 pmt
    for (G4int j = 0; j < sizeof(pmt_tubs_names)/sizeof(G4String); j++)
    {
      // vectors of generators for each piece
      if (j==0)
      {
        steel_tube1_gen_ = new CylinderPointSampler(steel_tube1_in_rad_, steel_tube1_out_rad_, steel_tube1_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_tube1_gen_vector_.push_back(steel_tube1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube1_gen_);
        steel_tube1_gen_ = new CylinderPointSampler(steel_tube1_in_rad_, steel_tube1_out_rad_, steel_tube1_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_tube1_gen_vector_.push_back(steel_tube1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube1_gen_);
      }

      if (j==1)
      {
        steel_tube2_gen_ = new CylinderPointSampler(steel_tube2_in_rad_, steel_tube2_out_rad_, steel_tube2_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_tube2_gen_vector_.push_back(steel_tube2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube2_gen_);

        steel_tube2_gen_ = new CylinderPointSampler(steel_tube2_in_rad_, steel_tube2_out_rad_, steel_tube2_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_tube2_gen_vector_.push_back(steel_tube2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube2_gen_);

      }

      if (j==2)
      {
        steel_cone_gen_  = new ConePointSampler(0., steel_cone_out_rad_z_neg_, 0., steel_cone_out_rad_z_pos_, steel_cone_length_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_cone_gen_vector_.push_back(steel_cone_gen_);
        pmt_capsul_cone_gen_vector_.push_back(steel_cone_gen_);

        steel_cone_gen_  = new ConePointSampler(0., steel_cone_out_rad_z_neg_, 0., steel_cone_out_rad_z_pos_, steel_cone_length_, 0.*deg, 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_cone_gen_vector_.push_back(steel_cone_gen_);
        pmt_capsul_cone_gen_vector_.push_back(steel_cone_gen_);

      }

      if (j==3)
      {
        steel_inside_tube_gen_ = new CylinderPointSampler(0., steel_inside_tube_out_rad_, steel_inside_tube_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_inside_tube_gen_vector_.push_back(steel_inside_tube_gen_);
        pmt_capsul_gen_vector_.push_back(steel_inside_tube_gen_);

        steel_inside_tube_gen_ = new CylinderPointSampler(0., steel_inside_tube_out_rad_, steel_inside_tube_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_inside_tube_gen_vector_.push_back(steel_inside_tube_gen_);
        pmt_capsul_gen_vector_.push_back(steel_inside_tube_gen_);
      }

      if (j==4)
      {
        steel_tube_end_gen_ = new CylinderPointSampler(0., steel_tube_end_out_rad_, steel_tube_end_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_tube_end_gen_vector_.push_back(steel_tube_end_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube_end_gen_);
        steel_tube_end_gen_ = new CylinderPointSampler(0., steel_tube_end_out_rad_, steel_tube_end_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_tube_end_gen_vector_.push_back(steel_tube_end_gen_);
        pmt_capsul_gen_vector_.push_back(steel_tube_end_gen_);
      }

      if (j==5)
      {
        steel_window_tube1_gen_ = new CylinderPointSampler(steel_window_tube1_in_rad_, steel_window_tube1_out_rad_, steel_window_tube1_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_window_tube1_gen_vector_.push_back(steel_window_tube1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube1_gen_);
        steel_window_tube1_gen_ = new CylinderPointSampler(steel_window_tube1_in_rad_, steel_window_tube1_out_rad_, steel_window_tube1_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_window_tube1_gen_vector_.push_back(steel_window_tube1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube1_gen_);
      }

      if (j==6)
      {
        steel_window_tube2_gen_ = new CylinderPointSampler(steel_window_tube2_in_rad_, steel_window_tube2_out_rad_, steel_window_tube2_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_window_tube2_gen_vector_.push_back(steel_window_tube2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_gen_);
        steel_window_tube2_gen_ = new CylinderPointSampler(steel_window_tube2_in_rad_, steel_window_tube2_out_rad_, steel_window_tube2_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_window_tube2_gen_vector_.push_back(steel_window_tube2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_gen_);
      }

      if (j==7)
      {
        steel_window_tube3_gen_ = new CylinderPointSampler(steel_window_tube3_in_rad_, steel_window_tube3_out_rad_, steel_window_tube3_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_window_tube3_gen_vector_.push_back(steel_window_tube3_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube3_gen_);
        steel_window_tube3_gen_ = new CylinderPointSampler(steel_window_tube3_in_rad_, steel_window_tube3_out_rad_, steel_window_tube3_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_window_tube3_gen_vector_.push_back(steel_window_tube3_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube3_gen_);
      }

      if (j==8)
      {
        steel_window_tube2_inner1_gen_ = new CylinderPointSampler(steel_window_tube2_inner1_in_rad_, steel_window_tube2_inner1_out_rad_, steel_window_tube2_inner1_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_window_tube2_inner1_gen_vector_.push_back(steel_window_tube2_inner1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_inner1_gen_);
        steel_window_tube2_inner1_gen_ = new CylinderPointSampler(steel_window_tube2_inner1_in_rad_, steel_window_tube2_inner1_out_rad_, steel_window_tube2_inner1_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_window_tube2_inner1_gen_vector_.push_back(steel_window_tube2_inner1_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_inner1_gen_);
      }

      if (j==9)
      {
        steel_window_tube2_inner2_gen_ = new CylinderPointSampler(steel_window_tube2_inner2_in_rad_, steel_window_tube2_inner2_out_rad_, steel_window_tube2_inner2_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //steel_window_tube2_inner2_gen_vector_.push_back(steel_window_tube2_inner2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_inner2_gen_);
        steel_window_tube2_inner2_gen_ = new CylinderPointSampler(steel_window_tube2_inner2_in_rad_, steel_window_tube2_inner2_out_rad_, steel_window_tube2_inner2_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //steel_window_tube2_inner2_gen_vector_.push_back(steel_window_tube2_inner2_gen_);
        pmt_capsul_gen_vector_.push_back(steel_window_tube2_inner2_gen_);
      }

      if (j==10)
      {
        sapphire_window_gen_ = new CylinderPointSampler(0., sapphire_window_out_rad_, sapphire_window_length_, 0., 360.*deg, 0, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]));
        //sapphire_window_gen_vector_.push_back(sapphire_window_gen_);
        pmt_capsul_gen_vector_.push_back(sapphire_window_gen_);
        sapphire_window_gen_ = new CylinderPointSampler(0., sapphire_window_out_rad_, sapphire_window_length_, 0., 360.*deg, rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], - pmt_tubs_z_pos[j]));
        //sapphire_window_gen_vector_.push_back(sapphire_window_gen_);
        pmt_capsul_gen_vector_.push_back(sapphire_window_gen_);
      }
      

      new G4PVPlacement(nullptr, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i], pmt_tubs_z_pos[j]), pmt_tubs_logical_volumes[j], pmt_tubs_names[j], mother_logic_, false, i, true);
      new G4PVPlacement(rotation, G4ThreeVector(X_pmt_hole[i], Y_pmt_hole[i],  - pmt_tubs_z_pos[j]), pmt_tubs_logical_volumes[j], pmt_tubs_names[j], mother_logic_, false, i, true);

      //std::cout<<"Placing "<<pmt_tubs_names[j]<<" for PMT "<<i<<" at position: ("<<X_pmt_hole[i]<<", "<<Y_pmt_hole[i]<<", "<<pmt_tubs_z_pos[j]<<")"<<std::endl;
    
    }

  }

  G4LogicalVolume* plate_logic = new G4LogicalVolume(plate_solid_cp1, steel_, "PLATE");
  
  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., plate_z_pos_), plate_logic, "PLATE", mother_logic_, false, 0, true);
  new G4PVPlacement(rotation, G4ThreeVector(0., 0., - plate_z_pos_), plate_logic, "PLATE", mother_logic_, false, 1, true);

}

 GanESSPmtPlate::~GanESSPmtPlate()
  {
    delete plate_gen_;
    delete steel_tube1_gen_;
    delete steel_tube2_gen_;
    delete steel_cone_gen_;
    delete steel_inside_tube_gen_;
    delete steel_tube_end_gen_;
    delete steel_window_tube1_gen_;
    delete steel_window_tube2_gen_;
    delete steel_window_tube3_gen_;
    delete steel_window_tube2_inner1_gen_;
    delete steel_window_tube2_inner2_gen_;
    delete sapphire_window_gen_;

  }

G4ThreeVector GanESSPmtPlate::GenerateVertex(const G4String& region) const
{

  if (region == "PLATE")
  {
    int index = (int)(G4UniformRand());
    //std::cout<<index<<std::endl;
    return pmt_capsul_gen_vector_[index]->GenerateVertex(VOLUME); 
  }

  if (region == "STEEL_TUBE1")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 2;
    G4int ineg = 3;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_TUBE2")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 4;
    G4int ineg = 5;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_INSIDE_TUBE")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 6;
    G4int ineg = 7;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_TUBE_END")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 8;
    G4int ineg = 9;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_WINDOW_TUBE1")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 9;
    G4int ineg = 10;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos +  20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_WINDOW_TUBE2")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 11;
    G4int ineg = 12;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_WINDOW_TUBE3")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 13;
    G4int ineg = 14;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_WINDOW_TUBE2_INNER1")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 15;
    G4int ineg = 16;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "STEEL_WINDOW_TUBE2_INNER2")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 17;
    G4int ineg = 18;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }

  if (region == "SAPPHIRE_WINDOW")
  {
    int parity = (int)(G4UniformRand());
    int index = (int)(G4UniformRand() * n_pmts_);
    G4int ipos = 19;
    G4int ineg = 20;

    if (parity==0)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
    if (parity==1)
    {
      return pmt_capsul_gen_vector_[ipos + 20*index]->GenerateVertex(VOLUME); 
    }
  }




  /*if (region == "PLATE" ||
    region == "STEEL_TUBE1" ||
    region == "STEEL_TUBE2" ||
    region == "STEEL_INSIDE_TUBE" ||
    region == "STEEL_TUBE_END" ||
    region == "STEEL_WINDOW_TUBE1" ||
    region == "STEEL_WINDOW_TUBE2" ||
    region == "STEEL_WINDOW_TUBE3" ||
    region == "STEEL_WINDOW_TUBE2_INNER1" ||
    region == "STEEL_WINDOW_TUBE2_INNER2" ||
    region == "SAPPHIRE_WINDOW")
  {
    int index = (int)(G4UniformRand() * pmt_capsul_gen_vector_.size());
    std::cout<<"index generator is : "<<index<<std::endl;
    std::cout<<"size array : "<< pmt_capsul_gen_vector_.size()<<std::endl;
    if (index >= 0 && index <= pmt_capsul_gen_vector_.size() + 1)
    {
      //std::cout<<"enter in the loop generator"<<std::endl;
      std::cout<<"pmt_capsul_gen_vector_["<<index<<"] is : "<<pmt_capsul_gen_vector_[index]->GenerateVertex(VOLUME)<<std::endl;
      return pmt_capsul_gen_vector_[index]->GenerateVertex(VOLUME);
    }
  }*/


  if (region == "STEEL_CONE")
  {
    int index = (int)(G4UniformRand() * pmt_capsul_cone_gen_vector_.size());
    //std::cout<<"index generator is : "<<index<<std::endl;
    //std::cout<<"size array : "<< pmt_capsul_cone_gen_vector_.size()<<std::endl;
  
    if (index >= 0 && index <= pmt_capsul_cone_gen_vector_.size() + 1)
    {
      //std::cout<<"enter in the loop generator"<<std::endl;
      //std::cout<<"pmt_capsul_cone_gen_vector_["<<index<<"] is : "<<pmt_capsul_cone_gen_vector_[index]->GenerateVertex(VOLUME)<<std::endl;
      return pmt_capsul_cone_gen_vector_[index]->GenerateVertex(VOLUME);
    }

  }
  
}