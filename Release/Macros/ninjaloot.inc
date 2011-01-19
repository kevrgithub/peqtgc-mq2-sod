|-NinjaLoot.inc v1.2 by Mystikule
|- Mod's by Reno


#Event NotYourKill       "You may not loot this Corpse at this time."
#Event SetAutoLoot       "[MQ2] SetAutoLoot#*#"
#Event Autolootstatus    "[MQ2] Autoloot Status#*#"
#Event PartyExp			 "#*#You gain party experience#*#"

Sub LootSetup	
	/if (!${Defined[LootINIFile]}) /declare LootINIFile string outer "Bots_${Me.CleanName}.ini" 
	
	/if (!${Defined[AmLooter]}) /declare AmLooter outer bool FALSE

	/if (!${Ini[${LootINIFile},Autoloot,DoAutoLoot].Length}) {
		/call MakeLootINI
		/return
	}
	/varset AmLooter ${Ini[${LootINIFile},Autoloot,DoAutoLoot]}
	/if (!${AmLooter}) /return
	
	/echo Loading loot system....
		
  |- Make any undefined declares
  /if (!${Defined[NLLootINIFile]}) /declare NLLootINIFile string outer "loot.ini"
  /if (!${Defined[NLVersion]}) /declare NLVersion string outer 1.2
  /if (!${Defined[NLlootdist]}) /declare NLlootdist int outer 100
  /if (!${Defined[NLlootmode]}) /declare NLlootmode string outer

  |- Check version in ini and skip aliases if current
  /if (${Ini[${NLLootINIFile},Loot,Version].Equal[${NLVersion}]}) /call LoadAliases

  |- Declare corpseskip ID arrays
  /declare NLCorpseskip[20,2] int outer
  /declare NLTempCorpse[20] int outer

  |- If empty ini build it up
  /if (!${Ini[${NLLootINIFile},Loot,AutoLootMode].Length}) /call BuildINI

  |- Load variables from ini
  /if (${Ini[${NLLootINIFile},Loot,AutoLootMode].Length}) /varset NLlootmode ${Ini[${NLLootINIFile},Loot,AutoLootMode]}
  /if (${Ini[${NLLootINIFile},Loot,AutoLootDistance].Length}) /varset NLlootdist ${Ini[${NLLootINIFile},Loot,AutoLootDistance]}

  |- Echo NL status upon load
  /echo Ninjaloot.inc Loaded.
  /echo Ninjaloot Usage:
  /echo /autoloot <all|drop|nditem|off> <radius#>
  /echo /lootstatus <displays Autoloot status>
  /echo Autoloot Status
  /if (${NLlootmode.Equal[ALL]}) /docommand /lootnodrop never
  /if (${NLlootmode.Equal[DROP]}) /docommand /lootnodrop always
  /if (${NLlootmode.Equal[NDITEM]}) /docommand /lootnodrop always
  /if (${NLlootmode.Equal[OFF]}) /docommand /lootnodrop always
  
	/echo Loot system has been loaded!
/return

Sub NLAutoloot

  /doevents SetAutoLoot
  /doevents Autolootstatus

  /if (${NLlootmode.Equal[OFF]}) /return
  /if (!${Defined[lootslot]}) /declare lootslot int local
  /if (!${Defined[loottotal]}) /declare loottotal int local -1
  /if (!${Defined[w]}) /declare w int local
  /if (!${Defined[e]}) /declare e int local

  /call Updatecorpse

  |- Check for avialable corpses to loot
  /for e 1 to 20
    /varset NLTempCorpse[${e}] ${NearestSpawn[${e},NPC Corpse radius ${NLlootdist}].ID}
  /next e
  /for e 1 to 20
    /if (${NLTempCorpse[${e}]}<=0) {
      /echo Autoloot:  No More Corpses Found
      /return
    }
  |- Check if corpse is listed in Skip array
    /for w 1 to 20
      /if (${NLTempCorpse[${e}]}==${NLCorpseskip[${w},1]}) {
        /echo Autoloot:  Skipping ${NLTempCorpse[${e}]}; Per Skip List
        /next e
      }
    /next w
  |- Test Z distance
    /if (${Spawn[${NLTempCorpse[${e}]}].DistanceZ}>50) {
      /call Addcorpse
      /echo Autoloot:  Skipping ${NLTempCorpse[${e}]}; Z Plane out of limits
      /next e
    }
  |- Make sure Spawnid is a corpse
    /if (${Spawn[${NLTempCorpse[${e}]}].Type.NotEqual[Corpse]}) {
      /echo Autoloot:  Skipping ${NLTempCorpse[${e}]}; Not a corpse!
      /next e
    }
  |- Check if corpse is players corpse
    /if (${Spawn[${NLTempCorpse[${e}]}].Deity.ID}!=0) {
      /echo Autoloot:  Skipping ${NLTempCorpse[${e}]}; Is a PLAYERS corpse!
      /next e
    }
  |-Target corpse and begin looting
    /delay 1
    /squelch /target id ${NLTempCorpse[${e}]}
    /delay 1s ${Target.ID}==${NLTempCorpse[${e}]}
    /echo Autoloot:  Looting ${Target.CleanName}
    /delay 5s ${Target.ID}==${NLTempCorpse[${e}]}
    /if (${Target.ID}!=${NLTempCorpse[${e}]}) /next e
  |- Get 5 seconds (max) closer; warning, no detecting being stuck!
    /face fast ${If[${Me.Underwater},,nolook]}
    /if (${Me.Buff[Summon Drogmor].ID} || ${Me.Buff[Summon Horse].ID}) /dismount 
    /keypress forward hold
    /delay 5s ${Target.Distance}<5
    /keypress forward
  |- Check for full inventory
    /if (!${Me.FreeInventory}) {
      /beep
      /bca Autoloot:  ** INVENTORY FULL !
      /varset NLlootmode OFF
      /bca Autoloot:  ** AutoLooting is now OFF!
      /return
    }
    /delay 1s
    /loot
    /delay 1s ${Window[LootWnd].Open}
  |- Check if unable to loot at this time
    /doevents NotYourKill
  |- Corpse with no items
    /if (!${Corpse.Items}) {
      /echo Autoloot:  NO LOOT! Cheap Bastard!
      /notify LootWnd DoneButton leftmouseup
      /delay 2s !${Corpse.Open}
      /next e
    }
  |- Get total count of items on corpse
    /varset loottotal ${Math.Calc[${Corpse.Items}]}
  :autoxplootlag
    /if (${loottotal}!=${Corpse.Items}) {
      /varset loottotal ${Corpse.Items}
      /goto :autoxplootlag
    }
  |- Check items on Corpse
    /for lootslot 1 to ${loottotal}
    /delay 1s ${InvSlot[loot${lootslot}].Item.ID}!=0
  |- Check if item is LORE
    /if (${InvSlot[loot${lootslot}].Item.Lore}) {
      /echo Autoloot:  Lore ITEM Found
      /if (${FindItem[${InvSlot[loot${lootslot}].Item.Name}].InvSlot}) {
        /echo Autoloot:  Skipping; Found Loreitem in Characters Inventory
        /if (${lootslot}>=${loottotal}) {
          /call Itemsleft
          /next e
        }
        /next lootslot
      }
      /if (${FindItem[${InvSlot[loot${lootslot}].Item.Name}].InvSlot.Pack}) {
        /echo Autoloot:  Skipping; Found Loreitem in Characters Bags
        /if (${lootslot}>=${loottotal}) {
          /call Itemsleft
          /next e
        }
        /next lootslot
      }
    }
  |- Handling of NODROP items on corpse
    /if (${InvSlot[loot${lootslot}].Item.NoDrop}) {
      /echo Autoloot:  NO DROP ITEM Found
      /if (!${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Length}) {
        /ini "${NLLootINIFile}" "${InvSlot[loot${lootslot}].Item.Name.Left[1]}" "${InvSlot[loot${lootslot}].Item.Name}" NDITEM
        /echo Autoloot:  Adding ${InvSlot[loot${lootslot}].Item.Name} to ${NLLootINIFile} defaulting as NDITEM
      }
      /if (${NLlootmode.Equal[DROP]}) {
        /if (${lootslot}>=${loottotal}) {
          /call Itemsleft
          /next e
        }
        /next lootslot
      }
      /if (${NLlootmode.Equal[NDITEM]}) {
        /if (${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Equal[NDKeep]}) {
          /call LootItem ${lootslot} KEEP
          /if (${lootslot}>=${loottotal}) {
            /call Itemsleft
            /next e
          }
          /next lootslot
        }
        /if (${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Equal[NDDestroy]}) {
          /call LootItem ${lootslot} DESTROY
          /if (${lootslot}>=${loottotal}) {
            /call Itemsleft
            /next e
          }       
          /next lootslot
        }
        /if (${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Equal[NDITEM]}) {
          /echo Autoloot:  Skipping ${InvSlot[loot${lootslot}].Item.Name} per NDITEM flag
          /if (${lootslot}>=${loottotal}) {
            /call Itemsleft
            /next e
          }
          /next lootslot
        }
      }
      /if (${NLlootmode.Equal[ALL]}) {
        /call LootItem ${lootslot} KEEP
        /if (${lootslot}>=${loottotal}) {
          /call Itemsleft
          /next e
        }
        /next lootslot
      }
    }
  |- Loot droppable items and Keep/Destroy by ini flags
    /if (!${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Length}) {
      /ini "${NLLootINIFile}" "${InvSlot[loot${lootslot}].Item.Name.Left[1]}" "${InvSlot[loot${lootslot}].Item.Name}" Keep
      /echo Autoloot:  Adding ${InvSlot[loot${lootslot}].Item.Name} to ${NLLootINIFile} defaulting as Keep
    }
    /if (${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Equal[Keep]}) {
      /call LootItem ${lootslot} KEEP
      /if (${lootslot}>=${loottotal}) {
        /call Itemsleft
        /next e
      }                 
      /next lootslot
    }
    /if (${Ini[${NLLootINIFile},"${InvSlot[loot${lootslot}].Item.Name.Left[1]}","${InvSlot[loot${lootslot}].Item.Name}"].Equal[Destroy]}) {
      /call LootItem ${lootslot} DESTROY
      /if (${lootslot}>=${loottotal}) {
        /call Itemsleft
        /next e
      }         
      /next lootslot
    }
    /if (${lootslot}>=${loottotal}) {
      /call Itemsleft
      /next e
    }
    /next lootslot
  /next e
/return

Sub Itemsleft
  |-Link remaining items on corpse
  /if (${Corpse.Items}) {
    /notify LootWND LW_BroadcastButton leftmouseup
    /keypress enter chat
    /call Addcorpse
  }
  /notify LootWnd DoneButton leftmouseup
  /delay 1s !${Corpse.Open}
/return

Sub Addcorpse
  /if (!${Defined[e]}) /declare e int local
  /for e 20 downto 2
    /varset NLCorpseskip[${e},1] ${NLCorpseskip[${Math.Calc[${e}-1]},1]}
    /varset NLCorpseskip[${e},2] ${NLCorpseskip[${Math.Calc[${e}-1]},2]}
  /next e
  /varset NLCorpseskip[1,1] ${Target.ID}
  /varset NLCorpseskip[1,2] ${Math.Calc[${MacroQuest.Running}+1800000]}
/return

Sub addnotmykill
  /if (!${Defined[e]}) /declare e int local
  /for e 20 downto 2
    /if (${NLCorpseskip[${e},1]}!=0) /next e
  /varset NLCorpseskip[${e},1] ${Target.ID}
  /varset NLCorpseskip[${e},2] ${Math.Calc[${MacroQuest.Running}+180000]}
/return

Sub Updatecorpse
  /if (!${Defined[e]}) /declare e int local
  /for e 1 to 20
    /if (!${Spawn[${NLCorpseskip[${e},1]}].ID}) /varset NLCorpseskip[${e},1] 0
    /if (${NLCorpseskip[${e},2]}<=${MacroQuest.Running} && ${NLCorpseskip[${e},1]}!=0) /varset NLCorpseskip[${e},1] 0
  /next e
/return

Sub LootItem(int lootslot, string handling)
  /if (${handling.Equal[KEEP]}) /echo Autoloot:  Keeping a ${InvSlot[loot${lootslot}].Item.Name}... WOOT!
  /if (${handling.Equal[DESTROY]}) /echo Autoloot:  Destroying a ${InvSlot[loot${lootslot}].Item.Name}... Bubbye!
  :retryloot
  /if (${handling.Equal[KEEP]}) {
		/itemnotify loot${lootslot} rightmouseup
		/if (${InvSlot[loot${lootslot}].Item.ID}) /goto :retryloot
	}
  /if (${handling.Equal[DESTROY]}) {
		/itemnotify loot${lootslot} leftmouseup
		/delay 4s ${Cursor.ID}
		/if (!${Cursor.ID} && ${NLlootmode.Equal[DROP]}) {
			/if (${Window[ConfirmationDialogBox].Open}) /notify ConfirmationDialogBox No_Button leftmouseup
			/goto :retryloot
		}
		/if (!${Cursor.ID} && ${NLlootmode.NotEqual[DROP]}) {
			/if (${Window[ConfirmationDialogBox].Open}) /notify ConfirmationDialogBox Yes_Button leftmouseup
			/goto :retryloot
		}
	:NLLootItem
		/if (${handling.Equal[KEEP]}) /autoinventory
		/if (${handling.Equal[DESTROY]}) /destroy
		/if (${Cursor.ID}) /goto :NLLootItem
	}
/return

Sub LoadAliases
  /ini "${NLLootINIFile}" Loot Version "${NLVersion}"
  /squelch /alias /autoloot /echo SetAutoLoot
  /squelch /alias /lootstatus /echo Autoloot Status
/return

Sub BuildINI
  |- Build a retarded ini, but it makes it alpha ordered
  /echo Building INI file
  /ini "${NLLootINIFile}" Loot AutoLootMode OFF
  /ini "${NLLootINIFile}" Loot AutoLootDistance 100
  /ini "${NLLootINIFile}" A A NULL
  /ini "${NLLootINIFile}" B B NULL
  /ini "${NLLootINIFile}" C C NULL
  /ini "${NLLootINIFile}" D D NULL
  /ini "${NLLootINIFile}" E E NULL
  /ini "${NLLootINIFile}" F F NULL
  /ini "${NLLootINIFile}" G G NULL
  /ini "${NLLootINIFile}" H H NULL
  /ini "${NLLootINIFile}" I I NULL
  /ini "${NLLootINIFile}" J J NULL
  /ini "${NLLootINIFile}" K K NULL
  /ini "${NLLootINIFile}" L L NULL
  /ini "${NLLootINIFile}" M M NULL
  /ini "${NLLootINIFile}" N N NULL
  /ini "${NLLootINIFile}" O O NULL
  /ini "${NLLootINIFile}" P P NULL
  /ini "${NLLootINIFile}" Q Q NULL
  /ini "${NLLootINIFile}" R R NULL
  /ini "${NLLootINIFile}" S S NULL
  /ini "${NLLootINIFile}" T T NULL
  /ini "${NLLootINIFile}" U U NULL
  /ini "${NLLootINIFile}" V V NULL
  /ini "${NLLootINIFile}" W W NULL
  /ini "${NLLootINIFile}" X X NULL
  /ini "${NLLootINIFile}" Y Y NULL
  /ini "${NLLootINIFile}" Z Z NULL
/return

Sub Updateini
  /ini "${NLLootINIFile}" "Loot" "AutoLootMode" "${NLlootmode}"
  /ini "${NLLootINIFile}" "Loot" "AutoLootDistance" "${NLlootdist}"
/return

Sub Event_SetAutoLoot(string Line)
  |- Set the new loot distance, if supplied
  /if (${Line.Arg[4].Length}) /varset NLlootdist ${Line.Arg[4]}
  |- Set the mode
  /if (${Line.Arg[3].Equal[ALL]}) {
    /varset NLlootmode ALL
    /echo ** AutoLoot ON - Will loot ALL items
    /echo ****CAUTION**** Turning off requestor for NODROP items!!!  ****CAUTION****
    /echo ** AutoLoot distance is ${NLlootdist} ft.
    /docommand /lootnodrop never
    /call Updateini
    /return
  }
  /if (${Line.Arg[3].Equal[DROP]}) {
    /varset NLlootmode DROP
    /echo ** AutoLoot ON - Will loot only DROPABLE items
    /echo ** AutoLoot distance is ${NLlootdist} ft.
    /docommand /lootnodrop always
    /call Updateini
    /return
  }
  /if (${Line.Arg[3].Equal[NDITEM]}) {
    /varset NLlootmode NDITEM
    /echo ** AutoLoot ON - Will loot only SPECIFIED NO-DROP items
    /echo ** AutoLoot distance is ${NLlootdist} ft.
    /docommand /lootnodrop always
    /call Updateini
    /return
  }
  /if (${Line.Arg[3].Equal[OFF]}) {
    /varset NLlootmode OFF
    /echo ** AutoLoot OFF - Will NOT loot any items
    /call Updateini
    /return
  }
  /echo Syntax: 
  /echo /autoloot <all|drop|nditem|off> <radius#>
  /echo Defaulting to OFF
  /varset NLlootmode OFF
  /call Updateini
/return

Sub Event_Autolootstatus
  /if (${NLlootmode.Equal[ALL]}) {
    /echo ** AutoLoot ON - Will loot ALL items
    /echo ****CAUTION**** Turning off requestor for NODROP items!!!  ****CAUTION****
    /echo ** AutoLoot distance is ${NLlootdist} ft.
  }
  /if (${NLlootmode.Equal[DROP]}) {
    /echo ** AutoLoot ON - Will loot only DROPABLE items
    /echo ** AutoLoot distance is ${NLlootdist} ft.
  }
  /if (${NLlootmode.Equal[NDITEM]}) {
    /echo ** AutoLoot ON - Will loot only SPECIFIED NO-DROP items
    /echo ** AutoLoot distance is ${NLlootdist} ft.
  }
  /if (${NLlootmode.Equal[OFF]}) {
    /echo ** AutoLoot OFF - Will NOT loot any items
  }
/return

Sub Event_NotYourKill
	/if (!${Defined[NLLootINIFile]}) /return
	
  /echo Autoloot:  Adding corpse to unable to loot list
  /call addnotmykill
/return

Sub Event_PartyExp
	/if (!${Defined[NLLootINIFile]}) /return

	/if (${SpawnCount[npc radius 35]} > 0) /return

	/call NLAutoloot
/return

Sub MakeLootINI
	|- Build the INI file
	/echo Building Loot INI file.....
	
	|- Self buffs
	/ini "${LootINIFile}" "Autoloot" DoAutoLoot FALSE
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE	
/return
