%module(directors="1") CamelliaBackend

%include "std_map.i"
%include "std_vector.i"
%include "std_shared_ptr.i"

%include "utf8_ex_str.swg"
%include "utf8_std_string.i"



%{
#include "manager.h"
#include "attribute_registry.h"
#include "variant.h"

// live
#include "live/play/stage.h"

// data
#include "data/play/stage_data.h"
#include "data/action/continuous/modifier_action_data.h"
#include "data/action/instant/instant_action_data.h"
#include "data/basic/curve_data.h"

#include "helper/scripting_helper.h"
#include "helper/algorithm_helper.h"
%}

%include "global.h"


namespace camellia {
        class actor;
        class dialog;
        class variant;

        struct action_timeline_keyframe_data;
        struct activity_data;
        struct curve_point_data;
        struct action_timeline_data;
        struct action_timeline_track_data;
        struct dialog_data;
        struct text_region_data;
        struct beat_data;
        struct actor_data;
        struct stage_data;
        struct action_data;
        struct continuous_action_data;
        struct modifier_action_data;
        struct instant_action_data;
        struct text_region_attachment;
}

%shared_ptr(camellia::action_data)
%shared_ptr(camellia::continuous_action_data)
%shared_ptr(camellia::modifier_action_data)
%shared_ptr(camellia::instant_action_data)

namespace std {
        %template(TextValueMap) map<camellia::text_t, camellia::variant>;
        %template(KeyframeVector) vector<camellia::action_timeline_keyframe_data>;
        %template(TrackVector) vector<camellia::action_timeline_track_data>;
        %template(CurvePointDataVector) vector<camellia::curve_point_data>;
        %template(ActivityMap) map<camellia::integer_t, camellia::activity_data>;
        %template(AttachmentVector) vector<camellia::text_region_attachment>;
        %template(TextRegionDataVector) vector<camellia::text_region_data>;
        %template(BeatDataVector) vector<camellia::beat_data>;
        %template(ActorDataMap) map<camellia::hash_t, camellia::actor_data>;
        %template(ActionDataMap) map<camellia::hash_t, shared_ptr<camellia::action_data>>;
        %template(ScriptMap) map<camellia::hash_t, camellia::text_t>;
        %template(HashVariantMap) map<camellia::hash_t, camellia::variant>;
        %template(ByteVector) vector<unsigned char>;
}

%exception {
    try
    {
        $action
    }
    catch(const std::exception &e) {
        SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.what());
    }
    catch(...)
    {
        SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, "Unknown error");
    }
}

%rename("%(camelcase)s", %$isclass) "";
%rename("%(camelcase)s", %$isenum) "";
%rename("%(camelcase)s", %$isfunction) "";
%rename("%(lowercamelcase)s", %$isvariable) "";
%rename(Eq) operator==;
%rename(Ue) operator!=;
%rename(Assign) operator=;
%rename(GetInteger) operator integer_t;
%rename(GetNumber) operator number_t;
%rename(GetBoolean) operator boolean_t;

namespace camellia {
        %feature("director") manager;
        %feature("director") stage;
        %feature("director") dialog;
        %feature("director") text_region;
        %feature("director") actor;
}

%include "../include/rc_obj.h"
%include "../include/manager.h"
%include "../include/attribute_registry.h"
%include "../include/variant.h"

// live
%include "../include/live/play/timeline_evaluator.h"
%include "../include/live/play/stage.h"
%include "../include/live/play/dialog.h"
%include "../include/live/play/actor.h"

// data
%include "../include/data/basic/curve_data.h"

%include "../include/data/play/stage_data.h"
%include "../include/data/play/dialog_data.h"
%include "../include/data/play/beat_data.h"
%include "../include/data/play/actor_data.h"
%include "../include/data/play/activity_data.h"

%include "../include/data/action/action_timeline_data.h"
%include "../include/data/action/action_data.h"
%include "../include/data/action/continuous/continuous_action_data.h"
%include "../include/data/action/continuous/modifier_action_data.h"
%include "../include/data/action/instant/instant_action_data.h"

// helpers
%include "../include/helper/scripting_helper.h"
%include "../include/helper/algorithm_helper.h"