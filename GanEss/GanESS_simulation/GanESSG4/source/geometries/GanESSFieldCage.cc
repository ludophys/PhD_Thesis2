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

  // Aluminum for rings
  aluminum_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");
  aluminum_->SetMaterialPropertiesTable(new G4MaterialPropertiesTable());
}

void GanESSFieldCage::Construct()
{

// Copper rings
    G4Tubs* ring_solid = new G4Tubs("RINGS", ring_in_rad_, ring_out_rad_, ring_length_, 0.*deg, 360.*deg);

    G4LogicalVolume* ring_logic = new G4LogicalVolume(ring_solid, aluminum_, "RINGS");

    for (G4int i=0; i<2. * n_ring; i++)
    {
      // Positive side
      if (i<n_ring)
      {
        new G4PVPlacement(nullptr, G4ThreeVector(0., 0., ring_z_pos_ - (i*2.*(panel_length_ + ring_length_/2.)/n_ring)), ring_logic, "RINGS", mother_logic_, false, i, true);
        //std::cout<<"Ring position: "<<ring_z_pos_ - (i*2.*(panel_length_ + ring_length_)/n_ring)<<std::endl;
      }
      // Negative side
      else
      {
        new G4PVPlacement(nullptr, G4ThreeVector(0., 0., - (ring_z_pos_ - ((i - n_ring)*2.*(panel_length_ + ring_length_/2.)/n_ring))), ring_logic, "RINGS", mother_logic_, false, i, true);
        //std::cout<<"Ring position: "<<- (ring_z_pos_ - ((i - n_ring)*2.*(panel_length_ + ring_length_)/n_ring))<<std::endl;
      }
    }

  // Geometry of the panels
  // Definition of the polygon
  std::vector<G4TwoVector> pol_panels;
  G4int n_pts_panels = 17;
// Coordinates for panels
  //G4double Xcoord_panels[] = {0.000, 92.842, 92.243, 74.513, 72.162, 63.957, 57.234, 55.142, 49.520, 47.367, 39.661, 40.380, 34.281, 29.795, 23.071, 15.423, 14.175, -1.726};
  //G4double Ycoord_panels[] = {0.000, 33.791, 37.191, 33.584, 34.443, 32.324, 29.876, 25.390, 23.344, 26.285, 23.481, 21.357, 17.798, 19.889, 17.442, 13.792, 11.623, 2.990};

  // Coordinates for panels and small crossbars
G4double Xcoord_panels[] = {0.000, 92.842, 92.243, 74.513, 72.162, 63.957, 57.234, 55.142, 50.020, 29.347, 34.306, 34.281, 29.795, 23.071, 15.423, 14.175, -1.726};
G4double Ycoord_panels[] = {0.000, 33.791, 37.191, 33.584, 34.443, 32.324, 29.876, 25.390, 39.170, 31.645, 18.020, 17.798, 19.889, 17.442, 13.792, 11.623, 2.990};

  for (G4int i = 0; i < n_pts_panels; i++) 
    {
      pol_panels.emplace_back(Xcoord_panels[i] * mm, Ycoord_panels[i]*mm);
    }

  // Placement of the other panels according to the first one (very top one)
  G4double Xorigin_panels = -49.687 * mm;
  G4double Yorigin_panels = 280.949 * mm;

  G4double r_panels = std::hypot(Xorigin_panels, Yorigin_panels);
  std::cout<<"r_panels is : "<<r_panels<<std::endl;
  G4double theta0_panels = std::atan2(Yorigin_panels, Xorigin_panels);
  G4double deltaTheta_panels = 360.0 * deg / n_panel;

  // Smaller crossbars
  //G4Box* small_crossbars = new G4Box("SMALL_CROSSBARS", small_crossbars_x_, small_crossbars_y_, small_crossbars_length_);
  //G4LogicalVolume* small_crossbars_logic = new G4LogicalVolume(small_crossbars, teflon_, "SMALL_CROSSBARS");
   
  // Creation of the polygon
  G4ExtrudedSolid* PTFE_panel = new G4ExtrudedSolid("PTFE_PANEL", pol_panels, panel_length_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);
   for (G4int i = 0; i < 2.* n_panel; i++)
   //for (G4int i = 0; i < 1; i++)
   {

    G4VSolid* PTFE_panel_cp1 = PTFE_panel;

    G4double theta_panels = theta0_panels + i * deltaTheta_panels;
    G4double theta_small_crossbars = theta_panels - 10.*deg;
    G4RotationMatrix* rotation = new G4RotationMatrix();
    G4RotationMatrix* rotation2 = new G4RotationMatrix();
    G4double angle = (1 - i) * 20.0 * deg;
    G4double angle2 = (- i) * 20.0 * deg;
    rotation->rotateZ(angle); // Orientation of the piece around the field cage
    rotation2->rotateZ(angle2);

    // Coordinates of the other pieces according to the first one (Xorigin, Yorigin)
    G4double X_update = r_panels * std::cos(theta_panels);
    G4double Y_update = r_panels * std::sin(theta_panels);

    G4double X_update_crossbars = r_panels * std::cos(theta_small_crossbars);
    G4double Y_update_crossbars = r_panels * std::sin(theta_small_crossbars);
    
    // Subtraction of the ring volume from the panel holders

    // Position du panneau dans le mother volume
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

        // Check whether the ring intersects the panel
        /*if (std::abs(z_ring - z_panel) >
            panel_length_ + ring_length_)
        {
            continue;
        }*/

        // Position of the ring in the local panel frame
        G4ThreeVector ring_local = invRotation * (ring_global - panel_pos);

        // Boolean subtraction
        PTFE_panel_cp1 = new G4SubtractionSolid("PTFE_PANEL_SUB_" + std::to_string(i) + "_" + std::to_string(j), PTFE_panel_cp1, ring_solid, nullptr, ring_local);
    }
        G4LogicalVolume* PTFE_panel_logic = new G4LogicalVolume(PTFE_panel_cp1, teflon_, "PTFE_PANEL");
        
        if (i<n_panel)
      // Positive side
      {
        new G4PVPlacement(rotation, G4ThreeVector(X_update, Y_update, panel_z_pos_), PTFE_panel_logic, "PTFE_PANEL", mother_logic_, false, i, true);
      } 
      // Negative side   
      else
      {
        //new G4PVPlacement(rotation2, G4ThreeVector(X_update_crossbars, Y_update_crossbars, - panel_z_pos_), small_crossbars_logic, "SMALL_CROSSBARS", mother_logic_, false, i, true);
        new G4PVPlacement(rotation, G4ThreeVector(X_update, Y_update, -panel_z_pos_), PTFE_panel_logic, "PTFE_PANEL", mother_logic_, false, i, true);
      }
    //std::cout<<"x panels : "<<X_update<<" y panels : "<<Y_update<<std::endl;

}

   // Geometry of the crossbars
   // Definition of the polygon
  std::vector<G4TwoVector> pol_crossbars;

  G4int n_pts_crossbar = 8;

  G4double Xcoord_crossbars[] = {
      0.000, 4.854, 6.559, 18.743,
      18.743, 6.559, 4.854, 0.000
  };

  G4double Ycoord_crossbars[] = {
      0.000, -8.536, -9.536, -8.536,
      853.664, 854.664, 853.664, 845.128
  };

  for (G4int i = 0; i < n_pts_crossbar; i++) 
    {
      pol_crossbars.emplace_back(Xcoord_crossbars[i] * mm, Ycoord_crossbars[i]*mm);
    }

    // Placement of the other crossbars according to the first one defined
    G4double Xorigin_crossbars = 109.355*mm;
    G4double Yorigin_crossbars = 300.45*mm;
    //G4double Xorigin_crossbars = -113.182*mm;
    //G4double Yorigin_crossbars = 278.803*mm;

    //G4double r_crossbars = cathode_out_rad_ + 20.592*mm;

    //G4double r_crossbars = r_panels + 18.872*mm;
    G4double r_crossbars = std::hypot(Xorigin_crossbars, Yorigin_crossbars);
    std::cout<<"r_crossbars is : "<<r_crossbars<<std::endl;

    G4double theta0_crossbars = std::atan2(Yorigin_crossbars, Xorigin_crossbars);
    G4double deltaTheta_crossbars = 360.0 * deg / n_crossbar;
  
    for (G4int i = 0; i < n_crossbar; i++)
    {
      G4ExtrudedSolid* crossbar = new G4ExtrudedSolid("CROSSBAR", pol_crossbars, crossbar_width_, G4TwoVector(0., 0.), 1., G4TwoVector(0., 0.), 1.);
      G4LogicalVolume* crossbar_logic = new G4LogicalVolume(crossbar, teflon_, "CROSSBAR");
      
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
      
      //std::cout<<"x crossbar : "<<X_update_crossbars<<" y crossbar : "<<Y_update_crossbars<<std::endl;


      new G4PVPlacement(rotation, G4ThreeVector(X_update_crossbars, Y_update_crossbars, crossbars_z_pos_), crossbar_logic, "CROSSBAR", mother_logic_, false, i, true);
    }

    // Cathode
    G4Tubs* cathode_solid = new G4Tubs("CATHODE", 0., cathode_out_rad_, cathode_length_, 0.*deg, 360.*deg);
    G4LogicalVolume* cathode_logic = new G4LogicalVolume(cathode_solid, hdpe_, "CATHODE");
    
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), cathode_logic, "CATHODE", mother_logic_, false, 0, true);

    // EL
    G4VSolid* EL_final_logic = nullptr;
    G4Tubs* EL_solid = new G4Tubs("EL", 0., EL_out_rad_, EL_length_, 0.*deg, 360.*deg);
    //G4LogicalVolume* EL_logic = new G4LogicalVolume(EL_solid, hdpe_, "LOGIC_EL");
    G4Tubs* EL_sub_solid = new G4Tubs("EL_SUB", 0., EL_sub_out_rad_, EL_sub_length_, 0.*deg, 360.*deg);
    //G4LogicalVolume* EL_sub_logic = new G4LogicalVolume(EL_sub_solid, hdpe_, "LOGIC_EL_SUB");
    EL_final_logic = new G4SubtractionSolid("EL", EL_solid, EL_sub_solid, nullptr, G4ThreeVector(0., 0., EL_sub_z_pos_));
    G4LogicalVolume* EL_logic = new G4LogicalVolume(EL_final_logic, hdpe_, "EL");

    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., EL_z_pos_), EL_logic, "EL", mother_logic_, false, 0, true);

    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateX(180.*deg);
    new G4PVPlacement(rotation, G4ThreeVector(0., 0., - EL_z_pos_), EL_logic, "EL", mother_logic_, false, 0, true);

    // Vertex generators
    //outside_gen_       = new CylinderPointSampler(0., vessel_out_rad_ + 200.*mm, vessel_length_ + 300.*mm, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));

    //cathode_gen_ = new CylinderPointSampler(0., cathode_out_rad_, cathode_length_, 0., 360.*deg, 0, G4ThreeVector(0., 0., 0.));

}

 GanESSFieldCage::~GanESSFieldCage()
  {
    delete cathode_gen_;
  }

G4ThreeVector GanESSFieldCage::GenerateVertex(const G4String& region) const
{
  if (region == "FIELD_CAGE")
  {
    return cathode_gen_->GenerateVertex(VOLUME);
  }

  G4Exception(
        "[GanESSVessel]",
        "GenerateVertex()",
        FatalException,
        "Unknown vertex generation region!");

    return G4ThreeVector();
}