// ----------------------------------------------------------------------------
// nexus | GanESSFieldCage.h
//
// Geometry of the GanESS field cage.
//
// The GanESS group
// ----------------------------------------------------------------------------

#ifndef GANESS_FIELDCAGE_HH
#define GANESS_FIELDCAGE_HH

#include "nexus/GeometryBase.h"
#include <vector>

class G4Material;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4GenericMessenger;
class G4Navigator;

namespace nexus {

  class CylinderPointSampler;
  class BoxPointSampler;


  class GanESSFieldCage: public GeometryBase
  {
  public:
    GanESSFieldCage();
    ~GanESSFieldCage();

    void Construct() override;
    G4ThreeVector GenerateVertex(const G4String& region) const override;


    void SetMotherLogicalVolume(G4LogicalVolume* mother_logic);
    void SetMotherPhysicalVolume(G4VPhysicalVolume* mother_phys);
    void DefineMaterials();
  private:
    

    const G4double cathode_length_, cathode_out_rad_;
    const G4double EL_length_, EL_out_rad_, EL_z_pos_, EL_sub_length_, EL_sub_out_rad_, EL_sub_z_pos_;
    const G4double n_panel, panel_length_, panel_z_pos_;
    const G4double n_tpb_panel, tpb_panel_x_, tpb_panel_y_, tpb_panel_z_, tpb_panel_z_pos_;    const G4double n_crossbar, crossbar_width_, crossbars_z_pos_;
    const G4double n_small_crossbars, small_crossbars_x_, small_crossbars_y_, small_crossbars_length_, small_crossbars_z_pos_;
    const G4double n_ring, ring_length_ ,ring_in_rad_, ring_out_rad_, ring_z_pos_;
  

    // Pointers to materials definition
    G4Material* hdpe_;
    G4Material* tpb_;
    G4Material* teflon_;
    G4Material* copper_;
    G4Material* steel_;
    G4Material* aluminum_;


    G4LogicalVolume* mother_logic_;
    G4VPhysicalVolume* mother_phys_;

    G4Material* gas_;
    G4double pressure_;
    G4double temperature_;


    // Vertex generator
    CylinderPointSampler* cathode_gen_;
    CylinderPointSampler* EL_gen_;
    CylinderPointSampler* panel_gen_;

    CylinderPointSampler* gen_;
    std::vector<CylinderPointSampler*> ring_gen_;
    std::vector<CylinderPointSampler*> EL_gen_vector_;



  };

} //end namespace nexus
#endif
