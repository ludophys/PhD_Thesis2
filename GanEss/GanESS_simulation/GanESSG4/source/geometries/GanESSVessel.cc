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
    vessel_length_     (850./2. * mm), // real length
    vessel_in_rad_     (640./2. * mm),
    vessel_out_rad_    (664./2. * mm),

    // Flange dimensions
    flange_length_     (50./2. * mm),
    flange_in_rad_     (640./2. * mm),
    flange_out_rad_    (820./2. * mm),
    flange_z_pos_      (vessel_length_ + flange_length_),

    // Cover Flange dimensions
    cover_flange_length_  (50./2. * mm),
    cover_flange1_in_rad_ (665.2/2. * mm),
    cover_flange2_in_rad_ (560./2. * mm),
    cover_flange_out_rad_ (820./2. * mm),
    cover_flange_z_pos_  (flange_z_pos_ + flange_length_ + cover_flange_length_),
    //cover_flange_z_pos_  (2000*mm),

    cover_flange1_gen_in_rad_ (265./2.*mm),

    // Inside cover flange1 dimensions
    //Tubs
    cover_flange1_inner_in_rad_     (640./2. * mm),
    cover_flange1_inner_length_     (33.8/2. * mm),
    cover_flange1_inner_tubs_z_pos_ (cover_flange_z_pos_ - cover_flange_length_ + cover_flange1_inner_length_),

    //Box
    cover_flange1_inner_x_         (109.051/2.*mm),
    cover_flange1_inner_y_         (7./2.*mm),
    cover_flange1_inner_z_         (13./2.*mm),
    cover_flange1_inner_box_z_pos_ (cover_flange1_inner_tubs_z_pos_),
    //cover_flange_z_pos_  (2000 * mm),

    // Endcaps
    // Cylinder
    endcap_cyl_in_rad_  (640./2. * mm),
    endcap_cyl_out_rad_ (665.2/2.*mm),
    endcap_cyl_length_  (35./2.*mm),
    endcap_cyl_z_pos_   (cover_flange_z_pos_ + cover_flange_length_ + endcap_cyl_length_),
    
    // Cone
    endcap_cone_in_rad_zneg_  (endcap_cyl_in_rad_),
    endcap_cone_out_rad_zneg_ (endcap_cyl_out_rad_),
    endcap_cone_out_rad_zpos_ (101.6/2.*mm),
    endcap_cone_depth_        (151.687/2.*mm),
    endcap_cone_z_pos_        (endcap_cyl_z_pos_ + endcap_cyl_length_ + endcap_cone_depth_),
    gas_endcap_cone_depth_    (138.97/2.*mm),
    gas_endcap_cone_z_pos_    (endcap_cyl_z_pos_ + endcap_cyl_length_ + gas_endcap_cone_depth_),
    
    // Detector limits
    detector_z_lim_ (gas_endcap_cone_z_pos_ + endcap_cone_depth_),

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

    // Body solid
    G4Tubs* vessel_body_solid = new G4Tubs("VESSEL_BODY", 0., vessel_out_rad_, vessel_length_, 0.*deg, 360.*deg);

    // Gas vessel body
    G4Tubs* vessel_gas_body_solid = new G4Tubs("VESSEL_GAS_BODY", 0., vessel_in_rad_, vessel_length_, 0.*deg, 360.*deg);
    
    // Flanges
    G4Tubs* vessel_flange1_solid = new G4Tubs("VESSEL_FLANGE1", 0., flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union1_ = new G4UnionSolid("BODY_FLANGE1", vessel_body_solid, vessel_flange1_solid, nullptr, G4ThreeVector(0., 0., flange_z_pos_));
    
    G4Tubs* vessel_flange2_solid = new G4Tubs("VESSEL_FLANGE2", 0., flange_out_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union2_ = new G4UnionSolid("BODY_FLANGE2", vessel_union1_, vessel_flange2_solid, nullptr, G4ThreeVector(0., 0., -flange_z_pos_));

    // Gas Flanges
    G4Tubs* vessel_gas_flange1_solid = new G4Tubs("GAS_FLANGE1", 0., flange_in_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union1_ = new G4UnionSolid("BODY_GAS_FLANGE1", vessel_gas_body_solid, vessel_gas_flange1_solid, nullptr, G4ThreeVector(0., 0., flange_z_pos_));
    
    G4Tubs* vessel_gas_flange2_solid = new G4Tubs("GAS_FLANGE2", 0., flange_in_rad_, flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union2_ = new G4UnionSolid("BODY_GAS_FLANGE2", gas_union1_, vessel_gas_flange2_solid, nullptr, G4ThreeVector(0., 0., -flange_z_pos_));
    
    // Cover flange1
    G4Tubs* cover_flange1_solid = new G4Tubs("COVER_FLANGE1", 0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union3_ = new G4UnionSolid("BODY_COVER_FLANGE1", vessel_union2_, cover_flange1_solid, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_));
    
    // Gas cover flange1
    G4Tubs* gas_cover_flange1_solid = new G4Tubs("GAS_OVER_FLANGE1", 0., cover_flange1_in_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union3_ = new G4UnionSolid("BODY_GAS_COVER_FLANGE1", gas_union2_, gas_cover_flange1_solid, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_));

    // Geometry inside cover flange1
    // Inner tuber cover flange1
    G4Tubs* tubs_cf1_inner_solid = new G4Tubs("TUBS_CF1_INNER", cover_flange1_inner_in_rad_, cover_flange1_in_rad_, cover_flange1_inner_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union4_ = new G4UnionSolid("BODY_CF1_INNER_TUBS", vessel_union3_, tubs_cf1_inner_solid, nullptr, G4ThreeVector(0., 0., cover_flange1_inner_tubs_z_pos_));

    // Subtraction of inner tubes cover flange1 to sensitive gas volume
    G4VSolid* gas_union4_ = gas_union3_;
    gas_union4_ = new G4SubtractionSolid("SUB_INNER_TUBS_CF1", gas_union3_, tubs_cf1_inner_solid, nullptr, G4ThreeVector(0., 0., cover_flange1_inner_tubs_z_pos_));

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

    G4VSolid* vessel_union5_ = vessel_union4_;
    G4VSolid* gas_union5_ = gas_union4_;
    const G4double d_trgl = 62.30/2.;

    for (G4int i = 0; i < n_trgl; i++) {
        G4String name = "TRGL_" + std::to_string(i) + "_CF1_INNER";
        G4ExtrudedSolid* trgl_cf1_inner_solid = new G4ExtrudedSolid(name, polygon, cover_flange1_inner_z_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);
        G4RotationMatrix* rotation = new G4RotationMatrix();
        rotation->rotateZ(rotate_trglZ[i] * deg);
        G4double x = coord_trglX[i] - d_trgl*std::cos(rotate_trglX[i]*deg);
        G4double y = coord_trglY[i] - d_trgl*std::sin(rotate_trglX[i]*deg);
        G4ThreeVector position(x * mm, y * mm, cover_flange1_inner_box_z_pos_);
        gas_union5_ = new G4SubtractionSolid("SUB_INNER_TRGL_CF1", gas_union5_, trgl_cf1_inner_solid, rotation, position);
        vessel_union5_ = new G4UnionSolid("BODY_CF1_INNER_TRGL_" + std::to_string(i), vessel_union5_, trgl_cf1_inner_solid, rotation, position);
    }

    // Cover flange 2
    G4Tubs* cover_flange2_solid = new G4Tubs("COVER_FLANGE2", 0., cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg);

    // Gas cover flange 2
    G4Tubs* gas_cover_flange2_solid = new G4Tubs("GAS_COVER_FLANGE2", 0., cover_flange2_in_rad_, cover_flange_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union6_ = new G4UnionSolid("BODY_GAS_COVER_FLANGE2", gas_union5_, gas_cover_flange2_solid, nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_));

    // Geometry inside cover flange2
    const G4int n_trap = 6;

    G4double rotateX[] = {0., 60., 120., 180., 180.+60., 180.+120.};
    G4double rotateY[] = {-90., -30., 30., 90., 180.-30., 180.+30.};

    G4double coordX[] = {302.5, 151.25, -151.25, -302.5, -151.25, 151.25};
    G4double coordY[] = {0., 261.973, 261.973, 0., -261.973, -261.973};

    const G4double d = 48.87/2.;

    G4VSolid* cover_flange2_final = cover_flange2_solid;
    G4VSolid* gas_union7_ = gas_union6_;

    for (G4int i = 0; i < n_trap; i++) {
        G4String name = "TRAP_COVER_FLANGE" + std::to_string(i);
        G4Trap* trap = new G4Trap(name, 40.5/2.*mm, 0.*deg, 0.*deg, cover_flange_length_, 118.476/2.*mm, 118.476/2.*mm, 0.*deg, cover_flange_length_, 63.764/2.*mm, 63.764/2.*mm, 0.*deg);
        G4RotationMatrix* rotation = new G4RotationMatrix();
        rotation->rotateX(90.*deg);
        rotation->rotateY(rotateY[i]*deg);
        G4double x = coordX[i] - d*std::cos(rotateX[i]*deg);
        G4double y = coordY[i] - d*std::sin(rotateX[i]*deg);
        G4ThreeVector position(x*mm, y*mm, 0.);
        G4ThreeVector position_gas(x*mm, y*mm, -cover_flange_z_pos_);
        gas_union7_ = new G4UnionSolid("BODY_GAS_CF2_INNER_TRAP_" + std::to_string(i), gas_union7_, trap, rotation, position_gas);
        cover_flange2_final = new G4SubtractionSolid("COVER_FLANGE2_SUB_" + std::to_string(i), cover_flange2_final, trap, rotation, position);
    }
    
    G4UnionSolid* vessel_union6_ = new G4UnionSolid("BODY_COVER_FLANGE2", vessel_union5_, cover_flange2_final, nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_));

    // Endcaps
    // Cylinders
    G4Tubs* endcap_cyl1_solid = new G4Tubs("CYL_ENDCAP1", 0., endcap_cyl_out_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union7_ = new G4UnionSolid("BODY_CYL_ENDCAP1", vessel_union6_, endcap_cyl1_solid, nullptr, G4ThreeVector(0., 0., endcap_cyl_z_pos_));

    G4Tubs* endcap_cyl2_solid = new G4Tubs("CYL_ENDCAP1", 0., endcap_cyl_out_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union8_ = new G4UnionSolid("BODY_CYL_ENDCAP1", vessel_union7_, endcap_cyl2_solid, nullptr, G4ThreeVector(0., 0., -endcap_cyl_z_pos_));
    
    // Gas cylinders
    G4Tubs* gas_endcap_cyl1_solid = new G4Tubs("GAS_CYL_ENDCAP1", 0., endcap_cyl_in_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union8_ = new G4UnionSolid("BODY_GAS_CYL_ENDCAP1", gas_union7_, gas_endcap_cyl1_solid, nullptr, G4ThreeVector(0., 0., endcap_cyl_z_pos_));

    G4Tubs* gas_endcap_cyl2_solid = new G4Tubs("GAS_CYL_ENDCAP2", 0., endcap_cyl_in_rad_, endcap_cyl_length_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union9_ = new G4UnionSolid("BODY_GAS_CYL_ENDCAP2", gas_union8_, gas_endcap_cyl2_solid, nullptr, G4ThreeVector(0., 0., -endcap_cyl_z_pos_));

    // Endcap cones
    G4Cons* endcap_cone1_solid = new G4Cons("CONE_ENDCAP1", 0., endcap_cone_out_rad_zneg_, 0., endcap_cone_out_rad_zpos_, endcap_cone_depth_, 0.*deg, 360.*deg);
    G4UnionSolid* vessel_union9_ = new G4UnionSolid("BODY_CONE1_ENDCAP", vessel_union8_, endcap_cone1_solid, nullptr, G4ThreeVector(0.*mm, 0.*mm, endcap_cone_z_pos_));
    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateX(180.*deg);
    G4VSolid* endcap_cone2_solid = endcap_cone1_solid;
    G4UnionSolid* vessel_union10_ = new G4UnionSolid("BODY_CONE2_ENDCAP", vessel_union9_, endcap_cone2_solid, rotation, G4ThreeVector(0.*mm, 0.*mm, -endcap_cone_z_pos_));
    
    // Gas endcap cones
    G4Cons* gas_endcap_cone1_solid = new G4Cons("GAS_CONE_ENDCAP1", 0., endcap_cone_in_rad_zneg_, 0., endcap_cone_out_rad_zpos_, gas_endcap_cone_depth_, 0.*deg, 360.*deg);
    G4UnionSolid* gas_union10_ = new G4UnionSolid("BODY_GAS_CONE1_ENDCAP", gas_union9_, gas_endcap_cone1_solid, nullptr, G4ThreeVector(0.*mm, 0.*mm, gas_endcap_cone_z_pos_));
    G4VSolid* gas_endcap_cone2_solid = gas_endcap_cone1_solid;
    G4UnionSolid* gas_union11_ = new G4UnionSolid("BODY_CONE2_ENDCAP", gas_union10_, gas_endcap_cone2_solid, rotation, G4ThreeVector(0.*mm, 0.*mm, -gas_endcap_cone_z_pos_));

    // Logical volume
    G4LogicalVolume* vessel_logic = new G4LogicalVolume(vessel_union10_, materials::Steel316Ti(),"VESSEL");
    this->SetLogicalVolume(vessel_logic); // Mother volume
    
    // Gas properties
    G4Material* vessel_gas_mat = nullptr;
    if (gas_ == "naturalXe") {
      vessel_gas_mat = materials::GXe(pressure_, temperature_);
    }

    G4LogicalVolume* vessel_gas_logic = new G4LogicalVolume(gas_union11_, vessel_gas_mat, "VESSEL_GAS");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), vessel_gas_logic, "VESSEL_GAS", vessel_logic, false, 0, true);
    
    /// Set the gas volume as an ionization sensitive detector
    IonizationSD* ionisd = new IonizationSD("/GanESS/ACTIVE");
    vessel_gas_logic->SetSensitiveDetector(ionisd);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionisd);

    // Vertex generator in Vessel
    body_gen_  = new CylinderPointSampler(vessel_in_rad_, vessel_out_rad_, vessel_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
    flange1_gen_  = new CylinderPointSampler(flange_in_rad_, flange_out_rad_, flange_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., flange_z_pos_));
    flange2_gen_  = new CylinderPointSampler(flange_in_rad_, flange_out_rad_, flange_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., -flange_z_pos_));
    endcap_cyl1_gen_ = new CylinderPointSampler(endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., endcap_cyl_z_pos_));
    endcap_cyl2_gen_ = new CylinderPointSampler(endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., -endcap_cyl_z_pos_));
    endcap_cone1_gen_ = new ConePointSampler(endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, 0., endcap_cone_out_rad_zpos_, endcap_cone_depth_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., endcap_cone_z_pos_));
    endcap_cone2_gen_ = new ConePointSampler(0., endcap_cone_out_rad_zpos_, endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, endcap_cone_depth_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., -endcap_cone_z_pos_));
    cover_flange1_gen_ = new CylinderPointSampler(cover_flange1_gen_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., cover_flange_z_pos_)); 
    cover_flange2_gen_ = new CylinderPointSampler(cover_flange2_in_rad_, cover_flange_out_rad_, cover_flange_length_, 0.*deg, 360.*deg, nullptr, G4ThreeVector(0., 0., -cover_flange_z_pos_));

}

    GanESSVessel::~GanESSVessel()
  {
    delete body_gen_;
    delete flange1_gen_;
    delete flange2_gen_;
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
    if (region == "VESSEL") {
        G4double rand = G4UniformRand()*(endcap_cone_z_pos_ + endcap_cyl_length_);
        //G4double rand = G4UniformRand()*100*mm;
        std::cout<<"edge is : "<<(endcap_cone_z_pos_ + endcap_cyl_length_)<<std::endl;
        std::cout<<"edge2 is : "<<-endcap_cone_z_pos_ - endcap_cyl_length_<<std::endl;
        // For 1/2 of the points a minus is added to take in account the negative parts of the detector in the z-coordinate
        G4double parity = G4UniformRand();
        if (parity<=0.5)
        {
        rand = -rand;
        }

        std::cout<<"rand is : "<<rand<<std::endl;
        std::cout<<"interval1 :"<<-vessel_length_<<" ; "<<vessel_length_<<std::endl;
        std::cout<<"interval2 :"<<(flange_z_pos_ - flange_length_)<<" ; "<<(flange_z_pos_ + flange_length_)<<std::endl;
        std::cout<<"interval3 :"<<(-flange_z_pos_ - flange_length_)<<" ; "<<(-flange_z_pos_ + flange_length_)<<std::endl;
        std::cout<<"interval4 :"<<(cover_flange_z_pos_ - cover_flange_length_)<<" ; "<<(cover_flange_z_pos_ + cover_flange_length_)<<std::endl;
        std::cout<<"interval5 :"<<(-cover_flange_z_pos_ - cover_flange_length_)<<" ; "<<(-cover_flange_z_pos_ + cover_flange_length_)<<std::endl;
        std::cout<<"interval6 :"<<(endcap_cyl_z_pos_ - endcap_cyl_length_)<<" ; "<<(endcap_cyl_z_pos_ + endcap_cyl_length_)<<std::endl;
        std::cout<<"interval7 :"<<(-endcap_cyl_z_pos_ - endcap_cyl_length_)<<" ; "<<(-endcap_cyl_z_pos_ + endcap_cyl_length_)<<std::endl;
        std::cout<<"interval8 :"<<(endcap_cone_z_pos_ - endcap_cone_depth_)<<" ; "<<(endcap_cone_z_pos_ + endcap_cone_depth_)<<std::endl;
        std::cout<<"interval9 :"<<(-endcap_cone_z_pos_ - endcap_cone_depth_)<<" ; "<<(-endcap_cone_z_pos_ + endcap_cone_depth_)<<std::endl;

        if (rand >= -vessel_length_ && rand <= vessel_length_)
        {
            std::cout<<"generator is : body_gen_"<<std::endl;
            return body_gen_->GenerateVertex(VOLUME);
        }
        if (rand >= (flange_z_pos_ - flange_length_) &&
            rand <= (flange_z_pos_ + flange_length_))
        {   std::cout<<"generator is : flange1_gen_"<<std::endl;
            return flange1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-flange_z_pos_ - flange_length_) &&
            rand <= (-flange_z_pos_ + flange_length_))
        {
            std::cout<<"generator is : flange2_gen_"<<std::endl;
            return flange2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (cover_flange_z_pos_ - cover_flange_length_) &&
            rand <= (cover_flange_z_pos_ + cover_flange_length_))
        {
            std::cout<<"generator is : cover_flange1_gen_"<<std::endl;
            return cover_flange1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-cover_flange_z_pos_ - cover_flange_length_) &&
            rand <= (-cover_flange_z_pos_ + cover_flange_length_))
        {
            std::cout<<"generator is : cover_flange2_gen_"<<std::endl;
            return cover_flange2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (endcap_cyl_z_pos_ - endcap_cyl_length_) &&
            rand <= (endcap_cyl_z_pos_ + endcap_cyl_length_))
        {
            std::cout<<"generator is : endcap_cyl1_gen_"<<std::endl;
            return endcap_cyl1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-endcap_cyl_z_pos_ - endcap_cyl_length_) &&
            rand <= (-endcap_cyl_z_pos_ + endcap_cyl_length_))
        {
            std::cout<<"generator is : endcap_cyl2_gen_"<<std::endl;
            return endcap_cyl2_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (endcap_cone_z_pos_ - endcap_cone_depth_) &&
            rand <= (endcap_cone_z_pos_ + endcap_cone_depth_))
        {
            std::cout<<"generator is : endcap_cone1_gen_"<<std::endl;
            return endcap_cone1_gen_->GenerateVertex(VOLUME);
        }

        if (rand >= (-endcap_cone_z_pos_ - endcap_cone_depth_) &&
            rand <= (-endcap_cone_z_pos_ + endcap_cone_depth_))
        {
            std::cout<<"generator is : endcap_cone2_gen_"<<std::endl;
            return endcap_cone2_gen_->GenerateVertex(VOLUME);
        }
    }
    /*if (region == "VESSEL") {
        return body_gen_->GenerateVertex(VOLUME);
    }*/
    /*if (region == "VESSEL_FLANGE1") {
        return flange1_gen_->GenerateVertex(VOLUME);
    }*/
    /*if (region == "CYL_ENDCAP1") {
        return endcap_cyl1_gen_->GenerateVertex(VOLUME);
    }
    if (region == "CYL_ENDCAP2") {
        return endcap_cyl2_gen_->GenerateVertex(VOLUME);
    }
    if (region == "GAS_COVER_FLANGE1") {
        return cover_flange1_gen_->GenerateVertex(VOLUME);
    }
    if (region == "GAS_COVER_FLANGE2") {
        return cover_flange2_gen_->GenerateVertex(VOLUME);
    }

    // Generator for cones
    if (region == "CONE_ENDCAP1") {
        return endcap_cone1_gen_->GenerateVertex(VOLUME);
    }
    if (region == "CONE_ENDCAP2") {
        return endcap_cone2_gen_->GenerateVertex(VOLUME);
    }*/


    

    G4Exception(
        "[GanESSVessel]",
        "GenerateVertex()",
        FatalException,
        "Unknown vertex generation region!");

    return G4ThreeVector();
    }


