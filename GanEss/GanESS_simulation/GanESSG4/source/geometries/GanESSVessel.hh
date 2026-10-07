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
    const G4double vessel_in_rad_, vessel_thickness_;
    const G4double body_length_;

    // Gas properties
    G4String gas_;
    G4double pressure_, temperature_;

    // Vertex generators
    CylinderPointSampler* body_gen_;

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };

} // end namespace nexus

#endif
