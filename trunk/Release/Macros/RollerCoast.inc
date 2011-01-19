| # @file - RollerCoast.inc
| # @author - FZLives
| # @date - 10-25-2010
| #
| # Version 1.1
| #
| # This include allows guild loot rolling to be completely automated, reporting the top rolls after a set duration.
| # All roll sets must be in the format "10x LootLink"  Any length, value or order of random can be used as long as it starts with "10" (10 will not work).
| # RollerCoast defaults to the OFF state, to enable the include send "RollerCoast" into BoxChat general channel with the character you wish to parse with.



#Event RollerCoastOn "<#1#> RollerCoast"
#Event RollsStart "#*# guild, '10#1# #2#"
| #Event RollsStart "#*# says, '10#1# #2#"
#Event RollerRecord "**A magic die is rolled by #1#."
#Event RollRecord "**It could have been any number from 0 to 10#1#, but this time it turned up a #2#."
#Event LootTimerSet "<#1#> loot timer #2#"
#Event ForceAward "<#1#> force 10#2#"
#Event RCHelp "<#1#> RCHelp"



| ##
| # Primary /random parser Sub.  This should be called by any macro that included this file before the main loop.
| ##
Sub RollerCoast

	|- This variable determines if RollerCoast is active or not.
	/if (!${Defined[bRollerCoastOn]}) /declare bRollerCoastOn bool outer FALSE

	|- Change this variable value to change the default roll time (enter in 10ths of a second).
	/if (!${Defined[iDefaultRollTimerValue]}) /declare iDefaultRollTimerValue int outer 1200
	
	|- Holder variable for changing the timer without breaking rolls that are in progress.
	/if (!${Defined[iNewDefaultRollTimerValue]}) /declare iNewDefaultRollTimerValue int outer 1200
	
	|- In Progress counter, > 0 if tracking rolls
	/if (!${Defined[iRollsInProgress]}) /declare iRollsInProgress int outer 0
	
	|- All Clear boolean, allows bypass of vector erasure during idle process
	/if (!${Defined[bAllClear]}) /declare bAllClear bool outer FALSE
	
	|- In case it's a tie
	/if (!${Defined[bItsATie]}) /declare bItsATie bool outer FALSE
	
	|- Counter variables to hold the number of active loots being rolled on
	/if (!${Defined[iRollsCounterMinimum]}) /declare iRollsCounterMinimum int outer 0
	/if (!${Defined[iRollsCounterMaximum]}) /declare iRollsCounterMaximum int outer -1
	
	|- If no rolls are in progress and the timer has been changed, update the timer.
	/if (${iRollsInProgress} == 0 && !(${iDefaultRollTimerValue} == ${iNewDefaultRollTimerValue})) {
			/varset iDefaultRollTimerValue ${iNewDefaultRollTimerValue}
			/echo Loot timer changed to ${Math.Calc[${iDefaultRollTimerValue}/10]} seconds.
		}
	
	|- If rolls are in progress, check to see if their counters have expired.  If expired, call the roll result sub.
	/if (${iRollsInProgress} > 0) {
		/if (!${Defined[iCounter]}) /declare iCounter int local 0
		/for iCounter ${iRollsCounterMinimum} to ${iRollsCounterMaximum}
			/if (!${tRollTimer${Vector[RollNumbers].At[${iCounter}]}}) {
				/call RollResult ${Vector[RollNumbers].At[${iCounter}]}
			}
		/next iCounter
	}
	
	|- If no more rolls are in progress, clears all Vectors, HashMaps and counters to be ready for the next rolls.
	/if (${iRollsInProgress} == 0 && ${bAllClear} == FALSE ) {
		/declare iClearAll int local 1
		/for iClearAll 1 to 50
			/data ${Vector[${iClearAll}].Erase}
			/data ${Vector[${iClearAll}Names].Erase}
			/data ${HashMap[NamesAndRolls${iClearAll}].Erase}
			/data ${HashMap[LootNames].Erase}
		/next iClearAll
		/data ${Vector[InputRolls].Erase}
		/data ${Vector[InputRollerNames].Erase}
		/data ${Vector[RollNumbers].Erase}
		/varset iRollsCounterMinimum 0
		/varset iRollsCounterMaximum -1
		/echo ALL /RANDOM DATA IS CLEARED.
		/varset bAllClear TRUE
		/if (${bRollerCoastOn} == FALSE) {
			/echo RollerCoast is currently disabled.
			/echo Type "/bc RollerCoast" to enable.
			/echo Type "/bc RCHelp" for command list.
		}
	}
	
/return



| ##
| # Toggles boolean On/Off variable.
| ##
Sub Event_RollerCoastOn(line,sSender)
	/if (${Me.Name.NotEqual[${sSender}]}) /return
	/if (${bRollerCoastOn}) {
		/varset bRollerCoastOn FALSE
		/echo RollerCoast DISABLED.
	} else {
		/varset bRollerCoastOn TRUE
		/echo RollerCoast ENABLED.
	}
/return



| ##
| # Creates timers, indexes counters and stores names for rolls when someone in the raid issues a roll number.
| ##
Sub Event_RollsStart(line,iLootRollNumber,sLootItemName)
	|- If On/Off switch is off, don't parse anything.
	/if (${bRollerCoastOn} == FALSE) /return
	
	|- Random has a limit of 32000, making RollerCoast's limit 10999.  If the value is higher than 10999, reject the roll since no one can roll on it.
	/if (10${iLootRollNumber} > 10999) {
		/gu ****The roll specified is too large for anyone to roll!****
		/gu ****${sLootItemName} - 10${iLootRollNumber} IS SKIPPED!****
		/return
	}
	
	|- Increments the roll in progress variable so the macro will parse and store rolls.
	/varcalc iRollsInProgress ${iRollsInProgress} + 1
	
	|- Increments the RollCounterMaximum value for in progress roll processing.
	/varcalc iRollsCounterMaximum ${iRollsCounterMaximum} + 1
	
	|- Checks to see if the Roll Timer for the given roll is set, if not, sets it for the default timer value.
	/if (!${Defined[tRollTimer${iLootRollNumber}]}) /declare tRollTimer${iLootRollNumber} timer outer 0
	/varset tRollTimer${iLootRollNumber} ${iDefaultRollTimerValue}
	
	|- Reports roll is set and roll number.
	/echo Roll counter set for ${sLootItemName} - 10${iLootRollNumber}
	
	|- Sets All Clear to FALSE to allow data cleanup after all rolls are completed.
	/varset bAllClear FALSE
	
	|- Creates hashmap for recalling item names later using the roll number as an index.
	/data ${HashMap[LootNames].Insert[${iLootRollNumber},${sLootItemName}]}
	
	|- Stores the roll number into a vector for indexing recall.
	/data ${Vector[RollNumbers].Insert[${iLootRollNumber}]}
	
/return



| ##
| # Sorts and displays the roll results.
| ##
Sub RollResult(iRollNumber)
	
	|- Sorts the results of the requested loot item, compares that data to the HashMap then re-stores to two new vectors for display.
	/data ${Vector[${iRollNumber}].Sort[desc]}
	/declare iWinCounter int local 0
	/declare iWinCounterEnd int local 0
	/varcalc iWinCounterEnd ${Vector[${iRollNumber}].Size}-1
	/declare iWinNameCounter int local 0
	/declare iWinNameCounterEnd int local
	/varcalc iWinNameCounterEnd ${Vector[${iRollNumber}Names].Size}-1
	/for iWinCounter 0 to ${iWinCounterEnd}
		/for iWinNameCounter 0 to ${iWinNameCounterEnd}
			/if (${Vector[${iRollNumber}].At[${iWinCounter}]} == ${HashMap[NamesAndRolls${iRollNumber}].Find[${Vector[${iRollNumber}Names].At[${iWinNameCounter}]}]}) {
				/data ${HashMap[NamesAndRolls${iRollNumber}].Remove[${Vector[${iRollNumber}Names].At[${iWinNameCounter}]}]}
				/data ${Vector[Results].Insert[${Vector[${iRollNumber}Names].At[${iWinNameCounter}]}]}
			}
		/next iWinNameCounter
	/next iWinCounter
	
	|- Check for a first-place tie.
	/if (${Vector[${iRollNumber}].At[0]} == ${Vector[${iRollNumber}].At[1]}) {
		/varset bItsATie TRUE
	}
	
	|- If it's not a tie, announce the results.
	/if (${bItsATie} == FALSE) {
		/gu *****RESULTS FOR ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
|		/say *****RESULTS FOR ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
		/declare iWinners int local 0
		/declare iWinnersEnd int local 4
		/if (${Vector[Results].Size} < 5) /varcalc iWinnersEnd ${Vector[Results].Size}-1
		/for iWinners 0 to ${iWinnersEnd}
			/gu - ${Vector[${iRollNumber}].At[${iWinners}]} - ${Vector[Results].At[${iWinners}]}
|			/say - ${Vector[${iRollNumber}].At[${iWinners}]} - ${Vector[Results].At[${iWinners}]}
		/next iWinners
	}
	
	|- If it's a tie (for first place), announce the tie.  No rolls will result in a tie, if there were no rolls, announce that instead.
	/if (${bItsATie} == TRUE) {
		/if (${Vector[${iRollNumber}].At[0]} == NULL) {
			/gu ****There were NO ROLLS for ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
|			/say ****There were NO ROLLS for ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
		} else {
			/gu *****RESULTS FOR ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
			/gu WE HAVE A TIE!  ${Vector[Results].At[0]} and ${Vector[Results].At[1]} have TIED with a roll of ${Vector[${iRollNumber}].At[0]}.
|			/say *****RESULTS FOR ${HashMap[LootNames].Find[${iRollNumber}]} - 10${iRollNumber}*****
|			/say WE HAVE A TIE!  ${Vector[Results].At[0]} and ${Vector[Results].At[1]} have TIED with a roll of ${Vector[${iRollNumber}].At[0]}.
			/declare iWinners int local 2
			/declare iWinnersEnd int local 4
			/if (${Vector[Results].Size} < 5) /varcalc iWinnersEnd ${Vector[Results].Size}-1
			/gu The remaining rolls follow:
|			/say The remaining rolls follow:
			/for iWinners 2 to ${iWinnersEnd}
				/gu - ${Vector[${iRollNumber}].At[${iWinners}]} - ${Vector[Results].At[${iWinners}]}
|				/say - ${Vector[${iRollNumber}].At[${iWinners}]} - ${Vector[Results].At[${iWinners}]}
			/next iWinners
		}
	}	

	|- Decrements the Rolls In Progress counter.
	/varcalc iRollsInProgress ${iRollsInProgress}-1
	
	|- Increments the Rolls Counter so the correct results are given.
	/varcalc iRollsCounterMinimum ${iRollsCounterMinimum}+1
	
	|- Ensure that the tie flag is reset.
	/varset bItsATie FALSE
	
	|- Erase the Results vector for the next results display.
	/data ${Vector[Results].Erase}
/return



| ##
| # Stores the roller names.
| ##
Sub Event_RollerRecord(line,sLootRoller)
	/if (${iRollsInProgress} == 0) /return
	
	|- Stores the roller's names for all loot rolls in the order they were received.
	/data ${Vector[InputRollerNames].Insert[${sLootRoller}]}
/return



| ##
| # Stores the roll values.
| ##
Sub Event_RollRecord(line,iLootRollNumber,iLootRoll)
	/if (${iRollsInProgress} == 0) /return
	
	|- InputRoll vector stores the actual rolls made by players, all loot items will have their rolls in this vector in the same order as their names in InputRollerNames.
	/data ${Vector[InputRolls].Insert[${iLootRoll}]}
	
	|- This stores the rolls into a vector for each specific loot item for sorting and scoring.
	/data ${Vector[${iLootRollNumber}].Insert[${iLootRoll}]}
	
	|- This stores the roller's names into a vector for each specific loot item for HashMap retrieval.
	/if (!${Defined[NameHashCounter]}) /declare NameHashCounter int local 0
	/varcalc NameHashCounter ${Vector[InputRolls].Size}-1
	/data ${Vector[${iLootRollNumber}Names].Insert[${Vector[InputRollerNames].At[${NameHashCounter}]}]}
	
	|- This creates the HashMap for each loot item that contains roller and roll linked data.
	/data ${HashMap[NamesAndRolls${iLootRollNumber}].Insert[${Vector[InputRollerNames].At[${NameHashCounter}]},${Vector[InputRolls].At[${NameHashCounter}]}]}
/return



| ##
| # Changes the default roll timer. Give input in SECONDS.
| ##
Sub Event_LootTimerSet(line,sSender,iTimerValue)
	/if (${Me.Name.NotEqual[${sSender}]}) /return
	/varcalc iNewDefaultRollTimerValue ${iTimerValue}*10
/return



| ##
| # Immediately displays the current roll results for the requested roll number.
| ##
Sub Event_ForceAward(line,sSender,iLootRollNumber)
	/if (${Me.Name.NotEqual[${sSender}]}) /return
	/varset tRollTimer${iLootRollNumber} 0
/return



| ##
| # Prints the command list to MQ2 window.
| ##
Sub Event_RCHelp(line,sSender)
	/if (${Me.Name.NotEqual[${sSender}]}) /return
	/echo RollerCoast.inc command list:
	/echo /bc RollerCoast - Toggles roll parsing on/off
	/echo /bc loot timer xxx - Set timer delay in seconds
	/echo /bc force 10x - Force the rolls to be completed immediately
	/echo /bc RCHelp - Displays this command list
/return