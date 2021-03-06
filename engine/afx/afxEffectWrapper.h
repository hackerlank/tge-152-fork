
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX
// Copyright (C) Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_EFFECT_WRAPPER_H_
#define _AFX_EFFECT_WRAPPER_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"
#include "afxEffectDefs.h"
#include "afxConstraint.h"

class afxEffectWrapperData;

class afxEffectAdapterDesc
{
private:
  static Vector<afxEffectAdapterDesc*>* adapters;

public:
  /*C*/         afxEffectAdapterDesc();

  virtual bool  testEffectType(const SimDataBlock*) const=0;
  virtual bool  requiresStop(const afxEffectWrapperData*) const=0;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const=0;
  virtual bool  runsOnClient(const afxEffectWrapperData*) const=0;
  virtual void  prepEffect(afxEffectWrapperData*) const { }

  virtual afxEffectWrapper* create() const=0;

  static bool   identifyEffect(afxEffectWrapperData*);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

struct afxEffectTimingData
{
  F32 delay;
  F32 lifetime;
  F32 fade_in_time;
  F32 fade_out_time;
  F32 residue_lifetime;   
  F32 residue_fadetime;  

  afxEffectTimingData()
  {
    delay = 0;
    lifetime = 0;
    fade_in_time = 0;
    fade_out_time = 0;
    residue_lifetime = 0;
    residue_fadetime = 0;
  }
};

class afxXM_BaseData;

class afxEffectWrapperData : public GameBaseData, public afxEffectDefs
{
  typedef GameBaseData  Parent;

  bool                  do_id_convert;

public:
  enum  { MAX_CONDITION_STATES = 4 };

public:
  StringTableEntry      effect_name; 
  bool                  use_as_cons_obj;    
  bool                  use_ghost_as_cons_obj;    

  StringTableEntry      cons_spec; 
  StringTableEntry      pos_cons_spec;
  StringTableEntry      orient_cons_spec;
  StringTableEntry      aim_cons_spec;
  StringTableEntry      life_cons_spec;
  //
  afxConstraintDef      cons_def;
  afxConstraintDef      pos_cons_def;
  afxConstraintDef      orient_cons_def;
  afxConstraintDef      aim_cons_def;
  afxConstraintDef      life_cons_def;

  afxEffectTimingData   timing;
  
  F32                   scale_factor;       // scale size if applicable
  F32                   rate_factor;        // scale rate if applicable
  F32                   user_fade_out_time;

  bool                  is_looping;
  U32                   n_loops;
  F32                   loop_gap_time;

  bool                  ignore_time_factor;
  bool                  propagate_time_factor;

  ByteRange             ranking_range;
  ByteRange             lod_range;
  S32                   life_conds;
  U32                   exec_cond_on_bits[MAX_CONDITION_STATES];
  U32                   exec_cond_off_bits[MAX_CONDITION_STATES];
  U32                   exec_cond_bitmasks[MAX_CONDITION_STATES];

  S32                   data_ID;

  afxXM_BaseData*       xfm_modifiers[MAX_XFM_MODIFIERS];

  Box3F                 forced_bbox;
  bool                  update_forced_bbox;

  S8                    sort_priority;
  Point3F               direction;
  F32                   speed;
  F32                   mass;

  SimDataBlock*         effect_data;
  afxEffectAdapterDesc* effect_desc;

  void                  gather_cons_defs(Vector<afxConstraintDef>& defs);
  void                  pack_mods(BitStream*, afxXM_BaseData* mods[], bool packed);
  void                  unpack_mods(BitStream*, afxXM_BaseData* mods[]);

public:
  /*C*/             afxEffectWrapperData();

  virtual bool      onAdd();
  virtual void      packData(BitStream*);
  virtual void      unpackData(BitStream*);
  virtual bool      preload(bool server, char errorBuffer[256]);

  bool              requiresStop() { return effect_desc->requiresStop(this); }
  bool              runsOnServer() { return effect_desc->runsOnServer(this); }
  bool              runsOnClient() { return effect_desc->runsOnClient(this); }
  bool              runsHere(bool server_here) { return (server_here) ? runsOnServer() : runsOnClient(); }
  bool              testExecConditions(U32 conditions);

  F32               afterStopTime() { return timing.fade_out_time; }

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxEffectWrapperData);
};
DECLARE_CONSOLETYPE(afxEffectWrapperData);

inline bool afxEffectWrapperData::testExecConditions(U32 conditions)
{
  if (exec_cond_bitmasks[0] == 0)
    return true;

  if ((exec_cond_bitmasks[0] & conditions) == exec_cond_on_bits[0])
    return true;

  for (S32 i = 1; i < MAX_CONDITION_STATES; i++)
  {
    if (exec_cond_bitmasks[i] == 0)
      return false;
    if ((exec_cond_bitmasks[i] & conditions) == exec_cond_on_bits[i])
      return true;
  }
  return false;
}

typedef Vector<afxEffectWrapperData*> afxEffectList;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectWrapper 
//
//  NOTE -- this not a subclass of GameBase... it is only meant to exist on
//    the client-side.

class ShapeBase;
class GameBase;
class TSShape;
class TSShapeInstance;
class afxConstraint;
class afxConstraintMgr;
class afxChoreographer;
class afxXM_Base;

class afxEffectWrapper : public SimObject,  public afxEffectDefs
{
  typedef SimObject Parent;
  friend class afxEffectVector;

private:
  bool              test_life_conds();

protected:
  afxEffectWrapperData* datablock;

  afxEffectTimingData   ew_timing;

  F32               fade_in_end;
  F32               fade_out_start;
  F32               full_lifetime;

  F32               time_factor;
  F32               prop_time_factor;

  afxChoreographer* choreographer;
  afxConstraintMgr* cons_mgr;

  afxConstraintID   pos_cons_id;
  afxConstraintID   orient_cons_id;
  afxConstraintID   aim_cons_id;
  afxConstraintID   life_cons_id;

  afxConstraintID   effect_cons_id;

  F32               elapsed;
  F32               life_elapsed;
  F32               life_end;
  bool              stopped;
  bool              cond_alive;

  U32               n_updates;

  MatrixF           updated_xfm;
  Point3F           updated_pos;
  Point3F           updated_aim;
  Point3F           updated_scale;

  Point3F           updated_direction;
  F32               fade_value;

  bool              do_fades;
  bool              in_scope;
  bool              is_aborted;

  U8                effect_flags;

  afxXM_Base*       xfm_modifiers[MAX_XFM_MODIFIERS];

  F32               live_scale_factor;
  F32               live_fade_factor;

public:
  /*C*/             afxEffectWrapper();
  virtual           ~afxEffectWrapper();

  void              ew_init(afxChoreographer*, afxEffectWrapperData*, afxConstraintMgr*, 
                            F32 time_factor);

  F32               getFullLifetime() { return ew_timing.lifetime + ew_timing.fade_out_time; }
  F32               getTimeFactor() { return time_factor; }
  afxConstraint*    getPosConstraint() { return cons_mgr->getConstraint(pos_cons_id); }
  afxConstraint*    getOrientConstraint() { return cons_mgr->getConstraint(orient_cons_id); }
  afxConstraint*    getAimConstraint() { return cons_mgr->getConstraint(aim_cons_id); }
  afxConstraint*    getLifeConstraint() { return cons_mgr->getConstraint(life_cons_id); }
  afxChoreographer* getChoreographer() { return choreographer; }

  virtual bool      isDone();
  virtual bool      deleteWhenStopped() { return false; }
  F32               afterStopTime() { return ew_timing.fade_out_time; } 
  bool              isAborted() const { return is_aborted; }

  void              prestart();
  bool              start(F32 timestamp);
  bool              update(F32 dt);
  void              stop();
  void              cleanup(bool was_stopped=false);
  void              setScopeStatus(bool flag);

  virtual void      ea_set_datablock(SimDataBlock*) { }
  virtual bool      ea_start() { return true; };
  virtual bool      ea_update(F32 dt) { return true; };
  virtual void      ea_finish(bool was_stopped) { };
  virtual void      ea_set_scope_status(bool flag) {  };

  void              getUpdatedPosition(Point3F& pos) { pos = updated_pos;}
  void              getUpdatedTransform(MatrixF& xfm) { xfm = updated_xfm; }
  void              getUpdatedScale(Point3F& scale) { scale = updated_scale; }
  virtual void      getUpdatedBoxCenter(Point3F& pos) { pos = updated_pos;}

  virtual void      getUnconstrainedPosition(Point3F& pos) { pos.zero();}
  virtual void      getUnconstrainedTransform(MatrixF& xfm) { xfm.identity(); }

  F32               getMass() { return datablock->mass; }
  Point3F           getDirection() { return datablock->direction; } ///return updated_direction; }
  F32               getSpeed() { return datablock->speed; }

  virtual TSShape*          getTSShape() { return 0; }
  virtual TSShapeInstance*  getTSShapeInstance() { return 0; }

public:
  static void       initPersistFields();

  static afxEffectWrapper* ew_create(afxChoreographer*, afxEffectWrapperData*, 
                                     afxConstraintMgr*, F32 time_factor);

  DECLARE_CONOBJECT(afxEffectWrapper);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_EFFECT_WRAPPER_H_
