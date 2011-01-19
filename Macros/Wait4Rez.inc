| # @file - Wait4rez.inc 
| # @author - Unknown
| # @date - v2.1
| #
| # Handles autoaccept of rez and loots all corpses near the player.

| ##
| # Called after a player dies to accept rez and autoloot corpse.
| ##
Sub Wait4Rez
	/echo Awaiting rez: auto-accepting, and looting my corpse
	/consent group
	/delay 25
	
:rezmeffs

	/delay 600m ${Window[ConfirmationDialogBox].Open}
	/if (!${Window[ConfirmationDialogBox].Open}) /goto :rezmeffs
		
	/nomodkey /notify ConfirmationDialogBox Yes_Button leftmouseup
	
:zoneffs
	/delay 5m ${Spawn[${Me}'s].ID}
	/if (${Spawn[${Me}'s].ID}) /target ${Me}'s
		
	/delay 5s ${Target.ID}==${Spawn[${Me}'s].ID}	
	
	/if (${Target.CleanName.NotEqual[${Me}'s corpse]}) /goto :zoneffs
		
:corpsepull

	/if (${Target.Distance}>100 || !${Spawn[${Me}'s].ID}) /return
		
	/if (${Target.Distance}>20) /corpse
		
	/delay 5s ${Target.Distance}<20
	/loot
	
	/delay 5s ${Me.State.Equal[BIND]}
	/if (${Me.State.NotEqual[BIND]}) /goto :corpsepull
		
	/if (!${Defined[myitems]}) /declare myitems int local
	:LootLag
	/varset myitems ${Corpse.Items}
	/delay 3s ${myitems}!=${Corpse.Items}
	/if (${myitems}!=${Corpse.Items}) /goto :LootLag
	
	/if (!${Defined[i]}) /declare i int local
	/for i 1 to ${myitems}
		:lootagain
		/itemnotify loot${i} rightmouseup
		/delay 10s !${Corpse.Item[${i}].ID}
		/if (${Corpse.Item[${i}].ID}) /goto :lootagain
	/next i
	
	/nomodkey /notify LootWnd DoneButton leftmouseup
	
	/if (${Spawn[${Me}'s].ID}) /goto :zoneffs
/return