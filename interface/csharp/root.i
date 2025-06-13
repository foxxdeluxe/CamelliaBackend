%module(directors="1") CamelliaBackend

%include "std_map.i"
%include "std_vector.i"
%include "std_shared_ptr.i"

%include "utf8_ex_str.swg"
%include "utf8_std_string.i"



%{
#include "camellia.h"
%}

%include "camellia_typedef.h"

// Forward declarations
namespace camellia {
// From attribute_registry.h
class live_object;
class dirty_attribute_handler;
class attribute_registry;

// From data/stage_data.h
struct action_timeline_data;
struct action_data;
struct action_timeline_keyframe_data;
struct action_timeline_track_data;
struct modifier_action_data;
struct composite_action_data;
struct curve_point_data;
struct curve_data;
struct activity_data;
struct actor_data;
struct text_region_attachment_data;
struct text_region_attachment_text_data;
struct text_region_data;
struct dialog_data;
struct beat_data;
struct stage_data;

// From helper/scripting_helper.h
namespace scripting_helper {
class engine;
}

// From helper/text_layout_helper.h
namespace text_layout_helper {
class text_layout_engine;
class text_paragraph;
class paragraph_style;
class text_style;
}

// From live/action/action.h
class action_timeline_keyframe;
class action;
class modifier_action;
class composite_action;

// From live/action/action_timeline.h
class stage;
class action_timeline;

// From live/play/activity.h
class activity;

// From live/play/actor.h
class actor;

// From live/play/dialog.h
class dialog;
class text_region;

// From live/play/stage.h (stage already declared above)

// From manager.h
class manager;

// From variant.h
struct vector2;
struct vector3;
struct vector4;
class variant;
} // namespace camellia

%shared_ptr(camellia::action_data)
%shared_ptr(camellia::modifier_action_data)
%shared_ptr(camellia::composite_action_data)
%shared_ptr(camellia::stage_data)
%shared_ptr(camellia::action_timeline_keyframe_data)
%shared_ptr(camellia::action_timeline_track_data)
%shared_ptr(camellia::curve_point_data)
%shared_ptr(camellia::action_timeline_data)
%shared_ptr(camellia::activity_data)
%shared_ptr(camellia::action_timeline_data)
%shared_ptr(camellia::text_region_attachment_data)
%shared_ptr(camellia::text_region_attachment_text_data)
%shared_ptr(camellia::text_region_data)
%shared_ptr(camellia::beat_data)
%shared_ptr(camellia::actor_data)
%shared_ptr(camellia::dialog_data)

namespace std {
        %template(TextValueMap) map<camellia::text_t, camellia::variant>;
        %template(KeyframeVector) vector<shared_ptr<camellia::action_timeline_keyframe_data>>;
        %template(TrackVector) vector<shared_ptr<camellia::action_timeline_track_data>>;
        %template(CurvePointDataVector) vector<shared_ptr<camellia::curve_point_data>>;
        %template(ActivityMap) map<camellia::integer_t, shared_ptr<camellia::activity_data>>;
        %template(AttachmentVector) vector<shared_ptr<camellia::text_region_attachment_data>>;
        %template(TextRegionDataVector) vector<shared_ptr<camellia::text_region_data>>;
        %template(BeatDataVector) vector<shared_ptr<camellia::beat_data>>;
        %template(ActorDataMap) map<camellia::hash_t, shared_ptr<camellia::actor_data>>;
        %template(ActionDataMap) map<camellia::hash_t, shared_ptr<camellia::action_data>>;
        %template(ScriptMap) map<camellia::hash_t, camellia::text_t>;
        %template(HashVariantMap) map<camellia::hash_t, camellia::variant>;
        %template(ByteVector) vector<unsigned char>;
        %template(VariantVector) vector<camellia::variant>;
        %template(DataVector) vector<camellia::bytes_t>;
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

%warnfilter(473) stage;
%warnfilter(473) dialog;
%include "camellia.h"
%include "camellia_macro.h"
%include "attribute_registry.h"
%include "data/stage_data.h"
%include "helper/algorithm_helper.h"
%include "helper/scripting_helper.h"
%include "helper/text_layout_helper.h"
%include "live/action/action.h"
%include "live/action/action_timeline.h"
%include "live/play/activity.h"
%include "live/play/actor.h"
%include "live/play/dialog.h"
%include "live/play/stage.h"
%include "manager.h"
%include "variant.h"