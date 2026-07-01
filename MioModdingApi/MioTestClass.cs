using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace PdbToCSharp;

public static class MioTestClass {
  public static long ModuleBase = 0;
  public static unsafe Game* game {
    get {
      if (ModuleBase == 0) {
        throw new InvalidOperationException($"ModuleBase is not set. This must be set before accessing {nameof(game)}.");
      }
      return (Game*)(ModuleBase + 0x10EFBF0);
    }
  }
}

[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 170880)]
public unsafe struct Game {
  [FieldOffset(0)] public bool ghost_safety;
  [FieldOffset(4)] public float ghost_timer;
  [FieldOffset(8)] public String save_file_name;
  [FieldOffset(24)] public bool interpolate;
  [FieldOffset(72)] public ulong session_id;
  [FieldOffset(80)] public String build_id;
  [FieldOffset(96)] public Mio mio;
  [FieldOffset(142520)] public bool grid_secondary_generated;
  [FieldOffset(145192)] public bool the_one_boolean;
  [FieldOffset(145193)] public bool revert_spider_changes;
  [FieldOffset(146516)] public bool first_game_loop_frame;
  [FieldOffset(146517)] public bool first_sim_frame;
  [FieldOffset(146524)] public uint frame;
  [FieldOffset(146528)] public uint sim_frame;
  [FieldOffset(146532)] public uint gameplay_frame;
  [FieldOffset(146536)] public float time_sim;
  [FieldOffset(146540)] public bool first_sim_step;
  [FieldOffset(146541)] public bool last_sim_step;
  [FieldOffset(146544)] public int sim_steps;
  [FieldOffset(146776)] public bool first_run;
  [FieldOffset(146777)] public bool project_fight_ready;
  [FieldOffset(146778)] public bool true_ending;
  [FieldOffset(146779)] public bool gamescom_demo;
  [FieldOffset(146780)] public float gamescom_demo_end_timer;
  [FieldOffset(146784)] public float gamescom_demo_graveyard_timer;
  [FieldOffset(146788)] public float gamescom_demo_statue_timer;
  [FieldOffset(146792)] public bool nextfest_demo;
  [FieldOffset(146796)] public float nextfest_demo_statue_timer;
  [FieldOffset(146800)] public bool true_ending_mode;
  [FieldOffset(149400)] public String guardian_angel_save_entry;
  [FieldOffset(150056)] public bool mio_in_boss_fight;
  [FieldOffset(150292)] public bool mio_win_fight;
  [FieldOffset(151088)] public String audio_player_state;
  [FieldOffset(151104)] public Game__Kind_menu current_menu;
  [FieldOffset(151108)] public Game__Kind_menu next_menu;
  [FieldOffset(151112)] public Node2* closing_menu_ui_ref;
  [FieldOffset(151128)] public bool audio_zero_g;
  [FieldOffset(151132)] public Game__Audio_spine_param audio_spine_param;
  [FieldOffset(159500)] public float halyn_buffer_y_m;
  [FieldOffset(159504)] public float halyn_buffer_y_M;
  [FieldOffset(160192)] public double last_saved_time;
  [FieldOffset(160312)] public bool open_trinkets_now;
  [FieldOffset(160313)] public bool open_map_now;
  [FieldOffset(160314)] public bool open_map_at_hub_now;
  [FieldOffset(160315)] public bool open_travel_now;
  [FieldOffset(160316)] public bool open_demo_endscreen_now;
  [FieldOffset(160317)] public bool open_demo_true_endscreen_now;
  [FieldOffset(160318)] public bool open_inventory_now;
  [FieldOffset(160392)] public String last_displayed_title_id;
  [FieldOffset(160576)] public String current_zone_id;
  [FieldOffset(160596)] public bool sim_paused;
  [FieldOffset(160597)] public bool pause_request;
  [FieldOffset(169856)] public float dt_frame;
  [FieldOffset(169860)] public float dt_accum;
  [FieldOffset(169864)] public float sim_frac;
  [FieldOffset(169868)] public float time_warp;

  /// Struct type: Game::Init_flags (.?AW4Init_flags@Game@@)
  [CompilerGenerated]
  public enum Game__Init_flags : int {
    Init_none = 0,
    Init_default_save = 1,
    Init_new_save = 2,
    Init_load_save = 4,
    Init_gamescom_demo = 8,
    Init_nextfest_demo = 16,
    Init_true_ending = 32
  }

  /// Struct type: Game::Kind_menu (.?AW4Kind_menu@Game@@)
  [CompilerGenerated]
  public enum Game__Kind_menu : int {
    None = 0,
    System = 1,
    Tab_menu = 2,
    Workshop = 3,
    Fast_travel = 4,
    Demo = 5
  }

  /// Struct type: Game::Audio_spine_param (.?AW4Audio_spine_param@Game@@)
  [CompilerGenerated]
  public enum Game__Audio_spine_param : int {
    Deconnected = 0,
    Start_action = 1,
    Connected = 2
  }

  /// Struct type: Game::Gameplay_ruler (.?AW4Gameplay_ruler@Game@@)
  [CompilerGenerated]
  public enum Game__Gameplay_ruler : int {
    Jump_apogee = 1,
    Jump_curve = 2,
    Navigation_space = 4,
    Traversal_space = 8,
    Hook_distance = 16,
    Hook_angle = 32,
    Camera_span = 64
  }
}

/// Struct type: String (.?AUString@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 16)]
public unsafe struct String {
  [FieldOffset(0)] public Ptr data;
  [FieldOffset(8)] public uint size;
  [FieldOffset(12)] public uint is_static;

  // Manually added so we in C# land can see what this string is
  public ReadOnlySpan<char> AsSpan {
    get {
      if (data.data == null || size == 0) {
        return ReadOnlySpan<char>.Empty;
      }
      return new ReadOnlySpan<char>(data.data, (int)size);
    }
  }
}

/// Struct type: Ptr (.?AUPtr@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 8)]
public unsafe struct Ptr {
  [FieldOffset(0)] public byte* data;
}

/// Struct type: Mio (.?AUMio@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 141832)]
public unsafe struct Mio {
  [FieldOffset(760)] public Node2* node;
  [FieldOffset(768)] public Node2* look_node;
  [FieldOffset(776)] public Node2* rig_node;
  [FieldOffset(784)] public Node2* torch_node;
  [FieldOffset(792)] public Node2* combat_markup_node;
  [FieldOffset(800)] public Node2* head_node;
  [FieldOffset(808)] public Node2* left_foot_node;
  [FieldOffset(816)] public Node2* right_foot_node;
  [FieldOffset(1144)] public Vector3 velocity;
  [FieldOffset(1156)] public Vector3 buffered_velocity;
  [FieldOffset(1168)] public Filt_Vector3 smooth_velocity;
  [FieldOffset(1200)] public Filt_float foliage_velocity;
  [FieldOffset(1216)] public float rest_look_angle;
  [FieldOffset(1220)] public bool has_rest_look_target;
  [FieldOffset(1224)] public Vector3 rest_look_target;
  [FieldOffset(1240)] public Node2* ultra_forced_look_target;
  [FieldOffset(1248)] public Vector3 look_dir;
  [FieldOffset(1256)] public float look_angle;
  [FieldOffset(1312)] public Mio__Life life;
  [FieldOffset(2752)] public float PARAMS;
  [FieldOffset(2832)] public float evil_mio;
  [FieldOffset(2836)] public bool hair_enabled;
  [FieldOffset(2837)] public bool trailer_rig;
  [FieldOffset(2840)] public Vector3 random_hair_force;
  [FieldOffset(2856)] public Node2* cosmetic_look_target;
  [FieldOffset(2968)] public bool mio_is_down;
  [FieldOffset(139112)] public bool lazy_mio;
  [FieldOffset(139113)] public bool child_mio;
  [FieldOffset(139116)] public float child_mio_afraid;
  [FieldOffset(139120)] public Node2* sasuke_rig;
  [FieldOffset(139128)] public Node2* child_rig;
}

/// Struct type: Node2 (.?AUNode2@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 256)]
public unsafe struct Node2 {
  [FieldOffset(16)] public Node2__Flags flags;
  [FieldOffset(20)] public byte game_layer_override;
  [FieldOffset(21)] public byte game_layer;
  [FieldOffset(24)] public Aff _transform;
  [FieldOffset(224)] public Node2* parent;

  /// Struct type: Node2::Flags (.?AW4Flags@Node2@@)
  [CompilerGenerated]
  [Flags]
  public enum Node2__Flags : int {
    None = 0,
    Prop = 2,
    Transform = 4,
    Disabled = 8,
    Disabled_recursive = 16,
    Dead_next_frame = 32,
    Prev_data_invalid = 64,
    Has_first_world_transform = 128,
    Zombie = 256,
    World_space = 512,
    Hidden_recursive = 1024,
    Disabled_recursive_transitive = 2048,
    Hidden_recursive_transitive = 4096,
    Traversal_bit = 8192,
    Detached = 16384,
    Is_dirty = 32768,
    Transform_changed_since_save = 65536,
    Gpu_pipe_sync_needed = 131072,
    Mat_or_data_changed_since_render = 262144,
    Garbage = 524288,
    Gpu_pipe_prev_transform_sync_needed = 1048576,
    In_dirty_list = 2097152,
    Editor_node = 67108864,
    Editor_opened = 536870912
  }
}

/// Struct type: Mio::Life (.?AULife@Mio@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 656)]
public unsafe struct Mio__Life {
  [FieldOffset(12)] public bool death_anim;
  [FieldOffset(28)] public Vector3 death_hit_dir;
  [FieldOffset(40)] public bool refresh_next_spawn;
  [FieldOffset(44)] public Vector3 default_spawn_position;
  [FieldOffset(56)] public bool default_spawn_override;
  [FieldOffset(60)] public float death_anim_elapsed;
  [FieldOffset(64)] public bool death_in_boss_fight;
  [FieldOffset(128)] public Node2* hurtbox;
  [FieldOffset(136)] public Node2* hurtbox_parry;
  [FieldOffset(592)] public float invulnerable_remaining;
  [FieldOffset(596)] public float spawn_safe_remaining;
  [FieldOffset(600)] public int max_shield_points;
  [FieldOffset(604)] public int permanent_max_shield_points;
  [FieldOffset(608)] public int max_health_points;
  [FieldOffset(612)] public int shield_points;
  [FieldOffset(616)] public int health_points;
  [FieldOffset(620)] public bool heal_this_frame;
  [FieldOffset(621)] public bool damaged_this_frame;
  [FieldOffset(622)] public bool gain_additional_shield_this_frame;
  [FieldOffset(624)] public int additional_shields;
  [FieldOffset(628)] public int remaining_shield_fragments;
  [FieldOffset(632)] public int shield_fragments_to_new_shield;
  [FieldOffset(636)] public bool menu_force_open;
  [FieldOffset(640)] public int last_lost_scraps;
  [FieldOffset(644)] public int last_saved_scraps;
  [FieldOffset(648)] public float heal_ground_time;
  [FieldOffset(652)] public float heal_ground_remaining;
}

/// Struct type: Aff (.?AUAff@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 40)]
public struct Aff {
  [FieldOffset(0)] public Vector3 translation;
  [FieldOffset(12)] public Quaternion rotation;
  [FieldOffset(28)] public Vector3 scale;
}

/// Struct type: Filt<float> (.?AU?$Filt@M@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 16)]
public struct Filt_float {
  [FieldOffset(0)] public float v;
  [FieldOffset(4)] public float dv;
  [FieldOffset(8)] public float half_life;
  [FieldOffset(12)] public float damping_ratio;
}

/// Struct type: Filt<Vec<float,3> > (.?AU?$Filt@U?$Vec@M$02@@@@)
[CompilerGenerated]
[StructLayout(LayoutKind.Explicit, Size = 32)]
public struct Filt_Vector3 {
  [FieldOffset(0)] public Vector3 v;
  [FieldOffset(12)] public Vector3 dv;
  [FieldOffset(24)] public float half_life;
  [FieldOffset(28)] public float damping_ratio;
}
