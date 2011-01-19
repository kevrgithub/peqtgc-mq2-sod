// MQ2AutoSize.cpp : Resize spawns by distance or whole zone (client only)
//
// 2/09/2009: added parameters, merc, npc & everything spawn options,
//            bug fixes and code cleanup - pms
// 6/28/2008: finds its own offset - ieatacid
//
// version 0.9.3 (by Psycotic)
//
//////////////////////////////////////////////////////////////////////////////
// Usage:
//   This plugin will automatically resize configured spawns to the specified
//   size. You can configure it to only resize within a specific range and then
//   resize back to normal when your distance moves out of that range.
//   Current default range is set to 50 and may be changed via INI or cmd line
//   NOTE:  These effects are CLIENT SIDE ONLY!
//
// Commands:
//  /autosize              - Toggles zone-wide  AutoSize on/off
//  /autosize dist         - Toggles distance-based AutoSize on/off
//  /autosize pc           - Toggles AutoSize PC spawn types
//  /autosize npc          - Toggles AutoSize NPC spawn types
//  /autosize pets         - Toggles AutoSize pet spawn types
//  /autosize mercs        - Toggles AutoSize mercenary spawn types
//  /autosize target       - Resizes your target to sizetarget size
//  /autosize everything   - Toggles AutoSize all spawn types
//  /autosize self         - Toggles AutoSize for your character
//  /autosize range #      - Sets range for distance-based AutoSize
//
//  (Valid sizes 1 to 250)
//  /autosize size #       - Sets default size for "everything"
//  /autosize sizepc #     - Sets size for PC spawn types
//  /autosize sizenpc #    - Sets size for NPC spawn types
//  /autosize sizepets #   - Sets size for pet spawn types
//  /autosize sizetarget # - Sets size for target parameter
//  /autosize sizemercs #  - Sets size for mercenary spawn types
//  /autosize sizeself #   - Sets size for your character
//
//  /autosize status       - Display current plugin settings to chatwnd
//  /autosize help         - Display command syntax to chatwnd
//  /autosize save         - Save settings to INI file (auto on plugin unload)
//  /autosize load         - Load settings from INI file (auto on plugin load)
//
//////////////////////////////////////////////////////////////////////////////

#include "../MQ2Plugin.h"
const char* MODULE_NAME = "MQ2AutoSize";
PreSetup(MODULE_NAME);

const unsigned short SKIP_PULSES = 5;
const float          MIN_SIZE    = 1.0f;
const float          MAX_SIZE    = 250.0f;

unsigned short usiSkipPulse      = 0;
unsigned int iRange              = 50;
float fSize                      = 1.0f;
float fPCSize                    = 1.0f;
float fNPCSize                   = 1.0f;
float fPetSize                   = 1.0f;
float fMercSize                  = 1.0f;
float fSelfSize                  = 1.0f;
float fTargetSize                = 1.0f;
float fMountSize                 = 1.0f;
bool bSizeByRange                = false;
bool bSizeNoRange                = true;
bool bResizePC                   = true;
bool bResizeNPC                  = false;
bool bResizePets                 = false;
bool bResizeMercs                = false;
bool bResizeAll                  = false;
bool bResizeSelf                 = false;
bool bResizeMounts               = false;
bool bCommandUsed                = false;
bool bAutoSave                   = false;
char szTemp[MAX_STRING]          = {0};

unsigned long addrChangeHeight   = NULL;
PBYTE patternChangeHeight        = (PBYTE)"\x51\x56\x8B\xF1\x8B\x44\x24\x00\x8B\xCE\x89\x86\x00\x00\x00\x00"
                                          "\xE8\x00\x00\x00\x00\x8B\x8E\x00\x00\x00\x00\x6A\x00\x51\x8D\x8E"
                                          "\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xCE\xE8";
char maskChangeHeight[]          = "xxxxxxx?xxxx????x????xx????x?xxx????x????xxx";

class SizeClass
{
   public:
      void SizeFunc(float);
};

FUNCTION_AT_ADDRESS(void SizeClass::SizeFunc(float), addrChangeHeight);

void LoadINI()
{
    GetPrivateProfileString("Config", "ResizePC",     "on", szTemp, 10, INIFileName);
    bResizePC = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizeNPC",    NULL, szTemp, 10, INIFileName);
    bResizeNPC = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizePets",   NULL, szTemp, 10, INIFileName);
    bResizePets = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizeMercs",  NULL, szTemp, 10, INIFileName);
    bResizeMercs = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizeAll",    NULL, szTemp, 10, INIFileName);
    bResizeAll = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizeSelf",   NULL, szTemp, 10, INIFileName);
    bResizeSelf = (!strnicmp(szTemp, "on", 3));
    GetPrivateProfileString("Config", "ResizeMounts", NULL, szTemp, 10, INIFileName);
    bResizeMounts = (!strnicmp(szTemp, "on", 3));

    GetPrivateProfileString("Config", "SizeByRange",  NULL, szTemp, 10, INIFileName);
    bSizeByRange = (!strnicmp(szTemp, "on", 3));
    bSizeNoRange = !bSizeByRange;
    iRange = GetPrivateProfileInt("Config", "Range", iRange, INIFileName);

    fSize       = (float)GetPrivateProfileInt("Config", "SizeDefault", (int)fSize,       INIFileName);
    if (fSize < MIN_SIZE || fSize > MAX_SIZE) fSize = MIN_SIZE;
    fPCSize     = (float)GetPrivateProfileInt("Config", "SizePC",      (int)fPCSize,     INIFileName);
    if (fPCSize < 1.0f || fPCSize > 250.0f) fPCSize = 1.0f;
    fNPCSize    = (float)GetPrivateProfileInt("Config", "SizeNPC",     (int)fNPCSize,    INIFileName);
    if (fNPCSize < 1.0f || fNPCSize > 250.0f) fNPCSize = 1.0f;
    fPetSize    = (float)GetPrivateProfileInt("Config", "SizePets",    (int)fPetSize,    INIFileName);
    if (fPetSize < 1.0f || fPetSize > 250.0f) fPetSize = 1.0f;
    fMercSize   = (float)GetPrivateProfileInt("Config", "SizeMercs",   (int)fMercSize,   INIFileName);
    if (fMercSize < 1.0f || fMercSize > 250.0f) fMercSize = 1.0f;
    fTargetSize = (float)GetPrivateProfileInt("Config", "SizeTarget",  (int)fTargetSize, INIFileName);
    if (fTargetSize < 1.0f || fTargetSize > 250.0f) fTargetSize = 1.0f;
    fMountSize  = (float)GetPrivateProfileInt("Config", "SizeMount",   (int)fMountSize,  INIFileName);
    if (fMountSize < 1.0f || fMountSize > 250.0f) fMountSize = 1.0f;
    fSelfSize   = (float)GetPrivateProfileInt("Config", "SizeSelf",    (int)fSelfSize,   INIFileName);
    if (fSelfSize < 1.0f || fSelfSize > 250.0f) fSelfSize = 1.0f;
}

void SaveINI()
{
    WritePrivateProfileString("Config", "ResizePC",     bResizePC ? "on" : "off",           INIFileName);
    WritePrivateProfileString("Config", "ResizeNPC",    bResizeNPC ? "on" : "off",          INIFileName);
    WritePrivateProfileString("Config", "ResizePets",   bResizePets ? "on" : "off",         INIFileName);
    WritePrivateProfileString("Config", "ResizeMercs",  bResizeMercs ? "on" : "off",        INIFileName);
    WritePrivateProfileString("Config", "ResizeAll",    bResizeAll ? "on" : "off",          INIFileName);
    WritePrivateProfileString("Config", "ResizeSelf",   bResizeSelf ? "on" : "off",         INIFileName);
    WritePrivateProfileString("Config", "ResizeMounts", bResizeMounts ? "on" : "off",       INIFileName);
    WritePrivateProfileString("Config", "SizeByRange",  bSizeByRange ? "on" : "off",        INIFileName);
    WritePrivateProfileString("Config", "Range",        itoa(iRange, szTemp, 10),           INIFileName);
    WritePrivateProfileString("Config", "SizeDefault",  itoa((int)fSize, szTemp, 10),       INIFileName);
    WritePrivateProfileString("Config", "SizePC",       itoa((int)fPCSize, szTemp, 10),     INIFileName);
    WritePrivateProfileString("Config", "SizeNPC",      itoa((int)fNPCSize, szTemp, 10),    INIFileName);
    WritePrivateProfileString("Config", "SizePets",     itoa((int)fPetSize, szTemp, 10),    INIFileName);
    WritePrivateProfileString("Config", "SizeMercs",    itoa((int)fMercSize, szTemp, 10),   INIFileName);
    WritePrivateProfileString("Config", "SizeTarget",   itoa((int)fTargetSize, szTemp, 10), INIFileName);
    WritePrivateProfileString("Config", "SizeSelf",     itoa((int)fSelfSize, szTemp, 10),   INIFileName);
    WritePrivateProfileString("Config", "SizeMounts",   itoa((int)fMountSize, szTemp, 10),  INIFileName);
}

void ChangeSize(PSPAWNINFO pChangeSpawn, float fNewSize)
{
    if (GetGameState() != GAMESTATE_INGAME || !pChangeSpawn || !pChangeSpawn->SpawnID) return;
    ((SizeClass*)pChangeSpawn)->SizeFunc(fNewSize);
    bCommandUsed = true;
}

void SizePasser(PSPAWNINFO pSpawn)
{
    if ((!bSizeNoRange && !bSizeByRange) || GetGameState() != GAMESTATE_INGAME) return;
    PSPAWNINFO pChSpawn = (PSPAWNINFO)pCharSpawn;
    PSPAWNINFO pLPlayer = (PSPAWNINFO)pLocalPlayer;
    if (!pLPlayer || !pChSpawn->SpawnID || !pSpawn || !pSpawn->SpawnID) return;

    if (pSpawn->SpawnID == pLPlayer->SpawnID)
    {
        if (bResizeSelf) ChangeSize(pSpawn, fSelfSize);
        return;
    }

    switch(GetSpawnType(pSpawn))
    {
    case PC:
        if (bResizePC)
        {
            ChangeSize(pSpawn, fPCSize);
            return;
        }
        break;
    case NPC:
        if (bResizeNPC)
        {
            ChangeSize(pSpawn, fNPCSize);
            return;
        }
        break;
    case PET:
        if (bResizePets)
        {
            ChangeSize(pSpawn, fPetSize);
            return;
        }
        break;
    case MERCENARY:
        if (bResizeMercs)
        {
            ChangeSize(pSpawn, fMercSize);
            return;
        }
        break;
    case MOUNT:
        if (bResizeMounts && pSpawn->SpawnID != pChSpawn->SpawnID)
        {
            ChangeSize(pSpawn, fMountSize);
            return;
        }
        break;
    default:
        break;
    }

    if (bResizeAll && pSpawn->SpawnID != pChSpawn->SpawnID) ChangeSize(pSpawn, fSize);
}

void ResetPasser(PSPAWNINFO pSpawn)
{
    PSPAWNINFO pChSpawn = (PSPAWNINFO)pCharSpawn;
    PSPAWNINFO pLPlayer = (PSPAWNINFO)pLocalPlayer;
    if (GetGameState() != GAMESTATE_INGAME || !pLPlayer || !pChSpawn->SpawnID || !pSpawn || !pSpawn->SpawnID) return;

    if (pSpawn->SpawnID == pLPlayer->SpawnID)
    {
        if (bResizeSelf) ChangeSize(pSpawn, 0);
        return;
    }

    switch(GetSpawnType(pSpawn))
    {
    case PC:
        if (bResizePC)
        {
            ChangeSize(pSpawn, 0);
            return;
        }
        break;
    case NPC:
        if (bResizeNPC)
        {
            ChangeSize(pSpawn, 0);
            return;
        }
        break;
    case PET:
        if (bResizePets)
        {
            ChangeSize(pSpawn, 0);
            return;
        }
        break;
    case MERCENARY:
        if (bResizeMercs)
        {
            ChangeSize(pSpawn, 0);
            return;
        }
        break;
    case MOUNT:
        if (bResizeMounts && pSpawn->SpawnID != pChSpawn->SpawnID)
        {
            ChangeSize(pSpawn, 0);
            return;
        }
        break;
    default:
        break;
    }

    if (bResizeAll && pSpawn->SpawnID != pChSpawn->SpawnID) ChangeSize(pSpawn, 0);
}

void ResetAllByType(eSpawnType OurType)
{
    PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
    PSPAWNINFO pChSpawn = (PSPAWNINFO)pCharSpawn;
    PSPAWNINFO pLPlayer = (PSPAWNINFO)pLocalPlayer;
    if (GetGameState() != GAMESTATE_INGAME || !pLPlayer || !pChSpawn->SpawnID || !pSpawn || !pSpawn->SpawnID) return;

    while (pSpawn)
    {
        if (pSpawn->SpawnID == pLPlayer->SpawnID)
        {
            pSpawn = pSpawn->pNext;
            continue;
        }

        eSpawnType ListType = GetSpawnType(pSpawn);
        if (ListType == OurType) ChangeSize(pSpawn, 0);
        pSpawn = pSpawn->pNext;
    }
}

void ResizeAll()
{
    if (GetGameState() != GAMESTATE_INGAME) return;
    PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
    while (pSpawn)
    {
        SizePasser(pSpawn);
        pSpawn = pSpawn->pNext;
    }
}

void ResetAll()
{
    if (GetGameState() != GAMESTATE_INGAME) return;
    PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
    while (pSpawn)
    {
        ResetPasser(pSpawn);
        pSpawn = pSpawn->pNext;
    }
}

PLUGIN_API void OnAddSpawn(PSPAWNINFO pNewSpawn)
{
    if (bSizeNoRange) SizePasser(pNewSpawn);
}

PLUGIN_API void OnEndZone()
{
    ResizeAll();
}

PLUGIN_API void OnPulse()
{
    if (GetGameState() != GAMESTATE_INGAME || !bSizeByRange) return;
    if (usiSkipPulse < SKIP_PULSES)
    {
        usiSkipPulse++;
        return;
    }

    PSPAWNINFO pAllSpawns = (PSPAWNINFO)pSpawnList;
    float fDist = 0.0f;
    usiSkipPulse = 0;

    while (pAllSpawns)
    {
        fDist = GetDistance((PSPAWNINFO)pCharSpawn, pAllSpawns);
        if (fDist <= iRange)
        {
            SizePasser(pAllSpawns);
        }
        else if (fDist <= iRange + 50)
        {
            ResetPasser(pAllSpawns);
        }
        pAllSpawns = pAllSpawns->pNext;
    }
}

void OutputHelp()
{
    WriteChatf("\ay%s\aw:: Command Usage Help", MODULE_NAME);
    WriteChatf("  \ag/autosize\ax - Toggles zone-wide AutoSize on/off");
    WriteChatf("  \ag/autosize\ax \aydist\ax - Toggles distance-based AutoSize on/off");
    WriteChatf("  \ag/autosize\ax \ayrange #\ax - Sets range for distance checking");
    WriteChatf("--- Valid Resize Toggles ---");
    WriteChatf("  \ag/autosize\ax [ \aypc\ax | \aynpc\ax | \aypets\ax | \aymercs\ax | \aytarget\ax | \ayeverything\ax | \ayself\ax | \aymounts\ax ]");
    WriteChatf("--- Valid Size Syntax (1 to 250) ---");
    WriteChatf("  \ag/autosize\ax [ \aysize\ax | \aysizepc\ax | \aysizenpc\ax | \aysizepets\ax | \aysizemercs\ax | \aysizetarget\ax | \aysizeself\ax | \aysizemounts\ax ] [ \ay#\ax ]");
    WriteChatf("--- Other Valid Commands ---");
    WriteChatf("  \ag/autosize\ax [ \ayhelp\ax | \aystatus\ax | \aysave\ax | \ayload\ax ]");
}

void OutputStatus()
{
    char szMethod[100] = {0};
    char szOn[10] = "\agon\ax";
    char szOff[10] = "\aroff\ax";
    if (bSizeNoRange)
    {
        sprintf(szMethod, "\agAllZone\ax");
    }
    else if (bSizeByRange)
    {
        sprintf(szMethod, "\agRange\ax) RangeSize(\ag%d\ax", iRange);
    }
    else
    {
        sprintf(szMethod, "\arInactive\ax");
    }

    WriteChatf("\ay%s\aw:: Current Status -- Method: (%s)", MODULE_NAME, szMethod);
    WriteChatf("Toggles: PC(%s) NPC(%s) Pets(%s) Mercs(%s) Everything(%s) Self(%s) Mounts(%s)", bResizePC ? szOn : szOff, bResizeNPC ? szOn : szOff, bResizePets ? szOn : szOff, bResizeMercs ? szOn : szOff, bResizeAll ? szOn : szOff, bResizeSelf ? szOn : szOff, bResizeMounts ? szOn : szOff);
    WriteChatf("Sizes: PC(\ag%.0f\ax) NPC(\ag%.0f\ax) Pets(\ag%.0f\ax) Mercs(\ag%.0f\ax) Target(\ag%.0f\ax) Everything(\ag%.0f\ax) Self(\ag%.0f\ax) Mounts(\ag%.0f\ax)", fPCSize, fNPCSize, fPetSize, fMercSize, fTargetSize, fSize, fSelfSize, fMountSize);
}

void AutoSizeCmd(PSPAWNINFO pChar, char* szLine)
{
    char szCurArg[MAX_STRING] = {0};

    GetArg(szCurArg, szLine, 1);
    if (!*szCurArg)
    {
        bSizeNoRange = !bSizeNoRange;
        if (bSizeNoRange)
        {
            if (bSizeByRange)
            {
                bSizeByRange = false;
                WriteChatf("\ay%s\aw:: AutoSize (\ayRange\ax) now \ardisabled\ax!", MODULE_NAME);
            }
            ResizeAll();
        }
        else
        {
            ResetAll();
        }
        WriteChatf("\ay%s\aw:: AutoSize (\ayAllZone\ax) now %s\ax!", MODULE_NAME, bSizeNoRange ? "\agenabled" : "\ardisabled");
    }
    else
    {
        if (!strnicmp(szCurArg, "dist", 5))
        {
            bSizeByRange = !bSizeByRange;
            if (bSizeByRange)
            {
                if (bSizeNoRange)
                {
                    bSizeNoRange = false;
                    WriteChatf("\ay%s\aw:: AutoSize (\ayAllZone\ax) now \ardisabled\ax!", MODULE_NAME);
                }
            }
            else
            {
                ResetAll();
            }
            WriteChatf("\ay%s\aw:: AutoSize (\ayRange\ax) now %s\ax!", MODULE_NAME, bSizeByRange ? "\agenabled" : "\ardisabled");
            return;
        }
        else if (!strnicmp(szCurArg, "save", 5))
        {
            SaveINI();
            WriteChatf("\ay%s\aw:: Configuration file saved.", MODULE_NAME);
            return;
        }
        else if (!strnicmp(szCurArg, "load", 5))
        {
            LoadINI();
            WriteChatf("\ay%s\aw:: Configuration file loaded.", MODULE_NAME);
            return;
        }
        else if (!strnicmp(szCurArg, "range", 6))
        {
            GetArg(szCurArg, szLine, 2);
            if (atoi(szCurArg) > 0)
            {
                iRange = atoi(szCurArg);
                WriteChatf("\ay%s\aw:: Range set to \ag%d", MODULE_NAME, iRange);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Range not changed from \ag%d", MODULE_NAME, iRange);
            }
        }
        else if (!strnicmp(szCurArg, "size", 5))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Size set to \ag%.0f", MODULE_NAME, fSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Size not changed from \ag%.0f", MODULE_NAME, fSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizepc", 7))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fPCSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: PC Size set to \ag%.0f", MODULE_NAME, fPCSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: PC Size not changed from \ag%.0f", MODULE_NAME, fPCSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizenpc", 8))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fNPCSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: NPC Size set to \ag%.0f", MODULE_NAME, fNPCSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: NPC Size not changed from \ag%.0f", MODULE_NAME, fNPCSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizepets", 9))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fPetSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Pet size set to \ag%.0f", MODULE_NAME, fPetSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Pet size not changed from \ag%.0f", MODULE_NAME, fPetSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizemercs", 10))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fMercSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Merc size set to \ag%.0f", MODULE_NAME, fMercSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Merc size not changed from \ag%.0f", MODULE_NAME, fMercSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizetarget", 11))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fTargetSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Target size set to \ag%.0f", MODULE_NAME, fTargetSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Target size not changed from \ag%.0f", MODULE_NAME, fTargetSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizeself", 9))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fSelfSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Self size set to \ag%.0f", MODULE_NAME, fSelfSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Self size not changed from \ag%.0f", MODULE_NAME, fSelfSize);
            }
        }
        else if (!strnicmp(szCurArg, "sizemounts", 11))
        {
            GetArg(szCurArg, szLine, 2);
            if ((float)atof(szCurArg) > 0.0f && (float)atof(szCurArg) <= 250.0f)
            {
                fMountSize = (float)atof(szCurArg);
                WriteChatf("\ay%s\aw:: Mount size set to \ag%.0f", MODULE_NAME, fMountSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: Mount size not changed from \ag%.0f", MODULE_NAME, fMountSize);
            }
        }
        else if (!strnicmp(szCurArg, "pc", 3))
        {
            bResizePC = !bResizePC;
            if (!bResizePC) ResetAllByType(PC);
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayPC\ax) now %s\ax!", MODULE_NAME, bResizePC ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "npc", 4))
        {
            bResizeNPC = !bResizeNPC;
            if (!bResizeNPC) ResetAllByType(NPC);
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayNPC\ax) now %s\ax!", MODULE_NAME, bResizeNPC ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "everything", 11))
        {
            if (bResizeAll) ResetAll();
            bResizeAll = !bResizeAll;
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayEverything\ax) now %s\ax!", MODULE_NAME, bResizeAll ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "pets", 5))
        {
            bResizePets = !bResizePets;
            if (!bResizePets) ResetAllByType(PET);
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayPets\ax) now %s\ax!", MODULE_NAME, bResizePets ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "mercs", 6))
        {
            bResizeMercs = !bResizeMercs;
            if (!bResizeMercs) ResetAllByType(MERCENARY);
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayMercs\ax) now %s\ax!", MODULE_NAME, bResizeMercs ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "mounts", 7))
        {
            bResizeMounts = !bResizeMounts;
            if (!bResizeMercs) ResetAllByType(MOUNT);
            WriteChatf("\ay%s\aw:: AutoSize Option (\ayMounts\ax) now %s\ax!", MODULE_NAME, bResizeMounts ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "target", 7))
        {
            PSPAWNINFO pTheTarget = (PSPAWNINFO)pTarget;
            if (pTheTarget && GetGameState() == GAMESTATE_INGAME && pTheTarget->SpawnID)
            {
                ChangeSize(pTheTarget, fTargetSize);
                char szTarName[MAX_STRING] = {0};
                sprintf(szTarName, "%s", pTheTarget->DisplayedName);
                WriteChatf("\ay%s\aw:: Resized \ay%s\ax to \ag%.0f\ax", MODULE_NAME, szTarName, fTargetSize);
            }
            else
            {
                WriteChatf("\ay%s\aw:: \arYou must have a target to use this parameter.", MODULE_NAME);
            }
            return;
        }
        else if (!strnicmp(szCurArg, "self", 5))
        {
            bResizeSelf = !bResizeSelf;
            if (!bResizeSelf)
            {
                if (((PSPAWNINFO)pLocalPlayer)->Mount) ChangeSize((PSPAWNINFO)pLocalPlayer, 0);
                else ChangeSize((PSPAWNINFO)pCharSpawn, 0);
            }
            WriteChatf("\ay%s\aw:: AutoSize Option (\aySelf\ax) now %s\ax!", MODULE_NAME, bResizeSelf ? "\agenabled" : "\ardisabled");
        }
        else if (!strnicmp(szCurArg, "help", 5))
        {
            OutputHelp();
            return;
        }
        else if (!strnicmp(szCurArg, "status", 7))
        {
            OutputStatus();
            return;
        }
        else
        {
            WriteChatf("\ay%s\aw:: \arInvalid command parameter.", MODULE_NAME);
            return;
        }

        // if size change or everything, pets, mercs toggled and won't be handled onpulse
        if (bSizeNoRange) ResizeAll();
    }
}

// credit: radioactiveman/bunny771 ----------------------------------------
bool bDataCompare(const unsigned char* pucData, const unsigned char* pucMask, const char* szMask)
{
    for (; *szMask; ++szMask, ++pucData, ++pucMask)
        if (*szMask == 'x' && *pucData != *pucMask)
            return false;
    return (*szMask) == NULL;
}

unsigned long ulFindPattern(unsigned long ulAddress, unsigned long ulLen, unsigned char* pucMask, char* szMask)
{
    for (unsigned long i = 0; i < ulLen; i++)
        if (bDataCompare( (unsigned char*)(ulAddress + i), pucMask, szMask))
            return (unsigned long)(ulAddress + i);
    return 0;
}
// ------------------------------------------------------------------------

PLUGIN_API void InitializePlugin()
{
    addrChangeHeight = ulFindPattern(0x4A0000, 0x100000, patternChangeHeight, maskChangeHeight);

    if(addrChangeHeight)
    {
        AddCommand("/autosize", AutoSizeCmd);
        LoadINI();
    }
    else
    {
        WriteChatf("\ay%s\aw:: \arError:\ax Couldn't find offset. Unloading.", MODULE_NAME);
        EzCommand("/timed 1 /plugin mq2autosize unload");
    }
}

PLUGIN_API void ShutdownPlugin()
{
   if(addrChangeHeight)
   {
      RemoveCommand("/autosize");
      if (bCommandUsed) ResetAll();
      SaveINI();
   }
}
