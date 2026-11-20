// ----------------------------------------------------------------------------
// nexus | GanESSPmtPlate.hh
//
// Geometry of the GanESS Pmt Plate.
//
// The GanESS group
// ----------------------------------------------------------------------------

#ifndef GANESS_PMTPLATE_HH
#define GANESS_PMTPLATE_HH

#include "nexus/GeometryBase.h"
#include <vector>
#include "nexus/PmtR11410.h"

class G4Material;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4GenericMessenger;
class G4Navigator;

namespace nexus {

  class CylinderPointSampler;
  class BoxPointSampler;
  class ConePointSampler;


  class GanESSPmtPlate: public GeometryBase
  {
  public:
    GanESSPmtPlate();
    ~GanESSPmtPlate();

    void Construct() override;
    G4ThreeVector GenerateVertex(const G4String& region) const override;


    void SetMotherLogicalVolume(G4LogicalVolume* mother_logic);
    void SetMotherPhysicalVolume(G4VPhysicalVolume* mother_phys);
    void DefineMaterials();
  
  private:

    // Plate dimensions
    const G4double plate_out_rad_, plate_length_, plate_z_pos_;
    // PMTs
    const G4int n_pmts_;
    // Holes for pmts
    const G4double hole_pmt_out_rad_, hole_pmt_length_;

    // Steel pmt tubes
    const G4double steel_tube1_length_, steel_tube1_in_rad_, steel_tube1_out_rad_, steel_tube1_z_pos_;

    // Steel pmt tubes
    const G4double steel_tube2_length_, steel_tube2_in_rad_, steel_tube2_out_rad_, steel_tube2_z_pos_;

    // Steel cone
    const G4double steel_cone_length_, steel_cone_out_rad_z_neg_, steel_cone_out_rad_z_pos_, steel_cone_z_pos_;

    // Steel inside tube
    const G4double steel_inside_tube_length_, steel_inside_tube_out_rad_, steel_inside_tube_z_pos_;

    // Steel tube end
    const G4double steel_tube_end_length_, steel_tube_end_out_rad_, steel_tube_end_z_pos_;

    // PMT windows steel tubes
    const G4double steel_window_tube1_length_, steel_window_tube1_in_rad_, steel_window_tube1_out_rad_, steel_window_tube1_z_pos_;
    const G4double steel_window_tube2_length_, steel_window_tube2_in_rad_, steel_window_tube2_out_rad_, steel_window_tube2_z_pos_;
    const G4double steel_window_tube3_length_, steel_window_tube3_in_rad_, steel_window_tube3_out_rad_, steel_window_tube3_z_pos_;
    const G4double steel_window_tube2_inner1_length_, steel_window_tube2_inner1_in_rad_, steel_window_tube2_inner1_out_rad_, steel_window_tube2_inner1_z_pos_;
    const G4double steel_window_tube2_inner2_length_, steel_window_tube2_inner2_in_rad_, steel_window_tube2_inner2_out_rad_, steel_window_tube2_inner2_z_pos_;

    // Sapphire window
    const G4double sapphire_window_length_, sapphire_window_out_rad_, sapphire_window_z_pos_;
    
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

    // PMTs
    PmtR11410 pmt_;

    // Vertex generators (vectors are used for each piece of each pmt)
    std::vector<CylinderPointSampler*> plate_gen_vector_;
    CylinderPointSampler* plate_gen_;
    //std::vector<CylinderPointSampler*> steel_tube1_gen_vector_;
    CylinderPointSampler* steel_tube1_gen_;
    //std::vector<CylinderPointSampler*> steel_tube2_gen_vector_;
    CylinderPointSampler* steel_tube2_gen_;
    //std::vector<ConePointSampler*> steel_cone_gen_vector_;
    ConePointSampler* steel_cone_gen_;
    //std::vector<CylinderPointSampler*> steel_inside_tube_gen_vector_;
    CylinderPointSampler* steel_inside_tube_gen_;
    //std::vector<CylinderPointSampler*> steel_tube_end_gen_vector_;
    CylinderPointSampler* steel_tube_end_gen_;
    //std::vector<CylinderPointSampler*> steel_window_tube1_gen_vector_;
    CylinderPointSampler* steel_window_tube1_gen_;
    //std::vector<CylinderPointSampler*> steel_window_tube2_gen_vector_;
    CylinderPointSampler* steel_window_tube2_gen_;
    //std::vector<CylinderPointSampler*> steel_window_tube3_gen_vector_;
    CylinderPointSampler* steel_window_tube3_gen_;
    //std::vector<CylinderPointSampler*> steel_window_tube2_inner1_gen_vector_;
    CylinderPointSampler* steel_window_tube2_inner1_gen_;
    //std::vector<CylinderPointSampler*> steel_window_tube2_inner2_gen_vector_;
    CylinderPointSampler* steel_window_tube2_inner2_gen_;
    //std::vector<CylinderPointSampler*> sapphire_window_gen_vector_;
    CylinderPointSampler* sapphire_window_gen_;
    std::vector<CylinderPointSampler*> pmt_capsul_gen_vector_;
    std::vector<ConePointSampler*> pmt_capsul_cone_gen_vector_;


  };

} //end namespace nexus
#endif
