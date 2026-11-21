#include "Kyoto/Animation/CharacterCommon.hpp"


namespace pas {
static const char* skAnimationStateNames[] = {"Fall",
                                              "Getup",
                                              "LieOnGround",
                                              "Step",
                                              "Die",
                                              "Locomotion",
                                              "KnockBack",
                                              "MeleeAttack",
                                              "Turn",
                                              "LoopedAttack",
                                              "LoopedReaction",
                                              "GroundHit",
                                              "Generate",
                                              "Jump",
                                              "Hurled",
                                              "Slide",
                                              "Taunt",
                                              "Scripted",
                                              "ProjectileAttack",
                                              "Cover",
                                              "WallHang",
                                              "AdditiveIdle",
                                              "AdditiveAim",
                                              "AdditiveFlinch",
                                              "AdditiveReaction",
                                              "AdditiveHitReaction",
                                              "AdditiveAttack",
                                              "AdditivePose",
                                              "CodeDriven",
                                              0};

const char* GetAnimationStateName(EAnimationState state) { return skAnimationStateNames[state]; }
} // namespace pas
