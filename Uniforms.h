#pragma once

inline const char* Shader_SkyboxDefault = "DefaultSkybox";

inline const char* ID_UNIFORM_MODEL = "MATRIX_MODEL";
inline const char* ID_UNIFORM_VIEW = "MATRIX_VIEW";
inline const char* ID_UNIFORM_PROJECTION = "MATRIX_PROJECTION";

inline const char* ID_UNIFORM_TIME = "GLOBAL_TIME";
inline const char* ID_UNIFORM_VIEW_POS = "VIEW_POS";
inline const char* ID_UNIFORM_AMBIENT_LIGHT_COLOR = "_AMBIENT_LIGHT_COLOR";
inline const char* ID_UNIFORM_AMBIENT_LIGHT_INTENSITY = "_AMBIENT_LIGHT_INTENSITY";

inline const char* ID_COLOR_TINT = "_COLOR_TINT";
inline const char* ID_SPECULAR_POWER = "_SPECULAR_POWER";
inline const char* ID_BASE_MAP = "_BASE_MAP";
inline const char* ID_NORMAL_MAP = "_NORMAL_MAP";
inline const char* ID_ALPHA_CLIP_VALUE = "_ALPHA_CLIP_VALUE";
/// Tiling (xy) and Offset (zw)
inline const char* ID_BASE_MAP_TO = "_BASE_MAP_TO";

inline const char* ID_SKYBOX_CUBEMAP = "_SKYBOX";

inline const char* ID_UNIFORM_SCREEN_TEXTURE = "screenTexture";