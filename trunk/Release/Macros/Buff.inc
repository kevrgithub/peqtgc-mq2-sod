| # @file - Buff.inc
| # @author - Reno
| #
| # This include is designed to manage all the buffs requests that might come in to a toon via a tell

#Event CastABuff		"#2# tells you, '#1#'"
#Event CastABuffGroup	"#2# tells the group, '#1#'"
#Event CastABuffBC		"[#2#(msg)] #1#"
#Event CastABuffBC		"<#2#> #*# #1#"

| ##
| # Setup everything before we use it.
| # This should be called by any macro 
| # that included this file before the
| # main loop.
| ##
Sub BuffSetup
	/echo Loading buff system....
	
	|- Sections: Options, Buffs, Full Buff
	/if (!${Defined[BuffSpells]}) /declare BuffSpells string outer "BuffSpellsMap"
	/if (!${Defined[FullBuff]}) /declare FullBuff string outer "FullBuffSpellsArray"
	/if (!${Defined[SelfBuff]}) /declare SelfBuff string outer "SelfBuffSpellsArray"
	
	|- Name of the INi file
	/if (!${Defined[BuffINIFile]}) /declare BuffINIFile string outer "Bots_${Me.CleanName}.ini" 
	
	|- Variables in the Options area
	/if (!${Defined[MinBuffMana]}) /declare MinBuffMana int outer 
	/if (!${Defined[GemNo]}) /declare GemNo int outer 
	/if (!${Defined[OnlyGuild]}) /declare OnlyGuild outer 
	/if (!${Defined[BuffBySpellName]}) /declare BuffBySpellName outer	
	
	|- If no INI then make one
	/if (!${Ini[${BuffINIFile},Options,MinBuffMana].Length}) /call MakeBuffINI
	
	|- Load Some Variables Out of the Ini
	|-Buff Spells
	/if (${Ini[${BuffINIFile},Options,MinBuffMana].Length}) /varset MinBuffMana ${Ini[${BuffINIFile},Options,MinBuffMana]}
	/if (${Ini[${BuffINIFile},Options,GemToUse].Length}) /varset GemNo ${Ini[${BuffINIFile},Options,GemToUse]}
	/if (${Ini[${BuffINIFile},Options,OnlyBuffGuild].Length}) /varset OnlyGuild ${Ini[${BuffINIFile},Options,OnlyBuffGuild]}
	/if (${Ini[${BuffINIFile},Options,BuffBySpellName].Length}) /varset OnlyGuild ${Ini[${BuffINIFile},Options,BuffBySpellName]}	
	
	|- Build the array of buff's and their respective commands
	/call IniToHashMap ${BuffINIFile} Buffs ${BuffSpells}
	/call IniToDynArray ${BuffINIFile} "Full Buff" ${FullBuff}
	/call IniToDynArray ${BuffINIFile} "Self Buff" ${SelfBuff}
	
	/echo Buff system has been loaded!
/return

| ######################################################################################
| # Makes our Ini file if it doesn't exist, throwing in some default values
| # NOTE: This will end the macro, forcing the player to go manually update the INI
| ######################################################################################
Sub MakeBuffINI
	|- Build the INI file
	/echo Building Buff INI file.....
	
	|- BuffSpells section - tells us what spells to use on each class
	/ini "${BuffINIFile}" Options MinBuffMana 20
	/ini "${BuffINIFile}" Options GemToUse 5
	/ini "${BuffINIFile}" Options OnlyBuffGuild FALSE
	/ini "${BuffINIFile}" Options BuffBySpellName TRUE

	/ini "${BuffINIFile}" Buffs hov "Hand of Virtue"
	
	|- Buffs to be cast for a full round
	/ini "${BuffINIFile}" "Full Buff" buff1 "name of buff"
	/ini "${BuffINIFile}" "Full Buff" buff2 "name of buff"
	/ini "${BuffINIFile}" "Full Buff" buff3 "name of buff"
	/ini "${BuffINIFile}" "Full Buff" buff4 "name of buff"
	
	|- Self buffs
	/ini "${BuffINIFile}" "Self Buff" Spell "name of spell"
	/ini "${BuffINIFile}" "Self Buff" itemname "name of item"
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE
	
/return

| ##
| # Checks to see if we are able to cast the desired buff and casts it.
| #
| # @PARAM - string line - The line that triggered the event.
| # @PARAM - string command - The trigger for the spell.
| # @PARAM - string requestor - The name of the player requesting the buff.
| ##
Sub Event_CastABuff(string line, string command, string requestor)
	/call DispatchBuffCall "${command}" ${requestor} "tell"
/return

| ##
| # Checks to see if we are able to cast the desired buff and casts it.
| #
| # @PARAM - string line - The line that triggered the event.
| # @PARAM - string command - The trigger for the spell.
| # @PARAM - string requestor - The name of the player requesting the buff.
| ##
Sub Event_CastABuffGroup(string line, string command, string requestor)
	/call DispatchBuffCall "${command}" ${requestor} "group"
/return

| ##
| # Checks to see if we are able to cast the desired buff and casts it.
| #
| # @PARAM - string line - The line that triggered the event.
| # @PARAM - string command - The trigger for the spell.
| # @PARAM - string requestor - The name of the player requesting the buff.
| ##
Sub Event_CastABuffBC(string line, string command, string requestor)
	/call DispatchBuffCall "${command}" ${requestor} "bct"
/return

| ##
| # This will dispatch the buff call properly.
| #
| # @PARAM - string command - The trigger for the spell.
| # @PARAM - string requestor - The name of the player requesting the buff. 
| # @PARAM - string channel - (Optional) The channel to send the response to.
| ##
Sub DispatchBuffCall(string command, string requestor, string channel)
	/if (${SpawnCount[${requestor}]} < 1) /return
	
	| need to get rid of the case checking, lowercase and uppercase need to work
	/if (${command.Equal[buff]}) {
		/call CastAllBuffs ${requestor} ${channel}
	} else {
		/call DoBuff "${command}" ${requestor} ${channel}
	}
/return

| ##
| # Casts all buffs on a particular player.
| #
| # @PARAM - string requestor - The name of the player requesting the buff.
| # @PARAM - string channel -(Optional) The channel to send the response to.
| ##
Sub CastAllBuffs(string requestor, string channel)
	/declare i int local 0
	/declare size int local ${Vector[${FullBuff}].Size}
	
	/if (${size} < 1) /return
	
	|- Run through all of the buffs and cast them
	/for i 0 to ${size}	
		/call PauseUntilReady
		/call DoBuff "${Vector[${FullBuff}].At[${i}]}" ${requestor} ${channel}
	/next i
/return

| ##
| #
| ##
Sub PauseUntilReady
	/declare CanCast local FALSE
	:CastReady_Loop
	/if (${Me.SpellReady[1]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[2]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[3]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[4]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[5]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[6]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[7]}) {
		/varset CanCast TRUE
	} else /if (${Me.SpellReady[8]}) {
		/varset CanCast TRUE
	}
	/if (!${CanCast}) /goto :CastReady_Loop
/return

| ##
| # Checks to see if we are able to cast the desired buff and casts it.
| #
| # @PARAM - string command - The trigger for the spell.
| # @PARAM - string requestor - The name of the player requesting the buff.
| # @PARAM - string channel - (Optional) The channel to respond to (tell, bct), NULL gives no response.
| ##
Sub DoBuff(string command, string requestor, string channel)
	/declare BuffSpell string local ${HashMap[${BuffSpells}].Find[${command.Lower}]}
	/declare AASpell bool local FALSE
	
	/if (${Int[${command}]} > 0) /return
	
	|- Not in our alias's, try the full spell name if allowed
	/if (${BuffSpell.Equal[NULL]} && ${Me.Book[${command}]} && ${BuffBySpellName}) /varset BuffSpell ${command}
	
	/if (${BuffSpell.Equal[NULL]}) /return
	
	|- Turn off melody
	/if (${Me.Class.Name.Equal[Bard]} && ${Me.Casting.ID} != 0) /melody
	
	|- Maybe it is an AA ability?
	/if (${Me.AltAbility[${command}]} > 0) {
		/varset AASpell TRUE
		/varset BuffSpell ${command}
	}
	
	/if (${Me.Gem[${BuffSpell}]} > 0 && !${Me.SpellReady[${BuffSpell}]}) {
		/return
	}
	
	|- It isn't a spell
	/if (!${Spell[${BuffSpell}].ID} && !${AASpell}) /return
	| /gsay TWO ${command} [] ${AASpell}	
	
	|- they are not in zone
	/if (${SpawnCount[pc ${requestor}]} < 1) /return

	/if (${Spell[${BuffSpell}].SpellType.NotEqual[Self]}) {
		|- target the player
		/target pc ${requestor}
		/delay 5s ${Target.CleanName.Equal[${requestor}]}
	}
	
	| /gsay THREE
	|- Should we be casting?
	/declare Response string local ""
	/declare DoICast local FALSE
	/if (${Me.Book[${BuffSpell}]} < 1 && !${AASpell}) {
		/varset Response "I do not have the spell ${BuffSpell}"
	} else /if (${OnlyGuild} && ${Target.Guild.NotEqual[${Me.Guild}]}) {
		/return
	} else /if (${Me.PctMana} < ${MinBuffMana}) {
		/varset Response "I am to low on mana to buff you"
	} else /if (${Me.CurrentMana} < ${Spell[${BuffSpell}].Mana}) {
		/varset Response "I do not have the mana required to cast ${BuffSpell}"
	} else /if (${Target.Distance} > ${Spell[${BuffSpell}].Range} && !(${Target.Distance} < 100 && ${Spell[${BuffSpell}].Range} < 1) && (${requestor.NotEqual[${Me.CleanName}]})) {
		| /echo this is my catch
		/varset Response "You are out of range for ${BuffSpell}"
	} else /if (${Spell[${BuffSpell}].SpellType.NotEqual[Beneficial(Group)]} && ${Spell[${BuffSpell}].SpellType.NotEqual[Beneficial]}) {
		/varset Response "${BuffSpell} is not a buff"
	} else /if (!${Spell[${BuffSpell}].Stacks} && ${Me.CleanName.Equal[${requestor}]}) {
		| /gsay NO STACK
		/return
	} else {
		/varset Response "Casting ${BuffSpell}!!"
		/varset DoICast TRUE
	} 
	
	| /gsay FOUR=> ${Response} ${DoICast}
	|- Send the response
	/if (!${Defined[channel]}) {
		|- do not send the response
	} else /if (${channel.Equal[tell]}) {
		/tell ${requestor} ${Response}
	} else /if (${channel.Equal[bct]}) {
		/bct ${requestor} ${Response}
	} else /if (${channel.Equal[group]}) {
		/gsay ${requestor} => ${Response}
	}	
	
	|- cast the spell if we should
	/if (${DoICast}) {		
		/if (${AASpell}) {
			| /gsay In ALT
			/if (${Me.AltAbilityReady[${BuffSpell}]}) /call cast "${BuffSpell}" alt
		} else {
			/declare WaitTime int local ${Spell[${BuffSpell}].RecastTime}
			/if (${WaitTime} > 4) /varset WaitTime 4
			/call cast "${BuffSpell}" gem${GemNo} ${WaitTime}s
		}
	}
/return

| ##
| # Checks to see if we need to cast any of our self buffs on ourself.
| ##
Sub CheckSelfBuffs
	/if (${Me.Combat} || ${Attacking}) /return
	
	/declare i int local 0
	/declare size int local ${Vector[${SelfBuff}].Size}
	/varcalc size ${size}-1
	
	/for i 0 to ${size}
		/if (!${Defined[Buff]}) /declare Buff string local
		/varset Buff ${Vector[${SelfBuff}].At[${i}]}
		| /gsay BUFF IS ${Buff} SIZE WAS ${size}
		
		/if (!${Spell[${Buff}].ID}) {
			| /gsay ITEM ${Buff}
			/if (!${Me.Buff[${FindItem[${Buff}].Spell}].ID} && !${Me.Combat} && !${Me.Moving}) /call DoItemBuff "${Buff}" ${Me.CleanName}
		} else {
			| /gsay SPELL ${Buff}
			/if (!${Me.Buff[${Buff}].ID} && !${Me.Combat} && !${Me.Moving}) /call DoBuff "${Buff}" ${Me.CleanName}
		}
	/next i
/return

| ##
| # Checks to see if we are able to cast the desired item and casts it.
| #
| # @PARAM - string item - The item to be casted.
| # @PARAM - string requestor - The name of the player requesting the buff.
| # @PARAM - string channel - (Optional) The channel to respond to (tell, bct), NULL gives no response.
| ##
Sub DoItemBuff(string myitem, string requestor, string channel)
	/if (${FindItemCount[${myitem}]} < 1) /return
	/if (${Me.Class.Name.Equal[Bard]} && ${Me.Casting.ID} != 0) /melody
	
	|- they are not in zone
	/if (${SpawnCount[${requestor}]} < 1) /return
	
	/if (${Spell[FindItem[${myitem}].Spell].SpellType.NotEqual[Self]}) {
		|- target the player
		/target pc ${requestor}
		/delay 5s ${Target.CleanName.Equal[${requestor}]}
	}

	|- Should we be casting?
	/declare Response string local ""
	/declare DoICast local FALSE
	/if (${OnlyGuild} && ${Target.Guild.NotEqual[${Me.Guild}]}) {
		/return
	} else /if (${Target.Distance} > ${Spell[FindItem[${myitem}].Spell].Range} && ${Spell[FindItem[${myitem}].Spell].SpellType.NotEqual[Self]}) {
		/varset Response "You are out of range for ${myitem}"
	} else /if (${Spell[FindItem[${myitem}].Spell].SpellType.NotEqual[Beneficial(Group)]} && ${Spell[FindItem[${myitem}].Spell].SpellType.NotEqual[Beneficial]}) {
		/varset Response "${myitem} does not cast a buff"
	} else /if (!${FindItem[${myitem}].Spell.Stacks}) {
		/return
	} else {
		/varset Response "Casting ${myitem}!!"
		/varset DoICast TRUE
	} 

	|- Send the response
	/if (!${Defined[channel]}) {
		|- do not send a response
	} else /if (${channel.Equal[tell]}) {
		/tell ${requestor} ${Response}
	} else /if (${channel.Equal[bct]}) {
		/bct ${requestor} ${Response}
	} else /if (${channel.Equal[group]}) {
		/gsay ${requestor} => ${Response}
	}

	|- cast the spell if we should
	/if (${DoICast}) /call cast "${myitem}" item
/return
