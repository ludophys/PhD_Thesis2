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
#include "nexus/ConePointSampler.hh"
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
#include <G4Ellipsoid.hh>
#include <G4Cons.hh>
#include <G4ExtrudedSolid.hh>
#include <G4TwoVector.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <Randomize.hh>
#include <G4TransportationManager.hh>
#include <G4UnitsTable.hh>
#include <G4SubtractionSolid.hh>
#include <G4SDManager.hh>

#include <CLHEP/Units/SystemOfUnits.h>

#include <math.h>
#include <vector>
#include <algorithm>

using namespace nexus;

  GanESSVessel::GanESSVessel():
    GeometryBase(),

    // General vessel dimensions
    vessel_length_     (870./2. * mm), // real length
    vessel_in_rad_     (640./2. * mm),
    vessel_out_rad_    (664./2. * mm),

    // Flange dimensions
    flange_length_     (50./2. * mm),
    flange_in_rad_     (vessel_out_rad_),
    flange_out_rad_    (820./2. * mm),
    flange_z_pos_      (vessel_length_ + flange_length_ - 20./2.*mm),

    // Inner tubs flanges
    flange_inner_length_     (40./2. * mm),
    flange_inner_in_rad_     (vessel_in_rad_),
    flange_inner_out_rad_    (flange_in_rad_),
    flange_inner_z_pos_      (flange_z_pos_ + (flange_length_ - flange_inner_length_)),


    // Cover Flange dimensions
    cover_flange_length_  (50./2. * mm),
    cover_flange1_in_rad_ (665.2/2. * mm),
    cover_flange2_in_rad_ (560./2. * mm),
    //cover_flange2_in_rad_ (563./2. * mm),
    cover_flange_out_rad_ (820./2. * mm),
    cover_flange_z_pos_  (flange_z_pos_ + flange_length_ + cover_flange_length_),
    //cover_flange_z_pos_  (2000*mm),

    cover_flange1_gen_in_rad_ (265./2.*mm),

    // Inside cover flange1 dimensions
    //Tubs
    cover_flange1_inner_in_rad_     (640./2. * mm),
    cover_flange1_inner_length_     (40./2. * mm),
    cover_flange1_inner_tubs_z_pos_ (cover_flange_z_pos_ - cover_flange_length_ + cover_flange1_inner_length_),

    //Box
    cover_flange1_inner_x_         (109.051/2.*mm),
    cover_flange1_inner_z_         (13./2.*mm),
    //cover_flange_z_pos_  (2000 * mm),

    // Inside cover flange 2
    cover_flange2_inner1_out_rad_     (640./2. * mm),
    cover_flange2_inner1_length_     (5./2. * mm),
    cover_flange2_inner1_tubs_z_pos_ (- cover_flange_z_pos_ + cover_flange_length_ - cover_flange2_inner1_length_),
   
    cover_flange2_inner2_out_rad_     (569.999/2. * mm),
    cover_flange2_inner2_length_     (5./2. * mm),
    cover_flange2_inner2_tubs_z_pos_ (cover_flange2_inner1_tubs_z_pos_ + cover_flange2_inner1_length_ + cover_flange2_inner2_length_),

    // Endcaps
    // Cylinder
    endcap_cyl_in_rad_  (640./2. * mm),
    endcap_cyl_out_rad_ (665.2/2.*mm),
    //endcap_cyl_length_  (35./2.*mm), // beofre overlaps with pmts
    endcap_cyl_length_  (96.284/2.*mm),

    endcap_cyl_z_pos_   (cover_flange_z_pos_ + cover_flange_length_ + endcap_cyl_length_),
    
    // Cone
    endcap_cone_in_rad_zneg_  (endcap_cyl_in_rad_),
    endcap_cone_out_rad_zneg_ (endcap_cyl_out_rad_),
    endcap_cone_out_rad_zpos_ (101.6/2.*mm),
    endcap_cone_depth_        (151.687/2.*mm - endcap_cyl_length_ + 35./2.*mm),
    endcap_cone_z_pos_        (endcap_cyl_z_pos_ + endcap_cyl_length_ + endcap_cone_depth_),

    // Gas properties
    gas_         ("naturalXe"),
    pressure_    (13.5 * bar),
    temperature_ (293. * kelvin)
    
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
    // Material definition
    //steel_ = materials::Steel316Ti();

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

    G4Material* vessel_gas_mat = nullptr;
    
    if (gas_ == "naturalXe") 
    {
      vessel_gas_mat = materials::GXe(pressure_, temperature_);
    }

    G4Material* air_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    // Mother volume
    G4Tubs* mother_volume = new G4Tubs("MOTHER_VOLUME", 0., vessel_out_rad_ + 100.*mm, vessel_length_ + 300.*mm, 0., 360.*deg);
    G4LogicalVolume* mother_logic_ = new G4LogicalVolume(mother_volume, air_, "MOTHER_VOLUME");

    //mother_logic_->SetVisAttributes(G4VisAttributes::GetInvisible());
    this->SetLogicalVolume(mother_logic_);

    // Body solid
    G4Tubs* vessel_body_solid = new G4Tubs("VESSEL_BODY", vessel_in_rad_, vessel_out_rad_, vessel_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* vessel_body_logic = new G4LogicalVolume(vessel_body_solid, steel_, "VESSEL_BODY");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), vessel_body_logic, "VESSEL_BODY", mother_logic_, false, 0., true);
    // Gas vessel body
    G4Tubs* vessel_gas_body_solid = new G4Tubs("VESSEL_GAS_BODY", 0., vessel_in_rad_, vessel_length_, 0.*deg, 360.*deg);

    // Flanges
    G4Tubs* vessel_flange1_solid = new G4Tubs("VESSEL_FLANGE1", flange_in_rad_, flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* vessel_flange1_logic = new G4LogicalVolume(vessel_flange1_solid, steel_, "VESSEL_FLANGE1");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., flange_z_pos_), vessel_flange1_logic, "VESSEL_FLANGE1", mother_logic_, false, 0, true);

    G4Tubs* vessel_flange2_solid = new G4Tubs("VESSEL_FLANGE2", flange_in_rad_, flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* vessel_flange2_logic = new G4LogicalVolume(vessel_flange2_solid, steel_, "VESSEL_FLANGE2");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., -flange_z_pos_), vessel_flange2_logic, "VESSEL_FLANGE2", mother_logic_, false, 0, true);
    
    // Inner tube flanges
    G4Tubs* vessel_flange_inner_solid = new G4Tubs("VESSEL_INNER_FLANGE", flange_inner_in_rad_, flange_inner_out_rad_, flange_inner_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* vessel_flange_inner_logic = new G4LogicalVolume(vessel_flange_inner_solid, steel_, "VESSEL_INNER_FLANGE");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., flange_inner_z_pos_), vessel_flange_inner_logic, "VESSEL_INNER_FLANGE1", mother_logic_, false, 0., true);
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., - flange_inner_z_pos_), vessel_flange_inner_logic, "VESSEL_INNER_FLANGE2", mother_logic_, false, 1., true);
    flange2_inner_gen_ = new CylinderPointSampler(flange_inner_in_rad_, flange_inner_out_rad_, flange_inner_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., - flange_inner_z_pos_));

    // Gas Flanges
    G4Tubs* vessel_gas_flange1_solid = new G4Tubs("GAS_FLANGE1", 0., flange_inner_in_rad_, flange_inner_length_, 0.*deg, 360.*deg);
    
    G4UnionSolid* gas_union1_ = new G4UnionSolid("BODY_GAS_FLANGE1", vessel_gas_body_solid, vessel_gas_flange1_solid, nullptr, G4ThreeVector(0., 0., flange_inner_z_pos_));
    
    G4Tubs* vessel_gas_flange2_solid = new G4Tubs("GAS_FLANGE2", 0., flange_inner_in_rad_, flange_inner_length_, 0.*deg, 360.*deg);

    G4UnionSolid* gas_union2_ = new G4UnionSolid("BODY_GAS_FLANGE2", gas_union1_, vessel_gas_flange2_solid, nullptr, G4ThreeVector(0., 0.,  - flange_inner_z_pos_));

    // Cover flange1
    // Exotic shape so it is placed after gas volume so gas volume can be put as a mother volume (avoid to use too much boolean operations)
    G4Tubs* cover_flange1_solid = new G4Tubs("COVER_FLANGE1", cover_flange1_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* cover_flange1_logic = new G4LogicalVolume(cover_flange1_solid, steel_, "COVER_FLANGE1");

    // Geometry inside cover flange1
    // Inner tuber cover flange1
    G4Tubs* tubs_cf1_inner_solid = new G4Tubs("TUBS_CF1_INNER", cover_flange1_inner_in_rad_, cover_flange1_in_rad_, cover_flange1_inner_length_, 0.*deg, 360.*deg);

    // Triangles inside cover flange1
    const G4int n_trgl = 4;

    G4double rotate_trglX[] = {45., 3.*45., 5.*45., 7.*45.};
    G4double rotate_trglZ[] = {-3.*45., 3.*45., -7.*45., 7.*45.};
    G4double coord_trglX[] = {208.596, -208.596, -208.596, 208.596};
    G4double coord_trglY[] = {208.596, 208.596, -208.596, -208.596};

    std::vector<G4TwoVector> polygon;
    polygon.emplace_back(0. * mm, 0. * mm);
    polygon.emplace_back(cover_flange1_inner_x_, -62.3 * mm);
    polygon.emplace_back(-cover_flange1_inner_x_, -62.3 * mm);

    G4VSolid* tubs_cf1_inner_solid_cp1 = tubs_cf1_inner_solid;

    const G4double d_trgl = 63.30/2.;

    // Loop to place triangles inside cover flange 1. It is unified with the full volume
    G4ExtrudedSolid* trgl_cf1_inner_solid = new G4ExtrudedSolid("TRGL_CF1_INNER", polygon, cover_flange1_inner_z_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);

    for (G4int i = 0; i < n_trgl; i++) 
    {
        
        // Rotation around Z axis to place the triangles
        G4RotationMatrix* rotation = new G4RotationMatrix();
        rotation->rotateZ(rotate_trglZ[i] * deg);
       
        // Calculation of new x and y coordinatates according to the rotational angles
        G4double x = coord_trglX[i] - d_trgl*std::cos(rotate_trglX[i]*deg);
        G4double y = coord_trglY[i] - d_trgl*std::sin(rotate_trglX[i]*deg);
        G4ThreeVector position(x * mm, y * mm, 0.);

        // Union of each triangle with the tube inside the cover flange 1
        tubs_cf1_inner_solid_cp1 = new G4UnionSolid("BODY_CF1_INNER_TRGL_" + std::to_string(i), tubs_cf1_inner_solid_cp1, trgl_cf1_inner_solid, rotation, position);
    
    }

    G4LogicalVolume* tubs_cf1_inner_logic = new G4LogicalVolume(tubs_cf1_inner_solid_cp1, steel_, "TUBS_CF1_INNER");
    
    // Gas cover flange1 (full cylinder with same outer radius than the entire cover flange1)
    // (The subtraction is made by defining this gas volume as a mother volume for the cover flange)
    G4Tubs* gas_cover_flange1_solid = new G4Tubs("GAS_COVER_FLANGE1", 0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union3_ = new G4UnionSolid("BODY_GAS_CF1", gas_union2_, gas_cover_flange1_solid, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_));

    // Cover flange 2
    G4Tubs* cover_flange2_solid = new G4Tubs("COVER_FLANGE2", cover_flange2_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4VSolid* cover_flange2_cp1 = cover_flange2_solid;

    // Inner tube subtraction
    G4Tubs* cover_flange2_inner1_solid = new G4Tubs("COVER_FLANGE_INNER1", 0., cover_flange2_inner1_out_rad_, cover_flange2_inner1_length_, 0.*deg, 360.*deg);
    G4Tubs* cover_flange2_inner2_solid = new G4Tubs("COVER_FLANGE_INNER2", 0., cover_flange2_inner2_out_rad_, cover_flange2_inner2_length_, 0.*deg, 360.*deg);

    cover_flange2_cp1 = new G4SubtractionSolid("SUB_COVER_FLANGE_INNER1", cover_flange2_cp1, cover_flange2_inner1_solid, nullptr, G4ThreeVector(0., 0., (cover_flange2_inner1_tubs_z_pos_ + cover_flange_z_pos_)));
    G4VSolid* cover_flange2_cp2 = cover_flange2_cp1;
    
    cover_flange2_cp2 = new G4SubtractionSolid("SUB_COVER_FLANGE_INNER2", cover_flange2_cp2, cover_flange2_inner2_solid, nullptr, G4ThreeVector(0., 0.,   cover_flange2_inner2_tubs_z_pos_ + cover_flange_z_pos_));
    G4VSolid* cover_flange2_final = cover_flange2_cp1;

    // Geometry of the traps inside cover flange2
    const G4int n_trap = 6;

    G4double rotateX[] = {0., 60., 120., 180., 180.+60., 180.+120.};
    G4double rotateY[] = {-90., -30., 30., 90., 180.-30., 180.+30.};

    G4double coordX[] = {302.5, 151.25, -151.25, -302.5, -151.25, 151.25};
    G4double coordY[] = {0., 261.973, 261.973, 0., -261.973, -261.973};

    //const G4double d = 5.*mm;
    const G4double d = 8.*mm;

    G4Trap* trap = new G4Trap("TRAP_COVER_FLANGE2", 40.5/2.*mm, 0.*deg, 0.*deg, cover_flange_length_ + 1*mm, 118.476/2.*mm, 118.476/2.*mm, 0.*deg, cover_flange_length_ + 1*mm, 63.764/2.*mm, 63.764/2.*mm, 0.*deg);

    // Loop to place trap volumes inside cover flange2
    for (G4int i = 0; i < n_trap; i++)
    {

        G4RotationMatrix* rotation = new G4RotationMatrix();
        rotation->rotateX(90.*deg);
        rotation->rotateY(rotateY[i]*deg);
        
        // Calculation of x and y coordinates to place the trap volumes
        G4double x = coordX[i] - d*std::cos(rotateX[i]*deg);
        G4double y = coordY[i] - d*std::sin(rotateX[i]*deg);
        G4ThreeVector position(x*mm, y*mm, 0.);

        cover_flange2_final = new G4SubtractionSolid("COVER_FLANGE2_SUB_" + std::to_string(i), cover_flange2_final, trap, rotation, position);
    }    

    // Placed in the as it is for cover flange1
    G4LogicalVolume* cover_flange2_logic = new G4LogicalVolume(cover_flange2_final, steel_, "COVER_FLANGE2");
    
    // Gas cover flange 2
    G4Tubs* gas_cover_flange2_solid = new G4Tubs("GAS_COVER_FLANGE2", 0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union4_ = new G4UnionSolid("BODY_GAS_CF2", gas_union3_, gas_cover_flange2_solid, nullptr, G4ThreeVector(0., 0., - cover_flange_z_pos_));

    // Endcaps
    // Cylinders
    G4Tubs* endcap_cyl_solid = new G4Tubs("CYL_ENDCAP", endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* endcap_cyl_logic = new G4LogicalVolume(endcap_cyl_solid, steel_, "CYL_ENDCAP");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., endcap_cyl_z_pos_), endcap_cyl_logic, "CYL_ENDCAP1", mother_logic_, false, 0., true);
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., - endcap_cyl_z_pos_), endcap_cyl_logic, "CYL_ENDCAP2", mother_logic_, false, 1., true);

    // Gas cylinders
    G4Tubs* gas_endcap_cyl_solid = new G4Tubs("GAS_CYL_ENDCAP", 0., endcap_cyl_in_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union5_ = new G4UnionSolid("BODY_GAS_ENDCAP_CYL1", gas_union4_, gas_endcap_cyl_solid, nullptr, G4ThreeVector(0., 0., endcap_cyl_z_pos_));
    G4UnionSolid* gas_union6_ = new G4UnionSolid("BODY_GAS_ENDCAP_CYL2", gas_union5_, gas_endcap_cyl_solid, nullptr, G4ThreeVector(0., 0., - endcap_cyl_z_pos_));

    // Endcap cones
    G4Cons* endcap_cone_solid = new G4Cons("CONE_ENDCAP", endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, 0., endcap_cone_out_rad_zpos_, endcap_cone_depth_, 0.*deg, 360.*deg);
    G4LogicalVolume* endcap_cone_logic = new G4LogicalVolume(endcap_cone_solid, steel_, "CONE_ENDCAP");
    new G4PVPlacement(nullptr,G4ThreeVector(0., 0., endcap_cone_z_pos_), endcap_cone_logic, "CONE_ENDCAP", mother_logic_, false, 0., true);
    
    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateX(180.*deg);
    
    new G4PVPlacement(rotation, G4ThreeVector(0.*mm, 0.*mm, -endcap_cone_z_pos_), endcap_cone_logic, "CONE_ENDCAP", mother_logic_, false, 1., true);

    // Gas endcap cones
    G4Cons* gas_endcap_cone_solid = new G4Cons("GAS_CONE_ENDCAP", 0., endcap_cone_in_rad_zneg_, 0., 0., endcap_cone_depth_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union7_ = new G4UnionSolid("BODY_GAS_ENDCAP_CONE", gas_union6_, gas_endcap_cone_solid, nullptr, G4ThreeVector(0., 0., endcap_cone_z_pos_));
    G4UnionSolid* gas_union8_ = new G4UnionSolid("BODY_GAS_ENDCAP_CONE", gas_union7_, gas_endcap_cone_solid, rotation, G4ThreeVector(0., 0., - endcap_cone_z_pos_));

    // After unifying all the gas volumes, it is defined as Logical volue that can be used as mother volume for the other parts
    G4LogicalVolume* vessel_gas_logic = new G4LogicalVolume(gas_union8_, vessel_gas_mat, "VESSEL_GAS");
    internal_phys_vol_ = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), vessel_gas_logic, "VESSEL_GAS", mother_logic_, false, 0, true);
    internal_logic_vol_ = vessel_gas_logic;

    /// Set the gas volume as an ionization sensitive detector
    IonizationSD* ionisd = new IonizationSD("/GanESS/ACTIVE");
    vessel_gas_logic->SetSensitiveDetector(ionisd);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionisd);

    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_), cover_flange1_logic, "COVER_FLANGE1", vessel_gas_logic, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., cover_flange1_inner_tubs_z_pos_), tubs_cf1_inner_logic, "TUBS_CF1_INNER", vessel_gas_logic, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_), cover_flange2_logic, "COVER_FLANGE2", vessel_gas_logic, false, 0., true);

    // Vertex generator in Vessel
    outside_gen_       = new CylinderPointSampler(0., vessel_out_rad_ + 200.*mm, vessel_length_ + 300.*mm, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
    body_gen_          = new CylinderPointSampler(vessel_in_rad_, vessel_out_rad_, vessel_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
    flange1_gen_       = new CylinderPointSampler(flange_in_rad_, flange_out_rad_, flange_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., flange_z_pos_));
    flange1_inner_gen_ = new CylinderPointSampler(flange_inner_in_rad_, flange_inner_out_rad_, flange_inner_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., flange_inner_z_pos_));
    flange2_inner_gen_ = new CylinderPointSampler(flange_inner_in_rad_, flange_inner_out_rad_, flange_inner_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., - flange_inner_z_pos_));
    flange2_gen_       = new CylinderPointSampler(flange_in_rad_, flange_out_rad_, flange_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., -flange_z_pos_));
    endcap_cyl1_gen_   = new CylinderPointSampler(endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., endcap_cyl_z_pos_));
    endcap_cyl2_gen_   = new CylinderPointSampler(endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., -endcap_cyl_z_pos_));
    endcap_cone1_gen_  = new ConePointSampler(endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, 0., endcap_cone_out_rad_zpos_, endcap_cone_depth_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., endcap_cone_z_pos_));
    endcap_cone2_gen_  = new ConePointSampler(endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, 0., endcap_cone_out_rad_zpos_, endcap_cone_depth_, 0.*deg, 360.*deg, rotation, G4ThreeVector(0., 0., - endcap_cone_z_pos_));
    cover_flange1_gen_ = new CylinderPointSampler(cover_flange1_gen_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_)); 
    cover_flange2_gen_ = new CylinderPointSampler(0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_));

}

    GanESSVessel::~GanESSVessel()
  {
    delete outside_gen_;
    delete body_gen_;
    delete flange1_gen_;
    delete flange2_gen_;
    delete flange1_inner_gen_;
    delete flange2_inner_gen_;
    delete endcap_cyl1_gen_;
    delete endcap_cyl2_gen_;
    delete endcap_cone1_gen_;
    delete endcap_cone2_gen_;
    delete cover_flange1_gen_;
    delete cover_flange2_gen_;
  }

    G4ThreeVector GanESSVessel::GenerateVertex(const G4String& region) const
    {
    // Generators for all the simple cylinders
    if (region == "OUTSIDE")
        {
            return outside_gen_->GenerateVertex(VOLUME);
        }
        

    if (region == "VESSEL_BODY") {
        return body_gen_->GenerateVertex(VOLUME);
    }
    if (region == "VESSEL_FLANGE1") {
        return flange1_gen_->GenerateVertex(VOLUME);
    }

    if (region == "VESSEL_FLANGE2") {
        return flange2_gen_->GenerateVertex(VOLUME);
    }

    if (region == "VESSEL_INNER_FLANGE1") {
        return flange1_inner_gen_->GenerateVertex(VOLUME);
    }

    if (region == "VESSEL_INNER_FLANGE2") {
        return flange2_inner_gen_->GenerateVertex(VOLUME);
    }

    if (region == "COVER_FLANGE1" || region == "TUBS_CF1_INNER") {
        return cover_flange1_gen_->GenerateVertex(VOLUME);
    }
    if (region == "COVER_FLANGE2") {
        return cover_flange2_gen_->GenerateVertex(VOLUME);
    }

    if (region == "CYL_ENDCAP1") {
        return endcap_cyl1_gen_->GenerateVertex(VOLUME);
    }
    if (region == "CYL_ENDCAP2") {
        return endcap_cyl2_gen_->GenerateVertex(VOLUME);
    }

    // Generator for cones
    if (region == "CONE_ENDCAP") {
        G4double rand = G4UniformRand()*(1);
        if (rand<0.5)
        {
            return endcap_cone1_gen_->GenerateVertex(VOLUME);
        }
        else
        {
            return endcap_cone2_gen_->GenerateVertex(VOLUME);
        }
    }
    /*if (region == "CONE_ENDCAP2") {
        return endcap_cone2_gen_->GenerateVertex(VOLUME);
    }*/

    /*G4double rand = G4UniformRand()*(endcap_cone_z_pos_);
 
        // For 1/2 of the points a minus is added to take in account the negative parts of the detector in the z-coordinate
        G4double parity = G4UniformRand();
        if (parity<=0.5)
        {
        rand = -rand;
        }*/

        /*std::cout<<"rand is : "<<rand<<std::endl;
        std::cout<<"interval1 :"<<-vessel_length_<<" ; "<<vessel_length_<<std::endl;
        std::cout<<"interval2 :"<<(flange_z_pos_ - flange_length_)<<" ; "<<(flange_z_pos_ + flange_length_)<<std::endl;
        std::cout<<"interval3 :"<<(-flange_z_pos_ - flange_length_)<<" ; "<<(-flange_z_pos_ + flange_length_)<<std::endl;
        std::cout<<"interval4 :"<<(cover_flange_z_pos_ - cover_flange_length_)<<" ; "<<(cover_flange_z_pos_ + cover_flange_length_)<<std::endl;
        std::cout<<"interval5 :"<<(-cover_flange_z_pos_ - cover_flange_length_)<<" ; "<<(-cover_flange_z_pos_ + cover_flange_length_)<<std::endl;
        std::cout<<"interval6 :"<<(endcap_cyl_z_pos_ - endcap_cyl_length_)<<" ; "<<(endcap_cyl_z_pos_ + endcap_cyl_length_)<<std::endl;
        std::cout<<"interval7 :"<<(-endcap_cyl_z_pos_ - endcap_cyl_length_)<<" ; "<<(-endcap_cyl_z_pos_ + endcap_cyl_length_)<<std::endl;
        std::cout<<"interval8 :"<<(endcap_cone_z_pos_ - endcap_cone_depth_)<<" ; "<<(endcap_cone_z_pos_ + endcap_cone_depth_)<<std::endl;
        std::cout<<"interval9 :"<<(-endcap_cone_z_pos_ - endcap_cone_depth_)<<" ; "<<(-endcap_cone_z_pos_ + endcap_cone_depth_)<<std::endl;
*/
        /*if (rand >= -vessel_length_ && rand <= vessel_length_)
        {
            //std::cout<<"generator is : body_gen_"<<std::endl;
            return body_gen_->GenerateVertex(VOLUME);
        }
        if (rand >= (flange_z_pos_ - flange_length_) &&
            rand <= (flange_z_pos_ + flange_length_))
        {   //std::cout<<"generator is : flange1_gen_"<<std::endl;
            return flange1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-flange_z_pos_ - flange_length_) &&
            rand <= (-flange_z_pos_ + flange_length_))
        {
            //std::cout<<"generator is : flange2_gen_"<<std::endl;
            return flange2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (cover_flange_z_pos_ - cover_flange_length_) &&
            rand <= (cover_flange_z_pos_ + cover_flange_length_))
        {
            //std::cout<<"generator is : cover_flange1_gen_"<<std::endl;
            return cover_flange1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-cover_flange_z_pos_ - cover_flange_length_) &&
            rand <= (-cover_flange_z_pos_ + cover_flange_length_))
        {
            //std::cout<<"generator is : cover_flange2_gen_"<<std::endl;
            return cover_flange2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (endcap_cyl_z_pos_ - endcap_cyl_length_) &&
            rand <= (endcap_cyl_z_pos_ + endcap_cyl_length_))
        {
            //std::cout<<"generator is : endcap_cyl1_gen_"<<std::endl;
            return endcap_cyl1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-endcap_cyl_z_pos_ - endcap_cyl_length_) &&
            rand <= (-endcap_cyl_z_pos_ + endcap_cyl_length_))
        {
            //std::cout<<"generator is : endcap_cyl2_gen_"<<std::endl;
            return endcap_cyl2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (endcap_cone_z_pos_ - endcap_cone_depth_) &&
            rand <= (endcap_cone_z_pos_ + endcap_cone_depth_))
        {
            //std::cout<<"generator is : endcap_cone1_gen_"<<std::endl;
            return endcap_cone1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-endcap_cone_z_pos_ - endcap_cone_depth_) &&
            rand <= (-endcap_cone_z_pos_ + endcap_cone_depth_))
        {
            //std::cout<<"generator is : endcap_cone2_gen_"<<std::endl;
            return endcap_cone2_gen_->GenerateVertex(VOLUME);
        }
    }*/

    


    

    G4Exception(
        "[GanESSVessel]",
        "GenerateVertex()",
        FatalException,
        "Unknown vertex generation region!");

    return G4ThreeVector();
    }

G4LogicalVolume* GanESSVessel::GetInternalLogicalVolume()
  {
    return internal_logic_vol_;
  }

G4VPhysicalVolume* GanESSVessel::GetInternalPhysicalVolume()
    {
        return internal_phys_vol_;
    }





