#Event StartCast	"[#1#(msg)]#*#BEGINCASTING#*#"

Sub Event_StartCast(string line, string tank) 
	/if (!${Defined[RotSpell]}) /declare RotSpell string outer "Complete Heal"
	
	/if (${SpawnCount[pc ${tank}]} < 1) {
		/g >>>> ${tank} is not in my zone!! <<<<
	}
	
	/target pc ${tank}	
	/delay 3s ${Target.Name.Equal[${tank}]}
	/if (!${Target.Name.Equal[${tank}]}) /return
	
	/if (!${Me.Book[${RotSpell}]}) {
		/g >>>> I do not have the spell ${RotSpell}! <<<<
	} else /if (${Target.Distance} > ${Spell[${RotSpell}].Range}) {
		/g >>>> ${tank} IS OUT OF RANGE!! <<<<
	} else /if (${Me.CurrentMana} < ${Spell[${RotSpell}].Mana}) {
		/g >>>> NOT ENOUGH MANA TO CAST ${RotSpell} <<<<
	} 
	
	/g Casting ${RotSpell} ON ${tank}
	/call cast ${RotSpell} gem1 3s
	/doevents flush StartCast
	
/return

Sub BeginCHRotation
	/if (!${Defined[BHServer]}) /declare BHServer string outer "reno.is-a-geek.org"
	/if (!${Defined[BHPort]}) /declare BHPort int outer 21
	
	/bhcmd connect ${BHServer} ${BHPort}
	
/return

