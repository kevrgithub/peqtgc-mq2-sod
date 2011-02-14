#Event ToggleUsePet		"<#*#> #*# Use Pet#*#"

| ##
| #
| #
| ##
Sub PetSetup
	/echo Loading Pet system.....
	
	/if (!${Defined[PetINIFile]}) /declare PetINIFile string outer "Bots_${Me.CleanName}.ini"	

	/if (!${Defined[PetSpell]}) /declare PetSpell string outer 
	/if (!${Defined[UsePet]}) /declare UsePet outer
	/if (!${Defined[PetGemToUse]}) /declare PetGem string outer 
	
	/if (!${Ini[${PetINIFile},PetOptions,PetSpell].Length}) /call BuildPetINI
	
	/if (${Ini[${PetINIFile},PetOptions,PetSpell].Length}) /varset PetSpell ${Ini[${PetINIFile},PetOptions,PetSpell]}
	/if (${Ini[${PetINIFile},PetOptions,UsePetDefault].Length}) /varset UsePet ${Ini[${PetINIFile},PetOptions,UsePetDefault]}
	/if (${Ini[${PetINIFile},PetOptions,PetGemToUse].Length}) /varset PetGem ${Ini[${PetINIFile},PetOptions,PetGemToUse]}

	/echo Pet system has been loaded
/return

| ##
| #
| #
| ##
Sub BuildPetINI
	/echo Building Pets Ini file...
	
	/ini "${PetINIFile}" PetOptions PetSpell YourSpellHere
	/ini "${PetINIFile}" PetOptions UsePetDefault FALSE
	/ini "${PetINIFile}" PetOptions PetGemToUse 7
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE
/return

| ##
| # Check to see if we have a pet and recast if required
| ##
Sub CheckPetStatus
	/if (!${UsePet} || ${Stick.Status.Equal[ON]} || ${Me.Moving} || ${Attacking}) /return

	/if (${SpawnCount[${Me.Pet}]} < 1 && ${Me.PctMana} > 70 && !${Attacking}) {
		/call Cast "${PetSpell}" gem${PetGem} ${Spell[${PetSpell}].RecastTime}s
	}
	
/return

| ##
| # Toggles pet on/off
| ##
Sub Event_ToggleUsePet
	/if (!${Defined[UsePet]}) /declare UsePet outer TRUE
	
	/if (!${UsePet}) {
		/varset UsePet TRUE
	} else {
		/varset UsePet FALSE
	}

/return
