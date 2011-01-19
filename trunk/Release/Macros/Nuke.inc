| # @file - Nuke.inc
| # @author - Reno
| # @date - 8/31/2010
| #
| # This will handle all offensive nuking abilities for classes that use such abilities. WORK IN PROGRESS!
#Event NonMeleeDamage	 	"#*# was hit by non-melee for #1# points of damage."
#Event FullResist			"#*# resisted your |${CurrentNuke}| spell#*#"
#Event IZoned				"You have entered#*#"

| ##
| # 
| #
| ##
Sub NukeSetup
	/echo Loading nuke system.....
	
	/if (!${Defined[NukeINIFile]}) /declare NukeINIFile string outer "Bots_${Me.CleanName}.ini"	
	
	/if (!${Defined[PrimaryNuke]}) /declare PrimaryNuke string outer
	/if (!${Defined[SecondNuke]}) /declare SecondNuke string outer
	/if (!${Defined[LowResistNuke]}) /declare LowResistNuke string outer
	/if (!${Defined[RainNuke]}) /declare RainNuke string outer
	/if (!${Defined[AgroSpell]}) /declare AgroSpell string outer 
	/if (!${Defined[NukeAssistPct]}) /declare NukeAssistPct int outer
	/if (!${Defined[MinNukeMana]}) /declare MinNukeMana int outer
	/if (!${Defined[NukeGem]}) /declare NukeGem string outer 
	/if (!${Defined[UseNukes]}) /declare UseNukes bool outer 

	|- Insert the call to get from the INI here
	/if (!${Ini[${NukeINIFile},NukesOptions,PrimaryNuke].Length}) /call BuildNukeINI
	
	/if (${Ini[${NukeINIFile},NukesOptions,PrimaryNuke].Length}) /varset PrimaryNuke ${Ini[${NukeINIFile},NukesOptions,PrimaryNuke]}
	/if (${Ini[${NukeINIFile},NukesOptions,SecondaryNuke].Length}) /varset SecondNuke ${Ini[${NukeINIFile},NukesOptions,SecondaryNuke]}
	/if (${Ini[${NukeINIFile},NukesOptions,LowResistNuke].Length}) /varset LowResistNuke ${Ini[${NukeINIFile},NukesOptions,LowResistNuke]}	
	/if (${Ini[${NukeINIFile},NukesOptions,RainSpell].Length}) /varset RainNuke ${Ini[${NukeINIFile},NukesOptions,RainRain]}
	/if (${Ini[${NukeINIFile},NukesOptions,LoseAgroSpell].Length}) /varset AgroSpell ${Ini[${NukeINIFile},NukesOptions,LoseAgroSpell]}
	/if (${Ini[${NukeINIFile},NukesOptions,NukeAssistPct].Length}) /varset NukeAssistPct ${Ini[${NukeINIFile},NukesOptions,NukeAssistPct]}
	/if (${Ini[${NukeINIFile},NukesOptions,MinPctManaToNuke].Length}) /varset MinNukeMana ${Ini[${NukeINIFile},NukesOptions,MinPctManaToNuke]}
	/if (${Ini[${NukeINIFile},NukesOptions,GemToUse].Length}) /varset NukeGem ${Ini[${NukeINIFile},NukesOptions,GemToUse]}
	/if (${Ini[${NukeINIFile},NukesOptions,UseNukes].Length}) /varset UseNukes ${Ini[${NukeINIFile},NukesOptions,UseNukes]}
	
	/if (!${Defined[CurrentNuke]}) /declare CurrentNuke string outer
	/if (!${Defined[TotalDamage]}) /declare TotalDamage int outer 0
	/if (!${Defined[ResistCount]}) /declare ResistCount int outer 0 
	
	/varset CurrentNuke ${PrimaryNuke}
	
	/echo Nuke system has been loaded.
/return

| ##
| #
| #
| ##
Sub BuildNukeINI
	/echo Building Nuke INI...
	
	/ini "${NukeINIFile}" NukesOptions PrimaryNuke YourSpellHere
	/ini "${NukeINIFile}" NukesOptions SecondaryNuke YourSpellHere
	/ini "${NukeINIFile}" NukesOptions LowResistNuke YourSpellHere
	/ini "${NukeINIFile}" NukesOptions RainSpell YourSpellHere
	
	/ini "${NukeINIFile}" NukesOptions LoseAgroSpell LeaveThisForNoSpell
	/ini "${NukeINIFile}" NukesOptions NukeAssistPct 90
	/ini "${NukeINIFile}" NukesOptions MinPctManaToNuke 30
	
	/ini "${NukeINIFile}" NukesOptions GemToUse 1
	/ini "${NukeINIFile}" NukesOptions UseNukes FALSE
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE
/return

| ##
| #
| #
| ##
Sub CheckNuke
	/if (!${Defined[Attacking]}) /declare Attacking outer FALSE
	
	/if (!${Attacking} || !${UseNukes}) /return	
	/if (${SpawnCount[id ${AttackTarget}]} < 1) /return

	/if (${Target.ID} != ${AttackTarget}) /call GetAttackTarget
		
	/if (${Target.PctHPs} < ${NukeAssistPct} && ${Attacking}) {	
		/call DoINuke
		/call UseVeeshanClicky		
	}

/return

| ##
| #
| #
| ##
Sub Event_IZoned
	/if (${Defined[ResistCount]}) /varset ResistCount 0
	/if (${Defined[CurrentNuke]}) /varset CurrentNuke ${MainNuke}
	/if (${Defined[TotalDamage]}) /varset TotalDamage 0
/return

| ##
| #
| #
| ##
Sub Event_SpellFullResisted
	/varcalc ResistCount + 1
	
	/if (${ResistCount} > 2) {
		| move to a lower resist nuke
		/varset ResistCount 0
		/varset CurrentNuke ${LowResistNuke}
	}
/return

| ##
| #
| #
| ##
Sub Event_NonMeleeDamage(string Line, int Damage)
	/if (!${Defined[TotalDamage]}) /declare TotalDamage int outer 0
	/varcalc TotalDamage ${TotalDamage} + ${Damage}
/return

| ##
| #
| #
| ##
Sub DoINuke
	|- Check to see if spell is mem'd and if not ready to cast, move to the next nuke
	/if (!${Me.Gem[${CurrentNuke}]}) {
	
	}
	
	/if ((${Target.Distance} < ${Spell[${CurrentNuke}].Range}) && (${Target.PctHPs} < ${NukeAssistPct}) && !${Me.Moving} && (${Me.PctMana} > ${MinNukeMana})) {
		/call Cast "${CurrentNuke}" gem${NukeGem} 3s
		/doevents SpellFullResisted
		/doevents NonMeleeDamage
	}
	
	/if (${TotalDamage} > 10000 && ${AmIntCaster}) /call DecreaseAgro
	
	/if (${CurrentNuke.Equal[${PrimaryNuke}]}) {
		/varset CurrentNuke ${SecondNuke}
	} else /if (${CurrentNuke.Equal[${SecondNuke}]}) {
		/varset CurrentNuke ${PrimaryNuke}
	}
/return

| ##
| #
| #
| ##
Sub DecreaseAgro
	/if (${FindItemCount[Maelin's Leggings of Lore]} > 0) {
		/call cast "Maelin's Leggings of Lore" item
	} else /if (${AgroSpell.NotEqual[LeaveThisForNoSpell]}) {
			/call cast ${AgroSpell} gem${NukeGem} 3s
	} else {
		/declare i int local 0
		/for i 0 to 5
			/delay 1s
			/call GetAttackTarget
			/if (${Target.ID} != ${AttackTarget}) /return
		/next i
	}
	
	/varset TotalDamage 0
/return

| ##
| #
| ##
Sub UseVeeshanClicky
	/if (${Me.Class.Name.Equal[Wizard]} && ${FindItemCount[Aged Shissar Elementalist's Staff]} > 0) {
		/call DoClickyNuke "Aged Shissar Elementalist's Staff" 2100
	} else /if (${Me.Class.Name.Equal[Magician]} && ${FindItemCount[Aged Sarnak Channeler Staff]} > 0) {
		/call DoClickyNuke "Aged Sarnak Channeler Staff" 1200
	} else /if (${Me.Class.Name.Equal[Necromancer]} && ${FindItemCount[Aged Shissar Deathspeaker Staff]} > 0) {
		/call DoClickyNuke "Aged Shissar Deathspeaker Staff" 2100
	}
/return

| ##
| #
| ##
Sub DoClickyNuke(string ToClick, int RecastTime)
	/if (!${Me.Moving} && ${Attacking} && ${UseNukes} && ${Target.Type.Equal[NPC]} && ${FindItemCount[${ToClick}]} > 0 && ${Me.PctMana} <= 95 && ${Target.Distance} <= ${FindItem[${ToClick}].Spell.Range} && (!${Defined[t${FindItem[${ToClick}].Spell.ID}]} || ${t${FindItem[${ToClick}].Spell.ID}} == 0)) {
		/call Cast ${ToClick} item
		/delay 2s
		/if ( !${Defined[t${FindItem[${ToClick}].Spell.ID}]} ) {
			/declare t${FindItem[${ToClick}].Spell.ID}    timer    outer
		}

		| The re-use timer on the Aged Shissar Elementalist's Staff is 3:30, which is 2100 deciseconds.
		/varset t${FindItem[${ToClick}].Spell.ID} ${RecastTime}
	}
/return
