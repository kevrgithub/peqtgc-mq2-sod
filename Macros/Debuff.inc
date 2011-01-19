| # @file - Debuff.inc
| # @author - Reno
| #
| # Call DebuffSetup at the start of your macro
| # Call DebuffCycle to debuff your target
| # Note: You currently must have the NPC targeted before you call this

#Event DebuffNow 		"<#2#> #*# Debuff #1#"

| ####################################
| # Setup everything before we begin.
| # Called by any character using this
| # file before the main loop.
| ####################################
sub DebuffSetup
	/echo Loading debuff system....
	
	/if (!${Defined[DebuffINIFile]}) /declare DebuffINIFile string outer "Bots_${Me.CleanName}.ini"
	
	|- Our Slow spell
	/if (!${Defined[SpellSlow]}) /declare SpellSlow string outer
	|- Our Resist Debuff, unresistable
	/if (!${Defined[SpellResists]}) /declare SpellResists string outer
	|- Our cripple spell
	/if (!${Defined[SpellCripple]}) /declare SpellCripple string outer 

	|- Mana required to debuff
	/if (!${Defined[DebuffMana]}) /declare DebuffMana int outer

	|- Level of NPC's to slow
	/if (!${Defined[SlowLevel]}) /declare SlowLevel int outer
	|- Level of NPC's to cripple
	/if (!${Defined[CrippleLevel]}) /declare CrippleLevel int outer
	|- Level of NPC's to debuffresists
	/if (!${Defined[ResistsLevel]}) /declare ResistsLevel int outer

	|- Is it slowed?
	/if (!${Defined[MobSlowed]}) /declare MobSlowed outer FALSE
	|- Is it crippled?
	/if (!${Defined[MobCrippled]}) /declare MobCrippled outer FALSE
	|- Is it slow immune?
	/if (!${Defined[SlowImmune]}) /declare SlowImmune outer FALSE

	|- Number of time's to try to debuff?
	/if (!${Defined[Tries]}) /declare Tries int outer
	
	|- If INI doesn't exist, then build it
	/if (!${Ini[${DebuffINIFile},DebuffSpells,Slow].Length}) /call BuildDebuffINI
	
	|- Load some values from the IINI file
	/if (${Ini[${DebuffINIFile},DebuffSpells,Slow].Length}) /varset SpellSlow ${Ini[${DebuffINIFile},DebuffSpells,Slow]}
	/if (${Ini[${DebuffINIFile},DebuffSpells,Resists].Length}) /varset SpellResists ${Ini[${DebuffINIFile},DebuffSpells,Resists]}
	/if (${Ini[${DebuffINIFile},DebuffSpells,Cripple].Length}) /varset SpellCripple ${Ini[${DebuffINIFile},DebuffSpells,Cripple]}
	
	/if (${Ini[${DebuffINIFile},DebuffLevels,LevelToSlow].Length}) /varset SlowLevel ${Ini[${DebuffINIFile},DebuffLevels,LevelToSlow]}
	/if (${Ini[${DebuffINIFile},DebuffLevels,LevelToCripple].Length}) /varset CrippleLevel ${Ini[${DebuffINIFile},DebuffLevels,LevelToCripple]}
	/if (${Ini[${DebuffINIFile},DebuffLevels,LevelToDebuffResists].Length}) /varset ResistsLevel ${Ini[${DebuffINIFile},DebuffLevels,LevelToDebuffResists]}
	
	/if (${Ini[${DebuffINIFile},TriesToLand,Recasts].Length}) /varset Tries ${Ini[${DebuffINIFile},TriesToLand,Recasts]}
	/if (${Ini[${DebuffINIFile},Mana,MinManaToDebuff].Length}) /varset DebuffMana ${Ini[${DebuffINIFile},Mana,MinManaToDebuff]}
	
	/echo Debuff system has been loaded.
/return

| ###########################
| # Build the Ini to be used
| ###########################
sub BuildDebuffINI
	/echo Building Debuff INI...
	
	/ini "${DebuffINIFile}" DebuffSpells Slow YourSpellHere
	/ini "${DebuffINIFile}" DebuffSpells Resists YourSpellHere
	/ini "${DebuffINIFile}" DebuffSpells Cripple YourSpellHere
	
	/ini "${DebuffINIFile}" DebuffLevels LevelToSlow 65
	/ini "${DebuffINIFile}" DebuffLevels LevelToCripple 66
	/ini "${DebuffINIFile}" DebuffLevels LevelToDebuffResists 66
	
	/ini "${DebuffINIFile}" TriesToLand Recasts 3
	/ini "${DebuffINIFile}" Mana MinManaToDebuff 45
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE

/return

| ################################################
| # Call this when you want to debuff a target.
| # It must be targeted at the start of the call
| ################################################
sub DebuffCycle
	/if (${Me.PctMana} < ${DebuffMana}) /return
	
	|- Knock it's resists down a bit
	/if (${Target.Level} >= ${ResistsLevel}) /call CastDebuff "${SpellResists}"

	|- Knock it's attack speed down
	/if (${Target.Level} >= ${SlowLevel}) /call CastDebuff "${SpellSlow}"
	
	|- Knock it's attack power down
	/if (${Target.Level} >= ${CrippleLevel}) /call CastDebuff "${SpellCripple}"

	/if (!${Defined[DebuffToAll]}) /declare DebuffToAll outer FALSE
	
	/varset DebuffToAll FALSE
/return

| ##
| #
| ##
Sub CastDebuff(string ToCast)
	/declare CastCount int local 0
	
	|- Mana?
	/if (${Me.CurrentMana} < ${Spell[${ToCast}].Mana}) {
		/echo NO WAY
	} else {
	|- Come back here on resist or interupts
	:Recast_Loop
		|- Too many!
		/if (${CastCount} >= ${Tries}) {
			/varset CastCount 0
			/return
		}
		
		|- Stunned?! OMFG
		/if (${Me.Stunned}) {
			/delay 3s
			/goto :Recast_Loop
		}
		
		|- Cast the spell
		/call cast "${ToCast}" 3s

		|- What happened and what do we do now?
		/if (${Macro.Return.Equal["CAST_INTERRUPTED"]}) {
			/goto :Recast_Loop
		} else /if (${Macro.Return.Equal["CAST_IMMUNE"]}) {
			/echo *** ${Target.CleanName} is IMMUNE to ${ToCast} !    
			/call SpellStatus "*** ${Target.CleanName} is IMMUNE to ${ToCast} ***"
			/varset CastCount 0
		} else /if (${Macro.Return.Equal["CAST_RESISTED"]}) {
			/echo *** ${Target.CleanName} RESISTED ${ToCast} ! Trying again asap
			/varcalc CastCount ${CastCount}+1
			/goto :Recast_Loop
		} else /if (${Macro.Return.Equal["CAST_SUCCESS"]}) {
			/echo *** ${Target.CleanName} has been struck by ${ToCast}
			/varset CastCount 0
			/call SpellStatus "${ToCast} has landed on ${Target.CleanName} !!"
		}
	}
	
/return

| ##
| #
| ##
Sub SpellStatus(string Message)
	/if (${DebuffToAll}) {
		/guildsay ${Message}
	} else {
		/bca ${Message}
	}
/return

| ##############################################
| # Seperate event to handle immediate debuff
| # requests.
| ##############################################
Sub Event_DebuffNow(string Line, string DebuffTarget, string MainAssist)
	/if (!${Defined[DebuffToAll]}) /declare DebuffToAll outer FALSE
	
	/if (!${Defined[SpellSlow]}) /return
	
	/assist ${MainAssist}
	/delay 2s ${Target.Name.Equal[${DebuffTarget}]}
	
	/if (!${Target.Name.Equal[${DebuffTarget}]}) /return
	
	/varset DebuffToAll TRUE
	
	/call DebuffCycle
/return
