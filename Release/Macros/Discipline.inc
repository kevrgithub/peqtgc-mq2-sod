| +=========================================================================================+
| | Disciplines.inc                                                                             |
| |                                                                                            |
| | An include file of functions that help use and track Disciplines (Combat Abilities).    |
| |                                                                                            |
| |                                                                                            |
| | Examples of Use:                                                                        |
| |                                                                                            |
| | The UseDiscipline() Function:                                                            |
| | -----------------------------                                                            |
| |                                                                                            |
| | /call UseDiscipline "Healing Will"                                                    |
| | Attempts to start the Healing Will Discipline.                                    |
| |                                                                                            |
| | If the character is already running another discipline, is stunned, or is casting,|
| | it will keep trying for six seconds before giving up and returning FALSE.            |
| | It will also log the time when the discipline was used so that the                |
| | OutputDiscTimers() and DisciplineReadyIn() functions can accurately indicate when    |
| | the "Healing Will" discipline, and others on the same timer, will be ready again.    |
| | It will do the same if it attempts to use the discipline, but receives the        |
| | "You can use this discipline in X minutes Y seconds.".                            |
| |                                                                                            |
| | /call UseDiscipline "Furious" TRUE                                                    |
| | Attempts to start the Furious discipline                                            |
| |                                                                                            |
| | The second (optional) parameter is set to TRUE, indicating that as long as the    |
| | character is not stunned or casting a spell, it should immediately stop any other    |
| | discipline that may already be running, and then start the new one (if the new    |
| | disc is ready to be started. Otherwise it leaves the old one running.)            |
| |                                                                                            |
| | /call UseDiscipline "${DiscNameVariable}"                                                |
| | Starts the discipline whose name is stored in the string "DiscNameVariable".        |
| | The Discipline name may contain the text "Discipline", or you may omit it.        |
| | (That is, you can pass either "Healing Will" or "Healing Will Discipline").        |
| |                                                                                            |
| |                                                                                            |
| | The CheckDisciplines() Function:                                                        |
| | --------------------------------                                                        |
| |                                                                                            |
| | /call CheckDisciplines                                                                |
| | Call this function for every loop of your macro's "Main()" Subroutine so that it    |
| | can monitor and log manual discipline usage, and announce when disciplines become    |
| | available.                                                                        |
| |                                                                                            |
| |                                                                                            |
| | The OutputDiscTimers() Function:                                                        |
| | --------------------------------                                                        |
| |                                                                                            |
| |     /call OutputDiscTimers                                                                |
| | Prints out a list of disciplines that the current character can use, which ones    |
| | are ready immediately, and how long before other disciplines can be used again.    |
| |                                                                                            |
| |                                                                                            |
| | The DisciplineReadyIn() Function:                                                        |
| | ---------------------------------                                                        |
| |                                                                                            |
| | /call DisciplineReadyIn "Healing Will"                                                |
| | Returns, in ${Macro.Return}, the number of seconds until the "Healing Will" disc    |
| | will be ready for use. If it's ready immediately, returns zero. If the timer is    |
| | unknown, returns -1.                                                                |
| +=========================================================================================+



| Set DisciplineInformationOutputChannel to whatever output method (channel) you'd like to see
| messages relating to Disciplines output on (without the leading slash). The following are some
| possibilities. Uncomment the ONE that you prefer, or make your own:

| #define    DisciplineInformationOutputChannel    echo
| #define    DisciplineInformationOutputChannel    bc
| #define    DisciplineInformationOutputChannel    i say
| #define    DisciplineInformationOutputChannel    gsay
| #define    DisciplineInformationOutputChannel    tell CharacterName
| The following would output Discipline information to the in-game chat channel number 7:
| #define    DisciplineInformationOutputChannel    7

#define    DisciplineInformationOutputChannel    ${AnnounceChannel}



| *******************************************************************************************
| * UseDiscipline()                                                                            *
| *                                                                                            *
| * Use the discipline named szDiscName.                                                     *
| *                                                                                            *
| * If another discipline is already running, and if bOverrideCurrentDisc is FALSE (or is    *
| *    not passed at all), this function will return FALSE and leave the already running        *
| *    discipline on. Otherwise, it'll stop the current discipline and start the new one.        *
| *                                                                                            *
| * ${Macro.Return} will be set to TRUE or FALSE                                            *
| *                                                                                            *
| *    TRUE indicates that szDiscName was started.                                                *
| *                                                                                            *
| *    FALSE indicates that the discipline is not ready yet, or did not start successfully.    *
| *******************************************************************************************

Sub UseDiscipline(string szDiscName, bool bOverrideCurrentDisc)

    /if ( !${Defined[CasterClasses]} ) {

        /declare CasterClasses            string    outer    Cleric/Druid/Shaman/Necromancer/Enchanter/Wizard/Magician
    }

    /if ( ${CasterClasses.Find[${Me.Class.Name}]} ) {

        | Only Melee and Hybrid classes can use disciplines, not casters.

        /return FALSE
    }

    /if ( ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[No Effect]} ) {

        | There is a discipline already running. Regardless of what happens below, we should make sure that a timer has been
        | set up for this discipline type. Call the CheckDisciplines() function to do just that.

        /call CheckDisciplines
    }

    | szDiscName could be "Healing Will" or it could be "Healing Will Discipline". We need to strip the " Discipline" if it's
    | present. Search for the text "Discipline, and if so, cut off the last 11 characters of the string (which is the number
    | of characters in the word "Discipline" plus the space that precedes it).

    /declare szShortDiscName    string    local    ${If[${szDiscName.Find[Discipline]},${szDiscName.Left[-11]},${szDiscName}]}

    /if ( ${Select[${Spell[${szShortDiscName} Discipline]},NULL]} == 1 ) {

        /docommand /DisciplineInformationOutputChannel Unknown Discipline: "${szShortDiscName} Discipline"

        /return FALSE

    } else /if ( ${Spell[${szShortDiscName} Discipline].Level} == 255 ) {

        /docommand /DisciplineInformationOutputChannel "${szShortDiscName} Discipline" is not a Discipline that a ${Me.Class.Name} can use.

        /return FALSE

    } else /if ( ${Spell[${szShortDiscName} Discipline].Level} > ${Me.Level} ) {

        /docommand /DisciplineInformationOutputChannel I cannot use "${szShortDiscName} Discipline" until I'm level ${Spell[${szShortDiscName} Discipline].Level}

        /return FALSE
    }

    /if ( ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[${szShortDiscName} Discipline]} ) {

        /docommand /DisciplineInformationOutputChannel ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text} is already running.

        /return TRUE
    }


    /delay 60 !${Me.Stunned} && !${Me.Casting.ID} && ( ( ${Defined[bOverrideCurrentDisc]} && ${bOverrideCurrentDisc} ) || ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[No Effect]} )

    /if ( ${Me.Stunned} ) {

        /docommand /DisciplineInformationOutputChannel I cannot use a discipline while I'm stunned.

        /return FALSE
    }

    /if ( ${Me.Casting.ID} ) {

        /docommand /DisciplineInformationOutputChannel I cannot use a discipline while I'm casting a spell.

        /return FALSE
    }
    
    /if ( ( !${Defined[bOverrideCurrentDisc]} || !${bOverrideCurrentDisc} ) && ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[No Effect]} ) {

        /docommand /DisciplineInformationOutputChannel Cannot start ${szShortDiscName} Discipline, as I am already using ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}.

        /return FALSE
    }

    /declare szPreviouslyRunningDiscipline    string    local    ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}


    | Determine which timer this discipline is on...
    /call ReturnDisciplineTimer    "${szShortDiscName}"

    /declare iDiscTimer        int        local    ${Int[${Macro.Return}]}

    | if we get to this point and there's still a discipline running, then the bOverrideCurrentDisc parameter is set to TRUE,
    | meaning that we should stop the current Disc, and start the new one.
    /if ( ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[No Effect]} ) {

        /if ( ${Defined[timerDiscType${iDiscTimer}]} && ${timerDiscType${iDiscTimer}} > 0 ) {

            | The discipline we're supposed to start is not ready to use yet, and there's a discipline that's already running.
            | Leave it running, and print a message...
            /docommand /DisciplineInformationOutputChannel ${szShortDiscName} Discipline (Timer #${iDiscTimer}) will not be ready for ${Int[${Math.Calc[${timerDiscType${iDiscTimer}}/600]}]} minutes, ${Int[${Math.Calc[(${timerDiscType${iDiscTimer}}%600)/10]}]} seconds. Leaving ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text} running.

            /return FALSE

        } else {

            /notify CombatAbilityWnd CAW_CombatEffectButton leftmouseup
        }
    }

    | Clear the message queue of DisciplineNotReady events
    /doevents DisciplineNotReady
    /doevents DisciplineNotReady
    /doevents DisciplineNotReady


    /if ( !${Defined[bUseDisciplineAttempt]} ) {

        /declare bUseDisciplineAttempt    bool    outer    TRUE

    } else {

        /varset bUseDisciplineAttempt    TRUE
    }

    /if ( !${Defined[iDisciplineReadyInSeconds]} ) {

        /declare iDisciplineReadyInSeconds    int    outer    -1

    } else {

        /varset iDisciplineReadyInSeconds    -1
    }


    /disc ${szShortDiscName}


    /delay 10 ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[${szShortDiscName} Discipline]}

    /doevents DisciplineNotReady

    /varset bUseDisciplineAttempt FALSE

    /if ( !${Defined[timerDiscType${iDiscTimer}]} ) {

        /declare timerDiscType${iDiscTimer}    timer    global    0
    }

    /if ( ${iDisciplineReadyInSeconds} != -1 ) {

        | This discipline was not ready to be started yet. iDisciplineReadyInSeconds is set to the number of seconds until it can be used.

        /docommand /DisciplineInformationOutputChannel ${szShortDiscName} Discipline will be ready in ${Int[${Math.Calc[${iDisciplineReadyInSeconds}/60]}]} minutes, ${Int[${Math.Calc[${iDisciplineReadyInSeconds}%60]}]} seconds.

        /varset timerDiscType${iDiscTimer}    ${iDisciplineReadyInSeconds}s

    } else /if ( ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[${szShortDiscName} Discipline]} ) {

        /varset timerDiscType${iDiscTimer}    ${Int[${Spell[${szShortDiscName} Discipline].RecastTime}]}s

        /if ( ${szPreviouslyRunningDiscipline.NotEqual[No Effect]} ) {

            /docommand /DisciplineInformationOutputChannel Stopped ${szPreviouslyRunningDiscipline}, and started ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}.

        } else {

            /docommand /DisciplineInformationOutputChannel ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text} Started.
        }
    }

    /if ( !${Defined[bDiscType${iDiscTimer}ReadyAnnounced]} ) {

        /declare bDiscType${iDiscTimer}ReadyAnnounced    bool    global    FALSE

    } else {

        /varset bDiscType${iDiscTimer}ReadyAnnounced    FALSE
    }

/return ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[${szShortDiscName} Discipline]}



| *******************************************************************************************
| * CheckDisciplines()                                                                        *
| *                                                                                            *
| * This function should be called frequently. Perhaps for every loop in your Main()        *
| * subroutine. Alternatively, set up a timer and timer event function to call this            *
| * function frequently.                                                                    *
| *                                                                                            *
| * Announces when disciplines become available Also looks to see if the user manually        *
| * started a discipline. If so, it logs it, so that it can inform the user when that same    *
| * discipline, or another on its timer, becomes available for use.                            *
| *******************************************************************************************

Sub CheckDisciplines

    /if ( !${Defined[CasterClasses]} ) {

        /declare CasterClasses            string    outer    Cleric/Druid/Shaman/Necromancer/Enchanter/Wizard/Magician
    }

    /if ( ${CasterClasses.Find[${Me.Class.Name}]} ) {

        | Only Melee and Hybrid classes can use disciplines, not casters.

        /return
    }

    /if ( !${Defined[szLastDisciplineSeen]} ) {

        /declare szLastDisciplineSeen    string    outer    NULL
    }

    /if ( ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[${szLastDisciplineSeen}]} && ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[No Effect]} ) {

        | A new discipline was started since the last time this function was called

        | Determine which timer this discipline is on...
        /call ReturnDisciplineTimer    "${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}"

        /declare iDiscTimer        int        local    ${Int[${Macro.Return}]}

        /if ( !${Defined[timerDiscType${iDiscTimer}]} ) {

            /declare timerDiscType${iDiscTimer}    timer    global    ${Int[${Spell[${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}].RecastTime}]}s

        } else {

            /varset timerDiscType${iDiscTimer}    ${Int[${Spell[${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}].RecastTime}]}s
        }

        /if ( !${Defined[bDiscType${iDiscTimer}ReadyAnnounced]} ) {

            /declare bDiscType${iDiscTimer}ReadyAnnounced    bool    global    FALSE

        } else {

            /varset bDiscType${iDiscTimer}ReadyAnnounced    FALSE
        }

        /varset szLastDisciplineSeen    ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}
    }


    /declare iLoopCounter    int    local

    /for iLoopCounter    1    to    10

        /if ( ${Defined[timerDiscType${iLoopCounter}]} && ${timerDiscType${iLoopCounter}} == 0 && !${bDiscType${iLoopCounter}ReadyAnnounced} ) {

            /call DetermineUsableDiscsForTimer    ${iLoopCounter}

            /if ( ${Select[${Macro.Return},NULL]} != 1 ) {

                /docommand /DisciplineInformationOutputChannel Type ${iLoopCounter} Disciplines are now ready. (${Macro.Return})
                /popup Type ${iLoopCounter} Disciplines are now ready. (${Macro.Return})
            }

            /varset bDiscType${iLoopCounter}ReadyAnnounced    TRUE
        }

    /next iLoopCounter

/return



| *******************************************************************************************
| * OutputDiscTimers()                                                                        *
| *                                                                                            *
| * Outputs a list of disciplines by timer type, including information on whether            *
| * disciplines on a certain timer are ready for use, and if not, when they are expected to    *
| * become ready.                                                                            *
| *******************************************************************************************

Sub OutputDiscTimers

    /if ( !${Defined[CasterClasses]} ) {

        /declare CasterClasses            string    outer    Cleric/Druid/Shaman/Necromancer/Enchanter/Wizard/Magician
    }

    /if ( ${CasterClasses.Find[${Me.Class.Name}]} ) {

        | Only Melee and Hybrid classes can use disciplines, not casters.

        /return
    }

    /declare iLoopCounter    int    local

    /for iLoopCounter    1    to    10

        /call DetermineUsableDiscsForTimer    ${iLoopCounter}

        /if ( ${Select[${Macro.Return},NULL]} != 1 ) {

            /if ( ${Defined[timerDiscType${iLoopCounter}]} ) {

                /docommand /DisciplineInformationOutputChannel    Timer ${iLoopCounter} Discs: Ready ${If[${timerDiscType${iLoopCounter}}==0,now,in ${Int[${Math.Calc[${timerDiscType${iLoopCounter}}/600]}]} minutes, ${Int[${Math.Calc[(${timerDiscType${iLoopCounter}}%600)/10]}]} seconds]}. (${Macro.Return})
    
            } else {
    
                /docommand /DisciplineInformationOutputChannel    Timer ${iLoopCounter} Discs: Availability currently unknown. (${Macro.Return})
            }
        }

    /next iLoopCounter

/return



| *******************************************************************************************
| * DisciplineReadyIn()                                                                        *
| *                                                                                            *
| * Pass this function the name of a discipline, and it'll return, in ${Macro.Return}, the    *
| * number of seconds until the specified disc can be used again.                            *
| *                                                                                            *
| * If the discipline named szDiscName is ready to be used immediately, this function        *
| * returns 0 (zero).                                                                        *
| *                                                                                            *
| * If this macro has never seen the character successfully use this discipline (or another    *
| * discipline on the same timer as it), and has also not had the UseDiscipline() function    *
| * called for this discipline (or another on the same timer), it will not know when or if    *
| * the discipline is ready to be used. In this case, it returns -1.                        *
| *                                                                                            *
| * Note that ${Macro.Return} is of type string, so use ${Int[${Macro.Return}]} to convert    *
| * the return value of this function to a real integer.                                    *
| *                                                                                            *
| * If szDiscName is not a valid discipline name, or is not usable by the current character    *
| * (based on class and level), this function returns NULL.                                    *
| *******************************************************************************************

Sub DisciplineReadyIn(string szDiscName)

    | szDiscName could be "Healing Will" or it could be "Healing Will Discipline". We need to strip the " Discipline" if it's
    | present. Search for the text "Discipline, and if so, cut off the last 11 characters of the string (which is the number
    | of characters in the word "Discipline" plus the space that precedes it).

    /declare szShortDiscName    string    local    ${If[${szDiscName.Find[Discipline]},${szDiscName.Left[-11]},${szDiscName}]}

    | Determine which timer this discipline is on...
    /call ReturnDisciplineTimer    "${szShortDiscName}"

    /declare iDiscTimer        int        local    ${Int[${Macro.Return}]}

    /call DetermineUsableDiscsForTimer    ${iDiscTimer}

    /if ( ${Select[${Macro.Return},NULL]} == 1 ) {

        | This is an invalid discipline name, or is not usable by the current character based on class and/or level. Return NULL.

        /return NULL
    }

    /if ( !${Defined[timerDiscType${iDiscTimer}]} ) {

        | This is a valid discipline for the current character, but this macro has never seen the character actually use the discipline,
        | nor has it ever called the UseDiscipline() function to attempt to use it (which would create a timer if it tried to use it and
        | received the message that it was not ready for use yet).
        |
        | In either case, we simply don't know whether it's ready to use or not. Return -1 to indicate that.

        /return -1
    }

/return ${Math.Calc[(${timerDiscType${iDiscTimer}}+9)/10]}




| +-----------------------------------------------------------------------------------------+
| | Internal Functions |
| |                                                                                            |
| | The functions below this point are "Helper Functions" that are used by the externally-    |
| | callable functions above. The below    functions are not intended to be called by macros    |
| | that utilize this include library.                                                        |
| +-----------------------------------------------------------------------------------------+



| *******************************************************************************************
| * Event_DisciplineNotReady()                                                                *
| *                                                                                            *
| * Called when a character attempts to use a discipline that is not ready yet. Required    *
| * by the UseDiscipline() function.                                                        *
| *******************************************************************************************

#event DisciplineNotReady    "You can use this discipline in #1# minutes #2# seconds."

Sub Event_DisciplineNotReady(string szEventText, int iMinutes, int iSeconds)

    | Unfortunately, there's no good way to know what discipline the character tried to use that resulted in this message.
    | However, the UseDiscipline() function does know what discipline it's trying to use. So just before it uses a discipline,
    | it sets the outer variable bUseDisciplineAttempt to TRUE, and sets it to FALSE right after it attempts to use a disc.
    | So, if bUseDisciplineAttempt is TRUE when this Event occurs, we know it's occurring in response to an attempt to use
    | a discipline by the UseDiscipline() function. So set an outer variable to the time remaining.

    /if ( ${Defined[bUseDisciplineAttempt]} && ${bUseDisciplineAttempt} && ${Defined[iDisciplineReadyInSeconds]} ) {

        /varcalc iDisciplineReadyInSeconds    (${iMinutes} * 60)+${iSeconds}
    }

    /popup Discipline ready in ${iMinutes} minutes, ${iSeconds} seconds.

/return



| *******************************************************************************************
| * InitializeDisciplineTimerLists()                                                        *
| *                                                                                            *
| * Declares variables needed for the Discipline functions in this include library.            *
| *******************************************************************************************

Sub InitializeDisciplineTimerLists

    /if ( !${Defined[szDisciplinesOnTimer1]} ) {

        /declare szDisciplinesOnTimer1    string    outer    Aimshot/Counterattack/Defensive/Deftdance/Earthwalk/Evasive/Final Stand/Holyforge/Impenetrable/Lithe/Nimble/Protective Spirit/Spelldodge/Stonestance/Stonewall/Sureshot/Trueshot/Unholy Aura/Voiddance/Whirlwind
    }

    /if ( !${Defined[szDisciplinesOnTimer2]} ) {

        /declare szDisciplinesOnTimer2    string    outer    Assassin/Battle Focus/Bestial Fury/Cleaving Anger/Crystalpalm/Duelist/Fortitude/Furious/Hundred Fists/Indomitable/Innerflame/Kinesthetics/Leechcurse/Puretone/Reckless/Sanctification/Twisted Chance/Weapon Shield
    }

    /if ( !${Defined[szDisciplinesOnTimer3]} ) {

        /declare szDisciplinesOnTimer3    string    outer    Aggressive/Ashenhand/Berserking/Blind Rage/Blinding Speed/Burning Rage/Charge/Cleaving Rage/Deadeye/Deadly Precision/Deflection/Fellstrike/Focused Fury/Inspired Anger/Mighty Strike/Precision/Scaledfist/Silentfist/Thunderkick
    }

    /if ( !${Defined[szDisciplinesOnTimer4]} ) {

        /declare szDisciplinesOnTimer4    string    outer    Aura of Runes/Dreamwalk/Fearless/Focused Will/Healing Will/Imperceptible/Planeswalk/Resistant/Spirit of Rage/Sprint/Unflinching Will/Weapon Affinity
    }

    /if ( !${Defined[szDisciplinesOnTimer5]} ) {

        /declare szDisciplinesOnTimer5    string    outer    Brutal Onslaught/Frenzied Stabbing/Rapid Kick/Savage Onslaught/Vengeful Flurry
    }

    /if ( !${Defined[szDisciplinesOnTimer6]} ) {

        /declare szDisciplinesOnTimer6    string    outer    Counterforce/Deadly Aim/Shocking Defense/Unpredictable Rage
    }

    /if ( !${Defined[szDisciplinesOnTimer10]} ) {

        /declare szDisciplinesOnTimer10    string    outer    Speed Focus
    }

/return



| *******************************************************************************************
| * ReturnDisciplineTimer()                                                                    *
| *                                                                                            *
| * Pass this function the name of a discipline, and it will return, in ${Macro.Return},    *
| * the number of the Timer that discipline is on.                                            *
| *                                                                                            *
| *    If the discipline name is unknown, returns 0                                            *
| *******************************************************************************************

Sub ReturnDisciplineTimer(string szDiscName)

    /declare iLoopCounter        int        local    1

    | szDiscName could be "Healing Will" or it could be "Healing Will Discipline". We need to strip the " Discipline" if it's
    | present. Search for the text "Discipline, and if so, cut off the last 11 characters of the string (which is the number
    | of characters in the word "Discipline" plus the space that precedes it).

    /declare szShortDiscName    string    local    ${If[${szDiscName.Find[Discipline]},${szDiscName.Left[-11]},${szDiscName}]}

    :ReturnDisciplineTimerLoop

    /for iLoopCounter    1    to    6

        /if ( ${szDisciplinesOnTimer${iLoopCounter}.Find[${szShortDiscName}]} ) {

            /return ${iLoopCounter}
        }

    /next iLoopCounter

    /if ( ${szDisciplinesOnTimer10.Find[${szShortDiscName}]} ) {

        /return 10
    }

/return 0



| *******************************************************************************************
| * DetermineUsableDiscsForTimer()                                                            *
| *                                                                                            *
| * Returns, in ${Macro.Return}, a comma-delimited list of discipline names that the        *
| * current character could use (based on class and level) that are associated with Timer #    *
| * iDiscTimerType.                                                                            *
| *                                                                                            *
| * Returns NULL if there are no usable discs on that particular Timer.                        *
| *******************************************************************************************

Sub DetermineUsableDiscsForTimer(int iDiscTimerType)

    /if ( !${Defined[CasterClasses]} ) {

        /declare CasterClasses            string    outer    Cleric/Druid/Shaman/Necromancer/Enchanter/Wizard/Magician
    }

    /if ( ${CasterClasses.Find[${Me.Class.Name}]} ) {

        | Only Melee and Hybrid classes can use disciplines, not casters.

        /return NULL
    }

    /if ( !${Defined[szDisciplinesOnTimer${iDiscTimerType}]} ) {

        /return NULL
    }

    /declare iNumDiscsOnTimer    int        local    ${Int[${Math.Calc[${szDisciplinesOnTimer${iDiscTimerType}.Count[/]} + 1]}]}

    /declare iLoopCounter        int        local

    /for iLoopCounter    1    to    ${iNumDiscsOnTimer}

        /if ( ${Spell[${szDisciplinesOnTimer${iDiscTimerType}.Token[${iLoopCounter},/]} Discipline].Level} <= ${Me.Level} ) {

            /if ( !${Defined[szOutputString]} ) {

                /declare szOutputString        string    local    ${szDisciplinesOnTimer${iDiscTimerType}.Token[${iLoopCounter},/]}

            } else {

                /varset szOutputString    ${szOutputString}, ${szDisciplinesOnTimer${iDiscTimerType}.Token[${iLoopCounter},/]}
            }
        }

    /next iLoopCounter

/return ${szOutputString}


