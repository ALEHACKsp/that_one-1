#include "Util.h"
#include "../signature/csignature.h"
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

namespace Util {
  float Distance( Vector vOrigin, Vector vLocalOrigin ) {
    Vector Delta = vOrigin - vLocalOrigin;
    float Dist = sqrt( Delta.Length() );
    
    if( Dist < 1.0f ) {
      return 1.0f;
    }
    
    return Dist;
  }
  
  void vector_transform( const Vector &vSome, const matrix3x4 vMatrix, Vector &vOut ) {
    if( &vMatrix == nullptr ) {
      return;
    }
    
    Vector xm = ( Vector & )vMatrix[0];
    Vector ym = ( Vector & )vMatrix[1];
    Vector zm = ( Vector & )vMatrix[2];
    vOut.x = vSome.Dot( xm ) + vMatrix[0][3];
    vOut.y = vSome.Dot( ym ) + vMatrix[1][3];
    vOut.z = vSome.Dot( zm ) + vMatrix[2][3];
  }
  
  float VectorialDistanceToGround( Vector origin ) {
    auto *ground_trace = new trace_t();
    Ray_t ray;
    Vector endpos = origin;
    endpos.z -= 8192;
    ray.Init( origin, endpos );
    gInts.EngineTrace->TraceRay( ray, MASK_PLAYERSOLID, nullptr, ground_trace );
    return 8192.0f * ground_trace->fraction;
  }
  float DistanceToGround( CBaseEntity *ent ) {
    if( ent->GetFlags() & FL_ONGROUND ) {
      return 0;
    }
    
    Vector origin = ent->GetVecOrigin();
    float v1 = VectorialDistanceToGround( origin + Vector( 10.0f, 10.0f, 0.0f ) );
    float v2 = VectorialDistanceToGround( origin + Vector( -10.0f, 10.0f, 0.0f ) );
    float v3 = VectorialDistanceToGround( origin + Vector( 10.0f, -10.0f, 0.0f ) );
    float v4 = VectorialDistanceToGround( origin + Vector( -10.0f, -10.0f, 0.0f ) );
    return MIN( v1, MIN( v2, MIN( v3, v4 ) ) );
  }
  
  void FixMove( CUserCmd *pCmd, Vector m_vOldAngles, float m_fOldForward, float m_fOldSidemove ) {
    float deltaView;
    float f1;
    float f2;
    
    if( m_vOldAngles.y < 0.f ) {
      f1 = 360.0f + m_vOldAngles.y;
    } else {
      f1 = m_vOldAngles.y;
    }
    
    if( pCmd->viewangles.y < 0.0f ) {
      f2 = 360.0f + pCmd->viewangles.y;
    } else {
      f2 = pCmd->viewangles.y;
    }
    
    if( f2 < f1 ) {
      deltaView = abs( f2 - f1 );
    } else {
      deltaView = 360.0f - abs( f1 - f2 );
    }
    
    deltaView = 360.0f - deltaView;
    pCmd->forwardmove = cos( DEG2RAD( deltaView ) ) * m_fOldForward + cos( DEG2RAD( deltaView + 90.f ) ) * m_fOldSidemove;
    pCmd->sidemove = sin( DEG2RAD( deltaView ) ) * m_fOldForward + sin( DEG2RAD( deltaView + 90.f ) ) * m_fOldSidemove;
  }
  void lookAt( const bool silent, Vector vAngs, CUserCmd *pCommand ) {
    if( silent ) {
      pCommand->viewangles = vAngs;
    } else {
      pCommand->viewangles = vAngs;
      gInts.Engine->SetViewAngles( pCommand->viewangles );
    }
  }
  
  bool ShouldReflect( CBaseEntity *ent, int lTeamNum, const char *name ) {
    return !strcmp( name, "CTFProjectile_SentryRocket" ) || !strcmp( name, "CTFProjectile_Arrow" ) || !
           strcmp( name, "CTFProjectile_HealingBolt" ) || !strcmp( name, "CTFProjectile_EnergyBall" ) || !
           strcmp( name, "CTFProjectile_Flare" ) || !strcmp( name, "CTFProjectile_Jar" ) || !
           strcmp( name, "CTFProjectile_JarMilk" ) || !strcmp( name, "CTFProjectile_Cleaver" ) || !
           strcmp( name, "CTFProjectile_Throwable" ) || !strcmp( name, "CTFProjectile_Rocket" ) || !
           strcmp( name, "CTFGrenadePipebombProjectile" ) || !strcmp( name, "CTFProjectile_ThrowableRepel" ) || !
           strcmp( name, "CTFProjectile_ThrowableBrick" ) || !strcmp( name, "CTFProjectile_ThrowableBreadMonster" ) && ent->
           GetTeamNum() != lTeamNum;
  }
  
  Vector CalcAngle( const Vector &src, const Vector &dst ) {
    Vector vAngle;
    Vector delta( ( src.x - dst.x ), ( src.y - dst.y ), ( src.z - dst.z ) );
    double hyp = sqrt( delta.x * delta.x + delta.y * delta.y );
    vAngle.x = float( atanf( float( delta.z / hyp ) ) * 57.295779513082f );
    vAngle.y = float( atanf( float( delta.y / delta.x ) ) * 57.295779513082f );
    vAngle.z = 0.0f;
    
    if( delta.x >= 0.0 ) {
      vAngle.y += 180.0f;
    }
    
    return vAngle;
  }
  float GetFOV( Vector viewAngle, const Vector &aimAngle ) {
    Vector ang, aim;
    AngleVectors( viewAngle, &aim );
    AngleVectors( aimAngle, &ang );
    return RAD2DEG( acos( aim.Dot( ang ) / aim.LengthSqr() ) );
  }
  
  Vector EstimateAbsVelocity( CBaseEntity *ent ) {
    typedef void( __thiscall * EstimateAbsVelocityFn )( CBaseEntity *, Vector & );
    static DWORD dwFn = Signatures::GetClientSignature( "E8 ? ? ? ? F3 0F 10 4D ? 8D 85 ? ? ? ? F3 0F 10 45 ? F3 0F 59 C9 56 F3 0F 59 C0 F3 0F 58 C8 0F 2F 0D ? ? ? ? 76 07" ) + 0x1;
    static DWORD dwEstimate = ( ( *( PDWORD )( dwFn ) ) + dwFn + 4 );
    EstimateAbsVelocityFn vel = ( EstimateAbsVelocityFn )dwEstimate;
    Vector v;
    vel( ent, v );
    return v;
  }
  
  bool IsHeadshotWeapon( int Class, CBaseCombatWeapon *pWep ) {
    if( pWep->GetSlot() == 0 ) {
      auto wep = pWep->GetItemDefinitionIndex();
      
      if( Class == TF2_Sniper )
        if( wep != weaponid::Sniper_m_TheHuntsman && wep != weaponid::Sniper_m_FestiveHuntsman && wep != weaponid::Sniper_m_TheFortifiedCompound ) {
          return true;
        }
        
      if( Class == TF2_Spy )
        if( wep == weaponid::Spy_m_TheAmbassador || wep == weaponid::Spy_m_FestiveAmbassador ) {
          return true;
        }
    }
    
    return false;
  }
  bool CanAmbassadorHeadshot( CBaseCombatWeapon *wpn ) {
    return gInts.globals->curtime - wpn->m_flLastFireTime() >= 1.0f;
  }
  
  //tarAngle = enemy->GetEyeAngles()
  //wsc_spy_to_victim = enem->GetWorldSpaceCenter() - local->GetWorldSpaceCenter()
  bool canBackstab( Vector tarAngle, Vector wsc_spy_to_victim, Vector from_angle ) {
    wsc_spy_to_victim.z = 0;
    wsc_spy_to_victim.NormalizeInPlace();
    Vector eye_spy;
    AngleVectors( from_angle, &eye_spy );
    eye_spy.z = 0;
    eye_spy.NormalizeInPlace();
    Vector eye_victim;
    AngleVectors( tarAngle, &eye_victim );
    eye_victim.z = 0;
    eye_victim.NormalizeInPlace();
    
    if( Dot( wsc_spy_to_victim, eye_victim ) <= 0.01f ) {
      return false;
    }
    
    if( Dot( wsc_spy_to_victim, eye_spy ) <= 0.5f ) {
      return false;
    }
    
    if( Dot( eye_spy, eye_victim ) <= -0.3f ) {
      return false;
    }
    
    return true;
  }
  //double dunk code was bad anyway, might as well reuse for it for something useful...
  //February 14, 2008 Patch
  //Added a small delay(200 ms) before a zoomed Sniper shot can get a critical hit.
  //seems strange no one's mentioned this...
  bool canHeadshot( CBaseEntity *pLocal ) {
    static float timer = 0;
    static bool started = false;
    
    if( !( pLocal->GetCond() & tf_cond::TFCond_Zoomed ) ) {
      started = false;
      timer = 0;
    }
    
    if( !started ) {
      started = true;
      timer = gInts.globals->curtime;
    }
    
    if( started ) {
      if( gInts.globals->curtime - timer > 0.2f ) {
        return true;
      }
    }
    
    return false;
  }
  
  void minDist( weaponid id, float &dist ) {
    switch( id ) {
    case weaponid::Engi_s_TheShortCircuit: {
      dist = 17.0f;
      break;
    }
    
    case weaponid::Pyro_m_FlameThrower:
    case weaponid::Pyro_m_FlameThrowerR:
    case weaponid::Pyro_m_TheBackburner:
    case weaponid::Pyro_m_TheDegreaser:
    case weaponid::Pyro_m_ThePhlogistinator:
    case weaponid::Pyro_m_FestiveFlameThrower:
    case weaponid::Pyro_m_TheRainblower:
    case weaponid::Pyro_m_SilverBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_GoldBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_RustBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_BloodBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_CarbonadoBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_DiamondBotkillerFlameThrowerMkI:
    case weaponid::Pyro_m_SilverBotkillerFlameThrowerMkII:
    case weaponid::Pyro_m_GoldBotkillerFlameThrowerMkII:
    case weaponid::Pyro_m_FestiveBackburner:
    case weaponid::Pyro_m_ForestFire:
    case weaponid::Pyro_m_BarnBurner:
    case weaponid::Pyro_m_BovineBlazemaker:
    case weaponid::Pyro_m_EarthSkyandFire:
    case weaponid::Pyro_m_FlashFryer:
    case weaponid::Pyro_m_TurbineTorcher:
    case weaponid::Pyro_m_Autumn:
    case weaponid::Pyro_m_PumpkinPatch:
    case weaponid::Pyro_m_Nutcracker:
    case weaponid::Pyro_m_Balloonicorn:
    case weaponid::Pyro_m_Rainbow:
    case weaponid::Pyro_m_CoffinNail:
    case weaponid::Pyro_m_Warhawk:
    case weaponid::Pyro_m_NostromoNapalmer: {
      dist = 17.5f;
      break;
    }
    
    case weaponid::Medic_m_TheOverdose: {
      dist = 20.0f;
      break;
    }
    
    case weaponid::Demoman_m_TheIronBomber: {
      dist = 21.0f;
      break;
    }
    
    case weaponid::Scout_s_TheFlyingGuillotine:
    case weaponid::Scout_s_TheFlyingGuillotineG:
    case weaponid::Scout_s_MadMilk:
    case weaponid::Pyro_m_DragonsFury: {
      dist = 23.0f;
      break;
    }
    
    case weaponid::Medic_m_SyringeGun:
    case weaponid::Medic_m_SyringeGunR:
    case weaponid::Medic_m_TheBlutsauger:
    case weaponid::Demoman_m_TheLochnLoad: {
      dist = 26.0f;
      break;
    }
    
    case weaponid::Pyro_s_TheScorchShot:
    case weaponid::Pyro_s_TheDetonator:
    case weaponid::Pyro_s_TheFlareGun: {
      dist = 35.0f;
      break;
    }
    
    case weaponid::Pyro_s_TheManmelter: {
      dist = 45.0f;
      break;
    }
    
    //shotguns
    case weaponid::Engi_m_Autumn:
    case weaponid::Engi_m_BackwoodsBoomstick:
    case weaponid::Engi_m_CivicDuty:
    case weaponid::Engi_m_CoffinNail:
    case weaponid::Engi_m_DressedtoKill:
    case weaponid::Engi_m_EngineersShotgun:
    case weaponid::Engi_m_FestiveFrontierJustice:
    case weaponid::Engi_m_FestiveShotgun:
    case weaponid::Engi_m_FlowerPower:
    case weaponid::Engi_m_LightningRod:
    case weaponid::Engi_m_PanicAttack:
    case weaponid::Engi_m_RedBear:
    case weaponid::Engi_m_RusticRuiner:
    case weaponid::Engi_m_ShotgunR:
    case weaponid::Engi_m_TheFrontierJustice:
    case weaponid::Engi_m_TheWidowmaker:
    case weaponid::Heavy_s_TheFamilyBusiness:
    case weaponid::Pyro_s_TheReserveShooter:
    case weaponid::Scout_m_BabyFacesBlaster:
    case weaponid::Scout_m_BackcountryBlaster:
    case weaponid::Scout_m_BloodBotkillerScattergunMkI:
    case weaponid::Scout_m_BlueMew:
    case weaponid::Scout_m_CarbonadoBotkillerScattergunMkI:
    case weaponid::Scout_m_CoffinNail:
    case weaponid::Scout_m_Corsair:
    case weaponid::Scout_m_CountryCrusher:
    case weaponid::Scout_m_CurrentEvent:
    case weaponid::Scout_m_DiamondBotkillerScattergunMkI:
    case weaponid::Scout_m_FestiveForceANature:
    case weaponid::Scout_m_FestiveScattergun:
    case weaponid::Scout_m_FlowerPower:
    case weaponid::Scout_m_ForceANature:
    case weaponid::Scout_m_GoldBotkillerScattergunMkI:
    case weaponid::Scout_m_GoldBotkillerScattergunMkII:
    case weaponid::Scout_m_KillerBee:
    case weaponid::Scout_m_MacabreWeb:
    case weaponid::Scout_m_NightTerror:
    case weaponid::Scout_m_Nutcracker:
    case weaponid::Scout_m_RustBotkillerScattergunMkI:
    case weaponid::Scout_m_Scattergun:
    case weaponid::Scout_m_ScattergunR:
    case weaponid::Scout_m_ShottoHell:
    case weaponid::Scout_m_SilverBotkillerScattergunMkI:
    case weaponid::Scout_m_SilverBotkillerScattergunMkII:
    case weaponid::Scout_m_SpruceDeuce:
    case weaponid::Scout_m_TartanTorpedo:
    case weaponid::Scout_m_TheBackScatter:
    case weaponid::Scout_m_TheShortstop:
    case weaponid::Scout_m_TheSodaPopper:
    case weaponid::Soldier_s_SoldiersShotgun:
    case weaponid::Heavy_s_HeavysShotgun:
    case weaponid::Pyro_s_PyrosShotgun: {
      /*repeat skins
      case weaponid::Heavy_s_Autumn:
      case weaponid::Heavy_s_BackwoodsBoomstick:
      case weaponid::Heavy_s_CivicDuty:
      case weaponid::Heavy_s_CoffinNail:
      case weaponid::Heavy_s_DressedtoKill:
      case weaponid::Heavy_s_FestiveShotgun:
      case weaponid::Heavy_s_FlowerPower:
      case weaponid::Heavy_s_LightningRod:
      case weaponid::Heavy_s_PanicAttack:
      case weaponid::Heavy_s_RedBear:
      case weaponid::Heavy_s_RusticRuiner:
      case weaponid::Heavy_s_ShotgunR:
      case weaponid::Pyro_s_Autumn:
      case weaponid::Pyro_s_BackwoodsBoomstick:
      case weaponid::Pyro_s_CivicDuty:
      case weaponid::Pyro_s_CoffinNail:
      case weaponid::Pyro_s_DressedtoKill:
      case weaponid::Pyro_s_FestiveShotgun:
      case weaponid::Pyro_s_FlowerPower:
      case weaponid::Pyro_s_LightningRod:
      case weaponid::Pyro_s_PanicAttack:
      case weaponid::Pyro_s_RedBear:
      case weaponid::Pyro_s_RusticRuiner:
      case weaponid::Pyro_s_ShotgunR:
      case weaponid::Soldier_s_Autumn:
      case weaponid::Soldier_s_BackwoodsBoomstick:
      case weaponid::Soldier_s_CivicDuty:
      case weaponid::Soldier_s_CoffinNail:
      case weaponid::Soldier_s_DressedtoKill:
      case weaponid::Soldier_s_FestiveShotgun:
      case weaponid::Soldier_s_FlowerPower:
      case weaponid::Soldier_s_LightningRod:
      case weaponid::Soldier_s_PanicAttack:
      case weaponid::Soldier_s_RedBear:
      case weaponid::Soldier_s_RusticRuiner:
      case weaponid::Soldier_s_ShotgunR:
      case weaponid::Soldier_s_TheReserveShooter:
      */
      dist = gCvars.Aimbot_range.value ? gCvars.Aimbot_ranges.value : 9999.0f;
      break;
    }
    
    default: {
      dist = 9999.0f;
      break;
    }
    }
  }
  bool projSetup( float &speed, float &chargetime, float &gravity, bool &quick_release, weaponid id, CBaseCombatWeapon *wpn ) {
    //
    switch( id ) {
    //ignore
    //practice
    case weaponid::Soldier_m_RocketJumper:
    case weaponid::Demoman_s_StickyJumper:
    
    //buff
    case weaponid::Scout_s_CritaCola:
    case weaponid::Scout_s_BonkAtomicPunch:
    case weaponid::Scout_s_FestiveBonk:
    case weaponid::Soldier_s_TheBuffBanner:
    case weaponid::Soldier_s_FestiveBuffBanner:
    case weaponid::Soldier_s_TheConcheror:
    case weaponid::Soldier_s_TheBattalionsBackup:
    case weaponid::Pyro_s_ThermalThruster:
    
    //heavy food
    case weaponid::Heavy_s_Sandvich:
    case weaponid::Heavy_s_RoboSandvich:
    case weaponid::Heavy_s_FestiveSandvich:
    case weaponid::Heavy_s_TheBuffaloSteakSandvich:
    case weaponid::Heavy_s_SecondBanana:
    case weaponid::Heavy_s_Fishcake:
    case weaponid::Heavy_s_TheDalokohsBar:
    
    //engi + spy misc
    case weaponid::Engi_p_ConstructionPDA:
    case weaponid::Engi_p_ConstructionPDAR:
    case weaponid::Engi_p_DestructionPDA:
    case weaponid::Engi_p_PDA:
    case weaponid::Spy_s_Sapper:
    case weaponid::Spy_s_SapperR:
    case weaponid::Spy_s_TheRedTapeRecorder:
    case weaponid::Spy_s_TheRedTapeRecorderG:
    case weaponid::Spy_s_TheApSapG:
    case weaponid::Spy_s_FestiveSapper:
    case weaponid::Spy_s_TheSnackAttack:
    case weaponid::Spy_d_DisguiseKitPDA:
    case weaponid::Spy_w_InvisWatch:
    case weaponid::Spy_w_InvisWatchR:
    case weaponid::Spy_w_TheDeadRinger:
    case weaponid::Spy_w_TheCloakandDagger:
    case weaponid::Spy_w_EnthusiastsTimepiece:
    case weaponid::Spy_w_TheQuackenbirdt:
    
    //medic guns
    case weaponid::Medic_s_MediGun:
    case weaponid::Medic_s_MediGunR:
    case weaponid::Medic_s_TheKritzkrieg:
    case weaponid::Medic_s_TheQuickFix:
    case weaponid::Medic_s_FestiveMediGun:
    case weaponid::Medic_s_SilverBotkillerMediGunMkI:
    case weaponid::Medic_s_GoldBotkillerMediGunMkI:
    case weaponid::Medic_s_RustBotkillerMediGunMkI:
    case weaponid::Medic_s_BloodBotkillerMediGunMkI:
    case weaponid::Medic_s_CarbonadoBotkillerMediGunMkI:
    case weaponid::Medic_s_DiamondBotkillerMediGunMkI:
    case weaponid::Medic_s_SilverBotkillerMediGunMkII:
    case weaponid::Medic_s_GoldBotkillerMediGunMkII:
    case weaponid::Medic_s_TheVaccinator:
    case weaponid::Medic_s_MaskedMender:
    case weaponid::Medic_s_WrappedReviver:
    case weaponid::Medic_s_ReclaimedReanimator:
    case weaponid::Medic_s_CivilServant:
    case weaponid::Medic_s_SparkofLife:
    case weaponid::Medic_s_Wildwood:
    case weaponid::Medic_s_FlowerPower:
    case weaponid::Medic_s_DressedToKill:
    case weaponid::Medic_s_HighRollers:
    case weaponid::Medic_s_CoffinNail:
    case weaponid::Medic_s_Blitzkrieg:
    case weaponid::Medic_s_Corsair: {
      return false;
    }
    
    //charge weap
    case weaponid::Sniper_m_TheClassic: {
      quick_release = true;
      break;
    }
    
    //activate weap
    case weaponid::Demoman_s_BloodBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_CarbonadoBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_DiamondBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_FestiveStickybombLauncher:
    case weaponid::Demoman_s_GoldBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_GoldBotkillerStickybombLauncherMkII:
    case weaponid::Demoman_s_RustBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_SilverBotkillerStickybombLauncherMkI:
    case weaponid::Demoman_s_SilverBotkillerStickybombLauncherMkII:
    case weaponid::Demoman_s_StickybombLauncher:
    case weaponid::Demoman_s_StickybombLauncherR:
    case weaponid::Demoman_s_TheQuickiebombLauncher:
    case weaponid::Demoman_s_TheScottishResistance: {
      speed = 900.0f;
      quick_release = true;
      break;
    }
    
    case weaponid::Sniper_m_TheHuntsman:
    case weaponid::Sniper_m_FestiveHuntsman:
    case weaponid::Sniper_m_TheFortifiedCompound: {
      chargetime = gInts.globals->curtime - wpn->GetChargeTime();
      speed = ( fminf( fmaxf( chargetime, 0.0f ), 1.0f ) * 800.0f ) + 1800.0f;
      gravity = ( fminf( fmaxf( chargetime, 0.0f ), 1.0f ) * -0.4f ) + 0.5f;
      quick_release = true;
      break;
    }
    
    case weaponid::Scout_s_TheFlyingGuillotine:
    case weaponid::Scout_s_TheFlyingGuillotineG:
    case weaponid::Pyro_m_DragonsFury: {
      speed = 3000;
      break;
    }
    
    case weaponid::Engi_m_TheRescueRanger:
    case weaponid::Medic_m_FestiveCrusadersCrossbow:
    case weaponid::Medic_m_CrusadersCrossbow: {
      speed = 2400;
      gravity = 0.2f;
      break;
    }
    
    case weaponid::Pyro_s_TheDetonator:
    case weaponid::Pyro_s_TheFlareGun:
    case weaponid::Pyro_s_FestiveFlareGun:
    case weaponid::Pyro_s_TheScorchShot:
    case weaponid::Pyro_s_TheManmelter: {
      speed = 2000;
      break;
    }
    
    case weaponid::Soldier_m_TheDirectHit: {
      speed = 1980;
      break;
    }
    
    case weaponid::Soldier_m_TheLibertyLauncher: {
      speed = 1545;
      break;
    }
    
    case weaponid::Demoman_m_TheLooseCannon: {
      speed = 1450;
      gravity = 0.4f;
      quick_release = true;
      break;
    }
    
    case weaponid::Demoman_m_TheLochnLoad: {
      speed = 1510;
      gravity = 0.4f;
      break;
    }
    
    case weaponid::Demoman_m_GrenadeLauncher:
    case weaponid::Demoman_m_GrenadeLauncherR:
    case weaponid::Demoman_m_FestiveGrenadeLauncher:
    case weaponid::Demoman_m_TheIronBomber:
    case weaponid::Demoman_m_Autumn:
    case weaponid::Demoman_m_MacabreWeb:
    case weaponid::Demoman_m_Rainbow:
    case weaponid::Demoman_m_SweetDreams:
    case weaponid::Demoman_m_CoffinNail:
    case weaponid::Demoman_m_TopShelf:
    case weaponid::Demoman_m_Warhawk:
    case weaponid::Demoman_m_ButcherBird: {
      speed = 1215;
      gravity = 0.4f;
      break;
    }
    
    case weaponid::Soldier_s_TheRighteousBison:
    case weaponid::Engi_m_ThePomson6000: {
      speed = 1200;
      break;
    }
    
    case weaponid::Soldier_m_RocketLauncher:
    case weaponid::Soldier_m_RocketLauncherR:
    case weaponid::Soldier_m_TheBlackBox:
    case weaponid::Soldier_m_TheCowMangler5000:
    case weaponid::Soldier_m_TheOriginal:
    case weaponid::Soldier_m_FestiveRocketLauncher:
    case weaponid::Soldier_m_SilverBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_GoldBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_RustBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_BloodBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_CarbonadoBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_DiamondBotkillerRocketLauncherMkI:
    case weaponid::Soldier_m_SilverBotkillerRocketLauncherMkII:
    case weaponid::Soldier_m_GoldBotkillerRocketLauncherMkII:
    case weaponid::Soldier_m_FestiveBlackBox:
    case weaponid::Soldier_m_TheAirStrike:
    case weaponid::Soldier_m_WoodlandWarrior:
    case weaponid::Soldier_m_SandCannon:
    case weaponid::Soldier_m_AmericanPastoral:
    case weaponid::Soldier_m_SmalltownBringdown:
    case weaponid::Soldier_m_ShellShocker:
    case weaponid::Soldier_m_AquaMarine:
    case weaponid::Soldier_m_Autumn:
    case weaponid::Soldier_m_BlueMew:
    case weaponid::Soldier_m_BrainCandy:
    case weaponid::Soldier_m_CoffinNail:
    case weaponid::Soldier_m_HighRollers:
    case weaponid::Soldier_m_Warhawk: {
      speed = 1100;
      break;
    }
    
    case weaponid::Soldier_m_TheBeggarsBazooka: {
      quick_release = true;
      speed = 1100;
      break;
    }
    
    case weaponid::Scout_s_MadMilk:
    case weaponid::Scout_s_MutatedMilk:
    case weaponid::Sniper_s_Jarate:
    case weaponid::Sniper_s_FestiveJarate:
    case weaponid::Pyro_s_GasPasser: {
      speed = 1020;
      gravity = 0.5f;
      break;
    }
    
    case weaponid::Medic_m_SyringeGun:
    case weaponid::Medic_m_SyringeGunR:
    case weaponid::Medic_m_TheBlutsauger:
    case weaponid::Medic_m_TheOverdose: {
      speed = 1000;
      break;
    }
    
    default: {
      break;
    }
    }
    
    return true;
  }
  
  Color team_color( CBaseEntity *pLocal, CBaseEntity *pEntity ) {
    if( gCvars.color_type.value == 0 ) { //red/blue
      if( gCvars.aim_index == pEntity->GetIndex() && ( gCvars.ESP_target.value > 1 ) ) {
        return gCvars.color_aim.get_color();
      } else if( pEntity->GetTeamNum() == 2 || ( pEntity->GetTeamNum() == 3 && gCvars.Ignore_E_disguise.value &&  pEntity->GetCond() & TFCond_Disguised ) ) {
        return gCvars.color_red.get_color();
      } else if( pEntity->GetTeamNum() == 3 || ( pEntity->GetTeamNum() == 2 && gCvars.Ignore_E_disguise.value &&  pEntity->GetCond() & TFCond_Disguised ) ) {
        return gCvars.color_blue.get_color();
      }
      
      return Colors::White;
    } else { //ally/enemy
      if( gCvars.aim_index == pEntity->GetIndex() && ( gCvars.ESP_target.value > 1 ) ) {
        return gCvars.color_aim.get_color();
      } else if( pEntity->GetTeamNum() == pLocal->GetTeamNum() ) {
        return gCvars.color_ally.get_color();
      } else {
        return gCvars.color_enemy.get_color();
      }
      
      return Colors::White;
    }
  }
}
