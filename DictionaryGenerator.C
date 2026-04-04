#include "/opt/allpix/include/objects/MCParticle.hpp"
#include "/opt/allpix/include/objects/MCTrack.hpp"
#include "/opt/allpix/include/objects/PixelCharge.hpp"
#include "/opt/allpix/include/objects/PixelHit.hpp"
#include "/opt/allpix/include/objects/DepositedCharge.hpp"
#include "/opt/allpix/include/objects/PropagatedCharge.hpp"

#ifdef __CLING__
#pragma link C++ class std::vector<allpix::MCParticle*>+;
#pragma link C++ class std::vector<allpix::MCTrack*>+;
#pragma link C++ class std::vector<allpix::DepositedCharge*>+;
#pragma link C++ class std::vector<allpix::PropagatedCharge*>+;
#pragma link C++ class std::vector<allpix::PixelCharge*>+;
#pragma link C++ class std::vector<allpix::PixelHit*>+;
#endif

