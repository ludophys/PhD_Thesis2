// ----------------------------------------------------------------------------
// nexus | GanESS.cc
//
// Main class that constructs the geometry of the GanESS detector.
//
// The GanESS Group
// ----------------------------------------------------------------------------


#include "GanESS.hh"
#include "nexus/FactoryBase.h"

#ifndef REGISTER_CLASS
#error "REGISTER_CLASS n'est pas defini ici"
#endif


// GanESS geometry components
#include "GanESSVessel.hh"
#include "GanESSFieldCage.hh"
#include "GanESSPmtPlate.hh"
//#include "GanESSInnerElements.hh"

// G4 classes
#include <G4GenericMessenger.hh>
#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4NistManager.hh>
#include <G4UserLimits.hh>

using namespace nexus;

  REGISTER_CLASS(GanESS, GeometryBase)

  GanESS::GanESS():
    GeometryBase(),

    // Lab dimensions
    lab_size_ (5. * m)

  {
    msg_ = new G4GenericMessenger(this, "/Geometry/GanESS/", "Control commands of geometry GanESS.");

    vessel_ = new GanESSVessel();
    field_cage_ = new GanESSFieldCage();
    pmt_plate_ = new GanESSPmtPlate();
  }

  GanESS::~GanESS()
  {
   delete vessel_;
   delete field_cage_;
   delete pmt_plate_;
  }

  void GanESS::Construct()
  {
    // Lab volume
    G4Box* lab_solid = new G4Box("LAB", lab_size_/2., lab_size_/2., lab_size_/2.);
    lab_logic_ = new G4LogicalVolume(lab_solid,G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "LAB");
    lab_logic_->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic_);

    // Construct the geometry of the GanESS detector
    // VESSEL
    vessel_->Construct();
    G4LogicalVolume* vessel_logic = vessel_->GetLogicalVolume();
    G4LogicalVolume* vessel_internal_logic  =
      vessel_->GetInternalLogicalVolume();
    G4VPhysicalVolume* vessel_internal_phys =
      vessel_->GetInternalPhysicalVolume();
    //G4LogicalVolume* vessel_internal_logic  = vessel_->GetInternalLogicalVolume();
    //G4VPhysicalVolume* vessel_internal_phys = vessel_->GetInternalPhysicalVolume();
    new G4PVPlacement(0,G4ThreeVector(),vessel_logic,"VESSEL",lab_logic_,false,0);

    
    //G4LogicalVolume* field_cage_logic = field_cage_->GetLogicalVolume();
    field_cage_->SetMotherLogicalVolume(vessel_internal_logic);
    field_cage_->SetMotherPhysicalVolume(vessel_internal_phys);
    field_cage_->DefineMaterials();
    field_cage_->Construct();
    //new G4PVPlacement(0,G4ThreeVector(),field_cage_logic,"PTFE_PANEL",lab_logic_,false,0);
    
    /*pmt_plate_->SetMotherLogicalVolume(vessel_internal_logic);
    pmt_plate_->SetMotherPhysicalVolume(vessel_internal_phys);
    pmt_plate_->DefineMaterials();
    pmt_plate_->Construct();*/


  }

    G4ThreeVector GanESS::GenerateVertex(const G4String& region) const
  {
    // Vertex generation in the vessel volume
    /*if (region == "VESSEL" || region == "VESSEL_FLANGE1" || region == "VESSEL_FLANGE2" || region == "CYL_ENDCAP1" || region == "CYL_ENDCAP2" || region == "CONE_ENDCAP1" || region == "CONE_ENDCAP2" || region == "GAS_COVER_FLANGE1" || region == "GAS_COVER_FLANGE2") {
      return vessel_->GenerateVertex(region);
    }*/
   if (region == "OUTSIDE")
   {
        return vessel_->GenerateVertex(region);
   }

    if (region == "VESSEL_BODY" || 
        region == "VESSEL_FLANGE1" || 
        region == "VESSEL_FLANGE2" ||
        region == "VESSEL_INNER_FLANGE1" ||
        region == "VESSEL_INNER_FLANGE2" ||
        region == "CYL_ENDCAP1" ||
        region == "CYL_ENDCAP2" ||
        region == "CONE_ENDCAP" ||
        region == "COVER_FLANGE1" ||
        region == "TUBS_CF1_INNER" ||
        region == "COVER_FLANGE2")
    {

        return vessel_->GenerateVertex(region);
    }

    if (region == "PTFE_PANEL" || 
        region == "CROSSBAR" ||
        region == "RINGS_Z+" || 
        region == "RINGS_Z-" ||
        region == "CATHODE" || 
        region == "EL")
    {
        return field_cage_->GenerateVertex(region);
    }

    if (region == "PLATE" ||
    region == "STEEL_TUBE1" ||
    region == "STEEL_TUBE2" ||
    region == "STEEL_INSIDE_TUBE" ||
    region == "STEEL_TUBE_END" ||
    region == "STEEL_WINDOW_TUBE1" ||
    region == "STEEL_WINDOW_TUBE2" ||
    region == "STEEL_WINDOW_TUBE3" ||
    region == "STEEL_WINDOW_TUBE2_INNER1" ||
    region == "STEEL_WINDOW_TUBE2_INNER2" ||
    region == "SAPPHIRE_WINDOW" || 
    region == "STEEL_CONE")
    {
        return pmt_plate_->GenerateVertex(region);
    }

    G4Exception("[GanESS]", "GenerateVertex()", FatalException, "Unknown vertex generation region!");

    return G4ThreeVector();
  }