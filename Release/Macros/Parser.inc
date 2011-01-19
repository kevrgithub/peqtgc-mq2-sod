#event Outgoing		"You slash #*# for #1# points of damage."
#event Outgoing		"You crush #*# for #1# points of damage."
#event Outgoing		"You hit #*# for #1# points of damage."
#event Outgoing		"You kick #*# for #1# points of damage."
| #event Spells  		"#*# was hit by non-melee for #1# points of damage."
#Event Incoming     "#*# hits YOU for #1#"
#Event EndParse		"<#1#> End"

Sub Main
	/call ParseSetup
	
	:Main_Loop
		/doevents
	
		
	/goto :Main_Loop
/return

Sub ParseSetup
	|- Total time for the fight
	/if (!${Defined[TotalTime]}) /declare TotalTime int outer 0
	
	|- List to be used for dealt hits
	/if (!${Defined[HitsOut]}) /declare HitsOut string outer "HitsOut"
	
	|- List to be used for dealt hits
	/if (!${Defined[HitsIn]}) /declare HitsIn string outer "HitsIn"	
	
	|- Timer to track duration
	/if (!${Defined{StartTimer]}) /declare StartTimer timer outer 0
	
	|- Are we currently parsing
	/if (!${Defined[Parsing]}) /declare Parsing bool outer FALSE	
/return

Sub Event_Outgoing(string Line, int Damage) 
	/if (!${Parsing}) {
		/varset Parsing TRUE
		/varset StartTimer 1000000
	}
	/data ${Vector[${HitsOut}].Insert[${Damage}]}
/return

Sub Event_Incoming(string Line, int Damage)
 	/if (!${Parsing}) {
		/varset Parsing TRUE
		/varset StartTimer 1000000
	}
	/data ${Vector[${HitsIn}].Insert[${Damage}]}
/return

Sub Event_EndParse(string Line, string Sender)
	/if (${Me.Name.NotEqual[${Sender}]}) /return
	/varset Parsing FALSE
	/varcalc TotalTime 1000000 - ${StartTimer}
	/varcalc TotalTime ${TotalTime} / 10
	
	/declare damageOut int local 0
	/call FindTotal "${HitsOut}"
	/varset damageOut ${Macro.Return}
	
	/guildsay >>> PARSE RESULTS <<<
	/guildsay Fight Time: ${TotalTime}
	/guildsay   ** Incoming Stats **
	/call EchoResults "${HitsIn}"
	/guildsay   ********************
	/guildsay   ** Outgoing Stats **
	/call EchoResults "${HitsOut}"
	/guildsay **********************
	/guildsay >>> END RESULTS <<<
	
	/call ResetParser
/return

Sub EchoResults(string ArrayName)
	/call FindTotal "${ArrayName}"	
	/declare damage int local ${Macro.Return}
	/call FindMinimum "${ArrayName}"
	/declare min int local ${Macro.Return}
	/call FindMaximum "${ArrayName}"
	/declare max int local ${Macro.Return}
	/call FindAverage "${ArrayName}"
	/declare avg int local ${Macro.Return}
	/call FindDPS "${ArrayName}" ${TotalTime}
	/declare dps ${Macro.Return}
	
	/guildsay Total: ${damage}
	/guildsay Minimum: ${min}
	/guildsay Maximum: ${max}
	/guildsay Average: ${avg}
	/guildsay DPS: ${dps}
/return

Sub FindMinimum(string ArrayName)
	/declare minimum int local 0
	/declare i int local 0
	/declare end int local ${Vector[${ArrayName}].Size}
	/varcalc end ${end} - 1
	/for i 1 to ${end}
		/if (${Vector[${ArrayName}].At[${i}]} < ${minimum} || ${minimum} == 0) {
			/varset minimum ${Vector[${ArrayName}].At[${i}]}
		}
	/next i
	
/return ${minimum}

Sub FindMaximum(string ArrayName)
	/declare maximum int local 0
	/declare i int local 0
	/declare end int local ${Vector[${ArrayName}].Size}
	/varcalc end ${end} - 1
	/for i 1 to ${end}
		/if (${Vector[${ArrayName}].At[${i}]} > ${maximum}) {
			/varset maximum ${Vector[${ArrayName}].At[${i}]}
		}
	/next i
	
/return ${maximum}

Sub FindAverage(string ArrayName)
	/declare total int local 0
	/declare average int local 0
	/declare i int local 0
	/declare end int local ${Vector[${ArrayName}].Size}
	/varcalc end ${end} - 1
	/for i 1 to ${end}
		/varcalc total ${total} + ${Vector[${ArrayName}].At[${i}]}
	/next i
	
	/varcalc average ${total} / ${Vector[${ArrayName}].Size}
	
/return ${average}

Sub FindTotal(string ArrayName) 
	/declare total int local 0
	/declare i int local 0
	/declare end int local ${Vector[${ArrayName}].Size}
	/varcalc end ${end} - 1
	/for i 1 to ${end}
		/varcalc total ${total} + ${Vector[${ArrayName}].At[${i}]}
	/next i
	
/return ${total}

Sub FindDPS(string ArrayName, int TotalTime)
	/declare total int local 0
	/declare DPS
	/call FindTotal ${ArrayName}
	/varset total ${Macro.Return}
	
	/varcalc DPS ${total} / ${TotalTime}
	
/return ${DPS}

Sub ResetParser
	/varset TotalTime 0
	/varset StartTimer 0
/return
