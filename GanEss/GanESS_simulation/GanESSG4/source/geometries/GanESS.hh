// ----------------------------------------------------------------------------
// nexus | GanESS.hh
//
// Main class that constructs the geometry of the GanESS detector.
//
// The GanESS Group
// ----------------------------------------------------------------------------

#ifndef GANESS_HH
#define GANESS_HH

#include "nexus/GeometryBase.h"

class G4LogicalVolume;
class G4GenericMessenger;

namespace nexus {

    class GanESSVessel;

  class GanESS : public GeometryBase
  {
  public:
    GanESS();
    ~GanESS();

    /// Generate a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

  private:
    void Construct();

  private:
    /// Size of the laboratory volume
    const G4double lab_size_;

    /// Pointers to logical volumes
    G4LogicalVolume* lab_logic_;

    GanESSVessel* vessel_;

    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif // GANESS_HH