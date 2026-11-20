// ----------------------------------------------------------------------------
// nexus | GanESSFieldCage.cc
//
// Geometry of the GanESS field cage.
//
// The GanESS group
// ----------------------------------------------------------------------------

#include "GanESSFieldCage.hh"
#include "nexus/Next100Stave.h"
#include "nexus/MaterialsList.h"
#include "nexus/Visibilities.h"
#include "nexus/IonizationSD.h"
#include "nexus/OpticalMaterialProperties.h"
#include "nexus/UniformElectricDriftField.h"
#include "nexus/XenonProperties.h"
#include "nexus/CylinderPointSampler.h"
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


GanESSFieldCage::GanESSFieldCage():
  GeometryBase(),
  // Dimensions

  // Cathode
  cathode_length_  (3./2. * mm),
  cathode_out_rad_ (601.4/2. * mm),

    // EL
  EL_length_  (9.5/2.*mm),
  EL_out_rad_ (601.4/2.*mm),
  EL_z_pos_   (12.5*mm+EL_length_),

  // EL subtraction
  EL_sub_length_  (9./2.*mm),
  EL_sub_out_rad_ (525.4/2.*mm),
  EL_sub_z_pos_   (EL_length_ - EL_sub_length_),

  // Panels
  n_panel       (18),
  panel_length_ (400./2.*mm),
  panel_z_pos_  (227. * mm),

  //TPB coating panels
  n_tpb_panel (n_panel),
  //tpb_panel_x_ (98.598/2. * mm),
  tpb_panel_x_ (0.5 * std::hypot(92.842, 33.791) * mm),
  tpb_panel_y_ (0.01/2. * mm),
  tpb_panel_z_ (panel_length_),
  tpb_panel_z_pos_ (panel_z_pos_),


  // Crossbars
  n_crossbar       (6),
  crossbar_width_  (22./2.*mm),
  crossbars_z_pos_ (862.2/2.*mm),

  // Smaller crossbars
  n_small_crossbars (n_panel),
  small_crossbars_x_ (22./2. * mm),
  small_crossbars_y_ (14.5/2. * mm),
  small_crossbars_length_ (410./2.*mm),
  small_crossbars_z_pos_ (228.05/2.*mm),

  // Rings
  n_ring        (27),
  ring_length_  (10./2. * mm),
  ring_in_rad_  (582./2. * mm),
  ring_out_rad_ (588./2. * mm),
  //ring_out_rad_ (585./2. * mm),
  ring_z_pos_   (panel_z_pos_ + panel_length_ - ring_length_)

{

  /// Define new categories
  new G4UnitDefinition("kilovolt/cm","kV/cm","Electric field", kilovolt/cm);
  new G4UnitDefinition("mm/sqrt(cm)","mm/sqrt(cm)","Diffusion", mm/sqrt(cm));
  new G4UnitDefinition("mm/microsecond","mm/microsecond","drift velocity", mm/microsecond);

}


void GanESSFieldCage::SetMotherLogicalVolume(G4LogicalVolume* mother_logic)
{
  mother_logic_ = mother_logic;
}


void GanESSFieldCage::SetMotherPhysicalVolume(G4VPhysicalVolume* mother_phys)
{
  mother_phys_ = mother_phys;
}


void GanESSFieldCage::DefineMaterials()
{
  /// Read gas properties from mother volume
  gas_         = mother_logic_->GetMaterial();
  pressure_    = gas_->GetPressure();
  temperature_ = gas_->GetTemperature();
  //sc_yield_    = gas_->GetMaterialPropertiesTable()->GetConstProperty("SCINTILLATIONYIELD");
  //e_lifetime_  = gas_->GetMaterialPropertiesTable()->GetConstProperty("ATTACHMENT");

  /// High density polyethylene for the field cage
  hdpe_ = materials::HDPE();

  /// Copper for field rings
  copper_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Cu");
  copper_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

  /// Teflon for the light tube
  teflon_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
  // teflon is the material used in the light-tube  and is covered by
  // a G4LogicalSkinSurface. In Geant4 11.0.0, a bug in treating the
  // OpBoundaryProcess produced in the surface makes the code fail. This is
  // avoided by setting an empty G4MaterialPropertiesTable of the G4Material.
  teflon_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

  /// TPB coating
  tpb_ = materials::TPB();
  tpb_->SetMaterialPropertiesTable(opticalprops::TPB());

  /// Steel
  steel_ = materials::Steel316Ti();
  // In Geant4 11.0.0, a bug in treating the OpBoundaryProcess produced in the
  // surface makes the code fail. This is avoided by setting
  // an empty G4MaterialPropertiesTable of the G4Material.
  steel_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());

  // AISI-304L (https://www.metalcor.de/en/datenblatt/7/)
    /*G4double density = 7.9 * g/cm3;

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
    steel_->AddElement(S,  0.00015);*/

  // Aluminum for rings
  aluminum_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");
  aluminum_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());
}

void GanESSFieldCage::Construct()
{

  // Copper rings
  G4Tubs* ring_solid = new G4Tubs("RINGS", ring_in_rad_, ring_out_rad_, ring_length_, 0.*deg, 360.*deg);
  G4LogicalVolume* ring_logic = new G4LogicalVolume(ring_solid, aluminum_, "RINGS");

  // Loop to place the rings
  G4double ring_step = (panel_length_ + ring_length_/2.)/n_ring;
  for (G4int i=0; i<n_ring; i++)
  {
    if (i<n_ring)
    {
      new G4PVPlacement(nullptr, G4ThreeVector(0., 0., ring_z_pos_ - (i*2.*ring_step)), ring_logic, "RINGS_Z+", mother_logic_, false, i, true);
      new G4PVPlacement(nullptr, G4ThreeVector(0., 0., - (ring_z_pos_ - (i*2.*ring_step))), ring_logic, "RINGS_Z-", mother_logic_, false, i, true);

    }
  }

  // Geometry of the panels
  // Definition of the polygon
  std::vector<G4TwoVector> pol_panels;
  G4int n_pts_panels = 17;

  // Coordinates for panels and small crossbars
  G4double Xcoord_panels[] = {0.000, 92.842, 92.243, 74.513, 72.162, 63.957, 57.234, 55.142, 50.020, 29.347, 34.306, 34.281, 29.795, 23.071, 15.423, 14.175, -1.726};
  G4double Ycoord_panels[] = {0.000, 33.791, 37.191, 33.584, 34.443, 32.324, 29.876, 25.390, 39.170, 31.645, 18.020, 17.798, 19.889, 17.442, 13.792, 11.623, 2.990};

  // Loop to create the polygon
  for (G4int i = 0; i < n_pts_panels; i++) 
    {
      pol_panels.emplace_back(Xcoord_panels[i] * mm, Ycoord_panels[i]*mm);
    }

  // Placement of the other panels according to the first one (very top one)
  G4double Xorigin_panels = -49.687 * mm;
  G4double Yorigin_panels = 280.949 * mm;

  G4double r_panels = std::hypot(Xorigin_panels, Yorigin_panels);
  G4double theta0_panels = std::atan2(Yorigin_panels, Xorigin_panels);
  G4double deltaTheta_panels = 360.0 * deg / n_panel;

  // Placement of the other tpb panels according to the first one (very top one)
  /*G4double Xorigin_tpb_panels = -96.234 * mm + 0.3425*tpb_panel_y_;
  G4double Yorigin_tpb_panels = 263.9805 * mm - 0.9395*tpb_panel_y_;

  G4double r_tpb_panels = std::hypot(Xorigin_tpb_panels, Yorigin_tpb_panels);
  G4double theta0_tpb_panels = std::atan2(Yorigin_tpb_panels, Xorigin_tpb_panels);
  G4double deltaTheta_tpb_panels = 360.0 * deg / n_panel;*/

  // TPB panels: the inner face of a PTFE panel is the first edge of its polygon, (0,0) -> (X[1],Y[1])
  const G4double face_angle = std::atan2(Ycoord_panels[1], Xcoord_panels[1]);   // 20 deg
  const G4TwoVector face_mid(0.5 * Xcoord_panels[1] * mm, 0.5 * Ycoord_panels[1] * mm);
  // unit vector perpendicular to the face, pointing towards the axis of the cage (inside)
  const G4TwoVector inward(std::sin(face_angle), -std::cos(face_angle));
  // centre of the TPB box in the local frame of the PTFE panel: it lies just inside the face,
  // so its outer side is exactly on the PTFE surface (no overlap)
  const G4TwoVector tpb_local = face_mid + tpb_panel_y_ * inward;

  // Creation of the polygon
  G4ExtrudedSolid* PTFE_panel = new G4ExtrudedSolid("PTFE_PANEL", pol_panels, panel_length_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);
  
  // TPB coating panels to place at the surface of the PTFE panels
  G4Box* TPB_panel = new G4Box("TPB_PANEL", tpb_panel_x_, tpb_panel_y_, tpb_panel_z_);
  G4LogicalVolume* TPB_panel_logic = new G4LogicalVolume(TPB_panel, tpb_, "TPB_PANEL");

  for (G4int i = 0; i < 2.* n_panel; i++)
   //for (G4int i = 0; i < 1; i++)
   {

    G4VSolid* PTFE_panel_cp1 = PTFE_panel;

    G4double theta_panels = theta0_panels + i * deltaTheta_panels;
    //G4double theta_tpb_panels = theta0_tpb_panels + i * deltaTheta_panels;
    G4double theta_small_crossbars = theta_panels - 10.*deg;

    G4RotationMatrix* rotation = new G4RotationMatrix();
    //G4RotationMatrix* rotation2 = new G4RotationMatrix();

    G4double angle = (1 - i) * 20.0 * deg;
    //G4double angle2 =  - (1+i) * 20.*deg;
    std::cout<<"i is : "<<i<<"angle is (rotatez) : "<<angle<<std::endl;
    rotation->rotateZ(angle); // Orientation of the piece around the field cage
    //rotation2->rotateZ(angle2);

    // Coordinates of the other pieces according to the first one (Xorigin, Yorigin)
    G4double X_update = r_panels * std::cos(theta_panels);
    G4double Y_update = r_panels * std::sin(theta_panels);

    //G4double X_update_tpb = r_tpb_panels * std::cos(theta_tpb_panels);
    //G4double Y_update_tpb = r_tpb_panels * std::sin(theta_tpb_panels);
    
    // Subtraction of the ring volume from the panel holders

    G4double side = (i < n_panel) ? 1. : -1.;

    G4double z_panel = side * panel_z_pos_;

    G4ThreeVector panel_pos(X_update, Y_update, z_panel);

    // Inverse transformation according to the G4PVPlacement
    // rotation convention
    G4RotationMatrix invRotation = *rotation;

    // Only subtract rings belonging to the same side
    for (G4int j = 0; j < n_ring; j++)
    {
        // Global position of the ring
        G4double z_ring = side * (ring_z_pos_ - j * 2. * (panel_length_ + ring_length_/2.) / n_ring);

        G4ThreeVector ring_global(0., 0., z_ring);

        // Position of the ring in the local panel frame
        G4ThreeVector ring_local = invRotation * (ring_global - panel_pos);

        // Boolean subtraction
        PTFE_panel_cp1 = new G4SubtractionSolid("PTFE_PANEL_SUB_" + std::to_string(i) + "_" + std::to_string(j), PTFE_panel_cp1, ring_solid, nullptr, ring_local);
    }
        G4LogicalVolume* PTFE_panel_logic = new G4LogicalVolume(PTFE_panel_cp1, teflon_, "PTFE_PANEL");
        
        if (i<n_panel)
      // Positive side
      {
       
      
          //new G4PVPlacement(rotation2, G4ThreeVector(X_update_tpb,Y_update_tpb, tpb_panel_z_pos_), TPB_panel_logic, "TPB_PANEL", mother_logic_, false, i, true);

        
        new G4PVPlacement(rotation, G4ThreeVector(X_update, Y_update, panel_z_pos_), PTFE_panel_logic, "PTFE_PANEL", mother_logic_, false, i, true);
        std::cout<<"i is : "<<i<<"X : "<<X_update<<"Y : "<<Y_update<<"z_pos : "<<panel_z_pos_<<"Rot : "<<rotation<<std::endl;
      } 
      // Negative side   
      else
      {        
        //new G4PVPlacement(rotation2, G4ThreeVector(X_update_tpb, Y_update_tpb, - tpb_panel_z_pos_), TPB_panel_logic, "TPB_PANEL", mother_logic_, false, i, true);
        new G4PVPlacement(rotation, G4ThreeVector(X_update, Y_update, -panel_z_pos_), PTFE_panel_logic, "PTFE_PANEL", mother_logic_, false, i, true);
      }

          // TPB placed on the inner face of THIS panel (same rotation and position as the PTFE panel)
    G4ThreeVector tpb_pos(tpb_local.x(), tpb_local.y(), 0.);
    tpb_pos.rotateZ(-angle);                       // physical rotation of the panel = -angle
    tpb_pos += G4ThreeVector(X_update, Y_update, side * tpb_panel_z_pos_);

    G4RotationMatrix* rotation_tpb = new G4RotationMatrix();
    rotation_tpb->rotateZ(angle - face_angle);     // x axis of the box along the inner face

    new G4PVPlacement(rotation_tpb, tpb_pos, TPB_panel_logic, "TPB_PANEL", mother_logic_, false, i, true);
}

   // Geometry of the crossbars
   // Definition of the polygon
    std::vector<G4TwoVector> pol_crossbars;

    G4int n_pts_crossbar = 8;

    G4double Xcoord_crossbars[] = {0.000, 4.854, 6.559, 18.743, 18.743, 6.559, 4.854, 0.000};
    G4double Ycoord_crossbars[] = {0.000, -8.536, -9.536, -8.536, 853.664, 854.664, 853.664, 845.128};

    for (G4int i = 0; i < n_pts_crossbar; i++) 
      {
        pol_crossbars.emplace_back(Xcoord_crossbars[i] * mm, Ycoord_crossbars[i]*mm);
      }

    // Placement of the other crossbars according to the first one defined
    G4double Xorigin_crossbars = 109.355*mm;
    G4double Yorigin_crossbars = 300.45*mm;

    G4double r_crossbars = std::hypot(Xorigin_crossbars, Yorigin_crossbars);
    G4double theta0_crossbars = std::atan2(Yorigin_crossbars, Xorigin_crossbars);
    G4double deltaTheta_crossbars = 360.0 * deg / n_crossbar;
  
    G4ExtrudedSolid* crossbar = new G4ExtrudedSolid("CROSSBAR", pol_crossbars, crossbar_width_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);
    G4LogicalVolume* crossbar_logic = new G4LogicalVolume(crossbar, teflon_, "CROSSBAR");

    // Loop to place the corssbars
    for (G4int i = 0; i < n_crossbar; i++)
    {
      G4double theta_crossbars = theta0_crossbars + i * deltaTheta_crossbars;
      G4RotationMatrix* rotation = new G4RotationMatrix();
      
      if (i==0)
      {
        G4double angle = 20.*deg;
        rotation->rotateZ(90.*deg + angle);
      }
      else
      {
        G4double angle = (20. - i*(3.*20.))*deg;
        rotation->rotateZ(90.*deg + angle);
      }

      rotation->rotateX(90.*deg);

      G4double X_update_crossbars = r_crossbars * std::cos(theta_crossbars);
      G4double Y_update_crossbars = r_crossbars * std::sin(theta_crossbars);
      
      new G4PVPlacement(rotation, G4ThreeVector(X_update_crossbars, Y_update_crossbars, crossbars_z_pos_), crossbar_logic, "CROSSBAR", mother_logic_, false, i, true);
    }

    /*G4Tubs* panel_gen = new G4Tubs("Panel_gen", ring_in_rad_ - 50./2.*mm, ring_out_rad_ + 100./2.*mm, 890./2. * mm, 0.*deg, 360.*deg);
    G4LogicalVolume* Panel_gen_logic = new G4LogicalVolume(panel_gen, steel_, "panel_gen");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), Panel_gen_logic, "panel_gen", mother_logic_, false, 0, true);*/

    // Cathode
    G4Tubs* cathode_solid = new G4Tubs("CATHODE", 0., cathode_out_rad_, cathode_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* cathode_logic = new G4LogicalVolume(cathode_solid, steel_, "CATHODE");
    
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), cathode_logic, "CATHODE", mother_logic_, false, 0, true);

    // EL
    G4VSolid* EL_final_logic = nullptr;
    
    G4Tubs* EL_solid = new G4Tubs("EL", 0., EL_out_rad_, EL_length_, 0.*deg, 360.*deg);
    G4Tubs* EL_sub_solid = new G4Tubs("EL_SUB", 0., EL_sub_out_rad_, EL_sub_length_, 0.*deg, 360.*deg);
    
    EL_final_logic = new G4SubtractionSolid("EL", EL_solid, EL_sub_solid, nullptr, G4ThreeVector(0., 0., EL_sub_z_pos_));
    
    G4LogicalVolume* EL_logic = new G4LogicalVolume(EL_final_logic, steel_, "EL");

    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., EL_z_pos_), EL_logic, "EL", mother_logic_, false, 0, true);
    
    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateX(180.*deg);
    
    new G4PVPlacement(rotation, G4ThreeVector(0., 0., - EL_z_pos_), EL_logic, "EL", mother_logic_, false, 1, true);


    // vertex generators
    cathode_gen_ = new CylinderPointSampler(0., cathode_out_rad_, cathode_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
    
    G4double EL_gen_z_pos_ = EL_z_pos_;
    for (G4int i=0; i<2.; i++)
    {
      
      EL_gen_ = new CylinderPointSampler(0., EL_out_rad_, EL_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., EL_gen_z_pos_));
      EL_gen_vector_.push_back(EL_gen_);
      EL_gen_z_pos_ = -EL_gen_z_pos_;
    }

    panel_gen_ = new CylinderPointSampler(ring_in_rad_ - 50./2.*mm, ring_out_rad_ + 100./2.*mm, 890./2. * mm, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));
    
    for (G4int i=0; i<2.*n_ring; i++)
      {
        if (i<n_ring)
        {
          gen_ = new CylinderPointSampler(ring_in_rad_, ring_out_rad_, ring_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., ring_z_pos_ - (i*2.*ring_step)));
          ring_gen_.push_back(gen_);
        }
        else
        {
          gen_ = new CylinderPointSampler(ring_in_rad_, ring_out_rad_, ring_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., - (ring_z_pos_ - ((i-n_ring)*2.*ring_step))));
          ring_gen_.push_back(gen_);
        }
  
      }
    }
 GanESSFieldCage::~GanESSFieldCage()
  {
    delete cathode_gen_;
    delete EL_gen_;
    delete gen_;
    delete panel_gen_;

  }

G4ThreeVector GanESSFieldCage::GenerateVertex(const G4String& region) const
{

  if (region == "PTFE_PANEL" || region == "CROSSBAR") 
  {
      return panel_gen_->GenerateVertex(VOLUME);
  }

  if (region == "RINGS_Z+" || region == "RINGS_Z-") 
  {
      int index = (int)(G4UniformRand() * ring_gen_.size());

      return ring_gen_[index]->GenerateVertex(VOLUME);
      
  }

  if (region == "CATHODE") 
  {    
    return cathode_gen_->GenerateVertex(VOLUME);
  }

  if (region == "EL") 
  {
    int index = (int)(G4UniformRand() * 2);

    return EL_gen_vector_[index]->GenerateVertex(VOLUME);   
  }

  G4Exception(
        "[GanESSFieldCage]",
        "GenerateVertex()",
        FatalException,
        "Unknown vertex generation region!");

    return G4ThreeVector();
}