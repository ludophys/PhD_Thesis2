// ----------------------------------------------------------------------------
// nexus | GanESSVessel.hh
//
// Vessel of the GanESS geometry.
//
// The GanESS Group
// ----------------------------------------------------------------------------

#ifndef GANESS_VESSEL_HH
#define GANESS_VESSEL_HH

#include "nexus/GeometryBase.h"

#include <G4Navigator.hh>


class G4GenericMessenger;
class G4VPhysicalVolume;

namespace nexus {

  class CylinderPointSampler;
  class SpherePointSampler;
  class ConePointSampler;

  class GanESSVessel: public GeometryBase
  {
  public:
    /// Constructor
    GanESSVessel();

    /// Destructor
    ~GanESSVessel();

    /// Generate a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    /// Builder
    void Construct();

  private:
    // Dimensions
    const G4double vessel_length_, vessel_in_rad_, vessel_out_rad_;
    const G4double flange_length_, flange_in_rad_, flange_out_rad_, flange_z_pos_;
    const G4double cover_flange_length_, cover_flange1_in_rad_, cover_flange2_in_rad_, cover_flange_out_rad_, cover_flange_z_pos_;
    const G4double cover_flange1_gen_in_rad_;
    const G4double cover_flange1_inner_in_rad_, cover_flange1_inner_length_, cover_flange1_inner_tubs_z_pos_;
    const G4double cover_flange1_inner_x_, cover_flange1_inner_y_, cover_flange1_inner_z_, cover_flange1_inner_box_z_pos_;
    const G4double endcap_cyl_in_rad_, endcap_cyl_out_rad_, endcap_cyl_length_, endcap_cyl_z_pos_;
    const G4double endcap_cone_in_rad_zneg_, endcap_cone_out_rad_zneg_, endcap_cone_out_rad_zpos_, endcap_cone_depth_, endcap_cone_z_pos_; 
    const G4double gas_endcap_cone_depth_, gas_endcap_cone_z_pos_;

    // Detector limits
    const G4double detector_z_lim_;


    // Gas properties
    G4String gas_;
    G4double pressure_, temperature_;

    // Vertex generators
    CylinderPointSampler* body_gen_; 
    CylinderPointSampler* flange1_gen_; 
    CylinderPointSampler* flange2_gen_;
    CylinderPointSampler* endcap_cyl1_gen_;
    CylinderPointSampler* endcap_cyl2_gen_;
    ConePointSampler* endcap_cone1_gen_;
    ConePointSampler* endcap_cone2_gen_;
    CylinderPointSampler* cover_flange1_gen_;
    CylinderPointSampler* cover_flange2_gen_;

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
