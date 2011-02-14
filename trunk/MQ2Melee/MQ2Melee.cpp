//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2Melee.cpp     |
// Author: s0rCieR          |
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// SHOW_ABILITY:    0=0ff, 1=Display every ability that plugin use.
// SHOW_ATTACKING:  0=0ff, 1=Display Attacking Target
// SHOW_ENRAGING:   0=0ff, 1=Display Enrage/Infuriate
// SHOW_FEIGN:      0=0ff, 1=Display Fallen Detected
// SHOW_OVERRIDE:   0=0ff, 1=Display Override Warning
// SHOW_STICKING:   0=0ff, 1=Display Stick Arguments
// SHOW_SWITCHING:  0=0ff, 1=Display Switch Melee/Range
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#define   PLUGIN_NAME  "MQ2Melee"   // Plugin Name
#define   PLUGIN_DATE   20061027    // Plugin Date
#define   PLUGIN_VERS      4.482    // Plugin Version
#define   NPC_TYPE        0x000A    // NPC TYPE

#define		MELEE_KEY					  "a"		// Plugin Melee Key (Not same as eq please)
#define		RANGE_KEY					  "z"   // Plugin Range Key

#define   SHOW_ABILITY         0
#define   SHOW_ATTACKING       1
#define   SHOW_CASTING         0
#define   SHOW_CONTROL         0
#define   SHOW_ENRAGING        1
#define   SHOW_FEIGN           1
#define   SHOW_OVERRIDE        1
#define   SHOW_PROVOKING       0
#define   SHOW_STICKING        1
#define   SHOW_STUNNING        0
#define   SHOW_SWITCHING       1

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#ifndef PLUGIN_API
  #include "../MQ2Plugin.h"
  PreSetup(PLUGIN_NAME);
  PLUGIN_VERSION(PLUGIN_VERS);
  #include <map>
  #include <string>
  #include "../Blech/Blech.h"
#endif PLUGIN_API

#define NOID                   -1
#define delay                 250

#define d_assassin1         10898
#define d_assassin2         10899
#define d_assassin3         10900
#define d_ashenhand          4508
#define d_heelofkanji        8473
#define d_silentfist         4507
#define d_thunderkick        4511

#define i_disarm               16
#define i_forage               27
#define i_harmtouch           105
#define i_intimidation         71
#define i_kick                 30
#define i_layhand             107
#define i_mend                 32
#define i_taunt                73

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

struct    infodata {long i,t; } *pinfodata;

infodata  callchal={552   ,4},  // aa: call of challenge
          escape  ={102   ,4},  // aa: escape
          feignid ={420   ,4},  // aa: imitate death
          feigndp ={428   ,4},  // aa: death peace
          feral1  ={247   ,4},  // aa: feral swipe
          feral2  ={647   ,4},  // aa: jagged claws
          joltbst1={362   ,4},  // aa: roar of thunder
          joltbst2={648   ,4},  // aa: roar of rolling thunder
          mendpet1={58    ,4},  // aa: mend companion
          mendpet2={418   ,4},  // aa: replenish companion
          stunmnk1={469   ,4},  // aa: stunning kick
          stunmnk2={600   ,4},  // aa: resounding kick
          stunpal1={73    ,4},  // aa: divine stun
          stunpal2={431   ,4},  // aa: celestial stun
          stunpal3={702   ,4},  // aa: hand of disruption
          twisted ={670   ,4},  // aa: twisted shank

          cmmding ={8000  ,3},  // disc: commanding voice
          cryhavoc={8003  ,3},  // disc: cry havoc
          fistswu ={8002  ,3},  // disc: fists of wu
          joltber1={4934  ,3},  // disc: diversive strike
          joltber2={4935  ,3},  // disc: distracting strike
          joltber3={4936  ,3},  // disc: confusing strike
          joltber4={6171  ,3},  // disc: baffling strike
          joltber5={10920 ,3},  // disc: jarring strike
          joltber6={10921 ,3},  // disc: jarring strike rk ii
          joltber7={10922 ,3},  // disc: jarring strike rk iii
          leop1   ={6752  ,3},  // disc: leopard claw
          leop2   ={6727  ,3},  // disc: dragon fang
          leop3   ={10944 ,3},  // disc: clawstriker flurry
          leop4   ={10945 ,3},  // disc: clawstriker flurry rk ii
          leop5   ={10946 ,3},  // disc: clawstriker flurry rk iii
          prowar_a={4608  ,3},  // disc: provoke
          prowar_b={4681  ,3},  // disc: bellow
          prowar_c={4682  ,3},  // disc: berate
          prowar_d={4697  ,3},  // disc: incite
          prowar_e={5015  ,3},  // disc: bellow of the mastruq
          prowar_f={5016  ,3},  // disc: ancient: chaos cry
          prowar_g={6173  ,3},  // disc: bazu bellow
          prowar_h={10974 ,3},  // disc: scowl
          prowar_i={10975 ,3},  // disc: scowl rk ii
          prowar_j={10976 ,3},  // disc: scowl rk iii
          rake    ={8782  ,3},  // disc: rake
          stunber1={4931  ,3},  // disc: head strike
          stunber2={4932  ,3},  // disc: head pummel
          stunber3={4933  ,3},  // disc: head crush
          stunber4={6170  ,3},  // disc: mind strike
          stunber5={10917 ,3},  // disc: temple blow
          stunber6={10918 ,3},  // disc: temple blow rk ii
          stunber7={10919 ,3},  // disc: temple blow rk iii
          thiefeye={8001  ,3},  // disc: thief's eye
          tstone  ={5225  ,3},  // disc: throw stone
          volley1 ={6754  ,3},  // disc: rage volley
          volley2 ={6729  ,3},  // disc: destroyer's volley
          volley3 ={10926 ,3},  // disc: giant slayer's volley
          volley4 ={10927 ,3},  // disc: giant slayer's volley rk ii
          volley5 ={10928 ,3},  // disc: giant slayer's volley rk iii
          strike1 ={4659  ,3},  // disc: sneak attack
          strike2 ={4685  ,3},  // disc: theif's vengeance
          strike3 ={4686  ,3},  // disc: assasin strike
          strike4 ={5017  ,3},  // disc: kyv strike
          strike5 ={5018  ,3},  // disc: ancient chaos strike
          strike6 ={6174  ,3},  // disc: daggerfall
          strike7 ={8470  ,3},  // disc: razor arc
          potfast0={77789 ,7},  // potion: Distillate of Divine Healing I
          potfast1={77790 ,7},  // potion: Distillate of Divine Healing II
          potfast2={77791 ,7},  // potion: Distillate of Divine Healing III
          potfast3={77792 ,7},  // potion: Distillate of Divine Healing IV
          potfast4={77793 ,7},  // potion: Distillate of Divine Healing V
          potfast5={77794 ,7},  // potion: Distillate of Divine Healing VI
          potfast6={77795 ,7},  // potion: Distillate of Divine Healing VII
          potfast7={77796 ,7},  // potion: Distillate of Divine Healing VIII
          potfast8={77797 ,7},  // potion: Distillate of Divine Healing IX
          potfast9={77798 ,7},  // potion: Distillate of Divine Healing X
          potover0={77779 ,7},  // potion: Distillate of Celestial Healing I 
          potover1={77780 ,7},  // potion: Distillate of Celestial Healing II 
          potover2={77781 ,7},  // potion: Distillate of Celestial Healing III 
          potover3={77782 ,7},  // potion: Distillate of Celestial Healing IV
          potover4={77783 ,7},  // potion: Distillate of Celestial Healing V
          potover5={77784 ,7},  // potion: Distillate of Celestial Healing VI
          potover6={77785 ,7},  // potion: Distillate of Celestial Healing VII 
          potover7={77786 ,7},  // potion: Distillate of Celestial Healing VIII
          potover8={77787 ,7},  // potion: Distillate of Celestial Healing IX
          potover9={77788 ,7},  // potion: Distillate of Celestial Healing X

          sbkstab ={8     ,2},  // skill: backstab
          sbash   ={10    ,2},  // skill: bash
          sbegging={67    ,2},  // skill: begging
          sdisarm ={16    ,2},  // skill: disarm
          sdrpunch={21    ,2},  // skill: dragon punch
          sestrike={23    ,2},  // skill: eagle strike
          sfeign  ={25    ,2},  // skill: feign death
          sflykick={26    ,2},  // skill: flying kick
          sforage ={27    ,2},  // skill: forage
          sfrenzy ={74    ,2},  // skill: frenzy
          sharmtou={105   ,2},  // skill: harmtouch
          shide   ={29    ,2},  // skill: hide
          sintim  ={71    ,2},  // skill: intimidation
          skick   ={30    ,2},  // skill: kick
          slayhand={107   ,2},  // skill: layhand
          smend   ={32    ,2},  // skill: mend
          sppocket={48    ,2},  // skill: pick pockets
          srndkick={38    ,2},  // skill: round kick
          ssensetr={62    ,2},  // skill: sense trap
          sslam   ={111   ,2},  // skill: slam
          ssneak  ={42    ,2},  // skill: sneak
          staunt  ={73    ,2},  // skill: taunt
          stigclaw={52    ,2},  // skill: tigerclaw
          
          feigns1 ={366   ,5},  // spell: feign death
          feigns2 ={3685  ,5},  // spell: comatose
          feigns3 ={1460  ,5},  // spell: death peace
          feigns4 ={10306 ,5},  // spell: last breath
          feigns5 ={10307 ,5},  // spell: last breath rk ii
          feigns6 ={10308 ,5},  // spell: last breath rk iii
          honora  ={10173 ,5},  // spell: challenge for honor
          honorb  ={10174 ,5},  // spell: challenge for honor rk ii
          honorc  ={10175 ,5},  // spell: challenge for honor rk iii
          joltrng1={1741  ,5},  // spell: jolt
          joltrng2={1296  ,5},  // spell: cinder jolt
          powera  ={10260 ,5},  // spell: challenge for power 
          powerb  ={10261 ,5},  // spell: challenge for power rk ii
          powerc  ={10262 ,5},  // spell: challenge for power rk iii
          stunpala={216   ,5},  // spell: stun
          stunpalb={123   ,5},  // spell: holy might
          stunpalc={3975  ,5},  // spell: force of akera
          stunpald={3245  ,5},  // spell: force of akilea
          stunpale={4977  ,5},  // spell: ancient force of chaos
          stunpalf={5284  ,5},  // spell: force of piety
          stunpalg={5299  ,5},  // spell: ancient force of jeron
          stunpalh={10158 ,5},  // spell: sacred force
          stunpali={10159 ,5},  // spell: sacred force rk ii
          stunpalj={10160 ,5},  // spell: sacred force rk iii
          terror1 ={1223  ,5},  // spell: terror of death
          terror2 ={1224  ,5},  // spell: terror of terris
          terror3 ={3405  ,5},  // spell: terror of thule
          terror4 ={5329  ,5},  // spell: terror of discord
          terror5 ={10257 ,5},  // spell: terror of vergalid
          terror6 ={10258 ,5},  // spell: terror of vergalid rk ii
          terror7 ={10259 ,5};  // spell: terror of vergalid rk iii

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PCHAR pAGGRO[]={
  "aggro",
  "[ON/OFF]?",
  "${If[${Select[${Me.Class.ShortName},WAR,PAL,SHD]},1,0]}",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pAGGRP[]={
  "aggropri",
  "[ID] Primary (Aggro)?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${meleemvi[aggro]},1,0]}",
};

PCHAR pAGGRS[]={
  "aggrosec",
  "[ID] Offhand (Aggro)?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${meleemvi[aggro]} && ${meleemvi[aggropri]},1,0]}",
};

PCHAR pARROW[]={
  "arrow",
  "[ID] item?",
  "0",
  "${If[${meleemvi[plugin]} && (${Me.Skill[archery]} || ${Me.Skill[throwing]}),1,0]}",
};

PCHAR pASSAS[]={
  "assasinate",
  "Sneak/Hide/Behind/Strike/Stab [ON/OFF]?",
  "${If[${Me.Skill[backstab]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${meleemvi[backstab]},1,0]}",
};

PCHAR pBKOFF[]={
  "backoff",
  "[#] Life% Below? 0=0ff",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && !${meleemvi[aggro]},1,0]}",
};

PCHAR pBSTAB[]={
  "backstab",
  "[ON/OFF]?",
  "${If[${Me.Skill[backstab]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[backstab]},1,0]}",
};

PCHAR pBASHS[]={
  "bash",
  "[#] Bash 0=0ff",
  "${If[${Me.Skill[bash]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[bash]},1,0]}",
};

PCHAR pBGING[]={
  "begging",
  "[ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[begging]},1,0]}",
};

PCHAR pBOWID[]={
  "bow",
  "[ID] spell/disc/aa/item?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[archery]},1,0]}",
};

PCHAR pCALLC[]={
  "callchallenge",
  "[ON/OFF]?",
  "${If[${Me.AltAbility[call of challenge]},1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.AltAbility[call of challenge]},1,0]}",
};

PCHAR pCHFOR[]={
  "challengefor",
  "[ON/OFF]?",
  "${If[${Me.Book[challenge for honor]} || ${Me.Book[challenge for honor rk. ii]} || ${Me.Book[challenge for honor rk. iii]} || ${Me.Book[challenge for power]} || ${Me.Book[challenge for power rk. ii]} || ${Me.Book[challenge for power rk. iii]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[aggro]} && (${Me.Book[challenge for honor]} || ${Me.Book[challenge for honor rk. ii]} || ${Me.Book[challenge for honor rk. iii]} || ${Me.Book[challenge for power]} || ${Me.Book[challenge for power rk. ii]} || ${Me.Book[challenge for power rk. iii]}),1,0]}",
};

PCHAR pCOMMG[]={
  "commanding",
  "[#] Endu% Above? 0=0ff",
  "${If[${Me.CombatAbility[commanding voice]},60,0]}",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[commanding voice]},1,0]}",
};

PCHAR pCRYHC[]={
  "cryhavoc",
  "[#] Endu% Above? 0=0ff",
  "${If[${Me.CombatAbility[cry havoc]},60,0]}",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[cry havoc]},1,0]}",
};

PCHAR pDISRM[]={
  "disarm",
  "[ON/OFF]?",
  "${If[${Me.Skill[disarm]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[disarm]},1,0]}",
};

PCHAR pDWNF0[]={
  "downflag0",
  "[ON/OFF] downflag0?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit0].Length},1,0]}",
};

PCHAR pDWNF1[]={
  "downflag1",
  "[ON/OFF] downflag1?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit1].Length},1,0]}",
};

PCHAR pDWNF2[]={
  "downflag2",
  "[ON/OFF] downflag2?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit2].Length},1,0]}",
};

PCHAR pDWNF3[]={
  "downflag3",
  "[ON/OFF] downflag3?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit3].Length},1,0]}",
};

PCHAR pDWNF4[]={
  "downflag4",
  "[ON/OFF] downflag4?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit4].Length},1,0]}",
};

PCHAR pDWNF5[]={
  "downflag5",
  "[ON/OFF] downflag5?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit5].Length},1,0]}",
};

PCHAR pDWNF6[]={
  "downflag6",
  "[ON/OFF] downflag6?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit6].Length},1,0]}",
};

PCHAR pDWNF7[]={
  "downflag7",
  "[ON/OFF] downflag7?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[downshit7].Length},1,0]}",
};

PCHAR pDRPNC[]={
  "dragonpunch",
  "[ON/OFF]?",
  "${If[${Me.Skill[dragon punch]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[dragon punch]},1,0]}",
};

PCHAR pEAGLE[]={
  "eaglestrike",
  "[ON/OFF]?",
  "${If[${Me.Skill[eagle strike]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[eagle strike]},1,0]}",
};

PCHAR pERAGE[]={
  "enrage",
  "[ON/OFF]?",
  "1",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pESCAP[]={
  "escape",
  "[#] Life% Below? 0=0ff",
  "${If[${Me.AltAbility[escape]},20,0]}",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]} && ${Me.AltAbility[escape]},1,0]}",
};

PCHAR pEVADE[]={
  "evade",
  "[#] Life% Below? 0=0ff",
  "${If[${Me.Skill[hide]} && ${Me.Class.ShortName.Equal[ROG]},1,0]}",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]} && ${Me.Skill[hide]} && ${Me.Class.ShortName.Equal[ROG]},1,0]}",
};

PCHAR pFEIGN[]={
  "feigndeath",
  "[#] Life% Below? 0=0ff",
  "${If[${Select[${Me.Class.ShortName},SHD,NEC,MNK]},30,0]}",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]} && ${Select[${Me.Class.ShortName},SHD,NEC,MNK]},1,0]}",
};

PCHAR pFACES[]={
  "facing",
  "[ON/OFF] Face Target (Range)?",
  "1",
  "${If[${meleemvi[plugin]} && ${meleemvi[range]},1,0]}",
};

PCHAR pFALLS[]={
  "falls",
  "[ON/OFF] Auto-Feign?",
  "0",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]} && ${Me.Class.ShortName.Equal[MNK]},1,0]}",
};

PCHAR pFERAL[]={
  "feralswipe",
  "[ON/OFF]?",
  "${If[${Me.AltAbility[feral swipe]},1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.AltAbility[feral swipe]},1,0]}",
};

PCHAR pFISTS[]={
  "fistsofwu",
  "[#] Endu% Above? 0=0ff",
  "${If[${Me.CombatAbility[fists of wu]},60,0]}",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[fists of wu]},1,0]}",
};

PCHAR pFLYKC[]={
  "flyingkick",
  "[ON/OFF]?",
  "${If[${Me.Skill[flying kick]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[flying kick]},1,0]}",
};

PCHAR pFORAG[]={
  "forage",
  "[ON/OFF]?",
  "${If[${Me.Skill[forage]},1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.Skill[forage]},1,0]}",
};

PCHAR pFRENZ[]={
  "frenzy",
  "[ON/OFF]?",
  "${If[${Me.Skill[frenzy]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[frenzy]} && ${Me.Skill[frenzy]},1,0]}",
};

PCHAR pHARMT[]={
  "harmtouch",
  "[ON/OFF]?",
  "${If[${Me.Skill[harm touch]}!=255,1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[harm touch]}!=255,1,0]}",
};

PCHAR pHIDES[]={
  "hide",
  "[ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[hide]},1,0]}",
};

PCHAR pHOLF0[]={
  "holyflag0",
  "[ON/OFF] holyflag0?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit0].Length},1,0]}",
};

PCHAR pHOLF1[]={
  "holyflag1",
  "[ON/OFF] holyflag1?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit1].Length},1,0]}",
};

PCHAR pHOLF2[]={
  "holyflag2",
  "[ON/OFF] holyflag2?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit2].Length},1,0]}",
};

PCHAR pHOLF3[]={
  "holyflag3",
  "[ON/OFF] holyflag3?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit3].Length},1,0]}",
};

PCHAR pHOLF4[]={
  "holyflag4",
  "[ON/OFF] holyflag4?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit4].Length},1,0]}",
};

PCHAR pHOLF5[]={
  "holyflag5",
  "[ON/OFF] holyflag5?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit5].Length},1,0]}",
};

PCHAR pHOLF6[]={
  "holyflag6",
  "[ON/OFF] holyflag6?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit6].Length},1,0]}",
};

PCHAR pHOLF7[]={
  "holyflag7",
  "[ON/OFF] holyflag7?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvs[holyshit7].Length},1,0]}",
};

PCHAR pINFUR[]={
  "infuriate",
  "[ON/OFF]?",
  "1",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pINTIM[]={
  "intimidation",
  "[ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[intimidation]},1,0]}",
};

PCHAR pJOLTS[]={
  "jolt",
  "Every [#] of Hits,0=0ff",
  "0",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]} && ${Select[${Me.Class.ShortName},BER,BST,RNG]},1,0]}",
};

PCHAR pKICKS[]={
  "kick",
  "[ON/OFF]?",
  "${If[${Me.Skill[kick]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[kick]},1,0]}",
};

PCHAR pLHAND[]={
  "layhand",
  "[#] MyLife% Below? 0=0ff",
  "${If[${Me.Skill[lay hands]}!=255,1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.Skill[lay hands]}!=255,1,0]}",
};

PCHAR pLCLAW[]={
  "leopardclaw",
  "[#] Endu% Above? 0=0ff",
  "${If[${Me.CombatAbility[leopard claw]} || ${Me.CombatAbility[dragon fang]} || ${Me.CombatAbility[clawstriker's flurry]} || ${Me.CombatAbility[clawstriker's flurry rk. ii]} || ${Me.CombatAbility[clawstriker's flurry rk. iii]},60,0]}",
  "${If[${meleemvi[plugin]} && (${Me.CombatAbility[leopard claw]} || ${Me.CombatAbility[dragon fang]} || ${Me.CombatAbility[clawstriker's flurry]} || ${Me.CombatAbility[clawstriker's flurry rk. ii]} || ${Me.CombatAbility[clawstriker's flurry rk. iii]}),1,0]}",
};

PCHAR pMELEE[]={
  "melee",
  "[ON/OFF] Melee Mode? 0=0ff",
  "${If[${Select[${Me.Class.ShortName},WAR,PAL,RNG,SHD,MNK,BRD,ROG,BST,BER]},1,0]}",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pMELEP[]={
  "meleepri",
  "[ID] Primary (Melee)?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && !${meleemvi[aggro]},1,0]}",
};

PCHAR pMELES[]={
  "meleesec",
  "[ID] Offhand (Melee)?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && !${meleemvi[aggro]},1,0]} && ${meleemvi[meleepri]}",
};

PCHAR pMENDS[]={
  "mend",
  "[#] MyLife% Below? 0=0ff",
  "${If[${Me.Skill[mend]},1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.Skill[mend]},1,0]}",
};

PCHAR pPETAS[]={
  "petassist",
  "[ON/OFF] Assist Me?",
  "${If[${Select[${Me.Class.ShortName},SHD,DRU,SHM,NEC,MAG,ENC,BST]},1,0]}",
  "${If[${meleemvi[plugin]} && ${Select[${Me.Class.ShortName},SHD,DRU,SHM,NEC,MAG,ENC,BST]},1,0]}",
};

PCHAR pPETDE[]={
  "petdelay",
  "[#] # Sec Delay Before Engaging?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[petassist]} && ${Select[${Me.Class.ShortName},SHD,DRU,SHM,NEC,MAG,ENC,BST]},1,0]}",
};

PCHAR pPETRN[]={
  "petrange",
  "[#] Target/Pet in this range?",
  "75",
  "${If[${meleemvi[plugin]} && ${meleemvi[petassist]} && ${Select[${Me.Class.ShortName},SHD,DRU,SHM,NEC,MAG,ENC,BST]},1,0]}",
};

PCHAR pPETMN[]={
  "petmend",
  "[#] Mend Pet Life % Below 0=0ff?",
  "20",
  "${If[${meleemvi[plugin]} && ${meleemvi[petassist]} && ${Me.AltAbility[mend companion]} && ${Select[${Me.Class.ShortName},SHD,DRU,SHM,NEC,MAG,ENC,BST]},1,0]}",
};

PCHAR pPICKP[]={
  "pickpocket",
  "[ON/OFF]?",
  "${If[${Me.Skill[pick pockets]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[pick pockets]},1,0]}",
};

PCHAR pPLUGS[]={
  "plugin",
  "[ON/OFF]?",
  "1",
  "1",
};

PCHAR pPOKER[]={
  "poker",
  "[ID] item?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[backstab]},1,0]}",
};

PCHAR pHFAST[]={
  "pothealfast",
  "[#] MyLife% Below? 0=0ff (FAST)",
  "${If[${meleemvi[idpothealfast]},30,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[idpothealfast]},1,0]}",
};

PCHAR pHOVER[]={
  "pothealover",
  "[#] MyLife% Below? 0=0ff (OVER)",
  "${If[${meleemvi[idpothealover]},60,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[idpothealover]},1,0]}",
};

PCHAR pPRVKM[]={
  "provokemax",
  "[#] Counter? ,1=try once, 0=0ff",
  "${If[${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[aggro]} && ${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},1,0]}",
};

PCHAR pPRVKE[]={
  "provokeend",
  "[#] Stop when Target Life% Below?",
  "${If[${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},20,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[aggro]} && ${meleemvi[provokemax]} && ${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},1,0]}",
};

PCHAR pPRVK0[]={
  "provoke0",
  "[ID] spell/disc/aa/item?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[aggro]} && ${meleemvi[provokemax]} && ${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},1,0]}",
};

PCHAR pPRVK1[]={
  "provoke1",
  "[ID] spell/disc/aa/item?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[aggro]} && ${meleemvi[provokemax]} && ${Select[${Me.Class.ShortName},WAR,PAL,SHD,MNK,BER]},1,0]}",
};

PCHAR pRAVOL[]={
  "ragevolley",
  "[#] Endu% Above? 0=0ff",
  "${If[${Me.CombatAbility[rage volley]} || ${Me.CombatAbility[destroyer's volley]} || ${Me.CombatAbility[giantslayer's volley]} || ${Me.CombatAbility[giantslayer's volley rk. ii]} || ${Me.CombatAbility[giantslayer's volley rk. iii]},60,0]}",
  "${If[${meleemvi[plugin]} && (${Me.CombatAbility[rage volley]} || ${Me.CombatAbility[destroyer's volley]} || ${Me.CombatAbility[giantslayer's volley]} || ${Me.CombatAbility[giantslayer's volley rk. ii]} || ${Me.CombatAbility[giantslayer's volley rk. iii]}),1,0]}",
};

PCHAR pRAKES[]={
  "rake",
  "[#] Endu% Above? 0=Off",
  "${If[${Me.CombatAbility[rake]},60,0]}",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[rake]},1,0]}",
};

PCHAR pRANGE[]={
  "range",
  "[#] Max Range? 0=0ff",
  "0",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pRESUM[]={
  "resume",
  "[#] Life% Above? 100=0ff",
  "20",
  "${If[${meleemvi[plugin]} && !${meleemvi[aggro]},1,0]}",
};

PCHAR pRKICK[]={
  "roundkick",
  "[ON/OFF]?",
  "${If[${Me.Skill[round kick]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[round kick]},1,0]}",
};

PCHAR pSENSE[]={
  "sensetraps",
  "[ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[sensetraps]},1,0]}",
};

PCHAR pSHIEL[]={
  "shield",
  "[ID] item?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[bash]},1,0]}",
};

PCHAR pSLAMS[]={
  "slam",
  "[ON/OFF]?",
  "${If[${Me.Skill[slam]}!=255,1,0]}",
  "${If[${meleemvi[plugin]} && ${Me.Skill[slam]}!=255,1,0]}",
};

PCHAR pSNEAK[]={
  "sneak",
  "[ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.Skill[sneak]},1,0]}",
};

PCHAR pSTAND[]={
  "standup",
  "[ON/OFF] Authorize to StandUp?",
  "0",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pSTIKR[]={
  "stickrange",
  "[#] Target in Range? 0=0ff",
  "${If[${Stick.Status.NotEqual[NULL]},75,0]}",
  "${If[${meleemvi[plugin]} && ${Stick.Status.NotEqual[NULL]},1,0]}",
};

PCHAR pSTIKD[]={
  "stickdelay",
  "[#] Sec to Wait Target in Range?",
  "0",
  "${If[${meleemvi[plugin]} && ${Stick.Status.NotEqual[NULL]} && ${meleemvi[stickrange]},1,0]}",
};

PCHAR pSTIKM[]={
  "stickmode",
  "[ON/OFF] Use stickcmd from ini?",
  "0",
  "${If[${meleemvi[plugin]} && ${Stick.Status.NotEqual[NULL]} && ${meleemvi[stickrange]},1,0]}",
};

PCHAR pSTRIK[]={
  "strike",
  "Use best sneak attack disc [ON/OFF]?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${meleemvi[backstab]} && ${meleemvi[idstrike]},1,0]}",
};

PCHAR pSTUNS[]={
  "stunning",
  "[#] Target Life% Below? 0=0ff",
  "0",
  "${If[${meleemvi[plugin]},1,0]}",
};

PCHAR pSTUN0[]={
  "stun0",
  "[ID] spell/disc/aa/item?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[stunning]},1,0]}",
};

PCHAR pSTUN1[]={
  "stun1",
  "[ID] spell/disc/aa/item?",
  "0",
  "${If[${meleemvi[plugin]} && ${meleemvi[stunning]},1,0]}",
};

PCHAR pTAUNT[]={
  "taunt",
  "[ON/OFF]?",
  "${If[${Me.Skill[taunt]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${meleemvi[aggro]} && ${Me.Skill[taunt]},1,0]}",
};

PCHAR pTHIEF[]={
  "thiefeye",
  "[ON/OFF]?",
  "${If[${Me.CombatAbility[thief's eye]},60,0]}",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[thief's eye]},1,0]}",
};

PCHAR pTHROW[]={
  "throwstone",
  "[#] Endu% Above? 0=0ff",
  "0",
  "${If[${meleemvi[plugin]} && ${Me.CombatAbility[throw stone]},1,0]}",
};

PCHAR pTIGER[]={
  "tigerclaw",
  "[ON/OFF]?",
  "${If[${Me.Skill[tiger claw]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.Skill[tiger claw]},1,0]}",
};

PCHAR pTWIST[]={
  "twistedshank",
  "[ON/OFF]?",
  "${If[${Me.AltAbility[twisted shank]},1,0]}",
  "${If[${meleemvi[plugin]} && ${meleemvi[melee]} && ${Me.AltAbility[twisted shank]},1,0]}",
};

PCHAR UI_PetBack="BackButton";    
PCHAR UI_PetAttk="AttackButton";
PCHAR UI_CombatBtn[]= {
  "CAW_Button0",
  "CAW_Button1",
  "CAW_Button2",
  "CAW_Button3",
  "CAW_Button4",
  "CAW_Button5",
  "CAW_Button6",
  "CAW_Button7",
  "CAW_Button8",
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

enum { 
  Nokey              =0,           // WinClick(KeyState) nokey pressed
  Shiftkey           =1,           // WinClick(KeyState) Shiftkey pressed
  Ctrlkey            =256,         // WinClick(KeyState) Ctrlkey pressed
};

enum { 
  Tiny               =0,           // Container Size - Tiny
  Small              =1,           // Container Size - Small
  Medium             =2,           // Container Size - Medium
  Large              =3,           // Container Size - Large
  Giant              =4,           // Container Size - Giant
  Huge               =5,           // Container Size - Huge
};

enum {
  st_x               =0x0000,      // SpawnType: NONE
  st_a               =0xFFFF,      // SpawnType: ANY
  st_c               =0x0030,      // SpawnType: CORPSE
  st_cn              =0x0020,      // SpawnType: CORPSENPC
  st_cp              =0x0010,      // SpawnType: CORPSEPLAYER
  st_w               =0x000C,      // SpawnType: PET
  st_wn              =0x0008,      // SpawnType: PETNPC
  st_wp              =0x0004,      // SpawnType: PETPLAYER
  st_n               =0x0002,      // SpawnType: NPC
  st_nc              =0x0022,      // SpawnType: NPC or CORPSENPC
  st_nw              =0x000A,      // SpawnType: NPC or PETNPC
  st_nwc             =0x002A,      // SpawnType: NPC or PETNPC or CORPSENPC
  st_p               =0x0001,      // SpawnType: PLAYER
  st_pc              =0x0011,      // SpawnType: PLAYER or CORPSEPLAYER
  st_pw              =0x0005,      // SpawnType: PLAYER or PETPLAYER
  st_pwc             =0x0015,      // SpawnType: PLAYER or PETPLAYER or CORPSEPLAYER
};

typedef VOID  (__cdecl *Function)(VOID);

bool          BardClass    =false; // Bard Class?
bool          Silenced     =false; // Silenced?

long          BuffMax      =25;    // Maximum Number of Buffs
long          PetsMax      =29;    // Maximum Number of Buffs on Pets
long          SongMax      =12;    // Maximum Number of Songs
long          GemsMax      =9;     // Maximum Number of Gems
long          SlotMax      =30;    // Maximum Number of Inventory Main Slots
long          WornMax      =22;    // Maximum Number of Worn Inventoty Slots

long          CbtSlot       =NOID;  //
long          InvSlot       =NOID;  // slot # where last item was found
PCONTENTS     InvCont       =NULL;  // contents

long          Sticking      =false; // Stick On/Off?
char          StickArg[128] ={0};   // Stick Argument?

bool          DebugReady    =false; // Use for debuffing Ability->Ready();

char          Reserved[MAX_STRING];
char          Workings[MAX_STRING];

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

DWORD         AACheck(DWORD id);
DWORD         AAPoint(DWORD index);
BOOL          AAReady(DWORD index);
LONG          Aggroed(DWORD id);
FLOAT         AngularDistance(float h1, float h2);
DOUBLE        AngularHeading(PSPAWNINFO t, PSPAWNINFO s);
BOOL          CACheck(DWORD id);
BOOL          CAPress(DWORD id);
BOOL          Casting(PCHAR command);
BOOL          CursorEmpty();
LONG          Discipline();
BOOL          Equip(DWORD ID, long SlotID);
BOOL          Equipped(DWORD id);
LONG          Evaluate(PCHAR zFormat, ...);
LONG          ItemCounts(DWORD ID, long B=0, long E=SlotMax);
PCONTENTS     ItemLocate(DWORD ID, long B=0, long E=SlotMax, long SlotID=NOID);
LONG          OkayToEquip(long Size=NOID);
LONG          PackFind(long Size);
PMQPLUGIN     Plugin(PCHAR PluginName);
VOID*         PluginEntry(PCHAR PluginName, PCHAR FuncName);
BOOL          SKCheck(DWORD id);
BOOL          SKReady(DWORD id);
BOOL          SKPress(DWORD id);
PCONTENTS     SlotContent(long SlotID);
LONG          SpawnMask(PSPAWNINFO x);
BOOL          SpellCheck(DWORD id);
LONG          SpellGemID(DWORD ID, LONG slotid=NOID);
BOOL          SpellReady(DWORD ID, LONG SlotID=NOID);
BOOL          Stick(PCHAR command);
LONG          Unequip(long SlotID);
VOID          WinClick(CXWnd *Wnd, PCHAR ScreenID, PCHAR ClickNotification, DWORD KeyState);
PSTR          WinTexte(CXWnd *Wnd, PCHAR ScreenID, PSTR Buffer);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#define GetSpawnID(spawnid) (PSPAWNINFO)GetSpawnByID(spawnid)

static inline BOOL WinState(CXWnd *Wnd) {
  return (Wnd && ((PCSIDLWND)Wnd)->Show);
}

static inline BOOL TypePack(PCONTENTS Item) {
  return(Item && Item->Item->Type==ITEMTYPE_PACK);
}

static inline BOOL TargetType(DWORD mask) {
  return (SpawnMask((PSPAWNINFO)pTarget)&mask);
}

static inline BOOL TargetID(DWORD ID) {
  return (ID && pTarget && ((PSPAWNINFO)pTarget)->SpawnID==ID);
}

static inline float SpeedRun(PSPAWNINFO x) {
  if(x && x->Mount) return x->Mount->SpeedRun;
  if(x) return x->SpeedRun;
  return 0.0f;
}

static inline BOOL SpawnType(PSPAWNINFO x, DWORD mask) {
  return (SpawnMask(x)&mask);
}

static inline PSPAWNINFO SpawnMe() {
  return (PSPAWNINFO)pCharSpawn;
}

static inline PSPAWNINFO SpawnMount() {
  return SpawnMe()?SpawnMe()->Mount:NULL;
}

static inline PSPAWNINFO SpawnPet() {
  return (SpawnMe() && (LONG)SpawnMe()->PetID>0)?GetSpawnID(SpawnMe()->PetID):NULL;
}

static inline DWORD StandState() {
  return SpawnMe()?SpawnMe()->StandState:0;
}

static inline BOOL Stackable(PCONTENTS Item) {
  return (Item && Item->Item->Type==ITEMTYPE_NORMAL && ((EQ_Item*)Item)->IsStackable());
}

static inline LONG StackUnit(PCONTENTS Item) {
  return (!Stackable(Item))?1:Item->StackCount;
}

static inline BOOL IsStunned() {
  return (GetCharInfo() && GetCharInfo()->Stunned);
}

static inline BOOL IsStanding() {
  return StandState()==STANDSTATE_STAND;
}

static inline BOOL IsSneaking() {
  return (SpawnMe() && SpawnMe()->Sneak);
}

static inline BOOL IsInvisible() {
  return (SpawnMe() && SpawnMe()->HideMode);
}

static inline BOOL IsGrouped() {
  return (GetCharInfo() && GetCharInfo()->pGroupInfo->pLeader);
}

static inline BOOL IsFeigning() {
  return StandState()==STANDSTATE_FEIGN;
}

static inline BOOL IsCasting() {
  return (SpawnMe() && ((long)GetCharInfo()->pSpawn->CastingData.SpellID > 0));
}

static inline BOOL InRange(PSPAWNINFO a, PSPAWNINFO b, float d) {
  if(!a || !b) return false;
  return (DistanceToSpawn(a,b)<=d);
}

static inline BOOL InGame() {
  return (gbInZone && gGameState==GAMESTATE_INGAME && SpawnMe() && GetCharInfo2() && GetCharInfo() && GetCharInfo()->Stunned!=3);
}

static inline CXWnd* XMLChild(CXWnd* window, PCHAR screenid) {
  if(window) return window->GetChildItem(screenid);
  return NULL;
}

static inline BOOL XMLEnabled(CXWnd* window) {
  return (window && ((PCSIDLWND)window)->Enabled);
}

DWORD AACheck(DWORD id) {
  if(pAltAdvManager)
    if(PCHARINFO2 Me=GetCharInfo2())
      if(_AALIST* AAList=Me->AAList)
        if(id)
          for(DWORD nAbility=0; nAbility<AA_CHAR_MAX_REAL; nAbility++)
            if(LONG AAIndex=AAList[nAbility].AAIndex)
              if(PALTABILITY ability=pAltAdvManager->GetAltAbility(AAIndex))
                if(ability->ID==id) return AAIndex;
  return false;
}

DWORD AAPoint(DWORD index) {
  if(PCHARINFO2 Me=GetCharInfo2())
    if(_AALIST* AAList=Me->AAList)
      if(index) 
        for(DWORD nAbility=0; nAbility<AA_CHAR_MAX_REAL; nAbility++)
          if(index==AAList[nAbility].AAIndex)
            return  AAList[nAbility].PointsSpent;
  return 0;
}

BOOL AAReady(DWORD index) {
  int result=0;
  if(pAltAdvManager)
    if(index)
      if(PALTABILITY ability=pAltAdvManager->GetAltAbility(index))
        if(pAltAdvManager->GetCalculatedTimer(pPCData,ability)>0)
          pAltAdvManager->IsAbilityReady(pPCData,ability,&result);
  return (result<0);
}

LONG Aggroed(DWORD id) {
  if(PSPAWNINFO self=SpawnMe())
    if(PSPAWNINFO kill=GetSpawnID(id))
      if(PSPAWNINFO targ=(pTarget)?(PSPAWNINFO)pTarget:NULL) {
		if(targ==kill && self->SpawnID==self->TargetOfTarget)  return  1; // im on hott
        if(fabs(AngularHeading(kill,self))<8.0f)       return  1; // it's facing me
        if(FindSpeed(kill)>0.0f && kill->HPCurrent<20) return -1; // it's moving
        if(InRange(self,targ,25.0f))                   return -1; // close enough
      }
  return 0;
}

FLOAT AngularDistance(float h1, float h2) {
  if(h1 == h2) return 0.0;
  if(fabs(h1 - h2) > 256.0) * (h1 < h2?&h1:&h2) += 512.0;
  return (fabs(h1 - h2) > 256.0)?(h2 - h1):(h1 - h2);
}

DOUBLE AngularHeading(PSPAWNINFO t, PSPAWNINFO s) {
  double Head=t->Heading-(float)atan2(s->X - t->X, s->Y - t->Y) * 256.0 / PI;
  if(Head > 256.0f) Head -= 512.0f; else if(Head < -256.0f) Head += 512.0f;
  return Head;
}

BOOL CACheck(DWORD id) {
  if(PCHARINFO2 Me=GetCharInfo2())
    if(DWORD* CombatAbilities=Me->CombatAbilities)
      if(id) 
        for(DWORD nCombat=0; nCombat<NUM_COMBAT_ABILITIES; nCombat++)
          if(id==CombatAbilities[nCombat]) return true;
  return false;
}

BOOL CAPress(DWORD id) {
  pCharData->DoCombatAbility(id);
  return true;
}

BOOL Casting(PCHAR command) {
  typedef void (__cdecl *fCALL)(PSPAWNINFO,PCHAR);
  if(fCALL request=(fCALL)PluginEntry("mq2cast","CastCommand")) {
    #if    SHOW_CASTING > 0
      WriteChatf("%s::Casting [\ay%s\ax].",PLUGIN_NAME,command);
    #endif SHOW_CASTING
    request(NULL,command);
    return true;
  }
  return false;
}

BOOL CursorEmpty() {
  if(PCHARINFO2 Me=GetCharInfo2()) 
    if(!Me->Cursor)
      if(!Me->CursorPlat)
        if(!Me->CursorGold)
          if(!Me->CursorSilver)
            if(!Me->CursorCopper)
              return true;
  return false;
}

LONG Discipline() {
  char temps[MAX_STRING];
  PSPELL spell=GetSpellByName(WinTexte((CXWnd*)pCombatAbilityWnd,"CAW_CombatEffectLabel",temps));
  return (spell)?spell->ID:0;
}

BOOL Equip(DWORD ID, long SlotID) {
  if(!OkayToEquip())      return false;
  if(!(SlotID < SlotMax)) return false;
  PCONTENTS fITEM=ItemLocate(ID,WornMax);
  if(!fITEM) fITEM=ItemLocate(ID,0,WornMax);
  if(!fITEM)              return false;
  if(InvSlot == SlotID)   return true;
  if(SlotID < WornMax) {
    if(!(fITEM->Item->Classes&(1<<((GetCharInfo2()->Class)-1)))) return false;
    if(fITEM->Item->RequiredLevel > GetCharInfo2()->Level)       return false;
    if(fITEM->Item->Diety && !(fITEM->Item->Diety&(1<<(GetCharInfo2()->Deity-200)))) return false;
    long MyRace=(DWORD)GetCharInfo2()->Race;
    switch((DWORD)MyRace) {
      case 128: MyRace=12;    break;
      case 130: MyRace=13;    break;
      case 330: MyRace=14;    break;
      case 522: MyRace=15;    break;
      default:  MyRace--;
    } 
    if(!(fITEM->Item->Races&(1<<MyRace))) return false;
    if(SlotID == 13)
      if(fITEM->Item->ItemType==1 || fITEM->Item->ItemType==4 || fITEM->Item->ItemType==35)
        if(PCONTENTS Sec=GetCharInfo2()->Inventory.Secondary)
          if(!Unequip(14)) return false;
    if(SlotID == 14)
      if(PCONTENTS Pri=GetCharInfo2()->Inventory.Primary)
        if(Pri->Item->ItemType==1 || Pri->Item->ItemType==4 || Pri->Item->ItemType==35)
          if(!Unequip(13)) return false;
  }
  long dSLOT=0;
  if(PCONTENTS dCONT=GetCharInfo2()->InventoryArray[SlotID]) {
    if(InvSlot<SlotMax) dSLOT=InvSlot;
    else {
      PCONTENTS fPACK=GetCharInfo2()->InventoryArray[22+(InvSlot-251)/10];
      if(fPACK && fPACK->Item->SizeCapacity >= dCONT->Item->Size) dSLOT=InvSlot;
      else dSLOT=PackFind(dCONT->Item->Size);
    }
    if(!dSLOT) return false;
  }
  char buffer[16];
  sprintf(buffer,"InvSlot%d",SlotID);
  pInvSlotMgr->MoveItem(InvSlot,0x1E,1,1);
  WinClick((CXWnd*)pInventoryWnd,buffer,"leftmouseup",Nokey);
  if(dSLOT) pInvSlotMgr->MoveItem(0x1E,dSLOT,1,1);
  return true;
}

BOOL Equipped(DWORD id) {
 if(id)
   for(int i=0; i<WornMax; i++)
     if(PCONTENTS Cont=GetCharInfo2()->InventoryArray[i])
       if(id==Cont->Item->ItemNumber) return true;
 return false;
}

LONG Evaluate(PCHAR zFormat, ...) {
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return 1;
  ParseMacroData(zOutput); 
  return atoi(zOutput);
}

LONG ItemCounts(DWORD ID, long B, long E) {
  long Count=0; InvSlot=NOID; InvCont=NULL;
  for(int iSlot=B; iSlot<E; iSlot++) {
    if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
      if(ID==cSlot->Item->ItemNumber) {
        Count+=StackUnit(cSlot);
        if(!InvCont) {
          InvCont=cSlot;
          InvSlot=iSlot;
        }
      } else if(TypePack(cSlot)) {
        for(int iPack=0; iPack<cSlot->Item->Slots; iPack++) {
          if(PCONTENTS cPack=cSlot->Contents[iPack]) {
            if(ID==cPack->Item->ItemNumber) {
              Count+=StackUnit(cPack);
              if(!InvCont) {
                InvCont=cPack;
                InvSlot=251+iPack+(iSlot-WornMax)*10;
              }
            }
          }
        }
      }
    }
  }          
  return Count;
}

PCONTENTS ItemLocate(DWORD ID, long B, long E, long SlotID) {
  if(SlotID!=NOID) 
    if(PCONTENTS find=SlotContent(SlotID)) 
      if(find->Item->ItemNumber==ID) {
        InvSlot=SlotID;
        return find;
      }
  for(int iSlot=B; iSlot<E; iSlot++)
    if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
      if(ID==cSlot->Item->ItemNumber) {
        InvSlot=iSlot;
        return cSlot;
      }
      if(TypePack(cSlot)) {
        for(int iPack=0; iPack<cSlot->Item->Slots; iPack++)
          if(PCONTENTS cPack=cSlot->Contents[iPack])
            if(ID==cPack->Item->ItemNumber) {
              InvSlot=(iSlot-WornMax)*10+iPack+251;
              return cPack;
            }
      }
    }
  InvSlot=NOID;
  return NULL;
}

LONG OkayToEquip(long Size) {
  if(!CursorEmpty() || IsCasting()) return false;
  if(Size!=NOID) return PackFind(Size);
  return true;
}

LONG PackFind(long Size) {
  long pSIZE=10;
  long pSLOT=0;
  for(int iSlot=WornMax; iSlot < SlotMax; iSlot++) {
    if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
      if(TypePack(cSlot) && cSlot->Item->Combine != 2 &&
        Size <= cSlot->Item->SizeCapacity && (!pSLOT || cSlot->Item->SizeCapacity < pSIZE)) {
        for(int iPack=0; iPack < cSlot->Item->Slots; iPack++) {
          if(!cSlot->Contents[iPack]) {
            pSLOT=(iSlot-22)*10+iPack+251;
            pSIZE=cSlot->Item->SizeCapacity;
            break;
          }
        }
      }
    } else if(!pSLOT) pSLOT=iSlot;
  }
  return pSLOT; 
} 

PMQPLUGIN Plugin(PCHAR PluginName) {
  long Length=strlen(PluginName)+1;
  PMQPLUGIN pLook=pPlugins;
  while(pLook && strnicmp(PluginName,pLook->szFilename,Length)) pLook=pLook->pNext;
  return pLook;
}

VOID* PluginEntry(PCHAR PluginName, PCHAR FuncName) {
  if(PMQPLUGIN pLook=Plugin(PluginName))
    if(void* entry=GetProcAddress(pLook->hModule,FuncName))
      return entry;
  return NULL;
}

BOOL SKCheck(DWORD id) {
  if(id<100 && (pSkillMgr->pSkill[id]->Activated && GetCharInfo2()->Skill[id]))     return true;
  if(id>100 && id<128 && GetCharInfo2()->Skill[id]!=0xFF && strlen(szSkills[id])>3) return true;
  return false;
}

BOOL SKReady(DWORD id) {
  if(id<100) {
    if(pSkillMgr->pSkill[id]->AltTimer==2) return gbAltTimerReady?true:false;
    return EQADDR_DOABILITYAVAILABLE[id]?true:false;
  }
  if(id==111) return gbAltTimerReady?true:false;
  if(id==105 || id==107) return LoH_HT_Ready();
  return false;
}

BOOL SKPress(DWORD id) {
  pCharData1->UseSkill((unsigned char)id,(EQPlayer*)pCharData1);
  return true;
}

PCONTENTS SlotContent(long SlotID) {
  if(SlotID>0) {
    long InvSlot=NOID;
    long SubSlot=NOID;
    if(!(SlotID < SlotMax)) {
      InvSlot=22+(SlotID-251)/10;
      SubSlot=(SlotID-1)%10;
    } 
    else InvSlot=SlotID;
    if(InvSlot<SlotMax) {
      if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[InvSlot]) {
        if(SubSlot<0) return cSlot;
        if(PCONTENTS cPack=cSlot->Contents[SubSlot]) return cPack;
      }
    }
  }
  return NULL;
}

LONG SpawnMask(PSPAWNINFO x) {
  if(!x)                        return st_x;
  if(x->Type==SPAWN_PLAYER)     return st_p;
  if(x->Type==SPAWN_CORPSE)     return x->Deity?st_cp:st_cn;
  if(x->Type!=SPAWN_NPC)        return st_x;
  if(strstr(x->Name,"s_Mount")) return st_x;
  if(!x->MasterID)              return st_n;
  PSPAWNINFO m=GetSpawnID(x->MasterID);
  return (!m || m->Type!=SPAWN_PLAYER)?st_wn:st_wp;
}

BOOL SpellCheck(DWORD ID) {
  if(ID)
    if(PCHARINFO2 Me=GetCharInfo2())
      for(DWORD nSlot=0; nSlot<NUM_BOOK_SLOTS; nSlot++)
        if(ID==Me->SpellBook[nSlot])
          return true;
  return false;
}

LONG SpellGemID(DWORD ID, LONG SlotID) {
  if(PCHARINFO2 Me=GetCharInfo2()) {
    if(SlotID!=NOID && ID==Me->MemorizedSpells[SlotID]) return SlotID;
    for(LONG GEM=0; GEM<GemsMax; GEM++)
      if(ID==Me->MemorizedSpells[GEM]) 
        return GEM;
  }
  return NOID;
}

BOOL SpellReady(DWORD ID, LONG SlotID) {
  if(pCastSpellWnd)
    if(PCHARINFO2 Me=GetCharInfo2()) {
      DWORD GemID=(SlotID!=NOID)?SlotID:SpellGemID(ID);
      if(GemID<(DWORD)GemsMax)
        if(Me->MemorizedSpells[GemID]==ID) 
          if((LONG)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[GemID]->spellicon!=NOID)
            if(BardClass || (LONG)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[GemID]->spellstate!=1)
              return true;
    }
  return false;
}

BOOL Stick(PCHAR command) {
  typedef void (__cdecl *fCALL)(PSPAWNINFO,PCHAR);
  if(fCALL request=(fCALL)PluginEntry("mq2moveutils","StickCommand")) {
    if(Evaluate("${If[${Stick.Active},1,0]}")) request(SpawnMe(),"off");
    if(command[0]) {
      #if    SHOW_STICKING > 0
        WriteChatf("%s::Sticking [\ay%s\ax].",PLUGIN_NAME,command);
      #endif SHOW_STICKING
      request(SpawnMe(),command);
    }
    Sticking=(command[0])?true:false;
    strcpy(StickArg,Sticking?command:"OFF");
    return true;
  }
  return false;
}

LONG Unequip(long SlotID) {
  if(SlotID<SlotMax) {
    PCONTENTS uCONT=GetCharInfo2()->InventoryArray[SlotID];
    if(!uCONT) return true;
    if(long uDEST=OkayToEquip(uCONT->Item->Size)) {
      pInvSlotMgr->MoveItem(SlotID,30,1,1);
      pInvSlotMgr->MoveItem(30,uDEST,1,1);
      return true;
    }
  }
  return false;
}

VOID WinClick(CXWnd *Wnd, PCHAR ScreenID, PCHAR ClickNotification, DWORD KeyState) {
  if(Wnd) if(CXWnd *Child=Wnd->GetChildItem(ScreenID)) {
    BOOL KeyboardFlags[4];
    *(DWORD*)&KeyboardFlags=*(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags;
    *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags=KeyState;
    SendWndClick2(Child,ClickNotification);
    *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags=*(DWORD*)&KeyboardFlags;
  }
}

PSTR WinTexte(CXWnd *Wnd, PCHAR ScreenID, PSTR Buffer) {
  Buffer[0]=0;
  if(Wnd)
    if(CXWnd *Child=(CXWnd*)Wnd->GetChildItem(ScreenID))
      GetCXStr(Child->WindowText,Buffer,2047);
  return Buffer;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class Ability {
  CHAR   COMM[16];    // Ability Cast Command
  DWORD  REUSE;       // Ability Reuse Time
  DWORD  READY;       // Ability Ready
  LONG   INDEX;       // Ability Index
  PSPELL EFFECT;      // Ability Spell Effect
  enum { UNKNOWN, ITEM, SKILL, DISC, AA, SPELL, CLICKY, POTION };

public:
  LONG   ID;          // Ability ID
  LONG   TYPE;        // Ability Type
  CHAR   NAME[128];   // Ability Name
  BOOL   SEARCH;      // Ability Searched

  BOOL Avail() {
    SEARCH=1;
    EFFECT=0;
    if(ID>NOID) {
      if(TYPE==SKILL || TYPE==UNKNOWN) {
        if(SKCheck(ID)) {
          strcpy(NAME,szSkills[ID]);
          REUSE=(ID==i_forage)?101750:delay*3;
          TYPE=SKILL;
          return true;
        }
      }
      if(TYPE==DISC || TYPE==UNKNOWN) {
        if(CACheck(ID)) {
          if(PSPELL spell=GetSpellByID(ID)) {
            strcpy(NAME,spell->Name);
            EFFECT=spell;
            REUSE=spell->CastTime+spell->RecastTime+delay*6;
            TYPE=DISC;
            return true;
          }
        }
      }
      if(TYPE==AA || TYPE==UNKNOWN) {
        if(long AAIndex=AACheck(ID)) {
          if(PALTABILITY ability=pAltAdvManager->GetAltAbility(AAIndex)) {
            if(PSPELL spell=GetSpellByID(ability->SpellID)) {
              strcpy(NAME,pCDBStr->GetString(ability->nName,1,NULL));
              sprintf(COMM,"%d|ALT",ID);
              EFFECT=spell;
              REUSE=pAltAdvManager->GetCalculatedTimer(pPCData,ability)*1000+spell->CastTime+delay*3;
              INDEX=AAIndex;
              TYPE=AA;
              return true;
            }
          }
        }
      }
      if(TYPE==SPELL || TYPE==UNKNOWN) {
        if(SpellCheck(ID)) {
          if(PSPELL spell=GetSpellByID(ID)) {
            strcpy(NAME,spell->Name);
            sprintf(COMM,"%d|GEM5",ID);
            EFFECT=spell;
            REUSE=spell->CastTime+spell->RecastTime+delay*3;
            TYPE=SPELL;
            return true;
          }
        }
      }
      if(TYPE==POTION || TYPE==CLICKY || TYPE==ITEM || TYPE==UNKNOWN) {
        if(PCONTENTS find=ItemLocate(ID)) {
          INDEX=InvSlot;
          strcpy(NAME,find->Item->Name);
          TYPE=ITEM;
          if(PSPELL spell=GetSpellByID(find->Item->Clicky.SpellID)) {
            sprintf(COMM,"%d|ITEM",ID);
            EFFECT=spell;
            REUSE=find->Item->CastTime+delay*3;
            TYPE=(find->Item->ItemType==21)?POTION:CLICKY;
          } 
          return true;
        }
      }
    }
    return false;
  }

  BOOL Found() {
    if(!SEARCH) Avail();
    return (ID>0 && TYPE!=UNKNOWN)?true:false;
  }

  LONG Check(string test) {
    if(!Found())              return 0x01;  // Ability Not Found
    if((DWORD)clock()<=READY) return 0x02;  // Ability Not Refreshed
    if(TYPE==SKILL) {
      if(SpawnMount()) switch(ID) {
        case i_disarm:        break;
        case i_harmtouch:     break;
        case i_intimidation:  break;
        case i_kick:          break;
        case i_layhand:       break;
        case i_mend:          break;
        case i_taunt:         break;
        default:              return 0x03;  // Ability Do Not Work on Mount
      }
      if(!SKReady(ID))        return 0x13;  // Ability Not Ready
    } else if(EFFECT) {
      if(!IsStanding())       return 0x05;  // Not Standing
      if(IsStunned())         return 0x06;  // Stunned
      if(TYPE>DISC) {
        if(IsInvisible())     return 0x04;  // Will Break Invisiblity
        if(Silenced)          return 0x07;  // Silenced
      }
      if(IsCasting())         return 0x08;  // already casting
      if(WinState((CXWnd*)pSpellBookWnd)) return 0x09;  // spellbook open
      if(EFFECT->CARecastTimerID && ((DWORD)pPCData->GetCombatAbilityTimer(EFFECT->CARecastTimerID)-(DWORD)time(NULL))<0) return 0x16; // dicipline timer not ready
      if((long)EFFECT->ReagentId[0]>0 && ItemCounts(EFFECT->ReagentId[0]) < (long)EFFECT->ReagentCount[0])        return 0x0A;  // out of reagent
      if(EFFECT->EnduranceCost && GetCharInfo2()->Endurance < EFFECT->EnduranceCost)                              return 0x0B;  // out of endurance
      if(EFFECT->Mana && GetCharInfo2()->Mana < EFFECT->Mana)                                                     return 0x0C;  // out of mana
      if(!EFFECT->SpellType) {
        if(!pTarget)                                                                                              return 0x0D;  // no target
        float SpellRange=(EFFECT->Range)?EFFECT->Range:EFFECT->AERange;
        if(SpellRange && !InRange(SpawnMe(),(PSPAWNINFO)pTarget,SpellRange))                                      return 0x0E;  // out of range
      } else if(EFFECT->DurationValue1>0) {
        if(EFFECT->DurationWindow) {
          for(int s=0; s<SongMax; s++) {
            if(PSPELL buff=GetSpellByID(GetCharInfo2()->ShortBuff[s].SpellID)) {
              if(EFFECT->ID==buff->ID)        return 0x0F; // already have
              if(!BuffStackTest(EFFECT,buff)) return 0x10; // not stacking 
            }
          }
        } else {
          for(int b=0; b<BuffMax; b++) {
            if(PSPELL buff=GetSpellByID(GetCharInfo2()->Buff[b].SpellID)) {
              if(EFFECT->ID==buff->ID)        return 0x0F; // already have
              if(!BuffStackTest(EFFECT,buff)) return 0x10; // not stacking 
            }
          }
        }
      }
      if(TYPE>DISC && !Evaluate("${If[${Cast.Ready[%s]},1,0]}",COMM)) return 0x11; // mq2cast not ready
      if(TYPE==AA) { 
        if(!AAReady(INDEX))       return 0x13;  // Ability Not Ready
      } else if(TYPE==SPELL) { 
        if(!pCastSpellWnd)        return 0x12;  // No Casting Spell Bar
        INDEX=SpellGemID(ID,INDEX);
        if(!SpellReady(ID,INDEX)) return 0x13;  // Ability Not Ready
      } else if(TYPE==POTION || TYPE==CLICKY) {
        PCONTENTS find=ItemLocate(ID,0,SlotMax,INDEX);
        INDEX=InvSlot;
        if(!find || !find->Charges || GetItemTimer(find)) return 0x13;  // Ability Not Ready
        if(!CursorEmpty())        return 0x14;  // Cursor Not Empty
      }
    }
    if(!test.empty() && !Evaluate((PCHAR)test.c_str())) return 0x15; // User Condition Abort
    return 0x00;
  }

  BOOL Ready(string test) {
    LONG Result=Check(test);
    if(DebugReady && Result) {
      PCHAR Message="";
      switch(Result) {
        case 0x01:  Message="NOT FOUND";                break;
        case 0x02:  Message="NOT REFRESHED";            break;
        case 0x03:  Message="NOT WORKING ON MOUNT";     break;
        case 0x04:  Message="WILL BREAK INVISIBILITY";  break;
        case 0x05:  Message="NOT STANDING";             break;
        case 0x06:  Message="STUNNED";                  break;
        case 0x07:  Message="SILENCED";                 break;
        case 0x08:  Message="ALREADY CASTING";          break;
        case 0x09:  Message="SPELLBOOK OPEN";           break;
        case 0x0A:  Message="OUT OF REAGENT";           break;
        case 0x0B:  Message="OUT OF ENDURANCE";         break;
        case 0x0C:  Message="OUT OF MANA";              break;
        case 0x0D:  Message="NO TARGET";                break;
        case 0x0E:  Message="OUT OF RANGE";             break;
        case 0x0F:  Message="ALREADY BUFFED WITH THIS"; break;
        case 0x10:  Message="BUFF NOT STACKING";        break;
        case 0x11:  Message="MQ2CAST NOT READY/FOUND";  break;
        case 0x12:  Message="NO SPELL BAR";             break;
        case 0x13:  Message="ABILITY NOT READY";        break;
        case 0x14:  Message="CURSOR NOT EMPTY";         break;
        case 0x15:  Message="USER CONDITION ABORT";     break;
        case 0x16:  Message="TIMER NOT READY";          break;
      }
      WriteChatf("Ability[%d][%d][%s] <<%s>>.",ID,TYPE,NAME,Message);
    }
    return (!Result);
  }

  BOOL Press() {
    BOOL Casted=false;
    if(Found() && (DWORD)clock()>READY) {
      #if    SHOW_ABILITY > 0
        WriteChatf("%s::Activate [\ay%s\ax].",PLUGIN_NAME,NAME);
      #endif SHOW_ABILITY
      if(TYPE==SKILL)     Casted=SKPress(ID);
      else if(TYPE==DISC) Casted=CAPress(ID);
      else if(TYPE>=AA)   Casted=Casting(COMM);
      if(Casted) READY=(DWORD)clock()+REUSE;
    }
    return Casted;
  }

  void Setup(LONG id, LONG type) {
    ID=id;            // Ability ID?
    TYPE=type;        // Ability Type?
    NAME[0]=0;        // Ability Name?
    COMM[0]=0;        // Ability Command?
    SEARCH=false;     // Ability Searched?
    READY=0;          // Ability Ready Time
    INDEX=NOID;       // Ability Index
    EFFECT=NULL;      // Ability Spell Effect
  }

  Ability(LONG id, LONG type) {
    Setup(id,type);
  }

  Ability() {
    Setup(0,UNKNOWN);
  }
};

class Option {
public:
  PCHAR    K;  // key?
  PCHAR    H;  // help?
  PCHAR    D;  // default?
  PCHAR    S;  // show?
  string  *C;  // condition?
  Ability *A;  // ability?
  LONG    *V;  // value?
  Function F;  // function?
  BOOL     U;  // update?

  Option(PCHAR k, PCHAR h, PCHAR d, PCHAR s, Function f, string *c) {
    K=k; H=h; D=d; S=s; F=f; U=false; C=c; A=NULL; V=NULL;
  }
  
  Option(PCHAR k, PCHAR h, PCHAR d, PCHAR s, Function f, Ability *a) {
    K=k; H=h; D=d; S=s; F=f; U=false; C=NULL; A=a; V=NULL;
  }

  Option(PCHAR k, PCHAR h, PCHAR d, PCHAR s, Function f, LONG *v) {
    K=k; H=h; D=d; S=s; F=f; U=false; C=NULL; A=NULL; V=v;
  }
  
  void Write() {
    if(K[0] && !C && Evaluate(S)) {
      long value=(A)?A->ID:(V)?*V:0;
      if(value>0) WriteChatf("%s::%s (\ag%d\ax) \ay%s\ax.",PLUGIN_NAME,K,value,H);
      else        WriteChatf("%s::%s (\ar0\ax) \ay%s\ax." ,PLUGIN_NAME,K,H);
    }
  }

  void Setup(PCHAR value) {
    if(C) *C=value;
    else if(A) A->Setup(atol(value),0);
    else if(V) {
      if(!stricmp("false",value) || !stricmp("off",value))    *V=0;
      else if(!stricmp("true",value) || !stricmp("on",value)) *V=1;
      else *V=atol(value);
    }
    if(F) this->F();
  }

  void *Value() {
    if(A) return &A->ID;
    else if(V) return V;
    else if(C) return C;
    return NULL;
  }

  long Ready() {
    if(A) return A->Ready("");
    return NOID;
  }
  
  void Reset() {
    if(K[0]) {
      if(C) *C=D;
      else {
        strcpy(Reserved,D);
        if(Reserved[0]) ParseMacroData(Reserved);
        long value=atol(Reserved);
        if(A) A->Setup(value,0);
        else if(V) *V=value;

      }
      if(F) this->F();
    }
  }
};

typedef map<string,Option> Liste;     // declare a type so more easy to refer
Liste     CmdListe;                   // settings from command or ini
Liste     IniListe;                   // settings from ini only
Liste     VarListe;                   // settings from var liste

CHAR      section[256];               // ini section

LONG      doAGGRO,
          doASSASINATE,
          doBACKOFF,
          doBACKSTAB,
          doBASH,
          doBEGGING,
          doCALLCHALLENGE,
          doCHALLENGEFOR,
          doCOMMANDING,
          doCRYHAVOC,
          doDISARM,
          doDRAGONPUNCH,
          doEAGLESTRIKE,
          doENRAGE,
          doESCAPE,
          doEVADE,
          doFEIGNDEATH,
          doFACING,
          doFALLS,
          doFERALSWIPE,
          doFISTSOFWU,
          doFLYINGKICK,
          doFORAGE,
          doFRENZY,
          doHARMTOUCH,
          doHIDE,
          doINFURIATE,
          doINTIMIDATION,
          doJOLT,
          doKICK,
          doLAYHAND,
          doLEOPARDCLAW,
          doMELEE,
          doMEND,
          doPETASSIST,
          doPETDELAY,
          doPETRANGE,
          doPETMEND,
          doPICKPOCKET,
          doSKILL,
          doSTRIKE,
          doPROVOKEMAX,
          doPROVOKEEND,
          doRAGEVOLLEY,
          doRAKE,
          doRANGE,
          doRESUME,
          doROUNDKICK,
          doSENSETRAP,
          doSLAM,
          doSNEAK,
          doSTAB,
          doSTAND,
          doSTICKDELAY,
          doSTICKRANGE,
          doSTICKMODE,
          doSTUNNING,
          doTAUNT,
          doTHIEFEYE,
          doTHROWSTONE,
          doTIGERCLAW,
          doTWISTEDSHANK,
          doDOWNFLAG[8],
          doPOTHEALFAST,
          doPOTHEALOVER,
          doHOLYFLAG[8];

LONG      elARROWS,
          elAGGROPRI,
          elAGGROSEC,
          elMELEEPRI,
          elMELEESEC,
          elPOKER,
          elRANGED,
          elSHIELD;

string    ifBACKSTAB,
          ifBASH,
          ifBEGGING,
          ifCALLCHALLENGE,
          ifCHALLENGEFOR,
          ifCOMMANDING,
          ifCRYHAVOC,
          ifDISARM,
          ifDRAGONPUNCH,
          ifEAGLESTRIKE,
          ifEVADE,
          ifFALLS,
          ifFERALSWIPE,
          ifFISTSOFWU,
          ifFLYINGKICK,
          ifFORAGE,
          ifFRENZY,
          ifHARMTOUCH,
          ifHIDE,
          ifINTIMIDATION,
          ifJOLT,
          ifKICK,
          ifLAYHAND,
          ifLEOPARDCLAW,
          ifMEND,
          ifPICKPOCKET,
          ifPOTHEALFAST,
          ifPOTHEALOVER,
          ifPROVOKE,
          ifRAGEVOLLEY,
          ifRAKE,
          ifROUNDKICK,
          ifSENSETRAP,
          ifSLAM,
          ifSNEAK,
          ifSTRIKE,
          ifSTUNNING,
          ifTAUNT,
          ifTHIEFEYE,
          ifTHROWSTONE,
          ifTIGERCLAW,
          ifTWISTEDSHANK,
          DOWNSHIT[8],
          HOLYSHIT[8],
          StickCMD;

Ability   idBACKSTAB,
          idBASH,
          idBEGGING,
          idCALLCHALLENGE,
          idCHALLENGEFOR,
          idCOMMANDING,
          idCRYHAVOC,
          idDISARM,
          idDRAGONPUNCH,
          idEAGLESTRIKE,
          idESCAPE,
          idFEIGN[2],
          idFERALSWIPE,
          idFISTSOFWU,
          idFLYINGKICK,
          idFORAGE,
          idFRENZY,
          idHARMTOUCH,
          idHIDE,
          idINTIMIDATION,
          idJOLT,
          idKICK,
          idLAYHAND,
          idLEOPARDCLAW,
          idMEND,
          idPETMEND,
          idPICKPOCKET,
          idPOTHEALFAST,
          idPOTHEALOVER,
          idPROVOKE[2],
          idRAGEVOLLEY,
          idRAKE,
          idROUNDKICK,
          idSENSETRAP,
          idSLAM,
          idSNEAK,
          idSTUN[2],
          idSTRIKE,
          idTAUNT,
          idTIGERCLAW,
          idTHIEFEYE,
          idTHROWSTONE,
          idTWISTEDSHANK;

DWORD     Shrouded       =false;       // True when shrouded.
bool      Binded         =false;       // Attack Key is Binded?
bool      Loaded         =false;       // Loaded?
bool      Moving         =false;       // Moving?
bool      Immobile       =false;       // Immobilized?
bool      AutoFire       =false;       // True when autofire is on. 
bool      HaveBash       =false;       // Have Two Hand Bash?
bool      HaveHold       =false;       // Have Pet Hold?
DWORD     BrokenFD       =0;           // Timer for Broken Feign Death

float     Travel         =0.0f;        // Travel Speed?
long      Health         =0;           // Current Health

DWORD     MeleeTime      =0;           // Melee Pulse Timer
long      MeleeTarg      =0;           // Melee Target ID 
long      MeleeType      =0;           // Melee Target Type
long      MeleeFlee      =0;           // Melee Target Fleeing?
long      MeleeLife      =0;           // Melee Target Life % 
long      MeleeCast      =0;           // Melee Target Cast ? 
long      MeleeSize      =0;           // Melee Name Size 
char      MeleeName[64]  ={0};         // Melee Name 

double    MeleeSpeed     =0.0f;        // Melee Target Speed
double    MeleeBack      =0.0f;        // Melee Target Angle Back 
double    MeleeView      =0.0f;        // Melee Target Angle View 
double    MeleeDist      =0.0f;        // Melee Distance to Target
double    MeleeKill      =0.0f;        // Melee Distance to Use Ability

long      onEVENT        =false;       // Ranged=0x8000,Begging=0x2000,PickPocket=0x1000,Feign=0x0040,Hide=0x0020,Backoff=0x0010,Infuriate=0x0002,Enrage=0x0001 
long      onSTICK        =false;       // Do Stick? (turn false when stick command is issue) 
long      onBELOW        =false;       // Below Flag? (turn false when no more provoke counter)
long      onCHALLENGEFOR =false;       // Challenge Flag? (turn to false when use once)

DWORD     PetInDist      =0;           // Pet Target in Range 
DWORD     PetOnAttk      =0;           // Pet Seen Attacking TimeStamp? 
DWORD     PetOnHold      =0;           // Pet Hold?
DWORD     PetOnWait      =0;           // Pet Wait Assist Delay TimeStamp 
DWORD     PetTarget      =0;           // Pet Target ID

DWORD     TimerAttk      =0;           // Timer Attk
DWORD     TimerBack      =0;           // Timer BackOff/Escape/Feign 
DWORD     TimerMove      =0;           // Timer Move 
DWORD     TimerLife      =0;           // Timer Life (Target his dieing)
DWORD     TimerFace      =0;           // Face Time Stamp when started 
DWORD     TimerStik      =0;           // Stik Time Stamp when started
DWORD     TimerStun      =0;           // Timer Stun

long      SwingHits      =0;           // Total Hits
long      TakenHits      =0;           // Under Hits

long      doHOLY=0;                    // Holy Shits while meleeing?
long      doDOWN=0;                    // Down Shits while downtime?

Blech    *pMeleeEvent=0;               // blech event list
bool      ColorArray[512];             // blech color filtering
long      SaveList[50];                // saved event list
long      SaveIndx;                    // saved event counters

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class MQ2MeleeType *pMeleeTypes=0; 
class MQ2MeleeType : public MQ2Type { 
private: 
  long isKill; 
  char Tempos[MAX_STRING]; 
public: 
  enum Information { 
    Enable=1, 
    Combat=2, 
    Casted=3, 
    Engage=4, 
    Status=5, 
    Target=6, 
    DiscID=7,
    Enrage=8, 
    Infuriate=9, 
    AggroMode=10, 
    MeleeMode=11, 
    RangeMode=12, 
    BackAngle=13, 
    ViewAngle=14, 
    Immobilize=15, 
    Ammunition=16, 
    BackStabbing=17, 
    GotAggro=18, 
  }; 
  MQ2MeleeType():MQ2Type("Melee") { 
    TypeMember(Enable); 
    TypeMember(Combat); 
    TypeMember(Casted); 
    TypeMember(Status); 
    TypeMember(Target); 
    TypeMember(DiscID); 
    TypeMember(GotAggro); 
    TypeMember(AggroMode); 
    TypeMember(MeleeMode); 
    TypeMember(RangeMode); 
    TypeMember(Enrage); 
    TypeMember(Infuriate); 
    TypeMember(BackAngle); 
    TypeMember(ViewAngle); 
    TypeMember(Immobilize); 
    TypeMember(Ammunition); 
    TypeMember(BackStabbing); 
  } 
  bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) { 
    PMQ2TYPEMEMBER pMember=MQ2MeleeType::FindMember(Member); 
    isKill=false; if(doSKILL) if(MeleeTarg) isKill=true; 
    if(pMember) switch((Information)pMember->ID) { 
    case Enable: 
      Dest.DWord=doSKILL; 
      Dest.Type=pBoolType; 
      return true; 
    case Combat: 
      Dest.DWord=isKill; 
      Dest.Type=pBoolType; 
      return true; 
    case Casted: 
      Dest.Int=(isKill && MeleeCast)?labs((DWORD)clock()-MeleeCast):60000; 
      Dest.Type=pIntType; 
      return true; 
    case Status: 
      Tempos[0]=0; 
      if(isKill) strcat(Tempos,"ENGAGED "); 
      else strcat(Tempos,"WAITING "); 
      if(*EQADDR_ATTACK) strcat(Tempos, "MELEE "); 
      else if(onEVENT&0x8000) strcat(Tempos,"RANGE "); 
      if(onEVENT&0x0001) strcat(Tempos,"ENRAGE "); 
      if(onEVENT&0x0002) strcat(Tempos,"INFURIATE "); 
      if(onEVENT&0x0010) strcat(Tempos,"BACKING "); 
      if(onEVENT&0x0020) strcat(Tempos,"ESCAPING "); 
      if(onEVENT&0x0040) strcat(Tempos,"FEIGNING "); 
      if(onEVENT&0x0200) strcat(Tempos,"EVADING ");
      if(onEVENT&0x0400) strcat(Tempos,"FALLING ");
      if(onEVENT&0x1000) strcat(Tempos,"STEALING ");
      if(onEVENT&0x2000) strcat(Tempos,"BEGGING ");
      Dest.Type=pStringType; 
      Dest.Ptr=Tempos; 
      return true; 
    case Target: 
      Dest.Int=isKill?MeleeTarg:0; 
      Dest.Type=pIntType; 
      return true; 
    case DiscID: 
      Dest.DWord=Discipline();
      Dest.Type=pIntType; 
      return true; 
    case GotAggro: 
      Dest.DWord=(Aggroed(MeleeTarg)>0);
      Dest.Type=pBoolType; 
      return true; 
    case AggroMode: 
      Dest.DWord=doAGGRO; 
      Dest.Type=pBoolType; 
      return true; 
    case MeleeMode: 
      Dest.DWord=doMELEE; 
      Dest.Type=pBoolType; 
      return true; 
    case RangeMode: 
      Dest.DWord=doRANGE; 
      Dest.Type=pBoolType; 
      return true; 
    case Enrage: 
      Dest.DWord=onEVENT&0x0001; 
      Dest.Type=pBoolType; 
      return true; 
    case Infuriate: 
      Dest.DWord=onEVENT&0x0002; 
      Dest.Type=pBoolType; 
      return true; 
    case BackAngle: 
      Dest.Float=pTarget?AngularDistance(((PSPAWNINFO)pTarget)->Heading,SpawnMe()->Heading):0.0f; 
      Dest.Type=pFloatType; 
      return true; 
    case ViewAngle: 
      Dest.Float=pTarget?(float)AngularHeading(SpawnMe(),(PSPAWNINFO)pTarget):0.0f; 
      Dest.Type=pFloatType; 
      return true; 
    case Immobilize: 
      Dest.DWord=Immobile; 
      Dest.Type=pBoolType; 
      return true; 
    case Ammunition: 
      Dest.DWord=ItemCounts(elARROWS); 
      if(PCONTENTS r=GetCharInfo2()->Inventory.Ammo) 
        if(r->Item->ItemNumber != elARROWS) 
          if(r->Item->ItemType==7 || r->Item->ItemType==19 || r->Item->ItemType==27) 
            Dest.DWord=ItemCounts(r->Item->ItemNumber); 
      Dest.Type=pIntType; 
      return true; 
    case BackStabbing: 
      Dest.DWord=doBACKSTAB; 
      Dest.Type=pBoolType; 
      return true; 
    } 
    strcpy(Tempos,"NULL"); 
    Dest.Type=pStringType; 
    Dest.Ptr=Tempos; 
    return true; 
  } 
  bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) { 
    strcpy(Destination,"TRUE"); 
    return true; 
  } 
  bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) { 
    return false; 
  } 
  bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) { 
    return false; 
  } 
  ~MQ2MeleeType() { } 
}; 

BOOL DataMelee(PCHAR Index, MQ2TYPEVAR &Dest) { 
  Dest.Type=pMeleeTypes; 
  Dest.DWord=1; 
  return true; 
} 

BOOL datameleemvb(PCHAR Index, MQ2TYPEVAR &Dest) {
  Dest.Type=pIntType;
  Dest.Int=NOID;
  Liste::iterator c;
  if(VarListe.end()!=(c=VarListe.find(Index)))
    Dest.Int=(*c).second.Ready();
  return true;
}

BOOL datameleemvi(PCHAR Index, MQ2TYPEVAR &Dest) {
  Dest.Type=pIntType;
  Dest.DWord=0;
  Liste::iterator c;
  if(CmdListe.end()!=(c=CmdListe.find(Index))) {
    if(long *V=(long*)(*c).second.Value()) Dest.DWord=*V;
    return true;
  }
  if(VarListe.end()!=(c=VarListe.find(Index))) {
    if(long *V=(long*)(*c).second.Value()) Dest.DWord=*V;
    return true;
  }   
  return true;
}

BOOL datameleemvs(PCHAR Index, MQ2TYPEVAR &Dest) {
  Dest.Type=pStringType;
  Dest.Ptr=&Workings;
  Liste::iterator c=IniListe.find(Index);
  if(IniListe.end()!=c) {
    if(string *S=(string*)(*c).second.Value())
      strcpy(Workings,S->c_str());
  } else Workings[0]=0;
  return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void AbilityFind(Ability *thisone, infodata *first, ...) {
  infodata *c=first;
  va_list  marker;
  va_start(marker,first);
  while(c) {
    thisone->Setup(c->i,c->t);
    if(thisone->Avail()) break;
    thisone->ID=0;
    c=va_arg(marker,infodata *);
  }
  va_end(marker);
}

void AttackON() {
  if(*EQADDR_ATTACK || onEVENT&0xFFF7 || IsFeigning() || !doMELEE || !TargetID(MeleeTarg)) return;
  EzCommand("/attack on");
  TimerAttk=(DWORD)clock();
} 

void AttackOFF() {
  if(*EQADDR_ATTACK) EzCommand("/attack off");
} 

bool BashCheck() {
  if(PCONTENTS off=GetCharInfo2()->Inventory.Secondary)
    if(off->Item->ItemType == 8) 
      return true;
  if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary)
    if(pri->Item->ItemType==1 || pri->Item->ItemType==4  || pri->Item->ItemType==35)
      if(HaveBash) return true;
  return (elSHIELD && ItemCounts(elSHIELD) && OkayToEquip(Giant));
} 

void BashPress() {
  long savedpri=0; // saved primary item id
  long savedoff=0; // saved offhand item id
  bool got2hand=0; // have a 2 handler equipped?
  if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary) {
    got2hand=(pri->Item->ItemType==1 || pri->Item->ItemType==4 || pri->Item->ItemType==35);
    savedpri=pri->Item->ItemNumber;
  }
  if(PCONTENTS off=GetCharInfo2()->Inventory.Secondary) savedoff=off->Item->ItemNumber;
  if(elSHIELD && ItemCounts(elSHIELD) && OkayToEquip(Giant)) Equip(elSHIELD,14);
  PCONTENTS sec=GetCharInfo2()->Inventory.Secondary;
  if((sec && sec->Item->ItemType==8) || (got2hand && HaveBash)) idBASH.Press();
  if(savedoff) Equip(savedoff,14);
  if(savedpri) Equip(savedpri,13);
} 

void Configure() {
  long Class=GetCharInfo2()->Class;
  long Races=GetCharInfo2()->Race;
  long Level=GetCharInfo2()->Level;
  sprintf(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
  sprintf(section,"%s_%d_%s_%s",PLUGIN_NAME,Level,pEverQuest->GetRaceDesc(Races),pEverQuest->GetClassDesc(Class));
  Shrouded=GetCharInfo2()->Shrouded; if(!Shrouded) section[strlen(PLUGIN_NAME)]=0;
  BuffMax=15;
  if(GetAAIndexByName("Embrace of the Dark Reign"))   BuffMax++;
  else if(GetAAIndexByName("Embrace of the Keepers")) BuffMax++;
  BuffMax+=(AAPoint(GetAAIndexByName("Mystical Attuning")))/5;
  if(SpawnMe()->Level>71) BuffMax++;
  if(SpawnMe()->Level>74) BuffMax++;
  GemsMax=(GetAAIndexByName("Mnemonic Retention"))?9:8;
  HaveHold=GetAAIndexByName("Pet Discipline")?true:false;
  HaveBash=GetAAIndexByName("2 Hand Bash")?true:false;
  BardClass=false;
  char keys[MAX_STRING*5];
  char temp[MAX_STRING];
  Liste::iterator c,i;
  Liste::iterator ec=CmdListe.end();
  Liste::iterator ei=IniListe.end();
  for(c=CmdListe.begin(); c!=ec; c++) (*c).second.Reset();
  for(i=IniListe.begin(); i!=ei; i++) (*i).second.Reset();

  idLEOPARDCLAW.Setup (0,0);
  idCHALLENGEFOR.Setup(0,0);
  idRAGEVOLLEY.Setup  (0,0);
  idPETMEND.Setup     (0,0);
  idJOLT.Setup        (0,0);
  idFERALSWIPE.Setup  (0,0);
  idFEIGN[0].Setup    (0,0);
  idFEIGN[1].Setup    (0,0);
  idPROVOKE[0].Setup  (0,0);
  idPROVOKE[1].Setup  (0,0);
  idSTRIKE.Setup      (0,0);
  idSTUN[0].Setup     (0,0);
  idSTUN[1].Setup     (0,0);

  AbilityFind(&idBACKSTAB,&sbkstab,0);
  AbilityFind(&idBASH,&sbash,0);
  AbilityFind(&idBEGGING,&sbegging,0);
  AbilityFind(&idDISARM,&sdisarm,0);
  AbilityFind(&idDRAGONPUNCH,&sdrpunch,0);
  AbilityFind(&idEAGLESTRIKE,&sestrike,0);
  AbilityFind(&idFLYINGKICK,&sflykick,0);
  AbilityFind(&idFORAGE,&sforage,0);
  AbilityFind(&idFRENZY,&sfrenzy,0);
  AbilityFind(&idHARMTOUCH,&sharmtou,0);
  AbilityFind(&idHIDE,&shide,0);
  AbilityFind(&idINTIMIDATION,&sintim,0);
  AbilityFind(&idKICK,&skick,0);
  AbilityFind(&idLAYHAND,&slayhand,0);
  AbilityFind(&idMEND,&smend,0);
  AbilityFind(&idPICKPOCKET,&sppocket,0);
  AbilityFind(&idROUNDKICK,&srndkick,0);
  AbilityFind(&idSENSETRAP,&ssensetr,0);
  AbilityFind(&idSLAM,&sslam,0);
  AbilityFind(&idSNEAK,&ssneak,0);
  AbilityFind(&idTAUNT,&staunt,0);
  AbilityFind(&idTIGERCLAW,&stigclaw,0);
  AbilityFind(&idCALLCHALLENGE,&callchal,0);
  AbilityFind(&idESCAPE,&escape,0);
  AbilityFind(&idTWISTEDSHANK,&twisted,0);
  AbilityFind(&idTHROWSTONE,&tstone,0);
  AbilityFind(&idCOMMANDING,&cmmding,0);
  AbilityFind(&idFISTSOFWU,&fistswu,0);
  AbilityFind(&idTHIEFEYE,&thiefeye,0);
  AbilityFind(&idRAKE,&rake,0);
  AbilityFind(&idCRYHAVOC,&cryhavoc,0);
  AbilityFind(&idPOTHEALOVER,&potover9,&potover8,&potover7,&potover6,&potover5,&potover4,&potover3,&potover2,&potover1,&potover0,0);
  AbilityFind(&idPOTHEALFAST,&potfast9,&potfast8,&potfast7,&potfast6,&potfast5,&potfast4,&potfast3,&potfast2,&potfast1,&potfast0,0);
  doSTAB=0;
  switch(Class) {
    case  1: // WAR
      AbilityFind(&idPROVOKE[1],&prowar_j,&prowar_i,&prowar_h,&prowar_g,&prowar_f,&prowar_e,&prowar_d,&prowar_c,&prowar_b,&prowar_a,0);
      break;
    case  3: // PAL
      AbilityFind(&idCHALLENGEFOR,&honorc,&honorb,&honora,0);
      AbilityFind(&idPROVOKE[0],&stunpal3,&stunpal2,&stunpal1,0);
      AbilityFind(&idPROVOKE[1],&stunpalj,&stunpali,&stunpalh,&stunpalg,&stunpalf,&stunpale,&stunpald,&stunpalc,&stunpalb,&stunpala,0);
      AbilityFind(&idSTUN[0],&stunpal3,&stunpal2,&stunpal1,0);
      AbilityFind(&idSTUN[1],&stunpalj,&stunpali,&stunpalh,&stunpalg,&stunpalf,&stunpale,&stunpald,&stunpalc,&stunpalb,&stunpala,0);
      break;
    case  4: // RNG
      AbilityFind(&idJOLT,&joltrng2,&joltrng1,0);
      break;
    case  5: // SHD
      AbilityFind(&idFEIGN[0],&feigns6,&feigns5,&feigns4,&feigns3,&feigns2,&feigns1,0);
      AbilityFind(&idFEIGN[1],&feigndp,0);
      AbilityFind(&idCHALLENGEFOR,&powerc,&powerb,&powera,0);
      AbilityFind(&idPROVOKE[1],&terror7,&terror6,&terror5,&terror4,&terror3,&terror2,&terror1,0);
      break;
    case  7: // MNK
      AbilityFind(&idFEIGN[0],&sfeign,0);
      AbilityFind(&idFEIGN[1],&feignid,0);
      AbilityFind(&idPROVOKE[0],&stunmnk2,&stunmnk1,0);
      AbilityFind(&idSTUN[0],&stunmnk2,&stunmnk1,0);
      AbilityFind(&idLEOPARDCLAW,&leop5,&leop4,&leop3,&leop2,&leop1,0);
      break;
    case  8: // BRD
      BardClass=true;
      break;
    case  9: // ROG
      AbilityFind(&idSTRIKE,&strike7,&strike6,&strike5,&strike4,&strike3,&strike2,&strike1,0);
      switch(AAPoint(GetAAIndexByName("Seized Opportunity"))) {
        case 18: doSTAB=256; break;
        case  9: doSTAB=192; break;
        case  3: doSTAB=128; break;
        case  0: doSTAB=64;  break;
      }
      break;
    case 11: // NEC
      AbilityFind(&idFEIGN[0],&feigns3,&feigns2,&feigns1,0);
      AbilityFind(&idFEIGN[1],&feigndp,0);
      AbilityFind(&idPETMEND,&mendpet2,&mendpet1,0);
      break;
    case 13: // MAG
      AbilityFind(&idPETMEND,&mendpet2,&mendpet1,0);
      break;
    case 15: // BST
      AbilityFind(&idFERALSWIPE,&feral2,&feral1,0);
      AbilityFind(&idPETMEND,&mendpet1,&mendpet2,0);
      AbilityFind(&idJOLT,&joltbst2,&joltbst1,0);
      break;
    case 16: // BER
      AbilityFind(&idJOLT,&joltber7,&joltber6,&joltber5,&joltber4,&joltber3,&joltber2,&joltber1,0);
      AbilityFind(&idPROVOKE[1],&stunber7,&stunber6,&stunber5,&stunber4,&stunber3,&stunber2,&stunber1,0);
      AbilityFind(&idSTUN[1],&stunber7,&stunber6,&stunber5,&stunber4,&stunber3,&stunber2,&stunber1,0);
      AbilityFind(&idRAGEVOLLEY,&volley5,&volley4,&volley3,&volley2,&volley1,0);
      break;
  }
  if(GetPrivateProfileString(section,NULL,"",keys,sizeof(keys),INIFileName)) {
    PCHAR pkeys=keys;
    while(pkeys[0]) {
      if(GetPrivateProfileString(section,pkeys,"",temp,sizeof(temp),INIFileName)) {
        _strlwr(pkeys);
        if(ec!=(c=CmdListe.find(pkeys)))      (*c).second.Setup(temp);
        else if(ei!=(i=IniListe.find(pkeys))) (*i).second.Setup(temp);
      }
      pkeys+=strlen(pkeys)+1;
    }
  }
  Loaded=true;
}

void Exporting() {
  char output[MAX_STRING];
  char defval[MAX_STRING];
  Liste::iterator c,e;
  WritePrivateProfileString(section,NULL,NULL,INIFileName);
  e=CmdListe.end();
  for(c=CmdListe.begin(); c!=e; c++) {
    output[0]=0;
    if((*c).second.C) 
      if(string *S=(string*)(*c).second.Value()) 
        strcpy(output,S->c_str());
    if((*c).second.A || (*c).second.V) 
      if(long *V=(long*)(*c).second.Value()) 
        itoa(*V,output,10);
    if(output[0]) {
      strcpy(defval,(*c).second.D);
      if(defval[0]) ParseMacroData(defval);
      if(strcmp(output,"0") || strcmp(output,defval)) 
        WritePrivateProfileString(section,(*c).second.K,output,INIFileName);
    }
  }
  e=IniListe.end();
  for(c=IniListe.begin(); c!=e; c++) {
    output[0]=0;
    if((*c).second.C) 
      if(string *S=(string*)(*c).second.Value()) 
        strcpy(output,S->c_str());
    if((*c).second.A || (*c).second.V) 
      if(long *V=(long*)(*c).second.Value()) 
        itoa(*V,output,10);
    if(output[0]) {
      strcpy(defval,(*c).second.D);
      if(defval[0]) ParseMacroData(defval);
      if(strcmp(output,"0") || strcmp(output,defval)) 
        WritePrivateProfileString(section,(*c).second.K,output,INIFileName);
    }
  }
  sprintf(output,"%1.3f",PLUGIN_VERS); WritePrivateProfileString(section,"version",output,INIFileName);
}

void MapInsert(Liste *MyList, Option MyOption) {
  MyList->insert(Liste::value_type(MyOption.K,MyOption));
}

void MeleeHelp() {
  WriteChatf("%s::-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-",PLUGIN_NAME); 
  WriteChatf("%s::Version [\ag%1.3f\ax] Loaded!",PLUGIN_NAME,PLUGIN_VERS); 
  WriteChatf("%s::-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-",PLUGIN_NAME);
  for(Liste::iterator i=CmdListe.begin(); i!=CmdListe.end(); i++) (*i).second.Write();
  WriteChatf("%s::-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-",PLUGIN_NAME);
  if(NULL==PluginEntry("mq2cast","CastCommand"))
    WriteChatf("%s::Required Latest [\arMQ2Cast\ax] for AA/SPELL/ITEM casting.",PLUGIN_NAME);
  if(NULL==PluginEntry("mq2moveutils","StickCommand"))
    WriteChatf("%s::Required Latest [\arMQ2MoveUtils\ax] for MOVEMENT.",PLUGIN_NAME);
}

void PetSEEN() {
  PetOnAttk=(DWORD)clock()+6000;
  PetOnHold=false;
}

void PetBACK(bool HoldOnly) {
  PetOnHold=true;
  if(doPETASSIST && pPetInfoWnd && XMLEnabled(XMLChild((CXWnd*)pPetInfoWnd,UI_PetBack))) {
    if(!HoldOnly) {
      #if    SHOW_CONTROL > 0
        WriteChatf("%s::Command [\ay%s\ax].",PLUGIN_NAME,"/pet back");
      #endif SHOW_CONTROL
      EzCommand("/pet back");
    }
    if(HaveHold) {
      #if    SHOW_CONTROL > 0
        WriteChatf("%s::Command [\ay%s\ax].",PLUGIN_NAME,"/pet hold");
      #endif SHOW_CONTROL
      EzCommand("/pet hold");
    }
  }
}

void PetATTK() {
  PetSEEN();
  if(doPETASSIST && pPetInfoWnd && !(onEVENT&0x0003) && TargetID(MeleeTarg) && XMLEnabled(XMLChild((CXWnd*)pPetInfoWnd,UI_PetAttk))) {
    #if    SHOW_CONTROL > 0
      WriteChatf("%s::Command [\ay%s\ax].",PLUGIN_NAME,"/pet attack");
    #endif SHOW_CONTROL
    EzCommand("/pet attack");
    PetTarget=MeleeTarg;
  }
}

void StickReset() {
  TimerStik=0;
  if(Sticking) Stick("");
  StickArg[0]=0;
  onSTICK=(doMELEE && doSTICKRANGE && !(onEVENT&0x8000) && Plugin("mq2moveutils"));
}

void RangeReset() {
  if(!doRANGE && AutoFire) EzCommand("/autofire");
  if(!doRANGE && (onEVENT&0x8000)) onEVENT&=0x7FFF;
}

void OtherReset() {
  MeleeTarg=0; 
  MeleeType=0; 
  MeleeLife=0; 
  MeleeCast=0; 
  MeleeFlee=0;
  TimerBack=0; 
  TimerLife=0; 
  TimerFace=0; 
  TimerStun=0; 
  PetTarget=0; 
  PetOnWait=0; 
  SwingHits=0; 
  TakenHits=0;
  onEVENT=0;   
  doDOWN=0;    
  doHOLY=0;
} 

void MeleeReset() {
  if(!doMELEE && *EQADDR_ATTACK) {
    AttackOFF();
    StickReset();
  }
}

void AggroReset() {
  BOOL onAGGRO=(doAGGRO && IsGrouped());
  onCHALLENGEFOR=(onAGGRO && idCHALLENGEFOR.Found());
  onBELOW=(onAGGRO && doPROVOKEMAX)?doPROVOKEMAX:0;
  if(doMELEE) {
    if(OkayToEquip(Giant)) {
      long PW=(IsGrouped() && doAGGRO)?elAGGROPRI:elMELEEPRI;
      if(PW && ItemCounts(PW)) {
        PCONTENTS pri=GetCharInfo2()->Inventory.Primary;
        if(!pri || pri->Item->ItemNumber!=PW) Equip(PW,13);
      }       
      if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary)
        if(pri->Item->ItemType==1 || pri->Item->ItemType==4 || pri->Item->ItemType==35) return;
      long SW=(IsGrouped() && doAGGRO)?elAGGROSEC:elMELEESEC;
      if(SW && ItemCounts(SW)) {
        PCONTENTS sec=GetCharInfo2()->Inventory.Secondary;
        if(!sec || sec->Item->ItemNumber!=SW) Equip(SW,14);
      }
    }
  }
}

VOID HideOFF() {
  if(IsInvisible() && idHIDE.Found()) idHIDE.Press();
}

VOID SneakOFF() {
  if(IsSneaking() && idSNEAK.Found()) idSNEAK.Press();
}

bool StabCheck() { 
  if(elPOKER && OkayToEquip(Giant))
    if(PCONTENTS fITEM=ItemLocate(elPOKER))
      if(fITEM->Item->ItemType==2) 
        return true;
  if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary)
    if(pri->Item->ItemType==2)
      return true;
  return false; 
} 

void StabPress() { 
  long saveid=0;
  if(elPOKER && OkayToEquip(Giant) && ItemLocate(elPOKER,0,SlotMax,InvSlot)) {
    if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary)
      if(pri->Item->ItemNumber!=elPOKER) {
        saveid=pri->Item->ItemNumber;
        Equip(elPOKER,13);
      }
  }
  if(PCONTENTS pri=GetCharInfo2()->Inventory.Primary)
    if(pri->Item->ItemType==2) {
      idBACKSTAB.Press();
      SwingHits++;
    }
  if(saveid) Equip(saveid,13);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API VOID ThrowIT(PSPAWNINFO pChar, PCHAR Cmd) {
  if(gbRangedAttackReady && pTarget && TargetType(NPC_TYPE) && 
     !InRange(SpawnMe(),(PSPAWNINFO)pTarget,35) &&
     fabs(AngularHeading(SpawnMe(),(PSPAWNINFO)pTarget))<50 &&
     LineOfSight(SpawnMe(),(PSPAWNINFO)pTarget)) {

    // test if we could do ranged with current ammo/range configuration
    long crT=99; long crI=0; long caT=99; long caI=0; long caQ=0; 
    if(PCONTENTS r=GetCharInfo2()->Inventory.Range) { 
      crI=r->Item->ItemNumber; 
      crT=r->Item->ItemType; 
    } 
    if(PCONTENTS a=GetCharInfo2()->Inventory.Ammo) { 
      caI=a->Item->ItemNumber; 
      caT=a->Item->ItemType; 
      if(caT == 7 || caT == 19 || caT == 27) caQ=ItemCounts(a->Item->ItemNumber); 
    } 
    if(!(caI && ((caT == 27 && crT == 5) || (caI == crI && (caT == 7 || caT == 19))))) {
      if(!OkayToEquip(4)) return;
      
      // grab information about user defined range/ammunition
      long erT=99; long eaT=99; long eaQ=0; 
      if(PCONTENTS r=ItemLocate(elRANGED)) erT=r->Item->ItemType;
      if(PCONTENTS a=ItemLocate(elARROWS)) {
        eaT=a->Item->ItemType; 
        if(eaT == 7 || eaT == 19 || eaT == 27) eaQ=ItemCounts(a->Item->ItemNumber); 
      } 

      // find equipping scenario (bow+arrow) or (throw/throw).
      long EquipRangeID=0; long EquipArrowID=0; 
      if((crT == 5 || erT == 5) && (caT == 27 || eaT == 27)) { 
        EquipRangeID=(crT== 5)?crI:elRANGED;
        EquipArrowID=(caT==27)?caI:elARROWS;
      } 
      else if((caQ > 2 && (caT == 7 || caT == 19)) || (eaQ > 2 && (eaT == 7 || eaT == 19))) 
        EquipArrowID=(caQ>2 &&(caT==7 || caT==19))?caI:elARROWS;
      else return; 
  
      // load equipping scenario found!
      if(EquipRangeID) { 
        if(crI!=EquipRangeID) Equip(EquipRangeID,11);
        if(caI!=EquipArrowID) Equip(EquipArrowID,21);
      } else { 
        if(crI) Unequip(11);
        if(caI!=EquipArrowID) Equip(EquipArrowID,21);
        ItemLocate(EquipArrowID,WornMax); long AmmPack=InvSlot;
        if(AmmPack) pInvSlotMgr->MoveItem(AmmPack,30,1,1);
        else WinClick((CXWnd*)pInventoryWnd,"InvSlot21","leftmouseup",Ctrlkey);
        WinClick((CXWnd*)pInventoryWnd,"InvSlot11","leftmouseup",Shiftkey); 
      } 
    } 

    // more sanity double check in case we didnt exchange stuff
    PCONTENTS rSlot=GetCharInfo2()->Inventory.Range; if(!rSlot) return; 
    PCONTENTS aSlot=GetCharInfo2()->Inventory.Ammo;  if(!aSlot) return; 
    long rType=rSlot->Item->ItemType; if(!(rType == 5 || rType == 7  || rType == 19)) return; 
    long aType=aSlot->Item->ItemType; if(!(aType == 7 || aType == 19 || aType == 27)) return; 
    long aKind=aSlot->Item->ItemNumber; long rKind=rSlot->Item->ItemNumber; 
    if(!((aType == 27 && rType == 5) || (aType!=27 && rKind == aKind))) return; 

    // good time to refill ammunitions?
    if(OkayToEquip()) {
      while(aSlot->StackCount < 80 && ItemCounts(aKind,WornMax)>0) { 
        ItemLocate(aKind,WornMax); long aPack=InvSlot;
        pInvSlotMgr->MoveItem(aPack,30,1,1);
        WinClick((CXWnd*)pInventoryWnd,"InvSlot21","leftmouseup",Shiftkey); 
        if(!CursorEmpty()) pInvSlotMgr->MoveItem(30,aPack,1,1);
      } 
      if(aType!=27 && aKind == rKind) { 
        if(aSlot->StackCount < 80 && rSlot->StackCount>1) { 
          pInvSlotMgr->MoveItem(11,30,1,1); 
          WinClick((CXWnd*)pInventoryWnd,"InvSlot21","leftmouseup",Shiftkey); 
          if(CursorEmpty()) WinClick((CXWnd*)pInventoryWnd,"InvSlot21","leftmouseup",Ctrlkey); 
          pInvSlotMgr->MoveItem(30,11,1,1); 
        } 
        while(rSlot->StackCount < 80 && ItemCounts(aKind,WornMax)>0) { 
          ItemLocate(aKind,WornMax); long aPack=InvSlot;
          pInvSlotMgr->MoveItem(aPack,30,1,1); 
          WinClick((CXWnd*)pInventoryWnd,"InvSlot11","leftmouseup",Shiftkey); 
          if(!CursorEmpty()) pInvSlotMgr->MoveItem(30,aPack,1,1); 
        } 
      } 
    } 

    // fire ranged attack if NPC on Target still in range and preserve the range slot.
    do_ranged(SpawnMe(),"");
    if(crI) if(rKind!=crI) if(OkayToEquip()) Equip(crI,11);
  }
} 

PLUGIN_API VOID Override(PSPAWNINFO pChar, PCHAR Cmd) {
  #if    SHOW_OVERRIDE > 0
    if(Cmd[0]) WriteChatf(Cmd,PLUGIN_NAME);
  #endif SHOW_OVERRIDE
  AttackOFF();
  if(AutoFire) { 
    AutoFire=false; 
    EzCommand("/autofire");
  } 
  PetBACK(false);
  StickReset();
  OtherReset();
} 

PLUGIN_API VOID Melee(PSPAWNINFO pChar, PCHAR Cmd) {
  char Tmp[MAX_STRING]; char Var[MAX_STRING]; char Set[MAX_STRING]; BYTE Parm=1; bool Help=true;
  Liste::iterator c; Liste::iterator ec=CmdListe.end();
  do { 
    GetArg(Tmp,Cmd,Parm++); _strlwr(Tmp);
    GetArg(Var,Tmp,1,FALSE,FALSE,FALSE,'=');
    GetArg(Set,Tmp,2,FALSE,FALSE,FALSE,'=');
    if(Var[0]) {
      c=CmdListe.find(Var);
      if(ec!=c) {
        (*c).second.Setup(Set);
        (*c).second.Write();
        Help=false;
      } else if(!Set[0] && (!stricmp(Var,"on") || !stricmp(Var,"off"))) {
        if(ec!=(c=CmdListe.find("plugin"))) (*c).second.Setup(Var);
        Help=false;
      } else if(!Set[0] && (!stricmp(Var,"reload") || !stricmp(Var,"load"))) {
        WriteChatf("%s::Loading...",PLUGIN_NAME);
        Configure();
        Help=false;
      } else if(!Set[0] && !stricmp(Var,"save")) {
        WriteChatf("%s::Saving...",PLUGIN_NAME);
        Exporting();
        Help=false;
      } else if(!Set[0] && !stricmp(Var,"reset")) {
        Override(NULL,"%s::Resetting...");
        Help=false;
      } else {
        WriteChatf("%s::Unsupported Argument <\ar%s\ax>",PLUGIN_NAME,Var); 
        break; 
      }
    }
  } while(strlen(Tmp));
  if(Help) MeleeHelp();
}

PLUGIN_API VOID KillThis(PSPAWNINFO pChar, PCHAR Cmd) { 
  if(doSKILL && pTarget && !TargetID(MeleeTarg) && TargetType(NPC_TYPE) && InGame()) {
    if(IsFeigning()) EzCommand("/stand");
    StickReset();
    OtherReset();
    AggroReset();
    MeleeTarg=((PSPAWNINFO)pTarget)->SpawnID;
    strcpy(MeleeName,((PSPAWNINFO)pTarget)->DisplayedName);
    MeleeSize=strlen(MeleeName)+1;
    MeleeType=SpawnMask((PSPAWNINFO)pTarget);
    onEVENT|=0x0008;
    #if    SHOW_ATTACKING > 0
      WriteChatf("%s::Attacking [\ay%s\ax].",PLUGIN_NAME,MeleeName);
    #endif SHOW_ATTACKING
  }
} 

PLUGIN_API VOID EnrageON(PSPAWNINFO pChar, PCHAR Cmd) {
  if(long val=atol(Cmd)) if(val!=MeleeTarg) return;
  if(doSKILL && doENRAGE && MeleeTarg && InGame()) {
    PSPAWNINFO KillTarg=GetSpawnID(MeleeTarg);
    if(!(onEVENT&0x0001)) {
      if(!(onEVENT&0x0002)) PetBACK(false);
      onEVENT|=0x0001;
      #if    SHOW_ENRAGING > 0
        WriteChatf("MQ2Melee::\arENRAGE\ax detected, taking action!");
      #endif SHOW_ENRAGING
    }
    if(*EQADDR_ATTACK && onEVENT&0x0003 && SpawnType(KillTarg,NPC_TYPE)) {
      double Back=fabs(AngularDistance(KillTarg->Heading,SpawnMe()->Heading));
      double View=fabs(AngularHeading(SpawnMe(),KillTarg));
      if(Back > 92 || View > 60 || onEVENT&0x0002) {
        onEVENT|=0x0008;
        AttackOFF(); 
      } 
    }
  }
}

PLUGIN_API VOID EnrageOFF(PSPAWNINFO pChar, PCHAR Cmd) {
  if(long val=atol(Cmd)) if(val!=MeleeTarg) return;
  if(doSKILL && doENRAGE && MeleeTarg && onEVENT&0x0001 && InGame()) {
    onEVENT&=0xFFFE;
    if(TargetID(PetTarget)) PetATTK(); 
    #if    SHOW_ENRAGING > 0
      WriteChatf("MQ2Melee:: \agENRAGE\ax ended, taking action!");
    #endif SHOW_ENRAGING
  }
} 

PLUGIN_API VOID InfuriateON(PSPAWNINFO pChar, PCHAR Cmd) { 
  if(long val=atol(Cmd)) if(val!=MeleeTarg) return;
  if(doSKILL && doINFURIATE && MeleeTarg && InGame()) {
    if(!(onEVENT&0x0002)) { 
      if(!(onEVENT&0x0001)) PetBACK(false); 
      onEVENT|=0x0002; 
      #if    SHOW_ENRAGING > 0
        WriteChatf("MQ2Melee::\arINFURIATE\ax detected, taking action!");
      #endif SHOW_ENRAGING
    } 
    if(*EQADDR_ATTACK) { 
      AttackOFF(); 
      onEVENT|=0x0008; 
    } 
  }
} 

PLUGIN_API VOID InfuriateOFF(PSPAWNINFO pChar, PCHAR Cmd) {
  if(long val=atol(Cmd)) if(val!=MeleeTarg) return;
  if(doSKILL && doINFURIATE && MeleeTarg && onEVENT&0x0002 && InGame()) {
    onEVENT&=0xFFFD; 
    if(TargetID(PetTarget)) PetATTK();
    #if    SHOW_ENRAGING > 0
      WriteChatf("MQ2Melee:\agINFURIATE\ax ended, taking action!");
    #endif SHOW_ENRAGING
  }
} 

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void DowntimeHandle() {
  if(doSENSETRAP && idSENSETRAP.Ready(ifSENSETRAP)) idSENSETRAP.Press();
  if(!MeleeTarg && (DWORD)clock()>TimerAttk && Immobile) {
    if(doDOWNFLAG[doDOWN] && DOWNSHIT[doDOWN].size()) EzCommand((PCHAR)DOWNSHIT[doDOWN].c_str());
    doDOWN=(++doDOWN)&7;
    if(doFORAGE && idFORAGE.Ready(ifFORAGE)) idFORAGE.Press();
    else if(doSNEAK && !IsSneaking() && idSNEAK.Ready(ifSNEAK)) idSNEAK.Press();
    else if(doHIDE && !IsInvisible() && idHIDE.Ready(ifHIDE)) idHIDE.Press();
  }
}

void MeleeHandle() {

  // Evaluate my moving speed taking into consideration mount.
  Travel=SpeedRun(SpawnMe());
  if(Moving=(Travel>0.05 || Travel<-0.05)) TimerMove=(DWORD)clock()+delay*5;
  Immobile=(!(MQ2Globals::gbMoving) && (!TimerMove || (DWORD)clock()>TimerMove));

  // watch pet animations put him on hold if he is inactive for too long.
  if(doPETASSIST) {
    if(PSPAWNINFO Pet=SpawnPet()) {
      if(Pet->Animation  < 13 || (Pet->Animation > 41  && Pet->Animation  <  45) ||
         Pet->Animation == 80 || Pet->Animation == 129 || Pet->Animation == 144) PetSEEN();
      else if(!PetOnHold && (DWORD)clock()>PetOnAttk && Pet->SpeedRun<0.05) PetBACK(true);
    }
  }
  
  // check opened windows that wont let us perform any melee actions.
  if(!BardClass  && WinState((CXWnd*)pCastingWnd))  return;
  if(WinState((CXWnd*)pSpellBookWnd))               return;
  if(WinState((CXWnd*)pLootWnd))                    return;
  if(WinState((CXWnd*)pBankWnd))                    return;
  if(WinState((CXWnd*)pMerchantWnd))                return;
  if(WinState((CXWnd*)pTradeWnd))                   return;
  if(WinState((CXWnd*)pGiveWnd))                    return;
  Silenced=false;

  // check detrimental buff that wont let ya perform melee actions.
  for(int b=0; b<BuffMax; b++) {
    long SpellID=GetCharInfo2()->Buff[b].SpellID;
    if(SpellID<1) continue;
    if(PSPELL spell=GetSpellByID(SpellID))
      for(int a=0; a<12; a++) switch(spell->Attrib[a]) {
        case 22: return;        // charmed
        case 23: return;        // feared
        case 31: return;        // mesmerized
        case 40: return;        // invulnerable
        case 96: Silenced=true; // silenced
      }
  }

  // check detrimental song that wont let ya perform melee actions.
  for(int s=0; s<SongMax; s++) {
    long SpellID=GetCharInfo2()->ShortBuff[s].SpellID;
    if(SpellID<1) continue;
    if(PSPELL spell=GetSpellByID(SpellID))
      for(int a=0; a<12; a++) switch(spell->Attrib[a]) {
        case 22: return;        // charmed
        case 23: return;        // feared
        case 31: return;        // mesmerized
        case 40: return;        // invulnerable
        case 96: Silenced=true; // silenced
      }
  }

  // check our health and perform some healing action if we can
  if((Health=GetCurHPS()*100/GetMaxHPS())<100) {
    Ability *UseThis=NULL;
    if(doMEND && Health<=doMEND && idMEND.Ready(ifMEND))                  UseThis=&idMEND;
    else if(doLAYHAND && Health<=doLAYHAND && idLAYHAND.Ready(ifLAYHAND)) UseThis=&idLAYHAND;
    else if(doPOTHEALFAST && Health<=doPOTHEALFAST && idPOTHEALFAST.Ready(ifPOTHEALFAST)) UseThis=&idPOTHEALFAST;
    else if(doPOTHEALOVER && Health<=doPOTHEALOVER && idPOTHEALOVER.Ready(ifPOTHEALOVER)) UseThis=&idPOTHEALOVER;
    if(UseThis) {
      if(UseThis->ID == idLAYHAND.ID) {
        PSPAWNINFO TargetSave=pTarget?(PSPAWNINFO)pTarget:NULL;
        *(PSPAWNINFO*)ppTarget=SpawnMe();
        idLAYHAND.Press();
        *(PSPAWNINFO*)ppTarget=TargetSave;
      } else UseThis->Press();
    }
  }

  // check if we are stunned, if so we can't perform any melee actions
  if(IsStunned()) return;
  
  // check if we still have a killing target or we acquiring a new one
  if(MeleeTarg && MeleeType!=SpawnMask(GetSpawnID(MeleeTarg))) {
    MeleeTarg=0;
    PetOnAttk=0;
  }
  if((pTarget && TargetType(NPC_TYPE)) && (*EQADDR_ATTACK || onEVENT&0x8000)) {
    if(!MeleeTarg) KillThis(NULL,"");
    else if(!TargetID(MeleeTarg)) {
      Override(NULL,"%s::\arTARGET SWITCH\ax taking actions.");
      return;
    }
    TimerAttk=(DWORD)clock()+delay*12;
  }
  if(MeleeTarg) if(PSPAWNINFO Tar=GetSpawnID(MeleeTarg)) {
    if(!MeleeLife || Tar->HPCurrent<MeleeLife) TimerLife=(DWORD)clock()+1500;
    MeleeLife=Tar->HPCurrent;
  }   

  // check if we should standup from an interrupted feign death
  if(BrokenFD && (DWORD)clock()>BrokenFD) {
    BrokenFD=false;
    if(IsFeigning()) {
      #if    SHOW_FEIGN > 0
        WriteChatf("%s::\arFAILED FEIGN DEATH\ax taking action!",PLUGIN_NAME);
      #endif SHOW_FEIGN
      EzCommand("/stand");
      return;
    }
  }

  // check it's a good time to perform some downtime actions?
  if(!IsCasting()) DowntimeHandle();

  // check it's a good time to drop combat?
  Ability *FeignDeath=NULL;
  if(idFEIGN[0].Ready(""))      FeignDeath=&idFEIGN[0];
  else if(idFEIGN[1].Ready("")) FeignDeath=&idFEIGN[1];
  Health=GetCurHPS()*100/GetMaxHPS();
  if(!doAGGRO && !(onEVENT&0x0FF0) && !IsFeigning() && !IsInvisible()) {
    bool fTime=(doFEIGNDEATH && Health<=doFEIGNDEATH && FeignDeath);
    bool eTime=(doESCAPE     && Health<=doESCAPE     && idESCAPE.Ready(""));
    bool bTime=(doBACKOFF    && Health<=doBACKOFF);

    if(fTime || eTime || bTime) {
      if(*EQADDR_ATTACK) {
        onEVENT|=0x0008;
        AttackOFF();
      } else {
        if(fTime)      onEVENT|=0x0040; 
        else if(eTime) onEVENT|=0x0020; 
        else if(bTime) onEVENT|=0x0010;
        if(onEVENT&0x0020)      idESCAPE.Press();
        else if(onEVENT&0x0040) FeignDeath->Press();
      } 
      return; 
    } 
  } 

  // check it's a good time to resume combat?
  if((IsFeigning() || IsInvisible() || onEVENT&0x0FF0) && !(onEVENT&0x4000)) {
    if(!TimerBack) TimerBack=(DWORD)clock()+delay;
    else if((DWORD)clock()>TimerBack && (doAGGRO || (onEVENT&0x0FF0 && Health>doRESUME))) {
      if((IsInvisible() || onEVENT&0x0220) && (!IsInvisible() || !(onEVENT&0x0020))) onEVENT&=0xFDDF;
      if((IsFeigning()  || onEVENT&0x0440) && (!IsFeigning() || (IsFeigning() && doSTAND))) { 
        onEVENT&=0xFBBF; 
        EzCommand("/stand");
        StickReset();
        return;
      }
      onEVENT&=0xF99F;
      TimerBack=false;
    }
  }

  // hold on?
  if(!MeleeTarg || !TargetID(MeleeTarg) || !IsStanding() || onEVENT&0x0FF2) {
    if(*EQADDR_ATTACK) AttackOFF(); 
    return; 
  }

  // target is in range? could we engage and kill it? 
  if((MeleeDist=DistanceToSpawn(SpawnMe(),(PSPAWNINFO)pTarget))>250) {
    Override(NULL,"");
    return;
  }
  MeleeSpeed=fabs(FindSpeed((PSPAWNINFO)pTarget));
  MeleeBack=fabs(AngularDistance(((PSPAWNINFO)pTarget)->Heading,SpawnMe()->Heading));
  MeleeView=fabs(AngularHeading(SpawnMe(),(PSPAWNINFO)pTarget));
  MeleeFlee=(MeleeFlee || (MeleeLife<=85 && MeleeSpeed>25.0f && IsMobFleeing(SpawnMe(),(PSPAWNINFO)pTarget)));
  MeleeKill=((PSPAWNINFO)pTarget)->AvatarHeight+12.0f;
  Sticking=Evaluate("${If[${Stick.Active},1,0]}");

  // time to handle dummy pet, check mending, check we have target in range, etc...
  if(doPETASSIST) if(PSPAWNINFO Pet=SpawnPet()) if(PSPAWNINFO Tar=(pTarget)?(PSPAWNINFO)pTarget:NULL) {
    if(doPETMEND && Pet->HPCurrent<=doPETMEND && idPETMEND.Ready("")) idPETMEND.Press();
    PetInDist=(!doPETRANGE || InRange(Pet,Tar,(float)doPETRANGE));
    if(!PetOnWait && PetInDist) PetOnWait=(DWORD)clock()+doPETDELAY*1000;
    if(PetOnWait && (DWORD)clock()>PetOnWait && PetInDist) {
      if((DWORD)clock()>TimerLife || (Tar->Animation!=26 && Tar->Animation!=32 && Tar->Animation!=110))
        if((DWORD)clock()>PetOnAttk) PetATTK();
    }
  }

  // are we discing? if so time to promote some actions?
  long disc=Discipline();
  if(disc && !(onEVENT&0x7007)) switch(disc) {
    case d_ashenhand:      // Ashenhand Discipline?
      if(doMELEE && MeleeDist<MeleeKill && idEAGLESTRIKE.Ready("")) idEAGLESTRIKE.Press(); break;
    case d_silentfist:     // Silentfist Discipline?
      if(doMELEE && MeleeDist<MeleeKill && idDRAGONPUNCH.Ready("")) idDRAGONPUNCH.Press(); break;
    case d_thunderkick:    // Thunderkick Discipline?
    case d_heelofkanji:    // Heel of Kanji?
      if(doMELEE && MeleeDist<MeleeKill && idFLYINGKICK.Ready("")) idFLYINGKICK.Press(); break;
    case d_assassin1:
    case d_assassin2:
    case d_assassin3:
      if(doMELEE && MeleeDist<MeleeKill && MeleeView<60 && MeleeBack<doSTAB && idBACKSTAB.Ready("") && StabCheck()) StabPress(); break;
  }

  // scripted rogue sequence striking/assasination codes
  if(doASSASINATE && doBACKSTAB && doMELEE && onSTICK>0 && !SwingHits && !TakenHits && MeleeSpeed<2.0f && !*EQADDR_ATTACK && StabCheck()) {
    if(!Moving && Immobile) {
      if(!IsSneaking() && idSNEAK.Ready("")) idSNEAK.Press();
      if(!IsInvisible() && idHIDE.Ready("")) idHIDE.Press();
      if(IsSneaking() && IsInvisible()) {
        if(doSTAB>191) sprintf(Reserved,"%2.2f id %d !front"    ,MeleeKill-3.0f,MeleeTarg);
        else           sprintf(Reserved,"%2.2f id %d behindonce",MeleeKill-3.0f,MeleeTarg);
        if(!Sticking && strcmp(Reserved,StickArg)) {
          Stick(Reserved);
          return;
        }
        if(MeleeDist>MeleeKill || MeleeView>60 || MeleeBack>doSTAB) SwingHits++;
        else if(idBACKSTAB.Ready("")) {
          if(Sticking) ("/stick off");
          if(doSTRIKE && idSTRIKE.Ready(ifSTRIKE)) idSTRIKE.Press();
          else StabPress();
        }
      }
    }
    if(Sticking && (SwingHits || TakenHits || !IsSneaking() || !IsInvisible())) Stick("");
    return;
  }

  // jolting times!
  if(doJOLT && !doAGGRO && SwingHits>doJOLT && idJOLT.Ready(ifJOLT)) {
    idJOLT.Press();
    SwingHits=1;
  }

  // handle melee
  if(doMELEE && !(onEVENT&0x8000)) {
    if(onEVENT&0x0008 && !(onEVENT&0xF007) && !*EQADDR_ATTACK) AttackON();

    if(onSTICK) {
      if(onSTICK > 0) {
        if(!TimerStik) if(!doSTICKRANGE || MeleeDist<doSTICKRANGE) TimerStik=(DWORD)clock()+doSTICKDELAY*1000;
        if(Immobile && !Sticking && (!doSTICKDELAY || (DWORD)clock()>TimerStik) && (!doSTICKRANGE || MeleeDist<doSTICKRANGE)) onSTICK=-1;
      }
      if(onSTICK<0) {
        if(doSTICKMODE) {
          strcpy(Reserved,StickCMD.c_str());
          ParseMacroData(Reserved);
        } else {
          long type=Aggroed(MeleeTarg);
          bool swim=(SpawnMe()->UnderWater==5);
          bool stab=(type<1 && doBACKSTAB && doSTAB<192);
          bool tank=(type>0 || (!IsInvisible() && (doAGGRO || !GetCharInfo()->pGroupInfo->pLeader)));
          double dist=MeleeKill-3.0f-(MeleeFlee*3.0f);
          sprintf(Reserved,"%2.2f id %d%s%s",dist,MeleeTarg,MeleeFlee?"":tank?" moveback":!stab?" !front":" behind",swim?" uw":"");
        }
        if(strcmp(Reserved,StickArg)) Stick(Reserved);
      }
    }

    // not behind enraged/infuriated target? 
    if(onEVENT&0x0003 && *EQADDR_ATTACK) { 
      #if    SHOW_ENRAGING > 0
        if(MeleeBack > 92) WriteChatf("%s::\arNOT BEHIND\ax enraged target, taking action!",PLUGIN_NAME);
      #endif SHOW_ENRAGING
      if(MeleeBack > 92 || onEVENT&0x0002) { 
        onEVENT|=0x0008; 
        AttackOFF(); 
        return; 
      } 
    } 

    // check target is in melee range? 
    if(MeleeDist<MeleeKill) {

      // attack is off, good time for stealing/begging or evading? 
      if(!*EQADDR_ATTACK) { 
        onEVENT&=0xBFFF;
        if(!MeleeFlee) {
          if(doPICKPOCKET && idPICKPOCKET.Ready(ifPICKPOCKET)) {
            idPICKPOCKET.Press();
            onEVENT|=0x1008;
          } else if(doBEGGING && !IsInvisible() && idBEGGING.Ready(ifBEGGING)) {
            idBEGGING.Press();
            onEVENT|=0x2008; 
          } 
          if(doEVADE && !doAGGRO && Immobile && !IsInvisible() && idHIDE.Ready(ifEVADE)) {
            idHIDE.Press();
            onEVENT|=0x0208; 
          } else if(doFALLS && !doAGGRO && FeignDeath->Ready(ifFALLS)) {
            FeignDeath->Press();
            onEVENT|=0x0408;
          }
        }
        if(onEVENT&0x0001 && !(onEVENT&0xFFF6) && MeleeBack<92) {
          #if    SHOW_ENRAGING > 0
            WriteChatf("%s::\agBEHIND\ax TARGET kicking attack ON!!!",PLUGIN_NAME);
          #endif SHOW_ENRAGING
          onEVENT&=0xFFF6; AttackON(); onEVENT|=0x0009;
        }

      // attack is on so lets do some dps?
      } else { 
        if(doBACKSTAB && MeleeBack<doSTAB && MeleeView<60 && idBACKSTAB.Ready(ifBACKSTAB) && StabCheck()) StabPress();
        if(doFLYINGKICK && idFLYINGKICK.Ready(ifFLYINGKICK)) idFLYINGKICK.Press();
        if(doDRAGONPUNCH && idDRAGONPUNCH.Ready(ifDRAGONPUNCH)) idDRAGONPUNCH.Press();
        if(doEAGLESTRIKE && idEAGLESTRIKE.Ready(ifEAGLESTRIKE)) idEAGLESTRIKE.Press();
        if(doTIGERCLAW && idTIGERCLAW.Ready(ifTIGERCLAW)) idTIGERCLAW.Press();
        if(doROUNDKICK && idROUNDKICK.Ready(ifROUNDKICK)) idROUNDKICK.Press();
        if(doBASH && idBASH.Ready(ifBASH) && BashCheck()) BashPress();
        if(doSLAM && idSLAM.Ready(ifSLAM)) idSLAM.Press();
        if(doFRENZY && idFRENZY.Ready(ifFRENZY)) idFRENZY.Press();
        if(doKICK && idKICK.Ready(ifKICK)) idKICK.Press();
        if(!disc && Immobile && !IsInvisible() && !MeleeFlee) {
          if(doPICKPOCKET && idPICKPOCKET.Ready(ifPICKPOCKET))    onEVENT|=0x4008;
          if(doBEGGING && idBEGGING.Ready(ifBEGGING))             onEVENT|=0x4008;
          if(doFALLS && !doAGGRO && FeignDeath->Ready(ifFALLS))   onEVENT|=0x4008;
          if(doEVADE && !(doAGGRO || !IsGrouped()) && idHIDE.Ready(ifEVADE)) onEVENT|=0x4008;
          if(onEVENT&0x4000 && *EQADDR_ATTACK) AttackOFF();
        }
        if(doDISARM && idDISARM.Ready(ifDISARM)) idDISARM.Press();
      }
      if(doINTIMIDATION && idINTIMIDATION.Ready(ifINTIMIDATION)) idINTIMIDATION.Press();
      if(doTAUNT && doAGGRO && idTAUNT.Ready(ifTAUNT)) idTAUNT.Press();
    }
    if(MeleeFlee) SneakOFF();
    if(onEVENT&0x3000) onEVENT&=0xCFFF;
  }

  // handle ranged? 
  if(doRANGE || onEVENT&0x8000) { 

    // should we face target? not moving, stopped >2sec, facing>2sec and not sticking?
    if(doFACING && Immobile && (DWORD)clock()>TimerFace && !Sticking) { 
      if(MeleeView>30) Face(SpawnMe(),"");
      TimerFace=(DWORD)clock()+delay*8; 
    } 

    // are we in good ranged for ranged? 
    if(MeleeDist<(doRANGE?doRANGE:250) && MeleeDist>35 && MeleeDist>MeleeKill+20) {
      if(!AutoFire && gbRangedAttackReady) ThrowIT(NULL,"");
      if(*EQADDR_ATTACK) {
        if(Sticking) Stick("");
        onEVENT|=0x8000;
        AttackOFF();
        #if    SHOW_SWITCHING > 0
          WriteChatf("%s::Switching [\ayRange\ax].",PLUGIN_NAME);
        #endif SHOW_SWITCHING
      } 

    // target too close? or too far? 
    } else if(!*EQADDR_ATTACK) { 
      if(AutoFire) {
        EzCommand("/autofire");
        AutoFire=false;
      }
      if(Immobile && onEVENT&0x8000) {
        onEVENT&=0x7FF7;
        if(doMELEE) {
          StickReset();
          onEVENT|=0x0008;
          AttackON();
          #if    SHOW_SWITCHING > 0
            WriteChatf("%s::Switching [\ayMelee\ax].",PLUGIN_NAME);
          #endif SHOW_SWITCHING
        }
      } 
    } 
  }

  // time to handle spell casting? 
  if(!TargetID(MeleeTarg)) return;
  long MyEndu=GetCharInfo2()->Endurance*100/GetMaxEndurance();

  // should we stun that target? 
  if(doSTUNNING && MeleeLife<=doSTUNNING) {
    Ability *UseThis=NULL;
    if(idSTUN[0].Ready(ifSTUNNING))      UseThis=&idSTUN[0];
    else if(idSTUN[1].Ready(ifSTUNNING)) UseThis=&idSTUN[1];
    if(UseThis) {
      #if    SHOW_STUNNING > 0
        WriteChatf("%s::Stunning [\ay%s\ax].",PLUGIN_NAME,MeleeName);
      #endif SHOW_STUNNING
      UseThis->Press();
    }
  } 

  // are we grouped?
  if(IsGrouped()) {

    // should we challenge for to maintain aggro over time?
    if(onCHALLENGEFOR && doAGGRO && MeleeDist<200 && Aggroed(MeleeTarg)==1 && idCHALLENGEFOR.Ready(ifCHALLENGEFOR)) { 
      #if    SHOW_PROVOKING>0 
        WriteChatf("%s::Challenging [\ay%s\ax].",PLUGIN_NAME,MeleeName); 
      #endif SHOW_PROVOKING 
      idCHALLENGEFOR.Press(); 
      onCHALLENGEFOR--; 
      return;
    }
    
    // should we provoke it? 
    if(onBELOW && doAGGRO && MeleeDist<100 && MeleeLife>doPROVOKEEND && Aggroed(MeleeTarg)<1) {
      Ability *UseThis=NULL; 
      if(idPROVOKE[0].Ready(ifPROVOKE))      UseThis=&idPROVOKE[0]; 
      else if(idPROVOKE[1].Ready(ifPROVOKE)) UseThis=&idPROVOKE[1]; 
      if(UseThis) { 
        #if    SHOW_PROVOKING>0 
          WriteChatf("%s::Provoking [\ay%s\ax].",PLUGIN_NAME,MeleeName); 
        #endif SHOW_PROVOKING 
        UseThis->Press(); 
        onBELOW--; 
        return; 
      } 
    } 
    
    // should we use short duration melee buff? 
    if(GetCharInfo2()->Endurance>200) { 
      if(doCOMMANDING && MyEndu>doCOMMANDING && idCOMMANDING.Ready(ifCOMMANDING)) idCOMMANDING.Press();
      if(doFISTSOFWU  && MyEndu>doFISTSOFWU  && idFISTSOFWU.Ready(ifFISTSOFWU))   idFISTSOFWU.Press();
      if(doCRYHAVOC   && MyEndu>doCRYHAVOC   && idCRYHAVOC.Ready(ifCRYHAVOC))     idCRYHAVOC.Press();
      if(doTHIEFEYE   && MyEndu>doTHIEFEYE   && idTHIEFEYE.Ready(ifTHIEFEYE))     idTHIEFEYE.Press();
    } 
  }

  // should we use destroyer's/rage volley?
  if(doRAGEVOLLEY && MyEndu>doRAGEVOLLEY && MeleeDist<175 && idRAGEVOLLEY.Ready(ifRAGEVOLLEY)) idRAGEVOLLEY.Press();

  // is target close enough for those? 
  if(MeleeDist < 50) {
    if(doRAKE && MyEndu>doRAKE && idRAKE.Ready(ifRAKE)) idRAKE.Press();
    if(doFERALSWIPE && idFERALSWIPE.Ready(ifFERALSWIPE)) idFERALSWIPE.Press();
    if(doLEOPARDCLAW && MyEndu>doLEOPARDCLAW && idLEOPARDCLAW.Ready(ifLEOPARDCLAW)) idLEOPARDCLAW.Press();
    if(doTHROWSTONE && MyEndu>doTHROWSTONE && idTHROWSTONE.Ready(ifTHROWSTONE)) idTHROWSTONE.Press();
    if(doCALLCHALLENGE && idCALLCHALLENGE.Ready(ifCALLCHALLENGE)) idCALLCHALLENGE.Press();
    if(doTWISTEDSHANK && idTWISTEDSHANK.Ready(ifTWISTEDSHANK)) idTWISTEDSHANK.Press();
  }

  // time to handle holy shit? 
  if(doHOLYFLAG[doHOLY] && HOLYSHIT[doHOLY].size()) EzCommand((PCHAR)HOLYSHIT[doHOLY].c_str());
  doHOLY=(++doHOLY)&7;
}

void KeyMelee(PCHAR NAME, BOOL Down) {
  if(Down && pTarget) {
		if(!doSKILL) EzCommand("/keypress AUTOPRIM");
		else if(!MeleeTarg) KillThis(NULL,"");
		else Override(NULL,"%s::\arOVERRIDE\ax taking actions!");
  }
} 

void KeyRange(PCHAR NAME, BOOL Down) {
	if(Down && pTarget) {
		if(!doSKILL) EzCommand("/keypress RANGED");
		else ThrowIT(NULL,"");
	}
}

void Bindding(bool BindMode) { 
  if(BindMode) {
    if(!Binded) { 
    	KeyCombo	MeleeCombo, RangeCombo;
    	RemoveMQ2KeyBind("MELEE"); AddMQ2KeyBind("MELEE",KeyMelee); ParseKeyCombo(MELEE_KEY,MeleeCombo); SetMQ2KeyBind("MELEE",false,MeleeCombo);
    	RemoveMQ2KeyBind("RANGE"); AddMQ2KeyBind("RANGE",KeyRange); ParseKeyCombo(RANGE_KEY,RangeCombo); SetMQ2KeyBind("RANGE",false,RangeCombo);
      Binded=true; 
    } 
  } else if(Binded) {
    RemoveMQ2KeyBind("MELEE");
    RemoveMQ2KeyBind("RANGE");
    Binded=false; 
  } 
} 

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void __stdcall AUTOFIREOFF(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  AutoFire=false;
  if(AutoFire) KeyMelee("",true);
}

void __stdcall AUTOFIREON(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(pTarget && TargetType(NPC_TYPE) && InGame()) {
    if(!AutoFire) KeyMelee("",true);
    if(MeleeTarg) onEVENT|=0x8000;
    AutoFire=true;
  }
}

void __stdcall CASTING(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(doSKILL && MeleeTarg && !strnicmp(pValues->Value,MeleeName,MeleeSize)) MeleeCast=(DWORD)clock();
}

void __stdcall ENRAGEON(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(MeleeTarg && !strnicmp(pValues->Value,MeleeName,MeleeSize)) EnrageON(NULL,"");
}

void __stdcall ENRAGEOFF(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(MeleeTarg && !strnicmp(pValues->Value,MeleeName,MeleeSize)) EnrageOFF(NULL,"");
}

void __stdcall INFURIATEON(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(MeleeTarg && !strnicmp(pValues->Value,MeleeName,MeleeSize)) InfuriateON(NULL,"");
}

void __stdcall INFURIATEOFF(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(MeleeTarg && !strnicmp(pValues->Value,MeleeName,MeleeSize)) InfuriateOFF(NULL,"");
}

void __stdcall PETATTK(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  PetSEEN();
}

void __stdcall PETBACK(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(!PetOnHold && HaveHold) PetBACK(true);
}

void __stdcall PETHOLD(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(!PetOnHold) {
    if(PetOnAttk) PetOnAttk=(DWORD)clock()+6000;
    PetOnHold=true;
  }
}

void __stdcall FALLEN(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(!doSKILL || ((long)pData && strnicmp(pValues->Value,GetCharInfo()->Name,strlen(GetCharInfo()->Name)+1))) return;
  #if    SHOW_FEIGN > 0
    WriteChatf("%s::\arFAILED FEIGN DEATH\ax taking action!",PLUGIN_NAME);
  #endif SHOW_FEIGN
  EzCommand("/stand");
}

void __stdcall BROKEN(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(doSKILL) BrokenFD=(DWORD)clock()+1;
}

void __stdcall RESUME(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(doSKILL) BrokenFD=0;
}

void __stdcall HIDEOFF(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(doSKILL) HideOFF();
}

void __stdcall SNEAKOFF(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(doSKILL) SneakOFF();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API VOID InitializePlugin() { 
  CmdListe.clear();
  MapInsert(&CmdListe,Option(pAGGRO[0],pAGGRO[1],pAGGRO[2],pAGGRO[3],AggroReset,&doAGGRO));
  MapInsert(&CmdListe,Option(pAGGRP[0],pAGGRP[1],pAGGRP[2],pAGGRP[3],NULL      ,&elAGGROPRI));
  MapInsert(&CmdListe,Option(pAGGRS[0],pAGGRS[1],pAGGRS[2],pAGGRS[3],NULL      ,&elAGGROSEC));
  MapInsert(&CmdListe,Option(pARROW[0],pARROW[1],pARROW[2],pARROW[3],NULL      ,&elARROWS));
  MapInsert(&CmdListe,Option(pASSAS[0],pASSAS[1],pASSAS[2],pASSAS[3],NULL      ,&doASSASINATE));
  MapInsert(&CmdListe,Option(pBKOFF[0],pBKOFF[1],pBKOFF[2],pBKOFF[3],NULL      ,&doBACKOFF));
  MapInsert(&CmdListe,Option(pBSTAB[0],pBSTAB[1],pBSTAB[2],pBSTAB[3],NULL      ,&doBACKSTAB));
  MapInsert(&CmdListe,Option(pBASHS[0],pBASHS[1],pBASHS[2],pBASHS[3],NULL      ,&doBASH));
  MapInsert(&CmdListe,Option(pBGING[0],pBGING[1],pBGING[2],pBGING[3],NULL      ,&doBEGGING));
  MapInsert(&CmdListe,Option(pCALLC[0],pCALLC[1],pCALLC[2],pCALLC[3],NULL      ,&doCALLCHALLENGE));
  MapInsert(&CmdListe,Option(pCHFOR[0],pCHFOR[1],pCHFOR[2],pCHFOR[3],NULL      ,&doCHALLENGEFOR));
  MapInsert(&CmdListe,Option(pCOMMG[0],pCOMMG[1],pCOMMG[2],pCOMMG[3],NULL      ,&doCOMMANDING));
  MapInsert(&CmdListe,Option(pCRYHC[0],pCRYHC[1],pCRYHC[2],pCRYHC[3],NULL      ,&doCRYHAVOC));
  MapInsert(&CmdListe,Option(pDISRM[0],pDISRM[1],pDISRM[2],pDISRM[3],NULL      ,&doDISARM));
  MapInsert(&CmdListe,Option(pDWNF0[0],pDWNF0[1],pDWNF0[2],pDWNF0[3],NULL      ,&doDOWNFLAG[0]));
  MapInsert(&CmdListe,Option(pDWNF1[0],pDWNF1[1],pDWNF1[2],pDWNF1[3],NULL      ,&doDOWNFLAG[1]));
  MapInsert(&CmdListe,Option(pDWNF2[0],pDWNF2[1],pDWNF2[2],pDWNF2[3],NULL      ,&doDOWNFLAG[2]));
  MapInsert(&CmdListe,Option(pDWNF3[0],pDWNF3[1],pDWNF3[2],pDWNF3[3],NULL      ,&doDOWNFLAG[3]));
  MapInsert(&CmdListe,Option(pDWNF4[0],pDWNF4[1],pDWNF4[2],pDWNF4[3],NULL      ,&doDOWNFLAG[4]));
  MapInsert(&CmdListe,Option(pDWNF5[0],pDWNF5[1],pDWNF5[2],pDWNF5[3],NULL      ,&doDOWNFLAG[5]));
  MapInsert(&CmdListe,Option(pDWNF6[0],pDWNF6[1],pDWNF6[2],pDWNF6[3],NULL      ,&doDOWNFLAG[6]));
  MapInsert(&CmdListe,Option(pDWNF7[0],pDWNF7[1],pDWNF7[2],pDWNF7[3],NULL      ,&doDOWNFLAG[7]));
  MapInsert(&CmdListe,Option(pDRPNC[0],pDRPNC[1],pDRPNC[2],pDRPNC[3],NULL      ,&doDRAGONPUNCH));
  MapInsert(&CmdListe,Option(pEAGLE[0],pEAGLE[1],pEAGLE[2],pEAGLE[3],NULL      ,&doEAGLESTRIKE));
  MapInsert(&CmdListe,Option(pERAGE[0],pERAGE[1],pERAGE[2],pERAGE[3],NULL      ,&doENRAGE));
  MapInsert(&CmdListe,Option(pESCAP[0],pESCAP[1],pESCAP[2],pESCAP[3],NULL      ,&doESCAPE));
  MapInsert(&CmdListe,Option(pEVADE[0],pEVADE[1],pEVADE[2],pEVADE[3],NULL      ,&doEVADE));
  MapInsert(&CmdListe,Option(pFEIGN[0],pFEIGN[1],pFEIGN[2],pFEIGN[3],NULL      ,&doFEIGNDEATH));
  MapInsert(&CmdListe,Option(pFACES[0],pFACES[1],pFACES[2],pFACES[3],NULL      ,&doFACING));
  MapInsert(&CmdListe,Option(pFALLS[0],pFALLS[1],pFALLS[2],pFALLS[3],NULL      ,&doFALLS));
  MapInsert(&CmdListe,Option(pFERAL[0],pFERAL[1],pFERAL[2],pFERAL[3],NULL      ,&doFERALSWIPE));
  MapInsert(&CmdListe,Option(pFISTS[0],pFISTS[1],pFISTS[2],pFISTS[3],NULL      ,&doFISTSOFWU));
  MapInsert(&CmdListe,Option(pFLYKC[0],pFLYKC[1],pFLYKC[2],pFLYKC[3],NULL      ,&doFLYINGKICK));
  MapInsert(&CmdListe,Option(pFORAG[0],pFORAG[1],pFORAG[2],pFORAG[3],NULL      ,&doFORAGE));
  MapInsert(&CmdListe,Option(pFRENZ[0],pFRENZ[1],pFRENZ[2],pFRENZ[3],NULL      ,&doFRENZY));
  MapInsert(&CmdListe,Option(pHARMT[0],pHARMT[1],pHARMT[2],pHARMT[3],NULL      ,&doHARMTOUCH));
  MapInsert(&CmdListe,Option(pHIDES[0],pHIDES[1],pHIDES[2],pHIDES[3],NULL      ,&doHIDE));
  MapInsert(&CmdListe,Option(pHOLF0[0],pHOLF0[1],pHOLF0[2],pHOLF0[3],NULL      ,&doHOLYFLAG[0]));
  MapInsert(&CmdListe,Option(pHOLF1[0],pHOLF1[1],pHOLF1[2],pHOLF1[3],NULL      ,&doHOLYFLAG[1]));
  MapInsert(&CmdListe,Option(pHOLF2[0],pHOLF2[1],pHOLF2[2],pHOLF2[3],NULL      ,&doHOLYFLAG[2]));
  MapInsert(&CmdListe,Option(pHOLF3[0],pHOLF3[1],pHOLF3[2],pHOLF3[3],NULL      ,&doHOLYFLAG[3]));
  MapInsert(&CmdListe,Option(pHOLF4[0],pHOLF4[1],pHOLF4[2],pHOLF4[3],NULL      ,&doHOLYFLAG[4]));
  MapInsert(&CmdListe,Option(pHOLF5[0],pHOLF5[1],pHOLF5[2],pHOLF5[3],NULL      ,&doHOLYFLAG[5]));
  MapInsert(&CmdListe,Option(pHOLF6[0],pHOLF6[1],pHOLF6[2],pHOLF6[3],NULL      ,&doHOLYFLAG[6]));
  MapInsert(&CmdListe,Option(pHOLF7[0],pHOLF7[1],pHOLF7[2],pHOLF7[3],NULL      ,&doHOLYFLAG[7]));
  MapInsert(&CmdListe,Option(pINFUR[0],pINFUR[1],pINFUR[2],pINFUR[3],NULL      ,&doINFURIATE));
  MapInsert(&CmdListe,Option(pINTIM[0],pINTIM[1],pINTIM[2],pINTIM[3],NULL      ,&doINTIMIDATION));
  MapInsert(&CmdListe,Option(pJOLTS[0],pJOLTS[1],pJOLTS[2],pJOLTS[3],NULL      ,&doJOLT));
  MapInsert(&CmdListe,Option(pKICKS[0],pKICKS[1],pKICKS[2],pKICKS[3],NULL      ,&doKICK));
  MapInsert(&CmdListe,Option(pLHAND[0],pLHAND[1],pLHAND[2],pLHAND[3],NULL      ,&doLAYHAND));
  MapInsert(&CmdListe,Option(pLCLAW[0],pLCLAW[1],pLCLAW[2],pLCLAW[3],NULL      ,&doLEOPARDCLAW));
  MapInsert(&CmdListe,Option(pMELEE[0],pMELEE[1],pMELEE[2],pMELEE[3],NULL      ,&doMELEE));
  MapInsert(&CmdListe,Option(pMELEP[0],pMELEP[1],pMELEP[2],pMELEP[3],NULL      ,&elMELEEPRI));
  MapInsert(&CmdListe,Option(pMELES[0],pMELES[1],pMELES[2],pMELES[3],NULL      ,&elMELEESEC));
  MapInsert(&CmdListe,Option(pMENDS[0],pMENDS[1],pMENDS[2],pMENDS[3],NULL      ,&doMEND));
  MapInsert(&CmdListe,Option(pBOWID[0],pBOWID[1],pBOWID[2],pBOWID[3],NULL      ,&elRANGED));
  MapInsert(&CmdListe,Option(pPETAS[0],pPETAS[1],pPETAS[2],pPETAS[3],NULL      ,&doPETASSIST));
  MapInsert(&CmdListe,Option(pPETDE[0],pPETDE[1],pPETDE[2],pPETDE[3],NULL      ,&doPETDELAY));
  MapInsert(&CmdListe,Option(pPETRN[0],pPETRN[1],pPETRN[2],pPETRN[3],NULL      ,&doPETRANGE));
  MapInsert(&CmdListe,Option(pPETMN[0],pPETMN[1],pPETMN[2],pPETMN[3],NULL      ,&doPETMEND));
  MapInsert(&CmdListe,Option(pPICKP[0],pPICKP[1],pPICKP[2],pPICKP[3],NULL      ,&doPICKPOCKET));
  MapInsert(&CmdListe,Option(pPOKER[0],pPOKER[1],pPOKER[2],pPOKER[3],NULL      ,&elPOKER));
  MapInsert(&CmdListe,Option(pHFAST[0],pHFAST[1],pHFAST[2],pHFAST[3],NULL      ,&doPOTHEALFAST));
  MapInsert(&CmdListe,Option(pHOVER[0],pHOVER[1],pHOVER[2],pHOVER[3],NULL      ,&doPOTHEALOVER));
  MapInsert(&CmdListe,Option(pPRVKM[0],pPRVKM[1],pPRVKM[2],pPRVKM[3],AggroReset,&doPROVOKEMAX));
  MapInsert(&CmdListe,Option(pPRVKE[0],pPRVKE[1],pPRVKE[2],pPRVKE[3],NULL      ,&doPROVOKEEND));
  MapInsert(&CmdListe,Option(pPRVK0[0],pPRVK0[1],pPRVK0[2],pPRVK0[3],NULL      ,&idPROVOKE[0]));
  MapInsert(&CmdListe,Option(pPRVK1[0],pPRVK1[1],pPRVK1[2],pPRVK1[3],NULL      ,&idPROVOKE[1]));
  MapInsert(&CmdListe,Option(pRAVOL[0],pRAVOL[1],pRAVOL[2],pRAVOL[3],NULL      ,&doRAGEVOLLEY));
  MapInsert(&CmdListe,Option(pRAKES[0],pRAKES[1],pRAKES[2],pRAKES[3],NULL      ,&doRAKE));
  MapInsert(&CmdListe,Option(pRANGE[0],pRANGE[1],pRANGE[2],pRANGE[3],RangeReset,&doRANGE));
  MapInsert(&CmdListe,Option(pRESUM[0],pRESUM[1],pRESUM[2],pRESUM[3],NULL      ,&doRESUME));
  MapInsert(&CmdListe,Option(pRKICK[0],pRKICK[1],pRKICK[2],pRKICK[3],NULL      ,&doROUNDKICK));
  MapInsert(&CmdListe,Option(pSENSE[0],pSENSE[1],pSENSE[2],pSENSE[3],NULL      ,&doSENSETRAP));
  MapInsert(&CmdListe,Option(pPLUGS[0],pPLUGS[1],pPLUGS[2],pPLUGS[3],NULL      ,&doSKILL));
  MapInsert(&CmdListe,Option(pSLAMS[0],pSLAMS[1],pSLAMS[2],pSLAMS[3],NULL      ,&doSLAM));
  MapInsert(&CmdListe,Option(pSNEAK[0],pSNEAK[1],pSNEAK[2],pSNEAK[3],NULL      ,&doSNEAK));
  MapInsert(&CmdListe,Option(pSTAND[0],pSTAND[1],pSTAND[2],pSTAND[3],NULL      ,&doSTAND));
  MapInsert(&CmdListe,Option(pSTIKR[0],pSTIKR[1],pSTIKR[2],pSTIKR[3],StickReset,&doSTICKRANGE));
  MapInsert(&CmdListe,Option(pSTIKD[0],pSTIKD[1],pSTIKD[2],pSTIKD[3],StickReset,&doSTICKDELAY));
  MapInsert(&CmdListe,Option(pSTIKM[0],pSTIKM[1],pSTIKM[2],pSTIKM[3],NULL      ,&doSTICKMODE));
  MapInsert(&CmdListe,Option(pSTUNS[0],pSTUNS[1],pSTUNS[2],pSTUNS[3],NULL      ,&doSTUNNING));
  MapInsert(&CmdListe,Option(pSTUN0[0],pSTUN0[1],pSTUN0[2],pSTUN0[3],NULL      ,&idSTUN[0]));
  MapInsert(&CmdListe,Option(pSTUN1[0],pSTUN1[1],pSTUN1[2],pSTUN1[3],NULL      ,&idSTUN[1]));
  MapInsert(&CmdListe,Option(pSTRIK[0],pSTRIK[1],pSTRIK[2],pSTRIK[3],NULL      ,&doSTRIKE));
  MapInsert(&CmdListe,Option(pTAUNT[0],pTAUNT[1],pTAUNT[2],pTAUNT[3],NULL      ,&doTAUNT));
  MapInsert(&CmdListe,Option(pTHIEF[0],pTHIEF[1],pTHIEF[2],pTHIEF[3],NULL      ,&doTHIEFEYE));
  MapInsert(&CmdListe,Option(pTHROW[0],pTHROW[1],pTHROW[2],pTHROW[3],NULL      ,&doTHROWSTONE));
  MapInsert(&CmdListe,Option(pTIGER[0],pTIGER[1],pTIGER[2],pTIGER[3],NULL      ,&doTIGERCLAW));
  MapInsert(&CmdListe,Option(pTWIST[0],pTWIST[1],pTWIST[2],pTWIST[3],NULL      ,&doTWISTEDSHANK));
  MapInsert(&CmdListe,Option(pSHIEL[0],pSHIEL[1],pSHIEL[2],pSHIEL[3],NULL      ,&elSHIELD));

  IniListe.clear();
  MapInsert(&IniListe,Option("backstabif"     ,"","","",NULL,&ifBACKSTAB));
  MapInsert(&IniListe,Option("bashif"         ,"","","",NULL,&ifBASH)); 
  MapInsert(&IniListe,Option("beggingif"      ,"","","",NULL,&ifBEGGING)); 
  MapInsert(&IniListe,Option("callchallengeif","","","",NULL,&ifCALLCHALLENGE)); 
  MapInsert(&IniListe,Option("challengeforif" ,"","","",NULL,&ifCHALLENGEFOR));
  MapInsert(&IniListe,Option("commandingif"   ,"","","",NULL,&ifCOMMANDING)); 
  MapInsert(&IniListe,Option("cryhavocif"     ,"","","",NULL,&ifCRYHAVOC)); 
  MapInsert(&IniListe,Option("disarmif"       ,"","","",NULL,&ifDISARM)); 
  MapInsert(&IniListe,Option("dragonpunchif"  ,"","","",NULL,&ifDRAGONPUNCH)); 
  MapInsert(&IniListe,Option("eaglestrikeif"  ,"","","",NULL,&ifEAGLESTRIKE)); 
  MapInsert(&IniListe,Option("evadeif"        ,"","","",NULL,&ifEVADE)); 
  MapInsert(&IniListe,Option("fallsif"        ,"","","",NULL,&ifFALLS)); 
  MapInsert(&IniListe,Option("feralswipeif"   ,"","","",NULL,&ifFERALSWIPE)); 
  MapInsert(&IniListe,Option("fistofwuif"     ,"","","",NULL,&ifFISTSOFWU)); 
  MapInsert(&IniListe,Option("flyingkickif"   ,"","","",NULL,&ifFLYINGKICK)); 
  MapInsert(&IniListe,Option("forageif"       ,"","","",NULL,&ifFORAGE)); 
  MapInsert(&IniListe,Option("frenzyif"       ,"","","",NULL,&ifFRENZY)); 
  MapInsert(&IniListe,Option("harmtouchif"    ,"","","",NULL,&ifHARMTOUCH)); 
  MapInsert(&IniListe,Option("pothealfastif"  ,"","","",NULL,&ifPOTHEALFAST)); 
  MapInsert(&IniListe,Option("pothealoverif"  ,"","","",NULL,&ifPOTHEALOVER)); 
  MapInsert(&IniListe,Option("hideif"         ,"","","",NULL,&ifHIDE)); 
  MapInsert(&IniListe,Option("intimidationif" ,"","","",NULL,&ifINTIMIDATION)); 
  MapInsert(&IniListe,Option("joltif"         ,"","","",NULL,&ifINTIMIDATION)); 
  MapInsert(&IniListe,Option("kickif"         ,"","","",NULL,&ifKICK)); 
  MapInsert(&IniListe,Option("layhandif"      ,"","","",NULL,&ifLAYHAND)); 
  MapInsert(&IniListe,Option("leopardclawif"  ,"","","",NULL,&ifLEOPARDCLAW)); 
  MapInsert(&IniListe,Option("mendif"         ,"","","",NULL,&ifMEND)); 
  MapInsert(&IniListe,Option("pickpocketif"   ,"","","",NULL,&ifPICKPOCKET)); 
  MapInsert(&IniListe,Option("provokeif"      ,"","","",NULL,&ifPROVOKE)); 
  MapInsert(&IniListe,Option("ragevolleyif"   ,"","","",NULL,&ifRAGEVOLLEY)); 
  MapInsert(&IniListe,Option("rakeif"         ,"","","",NULL,&ifRAKE)); 
  MapInsert(&IniListe,Option("roundkickif"    ,"","","",NULL,&ifROUNDKICK)); 
  MapInsert(&IniListe,Option("sensetrapif"    ,"","","",NULL,&ifSENSETRAP)); 
  MapInsert(&IniListe,Option("slamif"         ,"","","",NULL,&ifSLAM)); 
  MapInsert(&IniListe,Option("sneakif"        ,"","","",NULL,&ifSNEAK)); 
  MapInsert(&IniListe,Option("strikeif"       ,"","","",NULL,&ifSTRIKE)); 
  MapInsert(&IniListe,Option("stunningif"     ,"","","",NULL,&ifSTUNNING)); 
  MapInsert(&IniListe,Option("tauntif"        ,"","","",NULL,&ifTAUNT)); 
  MapInsert(&IniListe,Option("thiefeyeif"     ,"","","",NULL,&ifTHIEFEYE)); 
  MapInsert(&IniListe,Option("throwstoneif"   ,"","","",NULL,&ifTHROWSTONE)); 
  MapInsert(&IniListe,Option("tigerclawif"    ,"","","",NULL,&ifTIGERCLAW));
  MapInsert(&IniListe,Option("twistedshankif" ,"","","",NULL,&ifTWISTEDSHANK));
  MapInsert(&IniListe,Option("stickcmd"       ,"","","",NULL,&StickCMD)); 
  MapInsert(&IniListe,Option("downshit0"      ,"","","",NULL,&DOWNSHIT[0])); 
  MapInsert(&IniListe,Option("downshit1"      ,"","","",NULL,&DOWNSHIT[1])); 
  MapInsert(&IniListe,Option("downshit2"      ,"","","",NULL,&DOWNSHIT[2])); 
  MapInsert(&IniListe,Option("downshit3"      ,"","","",NULL,&DOWNSHIT[3])); 
  MapInsert(&IniListe,Option("downshit4"      ,"","","",NULL,&DOWNSHIT[4])); 
  MapInsert(&IniListe,Option("downshit5"      ,"","","",NULL,&DOWNSHIT[5])); 
  MapInsert(&IniListe,Option("downshit6"      ,"","","",NULL,&DOWNSHIT[6])); 
  MapInsert(&IniListe,Option("downshit7"      ,"","","",NULL,&DOWNSHIT[7])); 
  MapInsert(&IniListe,Option("holyshit0"      ,"","","",NULL,&HOLYSHIT[0])); 
  MapInsert(&IniListe,Option("holyshit1"      ,"","","",NULL,&HOLYSHIT[1])); 
  MapInsert(&IniListe,Option("holyshit2"      ,"","","",NULL,&HOLYSHIT[2])); 
  MapInsert(&IniListe,Option("holyshit3"      ,"","","",NULL,&HOLYSHIT[3])); 
  MapInsert(&IniListe,Option("holyshit4"      ,"","","",NULL,&HOLYSHIT[4])); 
  MapInsert(&IniListe,Option("holyshit5"      ,"","","",NULL,&HOLYSHIT[5])); 
  MapInsert(&IniListe,Option("holyshit6"      ,"","","",NULL,&HOLYSHIT[6])); 
  MapInsert(&IniListe,Option("holyshit7"      ,"","","",NULL,&HOLYSHIT[7]));

  VarListe.clear();
  MapInsert(&VarListe,Option("idleopardclaw"  ,"","","",NULL,&idLEOPARDCLAW));
  MapInsert(&VarListe,Option("idragevolley"   ,"","","",NULL,&idRAGEVOLLEY));
  MapInsert(&VarListe,Option("idbackstab"     ,"","","",NULL,&idBACKSTAB));
  MapInsert(&VarListe,Option("idbash"         ,"","","",NULL,&idBASH));
  MapInsert(&VarListe,Option("idbegging"      ,"","","",NULL,&idBEGGING));
  MapInsert(&VarListe,Option("idcallchallenge","","","",NULL,&idCALLCHALLENGE));
  MapInsert(&VarListe,Option("idcommanding"   ,"","","",NULL,&idCOMMANDING));
  MapInsert(&VarListe,Option("idcryhavoc"     ,"","","",NULL,&idCRYHAVOC));
  MapInsert(&VarListe,Option("iddisarm"       ,"","","",NULL,&idDISARM));
  MapInsert(&VarListe,Option("iddragonpunch"  ,"","","",NULL,&idDRAGONPUNCH));
  MapInsert(&VarListe,Option("ideaglestrike"  ,"","","",NULL,&idEAGLESTRIKE));
  MapInsert(&VarListe,Option("idescape"       ,"","","",NULL,&idESCAPE));
  MapInsert(&VarListe,Option("idferalswipe"   ,"","","",NULL,&idFERALSWIPE));
  MapInsert(&VarListe,Option("idfistsofwu"    ,"","","",NULL,&idFISTSOFWU));
  MapInsert(&VarListe,Option("idflyingkick"   ,"","","",NULL,&idFLYINGKICK));
  MapInsert(&VarListe,Option("idforage"       ,"","","",NULL,&idFORAGE));
  MapInsert(&VarListe,Option("idfrenzy"       ,"","","",NULL,&idFRENZY));
  MapInsert(&VarListe,Option("idharmtouch"    ,"","","",NULL,&idHARMTOUCH));
  MapInsert(&VarListe,Option("idhide"         ,"","","",NULL,&idHIDE));
  MapInsert(&VarListe,Option("idintimidation" ,"","","",NULL,&idINTIMIDATION));
  MapInsert(&VarListe,Option("idjolt"         ,"","","",NULL,&idJOLT));
  MapInsert(&VarListe,Option("idkick"         ,"","","",NULL,&idKICK));
  MapInsert(&VarListe,Option("idlayhand"      ,"","","",NULL,&idLAYHAND));
  MapInsert(&VarListe,Option("idmend"         ,"","","",NULL,&idMEND));
  MapInsert(&VarListe,Option("idpickpocket"   ,"","","",NULL,&idPICKPOCKET));
  MapInsert(&VarListe,Option("idrake"         ,"","","",NULL,&idRAKE));
  MapInsert(&VarListe,Option("idroundkick"    ,"","","",NULL,&idROUNDKICK));
  MapInsert(&VarListe,Option("idsensetrap"    ,"","","",NULL,&idSENSETRAP));
  MapInsert(&VarListe,Option("idslam"         ,"","","",NULL,&idSLAM));
  MapInsert(&VarListe,Option("idsneak"        ,"","","",NULL,&idSNEAK));
  MapInsert(&VarListe,Option("idstrike"       ,"","","",NULL,&idSTRIKE));
  MapInsert(&VarListe,Option("idtaunt"        ,"","","",NULL,&idTAUNT));
  MapInsert(&VarListe,Option("idthiefeye"     ,"","","",NULL,&idTHIEFEYE));
  MapInsert(&VarListe,Option("idthrowstone"   ,"","","",NULL,&idTHROWSTONE));
  MapInsert(&VarListe,Option("idtigerclaw"    ,"","","",NULL,&idTIGERCLAW));
  MapInsert(&VarListe,Option("idfeign0"       ,"","","",NULL,&idFEIGN[0]));
  MapInsert(&VarListe,Option("idfeign1"       ,"","","",NULL,&idFEIGN[1]));
  MapInsert(&VarListe,Option("idpetmend"      ,"","","",NULL,&idPETMEND));
  MapInsert(&VarListe,Option("idpothealfast"  ,"","","",NULL,&idPOTHEALFAST));
  MapInsert(&VarListe,Option("idpothealover"  ,"","","",NULL,&idPOTHEALOVER));
  MapInsert(&VarListe,Option("idprovoke0"     ,"","","",NULL,&idPROVOKE[0]));
  MapInsert(&VarListe,Option("idprovoke1"     ,"","","",NULL,&idPROVOKE[1]));
  MapInsert(&VarListe,Option("idstun0"        ,"","","",NULL,&idSTUN[0]));
  MapInsert(&VarListe,Option("idstun1"        ,"","","",NULL,&idSTUN[1]));

  ZeroMemory(ColorArray,sizeof(ColorArray));
  ColorArray[13] =true; // Attack/AutoFire/Enrage/Infuriate
  ColorArray[265]=true; // Hits Target
  ColorArray[267]=true; // Miss Target
  ColorArray[266]=true; // Hits me
  ColorArray[268]=true; // Miss me
  ColorArray[270]=true; // Failed Hide/Sneak
  ColorArray[273]=true; // Failed FD????
  ColorArray[288]=true; // Target begin to cast
  ColorArray[289]=true; // Spells results, broken fd
  ColorArray[306]=true; // NPC Enrage/Infuriate
  ColorArray[328]=true; // Pet Hits Target
  ColorArray[337]=true; // Pet Messages

  SaveIndx=0;
  pMeleeEvent=new Blech('#');
  ZeroMemory(SaveList,sizeof(SaveList)); 
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("Auto fire off#*#"                                                     ,AUTOFIREOFF  ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("Auto fire on#*#"                                                      ,AUTOFIREON   ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#Attacking #*# Master.#*#"                                          ,PETATTK      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#Sorry#*#calming down.#*#"                                          ,PETBACK      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#Waiting for your order to attack, Master.#*#"                      ,PETHOLD      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# begins casting a spell#*#"                                        ,CASTING      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# begins to cast#*#"                                                ,CASTING      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# has become ENRAGED#*#"                                            ,ENRAGEON     ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# is no longer enraged#*#"                                          ,ENRAGEOFF    ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# is infuriated#*#"                                                 ,INFURIATEON  ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# no longer infuriated#*#"                                          ,INFURIATEOFF ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*# has fallen to the ground#*#"                                      ,FALLEN       ,(void *)1);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#You are no longer feigning death#*#"                               ,BROKEN       ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#Your body aches from a wave of pain#*#"                            ,BROKEN       ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#The strength of your will allows you to resume feigning death#*#"  ,RESUME       ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#You failed to hide yourself#*#"                                    ,HIDEOFF      ,(void *)0);
  SaveList[SaveIndx++]=pMeleeEvent->AddEvent("#*#You are as quiet as a herd of running elephants#*#"                ,SNEAKOFF     ,(void *)0);

  pMeleeTypes= new MQ2MeleeType; 
  AddMQ2Data("Melee",DataMelee); 
  AddMQ2Data("meleemvb",datameleemvb);
  AddMQ2Data("meleemvi",datameleemvi);
  AddMQ2Data("meleemvs",datameleemvs);
  AddCommand("/EnrageON",EnrageON); 
  AddCommand("/EnrageOFF",EnrageOFF); 
  AddCommand("/InfuriateON",InfuriateON); 
  AddCommand("/InfuriateOFF",InfuriateOFF); 
  AddCommand("/KillThis",KillThis); 
  AddCommand("/Melee",Melee); 
  AddCommand("/ThrowIt",ThrowIT); 
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
  Bindding(GameState==GAMESTATE_INGAME);
  if(GameState==GAMESTATE_INGAME) {
    if(!Loaded) Configure();
  } else if(GameState!=GAMESTATE_LOGGINGIN) {
    if(Loaded) Loaded=false;
  }
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
  if(ColorArray[Color & 0x1FF] && doSKILL && gbInZone && pMeleeEvent) switch(Color) {
    case 265: SwingHits++; break;
    case 267: SwingHits++; break;
    case 266: TakenHits++; break;
    case 268: TakenHits++; break;
    default : pMeleeEvent->Feed(Line);
  }
  return 0;
}

PLUGIN_API VOID OnPulse(VOID) {
  if(gbInZone && Loaded && SpawnMe()) if(PCHARINFO2 Me=GetCharInfo2()) {
    if(Me->Shrouded!=Shrouded) {
      SetGameState(GAMESTATE_UNLOADING);
      SetGameState(GAMESTATE_INGAME);
    }
    if(doSKILL && (DWORD)clock()>MeleeTime) {
      MeleeTime=(DWORD)clock()+delay;
      MeleeHandle();
    }
  }
}

PLUGIN_API VOID ShutdownPlugin() {
  SetGameState(GAMESTATE_UNLOADING);
  RemoveCommand("/EnrageON"); 
  RemoveCommand("/EnrageOFF"); 
  RemoveCommand("/InfuriateON"); 
  RemoveCommand("/InfuriateOFF"); 
  RemoveCommand("/KillThis"); 
  RemoveCommand("/Melee"); 
  RemoveCommand("/ThrowIt"); 
  RemoveMQ2Data("Melee"); 
  RemoveMQ2Data("meleemvs");
  RemoveMQ2Data("meleemvi");
  RemoveMQ2Data("meleemvb");
  pMeleeEvent->Reset();
  delete pMeleeEvent;
  delete pMeleeTypes;
  Bindding(false);
}
