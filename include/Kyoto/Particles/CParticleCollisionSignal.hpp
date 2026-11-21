#ifndef _CPARTICLECOLLISIONSIGNAL
#define _CPARTICLECOLLISIONSIGNAL

#include "Kyoto/TSignal4.hpp"



class CVector3f;

// Guessed names. Particle collision callbacks modify the step and position, with restitution
// and damping supplied by the particle system. The native global is at 0x80782D38.
typedef TSignal4< void, CVector3f&, CVector3f&, float, float > TParticleCollisionSignal;
extern TParticleCollisionSignal gParticleCollisionSignal;


#endif // _CPARTICLECOLLISIONSIGNAL
