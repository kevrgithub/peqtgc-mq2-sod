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

	|- Knock it's resists down a bit
	/call Resists

	|- Knock it's attack speed down
	/call Slow
	
	|- Knock it's attack power down
	/call Cripple

	/if (!${Defined[DebuffToAll]}) /declare DebuffToAll outer FALSE
	
	/varset DebuffToAll FALSE
	/varset SlowImmune FALSE
/return

| #############################################################
| # This handles the casting of the slow spell
| # On a resist it will attempt to recast the number of times 
| # specified within the INI file
| #############################################################
sub Slow
	/declare CastCount int local 0
	
	|- Do we cast?
	/if ((${Me.PctMana} > ${DebuffMana}) && (${Target.Level} >= ${SlowLevel})) {
		|- Mana?
		/if (${Me.CurrentMana} < ${Spell[${SpellSlow}].Mana}) {
			/echo NO WAY
		} else {
		|- Come back here on resist or interupts
		:Slow_Loop
			|- To many!
			/if (${CastCount} >= ${Tries}) {
				/varset CastCount 0
				/return
			}
			
			|- Stunned?! OMFG
			/if (${Me.Stunned}) {
				/delay 3s
				/goto :Slow_Loop
			}
			
			|- Cast the spell
			/call cast "${SpellSlow}"

			|- What happened and what do we do now?
			/if (${Macro.Return.Equal["CAST_INTERRUPTED"]}) {
				/goto :Slow_Loop
			} else /if (${Macro.Return.Equal["CAST_IMMUNE"]}) {
				/echo *** ${Target.CleanName} is IMMUNE to my slow !         
				/varset MobSlowed TRUE
				/varset SlowImmune TRUE
				/varset CastCount 0
			} else /if (${Macro.Return.Equal["CAST_RESISTED"]}) {
					/echo *** ${Target.CleanName} RESISTED slow ! Trying again asap
					/varcalc CastCount ${CastCount}+1
					/goto :Slow_Loop
			} else /if (${Macro.Return.Equal["CAST_SUCCESS"]}) {
				/echo *** ${Target.CleanName} is SLOWED
				/varset MobSlowed TRUE
				/varset CastCount 0
				/call DebuffStatus "${SpellSlow}"
			}
			
			|- We got here, it's either slowed or I gave up
			/varset MobSlowed TRUE
		}
	}
	
/return

| ###############################################################
| # No resist checks as it is designed for an unresistable debuff 
| # such as Tash/Malo
| ###############################################################
sub Resists
	|- Mana?
	/if ((${Me.PctMana} > ${DebuffMana}) && (${Target.Level} >= ${ResistsLevel})) {
	:Resist_Loop
		|- Cast it if we can
		/if (${Me.Stunned}) {
			/delay 3s
			/goto :Resist_Loop
		} else /if (${Me.CurrentMana} < ${Spell[${SpellResists}].Mana}) {
			/echo NO WAY IN HELL
		} else {
			/call cast "${SpellResists}"
			/call DebuffStatus "${SpellResists}"
		}
	}	
	
/return

| ############################################
| # Cast's our weakening spell on the target
| ############################################
sub Cripple
	/declare CastCount int local 0
	
	|- Can I cast?
	/if ((${Me.PctMana} > ${DebuffMana}) && (${Target.Level} >= ${CrippleLevel})) {
		/if (${Me.CurrentMana} < ${Spell[${SpellCripple}].Mana}) {
			/echo NO WAY
		} else {
		:Cripple_Loop
			|- Am I past max tries?
			/if (${CastCount} >= ${Tries}) {
				/varset CastCount 0
				/return
			}
			
			|- OMFG! Stunned!
			/if (${Me.Stunned}) {
				/delay 3s
				/goto :Cripple_Loop
			}
			
			|- Cast the spell
			/call cast "${SpellCripple}" gem3 3s
			
			|- What happened and what do I do now?
			/if (${Macro.Return.Equal["CAST_INTERRUPTED"]}) {
				/goto :Cripple_Loop
			} else /if (${Macro.Return.Equal["CAST_IMMUNE"]}) {
				/echo *** ${Target.CleanName} is IMMUNE to my cripple !         
				/varset MobCrippled TRUE
				/varset CastCount 0
			} else /if (${Macro.Return.Equal["CAST_RESISTED"]}) {
				/echo *** ${Target.CleanName} RESISTED crippled ! Trying again asap
				/varcalc CastCount ${CastCount}+1
				/goto :Cripple_Loop
			} else /if (${Macro.Return.Equal["CAST_SUCCESS"]}) {
				/echo *** ${Target.CleanName} is CRIPPLED
				/varset MobCrippled TRUE
				/varset CastCount 0
				/call DebuffStatus "${SpellCripple}"
			}
			
			|- We got here, it's either slowed or I gave up
			/varset MobCrippled TRUE
		}
	}
	
/return

| ####################################################
| # Echos relevant information to bca or guildchat.
| #
| # @PARAM - string DebuffSpell - The spell to be cast.
| ####################################################
Sub DebuffStatus(string DebuffSpell)
	|- What level is it?
	/if (${SlowImmune} && ${DebuffToAll} && ${DebuffSpell.Equal[${SpellSlow}]}) {
		/guildsay ${Target.CleanName} is IMMUNE to ${DebuffSpell}
	} else /if (${Target.Level} > 66 && ${DebuffToAll}) {
		/guildsay ${DebuffSpell} has landed on ${Target.CleanName} 
	} else {
		/bca ${DebuffSpell} has landed on ${Target.CleanName} 
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
