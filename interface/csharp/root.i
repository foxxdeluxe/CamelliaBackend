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
%shared_ptr(camellia::stage_data)

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

%warnfilter(473) stage;
%warnfilter(473) dialog;
%include "camellia.h"