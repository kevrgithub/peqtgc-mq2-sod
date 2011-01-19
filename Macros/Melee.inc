#Event   Enrageon         "#*#|${Target.CleanName}| has become ENRAGED#*#" 
#Event   Enrageoff        "#*#|${Target.CleanName}| is no longer enraged#*#" 
#Event   ItHasDied         "#*# has been slain by #*#" 
#Event   ItHasDied         "#*#You have slain #*#" 

| ##
| #
| #
| ##
Sub MeleeSetup
	/echo Loading Melee system....
	/if (!${Defined[AmMelee]}) /declare AmMelee outer TRUE
	/if (!${Defined[DoTaunt]}) /declare DoTaunt outer FALSE
	/if (!${Defined[DoAssist]}) /declare DoAssist outer FALSE
	/if (!${Defined[AssistPct]}) /declare AssistPct int outer 0

	/if (!${Defined[MeleeINIFile]}) /declare MeleeINIFile string outer "Bots_${Me.CleanName}.ini"
	
	/if (!${Ini[${MeleeINIFile},MeleeOptions,AutoAssistMe].Length}) /call BuildMeleeINI
	
	/if (${Ini[${MeleeINIFile},MeleeOptions,AutoTaunt].Length}) /varset DoTaunt ${Ini[${MeleeINIFile},MeleeOptions,AutoTaunt]}
	/if (${Ini[${MeleeINIFile},MeleeOptions,AssistAtPct].Length}) /varset AssistPct ${Ini[${MeleeINIFile},MeleeOptions,AssistAtPct]}
	/if (${Ini[${MeleeINIFile},MeleeOptions,AutoAssistMe].Length}) /varset DoAssist ${Ini[${MeleeINIFile},MeleeOptions,AutoAssistMe]}
	
	/echo Melee system has been loaded.
/return

| ##
| #
| #
| ##
Sub BuildMeleeINI
	/echo Building Melee INI file...
	
	/ini "${MeleeINIFile}" MeleeOptions AutoAssistMe TRUE
	/ini "${MeleeINIFile}" MeleeOptions AssistAtPct 98
	/ini "${MeleeINIFile}" MeleeOptions AutoTaunt TRUE
	
	/if (!${Defined[INIFileWasBuilt]}) /declare INIFileWasBuilt outer TRUE
/return

| ##
| #
| #
| ##
Sub CheckMeleeStatus
	/if (!${Defined[Attacking]}) /declare Attacking outer FALSE
	/if (!${Defined[AttackTarget]}) /declare AttackTarget int outer 0
	
	/if (${DoTaunt} && ${Me.Combat} && (${Target.Type.Equal[NPC]} || ${Target.Type.Equal[Pet]})) {
		/if (${Me.AbilityReady[Taunt]} && ${Target.Distance} < 30) /doability "Taunt"
		/if (${Me.Class.Name.Equal[Warrior]} && ${Target.Distance} < 100) {
			/if (!${Defined[InciteTimer]}) /declare InciteTimer timer outer 0
			
			/if (${InciteTimer} == 0) {
				/disc Bellow of the Mastruq
				/varset InciteTimer 410
			}
		}
	}
	
	/if (${DoAssist} && !${Attacking} && ${Target.PctHPs} < ${AssistPct} && ${Me.Combat}) {
		/varset Attacking TRUE
		/varset AttackTarget ${Target.ID}
		/bca Combat ${Target.Name} 10
	}
	
	/if (${Me.Combat} && ${IsEnraged} && !${Stick.Behind} && ${Attacking}) {
		/attack off
	}
/return


| ##
| #
| #
| ##
Sub Event_Enrageon
	/if (!${Defined[IsEnraged]}) /declare IsEnraged outer FALSE
	/if (${Me.Combat} && !${Stick.Behind}) {
		/attack off
		/varset IsEnraged TRUE
	}

/return

| ##
| #
| #
| ##
Sub Event_Enrageoff
	/if (!${Defined[IsEnraged]}) /declare IsEnraged outer FALSE
	
	/if (${IsEnraged} && ${Target.ID} == ${AttackTarget} && ${AmMelee}) {
		/attack on	
		/varset IsEnraged FALSE
	}
	
/return

| ##
| #
| #
| ##
Sub Event_ItHasDied(string Line)
	/if (!${Defined[IsEnraged]}) /declare IsEnraged outer FALSE
	
	/varset IsEnraged FALSE

/return
