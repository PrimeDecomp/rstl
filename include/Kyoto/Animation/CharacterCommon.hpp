#ifndef _CHARACTERCOMMON
#define _CHARACTERCOMMON
#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

namespace pas {
enum ELocomotionType {
  kLT_Invalid = -1,
  kLT_Crouch = 0,
  kLT_Relaxed = 1,
  kLT_Lurk = 2,
  kLT_Combat = 3,
  kLT_Internal4 = 4,
  kLT_Internal5 = 5,
  kLT_Internal6 = 6,
  kLT_Internal7 = 7,
  kLT_Internal8 = 8,
  kLT_Internal9 = 9,
  kLT_Internal10 = 10,
  kLT_Internal11 = 11,
  kLT_Internal12 = 12,
  kLT_Internal13 = 13,
#if RSTL_CORRUPTION
  kLT_Internal14 = 14,
  kLT_Internal15 = 15,
  kLT_Internal16 = 16,
  kLT_Internal17 = 17,
  kLT_Internal18 = 18,
  kLT_Internal19 = 19,
  kLT_Internal20 = 20,
  kLT_Internal21 = 21,
  kLT_Internal22 = 22,
  kLT_Internal23 = 23
#else
  kLT_Internal14 = 14
#endif
};

enum ELocomotionAnim {
  kLA_Invalid = -1,
  kLA_Idle,
  kLA_Walk,
  kLA_Run,
  kLA_BackUp,
  kLA_StrafeLeft,
  kLA_StrafeRight,
  kLA_StrafeUp,
  kLA_StrafeDown
};

enum EAnimationState {
  kAS_Invalid = -1,
  kAS_Fall = 0,
  kAS_Getup = 1,
  kAS_LieOnGround = 2,
  kAS_Step = 3,
  kAS_Death = 4,
  kAS_Locomotion = 5,
  kAS_KnockBack = 6,
  kAS_MeleeAttack = 7,
  kAS_Turn = 8,
  kAS_LoopAttack = 9,
  kAS_LoopReaction = 10,
  kAS_GroundHit = 11,
  kAS_Generate = 12,
  kAS_Jump = 13,
  kAS_Hurled = 14,
  kAS_Slide = 15,
  kAS_Taunt = 16,
  kAS_Scripted = 17,
  kAS_ProjectileAttack = 18,
  kAS_Cover = 19,
  kAS_WallHang = 20,
  kAS_AdditiveIdle = 21,
  kAS_AdditiveAim = 22,
  kAS_AdditiveFlinch = 23,
#if RSTL_CORRUPTION
  kAS_AdditiveReaction = 24,
  kAS_AdditiveDirectionalReaction = 25, // Guessed name
  kAS_Unknown26 = 26,
  kAS_AdditiveLoopReaction = 27, // Guessed name
  kAS_CodeDriven = 28,           // Guessed name
#else
  kAS_AdditiveReaction = 24
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  kAnimState_FirstValid = kAS_Fall,
  kAnimState_LastValid = kAS_CodeDriven
#else
#if !RSTL_PRIME1
  , kAS_AdditiveDirectionalReaction = 25 // Guessed name
  , kAS_Unknown26 = 26
  , kAS_Unknown27 = 27
  , kAS_AdditiveLoopReaction = 28 // Guessed name
#endif
#endif
};
#if RSTL_CORRUPTION

// Guessed name
const char* GetAnimationStateName(EAnimationState state);
#endif

enum EHurledState {
  kHS_Invalid = -1,
  kHS_KnockIntoAir,
  kHS_KnockLoop,
  kHS_KnockDown,
  kHS_StrikeWall,
  kHS_StrikeWallFallLoop,
  kHS_OutOfStrikeWall,
  kHS_RecoverFromKnockLoop, // Guessed name
  kHS_RecoverFromStrikeWall // Guessed name
};

enum EFallState { kFS_Invalid = -1, kFS_Zero, kFS_One, kFS_Two };

enum EReactionType { kRT_Invalid = -1, kRT_Zero, kRT_One, kRT_Two, kRT_Three };

enum EAdditiveReactionType {
  kART_Invalid = -1,
  kART_Electrocution,
  kART_One,
  kART_Two,
  kART_IceBreakout,
  kART_Four,
  kART_Five,
  kART_Six,
  kART_Seven
};

enum EJumpType {
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  kJT_Invalid = -1,
#endif
  kJT_Normal,
  kJT_One,
  kJT_Ambush
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kJT_Three
#endif
};

enum EJumpState {
  kJS_Invalid = -1,
  kJS_IntoJump,
  kJS_AmbushJump,
  kJS_Loop,
  kJS_OutOfJump,
  kJS_WallBounceLeft,
  kJS_WallBounceRight
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kJS_ExitJump // Guessed name
#endif
};

enum EStepDirection {
  kSD_Invalid = -1,
  kSD_Forward = 0,
  kSD_Backward = 1,
  kSD_Left = 2,
  kSD_Right = 3,
  kSD_Up = 4,
#if RSTL_CORRUPTION
  kSD_Down = 5,
  kSD_Six = 6,  // Guessed name
  kSD_Seven = 7, // Guessed name
  kSD_Eight = 8 // Guessed name
#else
  kSD_Down = 5
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kSD_ForwardLeft = 6 // Guessed name
  , kSD_ForwardRight = 7 // Guessed name
  , kSD_BackwardLeft = 8 // Guessed name
  , kSD_BackwardRight = 9 // Guessed name
#endif
#endif
};

enum EStepType { kStep_Normal = 0, kStep_Dodge = 1, kStep_BreakDodge = 2, kStep_RollDodge = 3 };

enum ESeverity {
  kS_Invalid = -1,
  kS_Zero = 0,
  kS_One = 1,
  kS_Two = 2,
  kS_Three = 3,
  kS_Four = 4,
  kS_Five = 5,
  kS_Six = 6,
  kS_Seven = 7,
  kS_Eight = 8
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kS_Nine = 9
  , kS_Ten = 10
  , kS_Eleven = 11
#endif
};

enum EGetupType { kGetup_Invalid = -1, kGetup_Zero = 0, kGetup_One = 1, kGetup_Two = 2 };

enum ELoopState { kLS_Invalid = -1, kLS_Begin, kLS_Loop, kLS_End };

#if RSTL_CORRUPTION
enum ELoopAttackType { kLAT_Invalid = -1, kLAT_Zero, kLAT_One, kLAT_Two, kLAT_Three, kLAT_Four }; // kLAT_Four is a guessed name
#else
enum ELoopAttackType {
  kLAT_Invalid = -1,
  kLAT_Zero,
  kLAT_One,
  kLAT_Two,
  kLAT_Three
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kLAT_Four
  , kLAT_Five
  , kLAT_Six
#endif
};
#endif

enum EGenerateType {
  kGType_Invalid = -1,
  kGType_Zero,
  kGType_One,
  kGType_Two,
  kGType_Three,
  kGType_Four,
  kGType_Five,
  kGType_Six,
  kGType_Seven,
#if RSTL_CORRUPTION
  kGType_Eight,
  kGType_Nine,
  kGType_Ten,
  kGType_Eleven
#else
  kGType_Eight
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kGType_Nine // Guessed name
#endif
#endif
};

enum ESlideType { kSlide_Invalid = -1, kSlide_Zero = 0 };

enum ETauntType {
  kTT_Invalid = -1,
  kTT_Zero,
  kTT_One,
#if RSTL_CORRUPTION
  kTT_Two,
  kTT_Three,
  kTT_Four,
  kTT_Five,
  kTT_Six,
  kTT_Seven,
  kTT_Eight,
  kTT_Nine,
  kTT_Ten,
  kTT_Eleven,
  kTT_Twelve,
  kTT_Thirteen,
  kTT_Fourteen
#else
  kTT_Two
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kTT_Three // Guessed name
  , kTT_Four // Guessed name; Rezbit alert taunt.
  , kTT_Five // Guessed name
  , kTT_Six = 6 // Guessed name
  , kTT_Seven // Guessed name
  , kTT_Eight // Guessed name
  , kTT_Nine = 9 // Guessed name
  , kTT_Ten // Guessed name
  , kTT_Eleven // Guessed name
#endif
#endif
};

enum ECoverState { kCS_Invalid = -1, kCS_IntoCover, kCS_Cover, kCS_Lean, kCS_OutOfCover };

enum ECoverDirection { kCD_Invalid = -1, kCD_Left, kCD_Right };

enum ETurnDirection { kTD_Invalid = -1, kTD_Right, kTD_Left };

enum EWallHangState {
  kWHS_Invalid = -1,
  kWHS_IntoJump,
  kWHS_JumpArc,
  kWHS_JumpAirLoop,
  kWHS_IntoWallHang,
  kWHS_WallHang,
  kWHS_Five,
  kWHS_OutOfWallHang,
  kWHS_OutOfWallHangTurn,
  kWHS_DetachJumpLoop,
  kWHS_DetachOutOfJump
};
} // namespace pas

enum EBodyType {
  kBT_Invalid,
  kBT_BiPedal,
  kBT_Restricted,
  kBT_Flyer,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  kBT_Pitchable,
  kBT_RestrictedFlyer,
#else
  kBT_PitchableFlyer, // Cross-game name (DKCR HD); Echoes factory corroborated.
  kBT_Floater,        // Cross-game name (DKCR HD); Echoes factory corroborated.
#endif
  kBT_WallWalker,
#if RSTL_CORRUPTION
  kBT_AiMovedFlyer, // Cross-game name (DKCR HD); Echoes factory corroborated.
  kBT_4WayBlended,  // Guessed name from DKCR HD; Echoes blends four movement directions.
  kBT_Unknown9
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  kBT_NewFlyer
#else
  kBT_AiMovedFlyer, // Cross-game name (DKCR HD); Echoes factory corroborated.
  kBT_4WayBlended   // Guessed name from DKCR HD; Echoes blends four movement directions.
#endif
#endif
};

enum EBodyStateCmd {
  kBSC_Getup,
  kBSC_Step,
  kBSC_Die,
  kBSC_KnockDown,
  kBSC_KnockBack,
  kBSC_MeleeAttack,
  kBSC_ProjectileAttack,
  kBSC_LoopAttack,
#if RSTL_CORRUPTION
  kBSC_Unknown8, // Guessed name
#endif
  kBSC_LoopReaction,
  kBSC_LoopHitReaction,
  kBSC_ExitState,
  kBSC_LeanFromCover,
  kBSC_NextState,
#if !RSTL_PRIME1
  kBSC_AbortScripted, // Guessed name
#endif
  kBSC_MaintainVelocity,
  kBSC_Generate,
  kBSC_Hurled,
  kBSC_Jump,
#if RSTL_CORRUPTION
  kBSC_Unknown19,
  kBSC_Unknown20,
#else
#if !RSTL_PRIME1
  kBSC_Unknown18,
  kBSC_Unknown19,
#endif
#endif
  kBSC_Slide,
  kBSC_Taunt,
  kBSC_Scripted,
  kBSC_Cover,
  kBSC_WallHang,
  kBSC_Locomotion,
#if RSTL_CORRUPTION
  kBSC_Unknown27,
  kBSC_Unknown28,
#endif
  kBSC_AdditiveIdle,
  kBSC_AdditiveAim,
  kBSC_AdditiveFlinch,
  kBSC_AdditiveReaction,
#if RSTL_CORRUPTION
  kBSC_StopReaction,
  kBSC_AdditiveLoopReaction, // Guessed name
  kBSC_AdditiveWeight,       // Guessed name
  kBSC_Unknown36,
  kBSC_Unknown37
#else
  kBSC_StopReaction
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , kBSC_AdditiveLoopReaction // Guessed name
  , kBSC_AdditiveWeight // Guessed name
  , kBSC_Unknown33
#endif
#endif
};

#endif // _CHARACTERCOMMON
