#include "Kyoto/CAssetTypesList.hpp"
#include "Kyoto/Alloc/Assert.hpp"

#include <ctype.h>

static const unsigned int kAssetTypes[] = {
    'CLSN', 'CSPP', 'CMDL', 'CSKR', 'ANIM', 'CINF', 'TXTR', 'PLTT', 'FONT', 'MADF', 'MLVL',
    'MREA', 'MAPW', 'MAPA', 'SAVW', 'SAVA', 'PART', 'WPSC', 'SWHC', 'DPSC', 'ELSC', 'CRSC',
    'SPSC', 'SRSC', 'USRC', 'BFRC', 'AFSM', 'DCLN', 'AGSC', 'ATBL', 'CSNG', 'STRG', 'SCAN',
    'PATH', 'DGRP', 'HMAP', 'PTLA', 'STLC', 'EGMC', 'RULE', 'FSM2', 'SAND', 'CHAR', 'RSMP',
    'CSMP', 'APRJ', 'RAUD', 'CAUD', 'CTWK', 'FRME', 'HINT', 'MAPU', 'DUMB', 'CSMP',
};

struct STypeDescriptor {
  const char* description;
  const char* name;
  const char* extension;
  const char* command;
};

STypeDescriptor skEditorTypes[] = {
    {
        "Invalid type",
        "kInvalid",
        "----",
    },
    {
        "Maya native binary file",
        "kMayaBinary",
        "mb",
    },
    {
        "Raw texture file",
        "kRawTexture",
        "tga:jpg:jpeg",
    },
    {
        "Intermediate format data",
        "kTransport",
        "rtr",
        "%ASSETCOOKER%",
    },
    {
        "Raw animation data",
        "kRawAnimation",
        "artr",
    },
    {
        "Raw portal data",
        "kRawPortals",
        "prtr",
    },
    {
        "Base character pose",
        "kBasePose",
        "base",
    },
    {
        "Character Segment Master List",
        "kSML",
        "sml",
    },
    {
        "Character body parts 'to keep' list",
        "kBodyPartsToKeepList",
        "bptk",
    },
    {
        "Raw collision data",
        "kRawCollision",
        "crtr",
    },
    {
        "Cooked collision data",
        "kCookedCollision",
        "clsn",
    },
    {
        "Locator data",
        "kLocator",
        "lctr",
    },
    {
        "Cooked spatial primitive",
        "kCookedSpatialPrimitives",
        "cspp",
    },
    {
        "CModel instance",
        "kCModel",
        "cmdl",
    },
    {
        "Character skinning rules",
        "kCSkinRules",
        "cskr",
    },
    {
        "Texture MipMap data",
        "kCTexture",
        "txtr",
    },
    {
        "Character animation data",
        "kCharAnimData",
        "ani",
    },
    {
        "Character Information data",
        "kCharInfoData",
        "cin",
    },
    {
        "RGraphics color palette",
        "kCPalette",
        "PLTT",
    },
    {
        "RFont raster font",
        "kRasterFont",
        "rpff",
    },
    {
        "Animation Character Set",
        "kCAnimCharacterSet",
        "acs",
    },
    {
        "Raw Animation Set data",
        "kAnimationSet",
        "anim_set",
    },
    {
        "Character Editor file",
        "kCharacterSetRaw",
        "ced",
    },
    {
        "Meta-animation Data File",
        "kMetaAnimationData",
        "mad",
    },
    {
        "World Editor 2 Data File",
        "kWorldEditor",
        "we2:wld",
    },
    {
        "Metroid World",
        "kMetroidWorld",
        "mwld",
        "%WORLDCOOKER%",
    },
    {
        "Metroid Area",
        "kMetroidArea",
        "mrea",
    },
    {
        "Metroid World Map",
        "kMetroidWorldMap",
        "mapw",
    },
    {
        "Metroid Area Map",
        "kMetroidAreaMap",
        "mapa",
    },
    {
        "Metroid World Save Info",
        "kMetroidWorldSaveInfo",
        "savw",
    },
    {
        "Metroid Area Save Info",
        "kMetroidAreaSaveInfo",
        "sava",
    },
    {
        "Particle System",
        "kParticleGen",
        "part",
    },
    {
        "Weapon Particle System",
        "kWeaponParticleSystem",
        "wpsc",
    },
    {
        "Swoosh Particle System",
        "kSwooshParticleSystem",
        "swhc",
    },
    {
        "Decal Particle System",
        "kDecalParticleSystem",
        "dpsc",
    },
    {
        "Electric Particle System",
        "kElectricParticleSystem",
        "elsc",
    },
    {
        "Collision Response Particle System",
        "kCollisionResponseParticleSystem",
        "crsc",
    },
    {
        "Spawn Particle System",
        "kSpawnParticleSystem",
        "spsc",
    },
    {
        "Sorted Particle System",
        "kSortedParticleSystem",
        "srsc",
    },
    {
        "Parameterized Animation State Data",
        "kParameterEditorData",
        "pas_def:pdef",
    },
    {
        "Ai Finite State Machine Raw",
        "kAiFiniteStateMachineRaw",
        "fsm",
    },
    {
        "Ai Finite State Machine",
        "kAiFiniteStateMachine",
        "afsm",
    },
    {
        "New Finite State Machine",
        "kFiniteStateMachine2",
        "fsm2",
    },
    {
        "Dynamic Collision Data",
        "kDynamicCollision",
        "dcln",
    },
    {
        "Cooked Audio Group Set",
        "kAudioGroupSetCooked",
        "agsc",
    },
    {
        "Raw Audio Group Set",
        "kAudioGroupSetRaw",
        "agsr",
    },
    {
        "Reference Lights",
        "kReferenceLights",
        "refLight",
    },
    {
        "Scripting layer",
        "kScriptingLayer",
        "sl",
    },
    {
        "Potential Visible Set",
        "kPVS",
        "vis",
    },
    {
        "Pathfinding layer 2",
        "kPathfindingLayer",
        "pfl2",
        "NULL",
    },
    {
        "Pathfinding",
        "kPathfinding",
        "path",
    },
    {
        "Portal Area",
        "kPortalArea",
        "ptla",
    },
    {
        "Asset Dependency List",
        "kDependencyGroup",
        "dgrp",
    },
    {
        "Heightmap",
        "kHeightmap",
        "hght",
    },
    {
        "RGL compatible static-geometry object",
        "kOldDrawableObj",
        "drob",
    },
    {
        "RGL compatible skinned object",
        "kOldSkinnedObj",
        "skob",
    },
    {
        "RGL compatible material settings",
        "kOldResMaterial",
        "rmtt",
    },
    {
        "RGL compatible SceneGraph portal data",
        "kOldResPortalGraph",
        "rptg",
    },
    {
        "RGL compatible shader resource",
        "kOldResShader",
        "shader",
    },
    {
        "RGL compatible texture resource",
        "kOldResTexture",
        "rtex",
    },
    {
        "Audio Lookup Table",
        "kAudioLookupTable",
        "atbl",
    },
    {
        "MusyX Midi file",
        "kMusyXSong",
        "song",
    },
    {
        "Cooked MusyX Midi file",
        "kMusyXCookedSong",
        "csng",
    },
    {
        "Stringtable Asset",
        "kStringTable",
        "strg",
    },
    {
        "Scannable Asset",
        "kScannable",
        "scan",
    },
    {
        "Gui Frame",
        "kGuiFrame",
        "frme",
    },
    {
        "Raw String List",
        "kRawStringList",
        "stlr",
    },
    {
        "Cooked String List",
        "kStringList",
        "stlc",
    },
    {
        "Raw EditorObject to StaticGeometry Map",
        "kEditorGeometryMapRaw",
        "egmr",
    },
    {
        "Editor Object to Static Geometry Map",
        "egmc",
    },
    {
        "Generic XML file",
        "kXMLRaw",
        "xml",
    },
    {
        "Cooked Rule Asset",
        "kRuleCooked",
        "rule",
    },
    {
        "Cooked Character Data",
        "kCookedCharacterData",
        "char",
    },
    {
        "Cooked Source Animation Data",
        "kCookedSourceAnimationData",
        "sand",
    },
    {
        "Geometry Master List",
        "kGeometryMasterList",
        "gml",
    },
    {
        "Raw Audio file",
        "kRawAudioFile",
        "wav:aif:aiff:mp3:ogg",
    },
    {
        "Cooked Audio Data",
        "kCookedAudioFile",
        "csmp",
    },
    {
        "Audio Manager Project",
        "kAudioManagerProject",
        "aprj",
    },
    {
        "Audio Manager macro",
        "kRawAudioManagerGOB",
        "gob",
    },
    {
        "Cooked Audio Manager macro",
        "kCookedAudioManagerGOB",
        "caud",
    },
};

static const uint32 kNumAssetTypes = sizeof(kAssetTypes) / sizeof(kAssetTypes[0]);
void hack() {
  static const char* const unused1 = "";
  static const char* const unused2 = "/\\:";
  static const char* const unusedAssert1 = "CAssetTypesList.cpp(190) : ";
  static const char* const unusedAssertMsg1 = "pFileExtensList != NULL";
  static const char* const unusedAssertMsg2 = "can't init from NULL pointer";
  static const char* const unusedAssert2 = "CAssetTypesList.cpp(191) : ";
  static const char* const unusedAssertMsg3 = "pDescript != NULL";
}
unsigned int CAssetTypesList::GetFourCCForIndex(unsigned int index) {
  RS_VERIFY_THROW(419, ((uint32)index) < kNumAssetTypes, false,
                  "Invalid index into asset types list in CAssetTypesList::GetFourCCForIndex.");
  return kAssetTypes[index];
}

unsigned int CAssetTypesList::GetIndexForFourCC(unsigned int fourCC) {
  char* chars = reinterpret_cast< char* >(&fourCC);
  chars[0] = toupper(chars[0]);
  chars[1] = toupper(chars[1]);
  chars[2] = toupper(chars[2]);
  chars[3] = toupper(chars[3]);
  for (unsigned int i = 0; i < kNumAssetTypes; ++i) {
    if (fourCC == kAssetTypes[i]) {
      return i;
    }
  }
  return kInvalid;
}

