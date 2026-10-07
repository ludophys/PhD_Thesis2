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
  }

  GanESS::~GanESS()
  {
   delete vessel_;
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
    vessel_->Construct();
    G4LogicalVolume* vessel_logic = vessel_->GetLogicalVolume();
    new G4PVPlacement(0,G4ThreeVector(),vessel_logic,"VESSEL",lab_logic_,false,0);
  }

    G4ThreeVector GanESS::GenerateVertex(const G4String& region) const
  {
    // Vertex generation in the vessel volume
    if (region == "VESSEL") {
      return vessel_->GenerateVertex(region);
    }

    G4Exception("[GanESS]", "GenerateVertex()", FatalException, "Unknown vertex generation region!");

    return G4ThreeVector();
  }