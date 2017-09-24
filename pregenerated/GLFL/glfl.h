/*
  OpenGL Function Loader (GLFL) v1.2.0
  Copyright (C) 2017 Egor Mikhailov <blckcat@inbox.ru>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef GLFL_H_INCLUDED
#define GLFL_H_INCLUDED

#include <cstdint>

#ifdef GLFL_ENABLE_PROXY
#include <iostream>
#include <string>
#include <type_traits>
#include <cstdlib>
#endif


/* -- HOW TO USE --
 *
 * Create your window and context. Then do:
 *
 *     glfl::set_function_loader(..);
 *     glfl::load_*(..);
 *
 * After that you can call OpenGL functions.
 *
 * If you have multiple GL windows, you should first do following:
 *
 *     glfl::context my_context;
 *     glfl::set_context(my_context);
 *
 * `glfl::set_context(..)` determines which context is affected by `glfl::load_*()` and other functions.
 * When you call `gl*()` functions, pointers from the active context are used.
 *
 * There is a special `0` context which is active by default (you should use it if you have only one window).
 *
 *
 * To enable function hooking (intercepting calls), define GLFL_ENABLE_PROXY prior to inclusion of this header.
 * This macro is checked only at first inclusion.
 *
 * By default call hook will log all OpenGL calls.
 * It has some settings which are described below.
 *
 * You can provide a custom hook by defining GLFL_PROXY_NAME as a name of a proxy class.
 * The interface such class must provide is not documented. Default proxy implementation is located in glfl_proxy_proto__.h.
 *
 */

/* -- CALLING CONVENTIONS --
 *
 * All GL function pointers must be labeled as GLFL_API to enforce a proper calling convention.
 *
 *     void GLFL_API func(int x) {...}
 *
 *     void (GLFL_API *ptr)(int) = func;
 *
 * `GLFL_API` is defined as `APIENTRY` or `__stdcall` or `` (nothing) if it was not already defined by user prior to inclusion of this header.
 * Pointers from glfl::context, as well as optional fake (proxy) functions are labeled as GLFL_API.
 *
 */


#ifndef GLFL_API
// This logic was copied from GLEW.
#  ifdef APIENTRY
#    define GLFL_API APIENTRY
#  else
#    if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#      define GLFL_API __stdcall
#    else
#      define GLFL_API
#    endif
#  endif
#endif


typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLclampx;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
typedef unsigned short GLhalfARB;
typedef unsigned short GLhalf;
typedef GLint GLfixed;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef int64_t GLint64;
typedef uint64_t GLuint64;
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;
struct _cl_context;
struct _cl_event;
typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (*GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (*GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (*GLDEBUGPROCAMD)(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, void *userParam);
typedef unsigned short GLhalfNV;
typedef GLintptr GLvdpauSurfaceNV;


#if __cplusplus >= 201700L
#  define GLFL_CPP17(...) __VA_ARGS__
#else
#  define GLFL_CPP17(...)
#endif

#define GLFL_NODISCARD   GLFL_CPP17([[nodiscard]])
#define GLFL_FALLTHROUGH GLFL_CPP17([[fallthrough]];)


namespace glfl
{
    struct context;

    /* Returns a pointer to the current context.
     * There is a default context which is active by default.
     * If `set_active_context()` wasn't called yet and the default context
     * wasn't used in any way, this function returns 0. */
    GLFL_NODISCARD const context *active_context();

    /* Makes a context active.
     * set_active_context(0) activates the default context. */
    void set_active_context(context *);

    /* A special function for getting OpenGL function addresses should be provided by your window creating library. */
    using function_loader_t = void *(*)(const char *);
    void set_function_loader(function_loader_t);

    /* Holds loaded function pointers. */
    struct context
    {
        void *ptrs[3164] {};

        function_loader_t function_loader = 0;

        ~context()
        {
            if (this == active_context())
                set_active_context(0);
        }
    };

    /* Load all standard functions. */
    void load_all_versions();
    /* Load all standard functions and all extensions. */
    void load_everything();

    /* Load OpenGL versions.
     * Specifying an incorrect major+minor combinaton will load the newest version. */
    void load_gl(int major, int minor);
    void load_gl();
    void load_gles(int major, int minor);
    void load_gles();
    void load_glsc(int major, int minor);
    void load_glsc();

    /* The interface of the default (logging) proxy.
     * Functions below have no effect if it's disabled. You may reuse the interface for your own proxy. */
    namespace proxy
    {
        /* Functions marked as 'internal' should only be used in the implementation of the proxy.
         * Calls to functions marked 'msg' are logged unless the message callback is disabled; see below. */

        /* If enabled, `glGetError()` is called after each OpenGL function call except itself.
         * Enabled by default. */
        /*msg*/ void check_errors(bool);
        GLFL_NODISCARD bool check_errors();

        /* If enabled with `check_errors`, the program will be terminated
         * if the automatic call to `glGetError` reports any errors.
         * Enabled by default. */
        /*msg*/ void stop_on_errors(bool);
        GLFL_NODISCARD bool stop_on_errors();

        /* If enabled, all strings passed to GL functions are printed.
         * Otherwise they are printed as pointers.
         * Disabled by default. */
        /*msg*/ void print_string_arguments(bool);
        GLFL_NODISCARD bool print_string_arguments();


        /* Disable 'print' callback.
         * Since default 'message' and 'error' callbacks use `print()`, they won't print anything too. */
        /*msg*/ void disable_logging(bool);

        /* Disable 'message' callback. */
        void disable_messages(bool);


        using print_function_t = void (*)(const char *);

        /* Sets a callback for logging.
         * It has to add '\n' at the end of the string.
         * The default implementation is `std::puts()` (wrapped into a lambda). */
        void set_print_function(print_function_t);

        /* Calls the above callback. */
        void print(const char *);

        /* Similar to `set_print_function` but the string should be visually emphasized somehow.
         * The default implementation uses `print()`, so you don't have to override it separately.
         * The default implementation also appends '\n' to the beginning of the string; you probably should do the same. */
        void set_message_function(print_function_t);

        /* Calls the above callback. */
        void message(const char *);

        /* Similar to `set_print_function` but the string should be visually emphasized as error.
         * The default implementation uses `print()`, so you don't have to override it separately.
         * The default implementation also appends '\n' to the beginning of the string; you probably should do the same. */
        void set_error_function(print_function_t);

        /* Calls the above callback.
         * Then, if `stop_on_errors` is set, stops the program. */
        void error(const char *);


        /* This is incremented each time a GL function is called.
         * The default value is 0. Feel free to reset it. */
        GLFL_NODISCARD unsigned long long draw_call_count();
        /*msg*/ void reset_draw_call_count();
        /*internal*/ void incr_draw_call_count();


        /* The location of the last call.
         * Note that the actual call locations are not registered.
         * Rather, the last location where a function name was used is saved.
         * The difference is only noticeable when using function pointers. */
        GLFL_NODISCARD int line();
        GLFL_NODISCARD const char *file();
        /*internal*/ void location(const char *file, int line);
        /*internal*/ GLFL_NODISCARD bool line_changed(); // Calling this function resets the flag.
        /*internal*/ GLFL_NODISCARD bool file_changed(); // Calling this function resets the flag.


        /*internal*/ constexpr int index_of_glGetError = 889;
        /*internal*/ using type_of_glGetError = GLenum (GLFL_API *)();


        /*internal*/
        struct function_info
        {
            enum class type {other, enumeration, boolean, bitfield};
            const char *name;
            const char *const *param_names;
            type return_type;
            const type *param_types;
        };

        /*internal*/ GLFL_NODISCARD const function_info &get_function_info(int index);
    }


    /* Load extensions. */
    void load_extension_GL_3DFX_multisample(); // gl
    void load_extension_GL_3DFX_tbuffer(); // gl
    void load_extension_GL_3DFX_texture_compression_FXT1(); // gl
    void load_extension_GL_AMD_blend_minmax_factor(); // gl
    void load_extension_GL_AMD_compressed_3DC_texture(); // gles1|gles2
    void load_extension_GL_AMD_compressed_ATC_texture(); // gles1|gles2
    void load_extension_GL_AMD_conservative_depth(); // gl
    void load_extension_GL_AMD_debug_output(); // gl
    void load_extension_GL_AMD_depth_clamp_separate(); // gl
    void load_extension_GL_AMD_draw_buffers_blend(); // gl
    void load_extension_GL_AMD_framebuffer_sample_positions(); // disabled
    void load_extension_GL_AMD_gcn_shader(); // gl
    void load_extension_GL_AMD_gpu_shader_half_float(); // gl
    void load_extension_GL_AMD_gpu_shader_int64(); // gl
    void load_extension_GL_AMD_interleaved_elements(); // gl
    void load_extension_GL_AMD_multi_draw_indirect(); // gl
    void load_extension_GL_AMD_name_gen_delete(); // gl
    void load_extension_GL_AMD_occlusion_query_event(); // gl
    void load_extension_GL_AMD_performance_monitor(); // gl|glcore|gles2
    void load_extension_GL_AMD_pinned_memory(); // gl
    void load_extension_GL_AMD_program_binary_Z400(); // gles2
    void load_extension_GL_AMD_query_buffer_object(); // gl
    void load_extension_GL_AMD_sample_positions(); // gl
    void load_extension_GL_AMD_seamless_cubemap_per_texture(); // gl
    void load_extension_GL_AMD_shader_atomic_counter_ops(); // gl
    void load_extension_GL_AMD_shader_ballot(); // gl
    void load_extension_GL_AMD_shader_stencil_export(); // gl
    void load_extension_GL_AMD_shader_trinary_minmax(); // gl
    void load_extension_GL_AMD_shader_explicit_vertex_parameter(); // gl
    void load_extension_GL_AMD_sparse_texture(); // gl
    void load_extension_GL_AMD_stencil_operation_extended(); // gl
    void load_extension_GL_AMD_texture_texture4(); // gl
    void load_extension_GL_AMD_transform_feedback3_lines_triangles(); // gl
    void load_extension_GL_AMD_transform_feedback4(); // gl
    void load_extension_GL_AMD_vertex_shader_layer(); // gl
    void load_extension_GL_AMD_vertex_shader_tessellator(); // gl
    void load_extension_GL_AMD_vertex_shader_viewport_index(); // gl
    void load_extension_GL_ANDROID_extension_pack_es31a(); // gles2
    void load_extension_GL_ANGLE_depth_texture(); // gles2
    void load_extension_GL_ANGLE_framebuffer_blit(); // gles2
    void load_extension_GL_ANGLE_framebuffer_multisample(); // gles2
    void load_extension_GL_ANGLE_instanced_arrays(); // gles2
    void load_extension_GL_ANGLE_pack_reverse_row_order(); // gles2
    void load_extension_GL_ANGLE_program_binary(); // gles2
    void load_extension_GL_ANGLE_texture_compression_dxt3(); // gles2
    void load_extension_GL_ANGLE_texture_compression_dxt5(); // gles2
    void load_extension_GL_ANGLE_texture_usage(); // gles2
    void load_extension_GL_ANGLE_translated_shader_source(); // gles2
    void load_extension_GL_APPLE_aux_depth_stencil(); // gl
    void load_extension_GL_APPLE_client_storage(); // gl
    void load_extension_GL_APPLE_clip_distance(); // gles2
    void load_extension_GL_APPLE_color_buffer_packed_float(); // gles2
    void load_extension_GL_APPLE_copy_texture_levels(); // gles1|gles2
    void load_extension_GL_APPLE_element_array(); // gl
    void load_extension_GL_APPLE_fence(); // gl
    void load_extension_GL_APPLE_float_pixels(); // gl
    void load_extension_GL_APPLE_flush_buffer_range(); // gl
    void load_extension_GL_APPLE_framebuffer_multisample(); // gles1|gles2
    void load_extension_GL_APPLE_object_purgeable(); // gl
    void load_extension_GL_APPLE_rgb_422(); // gl|glcore|gles2
    void load_extension_GL_APPLE_row_bytes(); // gl
    void load_extension_GL_APPLE_specular_vector(); // gl
    void load_extension_GL_APPLE_sync(); // gles1|gles2
    void load_extension_GL_APPLE_texture_2D_limited_npot(); // gles1
    void load_extension_GL_APPLE_texture_format_BGRA8888(); // gles1|gles2
    void load_extension_GL_APPLE_texture_max_level(); // gles1|gles2
    void load_extension_GL_APPLE_texture_packed_float(); // gles2
    void load_extension_GL_APPLE_texture_range(); // gl
    void load_extension_GL_APPLE_transform_hint(); // gl
    void load_extension_GL_APPLE_vertex_array_object(); // gl
    void load_extension_GL_APPLE_vertex_array_range(); // gl
    void load_extension_GL_APPLE_vertex_program_evaluators(); // gl
    void load_extension_GL_APPLE_ycbcr_422(); // gl
    void load_extension_GL_ARB_ES2_compatibility(); // gl|glcore
    void load_extension_GL_ARB_ES3_1_compatibility(); // gl|glcore
    void load_extension_GL_ARB_ES3_2_compatibility(); // gl
    void load_extension_GL_ARB_ES3_compatibility(); // gl|glcore
    void load_extension_GL_ARB_arrays_of_arrays(); // gl|glcore
    void load_extension_GL_ARB_base_instance(); // gl|glcore
    void load_extension_GL_ARB_bindless_texture(); // gl|glcore
    void load_extension_GL_ARB_blend_func_extended(); // gl|glcore
    void load_extension_GL_ARB_buffer_storage(); // gl|glcore
    void load_extension_GL_ARB_cl_event(); // gl|glcore
    void load_extension_GL_ARB_clear_buffer_object(); // gl|glcore
    void load_extension_GL_ARB_clear_texture(); // gl|glcore
    void load_extension_GL_ARB_clip_control(); // gl|glcore
    void load_extension_GL_ARB_color_buffer_float(); // gl
    void load_extension_GL_ARB_compatibility(); // gl
    void load_extension_GL_ARB_compressed_texture_pixel_storage(); // gl|glcore
    void load_extension_GL_ARB_compute_shader(); // gl|glcore
    void load_extension_GL_ARB_compute_variable_group_size(); // gl|glcore
    void load_extension_GL_ARB_conditional_render_inverted(); // gl|glcore
    void load_extension_GL_ARB_conservative_depth(); // gl|glcore
    void load_extension_GL_ARB_copy_buffer(); // gl|glcore
    void load_extension_GL_ARB_copy_image(); // gl|glcore
    void load_extension_GL_ARB_cull_distance(); // gl|glcore
    void load_extension_GL_ARB_debug_output(); // gl|glcore
    void load_extension_GL_ARB_depth_buffer_float(); // gl|glcore
    void load_extension_GL_ARB_depth_clamp(); // gl|glcore
    void load_extension_GL_ARB_depth_texture(); // gl
    void load_extension_GL_ARB_derivative_control(); // gl|glcore
    void load_extension_GL_ARB_direct_state_access(); // gl|glcore
    void load_extension_GL_ARB_draw_buffers(); // gl
    void load_extension_GL_ARB_draw_buffers_blend(); // gl|glcore
    void load_extension_GL_ARB_draw_elements_base_vertex(); // gl|glcore
    void load_extension_GL_ARB_draw_indirect(); // gl|glcore
    void load_extension_GL_ARB_draw_instanced(); // gl
    void load_extension_GL_ARB_enhanced_layouts(); // gl|glcore
    void load_extension_GL_ARB_explicit_attrib_location(); // gl|glcore
    void load_extension_GL_ARB_explicit_uniform_location(); // gl|glcore
    void load_extension_GL_ARB_fragment_coord_conventions(); // gl|glcore
    void load_extension_GL_ARB_fragment_layer_viewport(); // gl|glcore
    void load_extension_GL_ARB_fragment_program(); // gl
    void load_extension_GL_ARB_fragment_program_shadow(); // gl
    void load_extension_GL_ARB_fragment_shader(); // gl
    void load_extension_GL_ARB_fragment_shader_interlock(); // gl
    void load_extension_GL_ARB_framebuffer_no_attachments(); // gl|glcore
    void load_extension_GL_ARB_framebuffer_object(); // gl|glcore
    void load_extension_GL_ARB_framebuffer_sRGB(); // gl|glcore
    void load_extension_GL_ARB_geometry_shader4(); // gl
    void load_extension_GL_ARB_get_program_binary(); // gl|glcore
    void load_extension_GL_ARB_get_texture_sub_image(); // gl|glcore
    void load_extension_GL_ARB_gpu_shader5(); // gl|glcore
    void load_extension_GL_ARB_gpu_shader_fp64(); // gl|glcore
    void load_extension_GL_ARB_gpu_shader_int64(); // gl
    void load_extension_GL_ARB_half_float_pixel(); // gl
    void load_extension_GL_ARB_half_float_vertex(); // gl|glcore
    void load_extension_GL_ARB_imaging(); // gl|glcore
    void load_extension_GL_ARB_indirect_parameters(); // gl|glcore
    void load_extension_GL_ARB_instanced_arrays(); // gl
    void load_extension_GL_ARB_internalformat_query(); // gl|glcore
    void load_extension_GL_ARB_internalformat_query2(); // gl|glcore
    void load_extension_GL_ARB_invalidate_subdata(); // gl|glcore
    void load_extension_GL_ARB_map_buffer_alignment(); // gl|glcore
    void load_extension_GL_ARB_map_buffer_range(); // gl|glcore
    void load_extension_GL_ARB_matrix_palette(); // gl
    void load_extension_GL_ARB_multi_bind(); // gl|glcore
    void load_extension_GL_ARB_multi_draw_indirect(); // gl|glcore
    void load_extension_GL_ARB_multisample(); // gl
    void load_extension_GL_ARB_multitexture(); // gl
    void load_extension_GL_ARB_occlusion_query(); // gl
    void load_extension_GL_ARB_occlusion_query2(); // gl|glcore
    void load_extension_GL_ARB_parallel_shader_compile(); // gl
    void load_extension_GL_ARB_pipeline_statistics_query(); // gl|glcore
    void load_extension_GL_ARB_pixel_buffer_object(); // gl
    void load_extension_GL_ARB_point_parameters(); // gl
    void load_extension_GL_ARB_point_sprite(); // gl
    void load_extension_GL_ARB_post_depth_coverage(); // gl
    void load_extension_GL_ARB_program_interface_query(); // gl|glcore
    void load_extension_GL_ARB_provoking_vertex(); // gl|glcore
    void load_extension_GL_ARB_query_buffer_object(); // gl|glcore
    void load_extension_GL_ARB_robust_buffer_access_behavior(); // gl|glcore
    void load_extension_GL_ARB_robustness(); // gl|glcore
    void load_extension_GL_ARB_robustness_isolation(); // gl|glcore
    void load_extension_GL_ARB_sample_locations(); // gl
    void load_extension_GL_ARB_sample_shading(); // gl|glcore
    void load_extension_GL_ARB_sampler_objects(); // gl|glcore
    void load_extension_GL_ARB_seamless_cube_map(); // gl|glcore
    void load_extension_GL_ARB_seamless_cubemap_per_texture(); // gl|glcore
    void load_extension_GL_ARB_separate_shader_objects(); // gl|glcore
    void load_extension_GL_ARB_shader_atomic_counter_ops(); // gl
    void load_extension_GL_ARB_shader_atomic_counters(); // gl|glcore
    void load_extension_GL_ARB_shader_ballot(); // gl
    void load_extension_GL_ARB_shader_bit_encoding(); // gl|glcore
    void load_extension_GL_ARB_shader_clock(); // gl
    void load_extension_GL_ARB_shader_draw_parameters(); // gl|glcore
    void load_extension_GL_ARB_shader_group_vote(); // gl|glcore
    void load_extension_GL_ARB_shader_image_load_store(); // gl|glcore
    void load_extension_GL_ARB_shader_image_size(); // gl|glcore
    void load_extension_GL_ARB_shader_objects(); // gl
    void load_extension_GL_ARB_shader_precision(); // gl|glcore
    void load_extension_GL_ARB_shader_stencil_export(); // gl|glcore
    void load_extension_GL_ARB_shader_storage_buffer_object(); // gl|glcore
    void load_extension_GL_ARB_shader_subroutine(); // gl|glcore
    void load_extension_GL_ARB_shader_texture_image_samples(); // gl|glcore
    void load_extension_GL_ARB_shader_texture_lod(); // gl
    void load_extension_GL_ARB_shader_viewport_layer_array(); // gl
    void load_extension_GL_ARB_shading_language_100(); // gl
    void load_extension_GL_ARB_shading_language_420pack(); // gl|glcore
    void load_extension_GL_ARB_shading_language_include(); // gl|glcore
    void load_extension_GL_ARB_shading_language_packing(); // gl|glcore
    void load_extension_GL_ARB_shadow(); // gl
    void load_extension_GL_ARB_shadow_ambient(); // gl
    void load_extension_GL_ARB_sparse_buffer(); // gl|glcore
    void load_extension_GL_ARB_sparse_texture(); // gl|glcore
    void load_extension_GL_ARB_sparse_texture2(); // gl|glcore|gles2
    void load_extension_GL_ARB_sparse_texture_clamp(); // gl
    void load_extension_GL_ARB_stencil_texturing(); // gl|glcore
    void load_extension_GL_ARB_sync(); // gl|glcore
    void load_extension_GL_ARB_tessellation_shader(); // gl|glcore
    void load_extension_GL_ARB_texture_barrier(); // gl|glcore
    void load_extension_GL_ARB_texture_border_clamp(); // gl
    void load_extension_GL_ARB_texture_buffer_object(); // gl
    void load_extension_GL_ARB_texture_buffer_object_rgb32(); // gl|glcore
    void load_extension_GL_ARB_texture_buffer_range(); // gl|glcore
    void load_extension_GL_ARB_texture_compression(); // gl
    void load_extension_GL_ARB_texture_compression_bptc(); // gl|glcore
    void load_extension_GL_ARB_texture_compression_rgtc(); // gl|glcore
    void load_extension_GL_ARB_texture_cube_map(); // gl
    void load_extension_GL_ARB_texture_cube_map_array(); // gl|glcore
    void load_extension_GL_ARB_texture_env_add(); // gl
    void load_extension_GL_ARB_texture_env_combine(); // gl
    void load_extension_GL_ARB_texture_env_crossbar(); // gl
    void load_extension_GL_ARB_texture_env_dot3(); // gl
    void load_extension_GL_ARB_texture_filter_minmax(); // gl
    void load_extension_GL_ARB_texture_float(); // gl
    void load_extension_GL_ARB_texture_gather(); // gl|glcore
    void load_extension_GL_ARB_texture_mirror_clamp_to_edge(); // gl|glcore
    void load_extension_GL_ARB_texture_mirrored_repeat(); // gl
    void load_extension_GL_ARB_texture_multisample(); // gl|glcore
    void load_extension_GL_ARB_texture_non_power_of_two(); // gl
    void load_extension_GL_ARB_texture_query_levels(); // gl|glcore
    void load_extension_GL_ARB_texture_query_lod(); // gl|glcore
    void load_extension_GL_ARB_texture_rectangle(); // gl
    void load_extension_GL_ARB_texture_rg(); // gl|glcore
    void load_extension_GL_ARB_texture_rgb10_a2ui(); // gl|glcore
    void load_extension_GL_ARB_texture_stencil8(); // gl|glcore
    void load_extension_GL_ARB_texture_storage(); // gl|glcore
    void load_extension_GL_ARB_texture_storage_multisample(); // gl|glcore
    void load_extension_GL_ARB_texture_swizzle(); // gl|glcore
    void load_extension_GL_ARB_texture_view(); // gl|glcore
    void load_extension_GL_ARB_timer_query(); // gl|glcore
    void load_extension_GL_ARB_transform_feedback2(); // gl|glcore
    void load_extension_GL_ARB_transform_feedback3(); // gl|glcore
    void load_extension_GL_ARB_transform_feedback_instanced(); // gl|glcore
    void load_extension_GL_ARB_transform_feedback_overflow_query(); // gl|glcore
    void load_extension_GL_ARB_transpose_matrix(); // gl
    void load_extension_GL_ARB_uniform_buffer_object(); // gl|glcore
    void load_extension_GL_ARB_vertex_array_bgra(); // gl|glcore
    void load_extension_GL_ARB_vertex_array_object(); // gl|glcore
    void load_extension_GL_ARB_vertex_attrib_64bit(); // gl|glcore
    void load_extension_GL_ARB_vertex_attrib_binding(); // gl|glcore
    void load_extension_GL_ARB_vertex_blend(); // gl
    void load_extension_GL_ARB_vertex_buffer_object(); // gl
    void load_extension_GL_ARB_vertex_program(); // gl
    void load_extension_GL_ARB_vertex_shader(); // gl
    void load_extension_GL_ARB_vertex_type_10f_11f_11f_rev(); // gl|glcore
    void load_extension_GL_ARB_vertex_type_2_10_10_10_rev(); // gl|glcore
    void load_extension_GL_ARB_viewport_array(); // gl|glcore
    void load_extension_GL_ARB_window_pos(); // gl
    void load_extension_GL_ARM_mali_program_binary(); // gles2
    void load_extension_GL_ARM_mali_shader_binary(); // gles2
    void load_extension_GL_ARM_rgba8(); // gles1|gles2
    void load_extension_GL_ARM_shader_framebuffer_fetch(); // gles2
    void load_extension_GL_ARM_shader_framebuffer_fetch_depth_stencil(); // gles2
    void load_extension_GL_ATI_draw_buffers(); // gl
    void load_extension_GL_ATI_element_array(); // gl
    void load_extension_GL_ATI_envmap_bumpmap(); // gl
    void load_extension_GL_ATI_fragment_shader(); // gl
    void load_extension_GL_ATI_map_object_buffer(); // gl
    void load_extension_GL_ATI_meminfo(); // gl
    void load_extension_GL_ATI_pixel_format_float(); // gl
    void load_extension_GL_ATI_pn_triangles(); // gl
    void load_extension_GL_ATI_separate_stencil(); // gl
    void load_extension_GL_ATI_text_fragment_shader(); // gl
    void load_extension_GL_ATI_texture_env_combine3(); // gl
    void load_extension_GL_ATI_texture_float(); // gl
    void load_extension_GL_ATI_texture_mirror_once(); // gl
    void load_extension_GL_ATI_vertex_array_object(); // gl
    void load_extension_GL_ATI_vertex_attrib_array_object(); // gl
    void load_extension_GL_ATI_vertex_streams(); // gl
    void load_extension_GL_DMP_program_binary(); // gles2
    void load_extension_GL_DMP_shader_binary(); // gles2
    void load_extension_GL_EXT_422_pixels(); // gl
    void load_extension_GL_EXT_YUV_target(); // gles2
    void load_extension_GL_EXT_abgr(); // gl
    void load_extension_GL_EXT_base_instance(); // gles2
    void load_extension_GL_EXT_bgra(); // gl
    void load_extension_GL_EXT_bindable_uniform(); // gl
    void load_extension_GL_EXT_blend_color(); // gl
    void load_extension_GL_EXT_blend_equation_separate(); // gl
    void load_extension_GL_EXT_blend_func_extended(); // gles2
    void load_extension_GL_EXT_blend_func_separate(); // gl
    void load_extension_GL_EXT_blend_logic_op(); // gl
    void load_extension_GL_EXT_blend_minmax(); // gl|gles1|gles2
    void load_extension_GL_EXT_blend_subtract(); // gl
    void load_extension_GL_EXT_buffer_storage(); // gles2
    void load_extension_GL_EXT_clear_texture(); // gles2
    void load_extension_GL_EXT_clip_cull_distance(); // gles2
    void load_extension_GL_EXT_clip_volume_hint(); // gl
    void load_extension_GL_EXT_cmyka(); // gl
    void load_extension_GL_EXT_color_buffer_float(); // gles2
    void load_extension_GL_EXT_color_buffer_half_float(); // gles2
    void load_extension_GL_EXT_color_subtable(); // gl
    void load_extension_GL_EXT_compiled_vertex_array(); // gl
    void load_extension_GL_EXT_conservative_depth(); // gles2
    void load_extension_GL_EXT_convolution(); // gl
    void load_extension_GL_EXT_coordinate_frame(); // gl
    void load_extension_GL_EXT_copy_image(); // gles2
    void load_extension_GL_EXT_copy_texture(); // gl
    void load_extension_GL_EXT_cull_vertex(); // gl
    void load_extension_GL_EXT_debug_label(); // gl|glcore|gles2
    void load_extension_GL_EXT_debug_marker(); // gl|glcore|gles2
    void load_extension_GL_EXT_depth_bounds_test(); // gl
    void load_extension_GL_EXT_direct_state_access(); // gl
    void load_extension_GL_EXT_discard_framebuffer(); // gles1|gles2
    void load_extension_GL_EXT_disjoint_timer_query(); // gles2
    void load_extension_GL_EXT_draw_buffers(); // gles2
    void load_extension_GL_EXT_draw_buffers2(); // gl
    void load_extension_GL_EXT_draw_buffers_indexed(); // gles2
    void load_extension_GL_EXT_draw_elements_base_vertex(); // gles2
    void load_extension_GL_EXT_draw_instanced(); // gl|glcore|gles2
    void load_extension_GL_EXT_draw_range_elements(); // gl
    void load_extension_GL_EXT_draw_transform_feedback(); // gles2
    void load_extension_GL_EXT_float_blend(); // gles2
    void load_extension_GL_EXT_fog_coord(); // gl
    void load_extension_GL_EXT_framebuffer_blit(); // gl
    void load_extension_GL_EXT_framebuffer_multisample(); // gl
    void load_extension_GL_EXT_framebuffer_multisample_blit_scaled(); // gl
    void load_extension_GL_EXT_framebuffer_object(); // gl
    void load_extension_GL_EXT_framebuffer_sRGB(); // gl
    void load_extension_GL_EXT_geometry_point_size(); // gles2
    void load_extension_GL_EXT_geometry_shader(); // gles2
    void load_extension_GL_EXT_geometry_shader4(); // gl
    void load_extension_GL_EXT_gpu_program_parameters(); // gl
    void load_extension_GL_EXT_gpu_shader4(); // gl
    void load_extension_GL_EXT_gpu_shader5(); // gles2
    void load_extension_GL_EXT_histogram(); // gl
    void load_extension_GL_EXT_index_array_formats(); // gl
    void load_extension_GL_EXT_index_func(); // gl
    void load_extension_GL_EXT_index_material(); // gl
    void load_extension_GL_EXT_index_texture(); // gl
    void load_extension_GL_EXT_instanced_arrays(); // gles2
    void load_extension_GL_EXT_light_texture(); // gl
    void load_extension_GL_EXT_map_buffer_range(); // gles1|gles2
    void load_extension_GL_EXT_misc_attribute(); // gl
    void load_extension_GL_EXT_multi_draw_arrays(); // gl|gles1|gles2
    void load_extension_GL_EXT_multi_draw_indirect(); // gles2
    void load_extension_GL_EXT_multisample(); // gl
    void load_extension_GL_EXT_multisampled_compatibility(); // gles2
    void load_extension_GL_EXT_multisampled_render_to_texture(); // gles1|gles2
    void load_extension_GL_EXT_multiview_draw_buffers(); // gles2
    void load_extension_GL_EXT_occlusion_query_boolean(); // gles2
    void load_extension_GL_EXT_packed_depth_stencil(); // gl
    void load_extension_GL_EXT_packed_float(); // gl
    void load_extension_GL_EXT_packed_pixels(); // gl
    void load_extension_GL_EXT_paletted_texture(); // gl
    void load_extension_GL_EXT_pixel_buffer_object(); // gl
    void load_extension_GL_EXT_pixel_transform(); // gl
    void load_extension_GL_EXT_pixel_transform_color_table(); // gl
    void load_extension_GL_EXT_point_parameters(); // gl
    void load_extension_GL_EXT_polygon_offset(); // gl
    void load_extension_GL_EXT_polygon_offset_clamp(); // gl|glcore|gles2
    void load_extension_GL_EXT_post_depth_coverage(); // gl|glcore|gles2
    void load_extension_GL_EXT_primitive_bounding_box(); // gles2
    void load_extension_GL_EXT_protected_textures(); // gles2
    void load_extension_GL_EXT_provoking_vertex(); // gl
    void load_extension_GL_EXT_pvrtc_sRGB(); // gles2
    void load_extension_GL_EXT_raster_multisample(); // gl|glcore|gles2
    void load_extension_GL_EXT_read_format_bgra(); // gles1|gles2
    void load_extension_GL_EXT_render_snorm(); // gles2
    void load_extension_GL_EXT_rescale_normal(); // gl
    void load_extension_GL_EXT_robustness(); // gles1|gles2
    void load_extension_GL_EXT_sRGB(); // gles1|gles2
    void load_extension_GL_EXT_sRGB_write_control(); // gles2
    void load_extension_GL_EXT_secondary_color(); // gl
    void load_extension_GL_EXT_separate_shader_objects(); // gl|glcore|gles2
    void load_extension_GL_EXT_separate_specular_color(); // gl
    void load_extension_GL_EXT_shader_framebuffer_fetch(); // gles2
    void load_extension_GL_EXT_shader_group_vote(); // gles2
    void load_extension_GL_EXT_shader_image_load_formatted(); // gl
    void load_extension_GL_EXT_shader_image_load_store(); // gl
    void load_extension_GL_EXT_shader_implicit_conversions(); // gles2
    void load_extension_GL_EXT_shader_integer_mix(); // gl|glcore|gles2
    void load_extension_GL_EXT_shader_io_blocks(); // gles2
    void load_extension_GL_EXT_shader_non_constant_global_initializers(); // gles2
    void load_extension_GL_EXT_shader_pixel_local_storage(); // gles2
    void load_extension_GL_EXT_shader_pixel_local_storage2(); // gles2
    void load_extension_GL_EXT_shader_texture_lod(); // gles2
    void load_extension_GL_EXT_shadow_funcs(); // gl
    void load_extension_GL_EXT_shadow_samplers(); // gles2
    void load_extension_GL_EXT_shared_texture_palette(); // gl
    void load_extension_GL_EXT_sparse_texture(); // gles2
    void load_extension_GL_EXT_sparse_texture2(); // gl
    void load_extension_GL_EXT_stencil_clear_tag(); // gl
    void load_extension_GL_EXT_stencil_two_side(); // gl
    void load_extension_GL_EXT_stencil_wrap(); // gl
    void load_extension_GL_EXT_subtexture(); // gl
    void load_extension_GL_EXT_tessellation_point_size(); // gles2
    void load_extension_GL_EXT_tessellation_shader(); // gles2
    void load_extension_GL_EXT_texture(); // gl
    void load_extension_GL_EXT_texture3D(); // gl
    void load_extension_GL_EXT_texture_array(); // gl
    void load_extension_GL_EXT_texture_border_clamp(); // gles2
    void load_extension_GL_EXT_texture_buffer(); // gles2
    void load_extension_GL_EXT_texture_buffer_object(); // gl
    void load_extension_GL_EXT_texture_compression_dxt1(); // gles1|gles2
    void load_extension_GL_EXT_texture_compression_latc(); // gl
    void load_extension_GL_EXT_texture_compression_rgtc(); // gl
    void load_extension_GL_EXT_texture_compression_s3tc(); // gl|glcore|gles2|glsc2
    void load_extension_GL_EXT_texture_cube_map(); // gl
    void load_extension_GL_EXT_texture_cube_map_array(); // gles2
    void load_extension_GL_EXT_texture_env_add(); // gl
    void load_extension_GL_EXT_texture_env_combine(); // gl
    void load_extension_GL_EXT_texture_env_dot3(); // gl
    void load_extension_GL_EXT_texture_filter_anisotropic(); // gl|gles1|gles2
    void load_extension_GL_EXT_texture_filter_minmax(); // gl|glcore|gles2
    void load_extension_GL_EXT_texture_format_BGRA8888(); // gles1|gles2
    void load_extension_GL_EXT_texture_integer(); // gl
    void load_extension_GL_EXT_texture_lod_bias(); // gl|gles1
    void load_extension_GL_EXT_texture_mirror_clamp(); // gl
    void load_extension_GL_EXT_texture_norm16(); // gles2
    void load_extension_GL_EXT_texture_object(); // gl
    void load_extension_GL_EXT_texture_perturb_normal(); // gl
    void load_extension_GL_EXT_texture_rg(); // gles2
    void load_extension_GL_EXT_texture_sRGB(); // gl
    void load_extension_GL_EXT_texture_sRGB_R8(); // gles2
    void load_extension_GL_EXT_texture_sRGB_RG8(); // gles2
    void load_extension_GL_EXT_texture_sRGB_decode(); // gl|glcore|gles2
    void load_extension_GL_EXT_texture_shared_exponent(); // gl
    void load_extension_GL_EXT_texture_snorm(); // gl
    void load_extension_GL_EXT_texture_storage(); // gles1|gles2
    void load_extension_GL_EXT_texture_swizzle(); // gl
    void load_extension_GL_EXT_texture_type_2_10_10_10_REV(); // gles2
    void load_extension_GL_EXT_texture_view(); // gles2
    void load_extension_GL_EXT_timer_query(); // gl
    void load_extension_GL_EXT_transform_feedback(); // gl
    void load_extension_GL_EXT_unpack_subimage(); // gles2
    void load_extension_GL_EXT_vertex_array(); // gl
    void load_extension_GL_EXT_vertex_array_bgra(); // gl
    void load_extension_GL_EXT_vertex_attrib_64bit(); // gl
    void load_extension_GL_EXT_vertex_shader(); // gl
    void load_extension_GL_EXT_vertex_weighting(); // gl
    void load_extension_GL_EXT_window_rectangles(); // gl|glcore|gles2
    void load_extension_GL_EXT_x11_sync_object(); // gl
    void load_extension_GL_FJ_shader_binary_GCCSO(); // gles2
    void load_extension_GL_GREMEDY_frame_terminator(); // gl
    void load_extension_GL_GREMEDY_string_marker(); // gl
    void load_extension_GL_HP_convolution_border_modes(); // gl
    void load_extension_GL_HP_image_transform(); // gl
    void load_extension_GL_HP_occlusion_test(); // gl
    void load_extension_GL_HP_texture_lighting(); // gl
    void load_extension_GL_IBM_cull_vertex(); // gl
    void load_extension_GL_IBM_multimode_draw_arrays(); // gl
    void load_extension_GL_IBM_rasterpos_clip(); // gl
    void load_extension_GL_IBM_static_data(); // gl
    void load_extension_GL_IBM_texture_mirrored_repeat(); // gl
    void load_extension_GL_IBM_vertex_array_lists(); // gl
    void load_extension_GL_IMG_bindless_texture(); // gles2
    void load_extension_GL_IMG_framebuffer_downsample(); // gles2
    void load_extension_GL_IMG_multisampled_render_to_texture(); // gles1|gles2
    void load_extension_GL_IMG_program_binary(); // gles2
    void load_extension_GL_IMG_read_format(); // gles1|gles2
    void load_extension_GL_IMG_shader_binary(); // gles2
    void load_extension_GL_IMG_texture_compression_pvrtc(); // gles1|gles2
    void load_extension_GL_IMG_texture_compression_pvrtc2(); // gles2
    void load_extension_GL_IMG_texture_env_enhanced_fixed_function(); // gles1
    void load_extension_GL_IMG_texture_filter_cubic(); // gles2
    void load_extension_GL_IMG_user_clip_plane(); // gles1
    void load_extension_GL_INGR_blend_func_separate(); // gl
    void load_extension_GL_INGR_color_clamp(); // gl
    void load_extension_GL_INGR_interlace_read(); // gl
    void load_extension_GL_INTEL_conservative_rasterization(); // gl|glcore|gles2
    void load_extension_GL_INTEL_fragment_shader_ordering(); // gl
    void load_extension_GL_INTEL_framebuffer_CMAA(); // gl|glcore|gles2
    void load_extension_GL_INTEL_map_texture(); // gl
    void load_extension_GL_INTEL_parallel_arrays(); // gl
    void load_extension_GL_INTEL_performance_query(); // gl|glcore|gles2
    void load_extension_GL_KHR_blend_equation_advanced(); // gl|glcore|gles2
    void load_extension_GL_KHR_blend_equation_advanced_coherent(); // gl|glcore|gles2
    void load_extension_GL_KHR_context_flush_control(); // gl|glcore|gles2
    void load_extension_GL_KHR_debug(); // gl|glcore|gles2
    void load_extension_GL_KHR_no_error(); // gl|glcore|gles2
    void load_extension_GL_KHR_robust_buffer_access_behavior(); // gl|glcore|gles2
    void load_extension_GL_KHR_robustness(); // gl|glcore|gles2
    void load_extension_GL_KHR_texture_compression_astc_hdr(); // gl|glcore|gles2
    void load_extension_GL_KHR_texture_compression_astc_ldr(); // gl|glcore|gles2
    void load_extension_GL_KHR_texture_compression_astc_sliced_3d(); // gl|glcore|gles2
    void load_extension_GL_MESAX_texture_stack(); // gl
    void load_extension_GL_MESA_pack_invert(); // gl
    void load_extension_GL_MESA_resize_buffers(); // gl
    void load_extension_GL_MESA_window_pos(); // gl
    void load_extension_GL_MESA_ycbcr_texture(); // gl
    void load_extension_GL_NVX_conditional_render(); // gl
    void load_extension_GL_NVX_gpu_memory_info(); // gl
    void load_extension_GL_NV_bindless_multi_draw_indirect(); // gl
    void load_extension_GL_NV_bindless_multi_draw_indirect_count(); // gl
    void load_extension_GL_NV_bindless_texture(); // gl|glcore|gles2
    void load_extension_GL_NV_blend_equation_advanced(); // gl|glcore|gles2
    void load_extension_GL_NV_blend_equation_advanced_coherent(); // gl|glcore|gles2
    void load_extension_GL_NV_blend_square(); // gl
    void load_extension_GL_NV_clip_space_w_scaling(); // gl
    void load_extension_GL_NV_command_list(); // gl
    void load_extension_GL_NV_compute_program5(); // gl
    void load_extension_GL_NV_conditional_render(); // gl|glcore|gles2
    void load_extension_GL_NV_conservative_raster(); // gl|glcore|gles2
    void load_extension_GL_NV_conservative_raster_dilate(); // gl
    void load_extension_GL_NV_conservative_raster_pre_snap_triangles(); // gl|glcore|gles2
    void load_extension_GL_NV_copy_buffer(); // gles2
    void load_extension_GL_NV_copy_depth_to_color(); // gl
    void load_extension_GL_NV_copy_image(); // gl
    void load_extension_GL_NV_coverage_sample(); // gles2
    void load_extension_GL_NV_deep_texture3D(); // gl
    void load_extension_GL_NV_depth_buffer_float(); // gl
    void load_extension_GL_NV_depth_clamp(); // gl
    void load_extension_GL_NV_depth_nonlinear(); // gles2
    void load_extension_GL_NV_draw_buffers(); // gles2
    void load_extension_GL_NV_draw_instanced(); // gles2
    void load_extension_GL_NV_draw_texture(); // gl
    void load_extension_GL_NV_evaluators(); // gl
    void load_extension_GL_NV_explicit_attrib_location(); // gles2
    void load_extension_GL_NV_explicit_multisample(); // gl
    void load_extension_GL_NV_fbo_color_attachments(); // gles2
    void load_extension_GL_NV_fence(); // gl|gles1|gles2
    void load_extension_GL_NV_fill_rectangle(); // gl|glcore|gles2
    void load_extension_GL_NV_float_buffer(); // gl
    void load_extension_GL_NV_fog_distance(); // gl
    void load_extension_GL_NV_fragment_coverage_to_color(); // gl|glcore|gles2
    void load_extension_GL_NV_fragment_program(); // gl
    void load_extension_GL_NV_fragment_program2(); // gl
    void load_extension_GL_NV_fragment_program4(); // gl
    void load_extension_GL_NV_fragment_program_option(); // gl
    void load_extension_GL_NV_fragment_shader_interlock(); // gl|glcore|gles2
    void load_extension_GL_NV_framebuffer_blit(); // gles2
    void load_extension_GL_NV_framebuffer_mixed_samples(); // gl|glcore|gles2
    void load_extension_GL_NV_framebuffer_multisample(); // gles2
    void load_extension_GL_NV_framebuffer_multisample_coverage(); // gl
    void load_extension_GL_NV_generate_mipmap_sRGB(); // gles2
    void load_extension_GL_NV_geometry_program4(); // gl
    void load_extension_GL_NV_geometry_shader4(); // gl
    void load_extension_GL_NV_geometry_shader_passthrough(); // gl|glcore|gles2
    void load_extension_GL_NV_gpu_program4(); // gl
    void load_extension_GL_NV_gpu_program5(); // gl
    void load_extension_GL_NV_gpu_program5_mem_extended(); // gl
    void load_extension_GL_NV_gpu_shader5(); // gl|glcore|gles2
    void load_extension_GL_NV_half_float(); // gl
    void load_extension_GL_NV_image_formats(); // gles2
    void load_extension_GL_NV_instanced_arrays(); // gles2
    void load_extension_GL_NV_internalformat_sample_query(); // gl|glcore|gles2
    void load_extension_GL_NV_light_max_exponent(); // gl
    void load_extension_GL_NV_multisample_coverage(); // gl
    void load_extension_GL_NV_multisample_filter_hint(); // gl
    void load_extension_GL_NV_non_square_matrices(); // gles2
    void load_extension_GL_NV_occlusion_query(); // gl
    void load_extension_GL_NV_packed_depth_stencil(); // gl
    void load_extension_GL_NV_parameter_buffer_object(); // gl
    void load_extension_GL_NV_parameter_buffer_object2(); // gl
    void load_extension_GL_NV_path_rendering(); // gl|glcore|gles2
    void load_extension_GL_NV_path_rendering_shared_edge(); // gl|glcore|gles2
    void load_extension_GL_NV_pixel_data_range(); // gl
    void load_extension_GL_NV_point_sprite(); // gl
    void load_extension_GL_NV_polygon_mode(); // gles2
    void load_extension_GL_NV_present_video(); // gl
    void load_extension_GL_NV_primitive_restart(); // gl
    void load_extension_GL_NV_read_buffer(); // gles2
    void load_extension_GL_NV_read_buffer_front(); // gles2
    void load_extension_GL_NV_read_depth(); // gles2
    void load_extension_GL_NV_read_depth_stencil(); // gles2
    void load_extension_GL_NV_read_stencil(); // gles2
    void load_extension_GL_NV_register_combiners(); // gl
    void load_extension_GL_NV_register_combiners2(); // gl
    void load_extension_GL_NV_robustness_video_memory_purge(); // gl
    void load_extension_GL_NV_sRGB_formats(); // gles2
    void load_extension_GL_NV_sample_locations(); // gl|glcore|gles2
    void load_extension_GL_NV_sample_mask_override_coverage(); // gl|glcore|gles2
    void load_extension_GL_NV_shader_atomic_counters(); // gl
    void load_extension_GL_NV_shader_atomic_float(); // gl
    void load_extension_GL_NV_shader_atomic_float64(); // gl
    void load_extension_GL_NV_shader_atomic_fp16_vector(); // gl|glcore|gles2
    void load_extension_GL_NV_shader_atomic_int64(); // gl
    void load_extension_GL_NV_shader_buffer_load(); // gl
    void load_extension_GL_NV_shader_buffer_store(); // gl
    void load_extension_GL_NV_shader_noperspective_interpolation(); // gles2
    void load_extension_GL_NV_shader_storage_buffer_object(); // gl
    void load_extension_GL_NV_shader_thread_group(); // gl
    void load_extension_GL_NV_shader_thread_shuffle(); // gl
    void load_extension_GL_NV_shadow_samplers_array(); // gles2
    void load_extension_GL_NV_shadow_samplers_cube(); // gles2
    void load_extension_GL_NV_stereo_view_rendering(); // gl
    void load_extension_GL_NV_tessellation_program5(); // gl
    void load_extension_GL_NV_texgen_emboss(); // gl
    void load_extension_GL_NV_texgen_reflection(); // gl
    void load_extension_GL_NV_texture_barrier(); // gl
    void load_extension_GL_NV_texture_border_clamp(); // gles2
    void load_extension_GL_NV_texture_compression_s3tc_update(); // gles2
    void load_extension_GL_NV_texture_compression_vtc(); // gl
    void load_extension_GL_NV_texture_env_combine4(); // gl
    void load_extension_GL_NV_texture_expand_normal(); // gl
    void load_extension_GL_NV_texture_multisample(); // gl
    void load_extension_GL_NV_texture_npot_2D_mipmap(); // gles2
    void load_extension_GL_NV_texture_rectangle(); // gl
    void load_extension_GL_NV_texture_shader(); // gl
    void load_extension_GL_NV_texture_shader2(); // gl
    void load_extension_GL_NV_texture_shader3(); // gl
    void load_extension_GL_NV_transform_feedback(); // gl
    void load_extension_GL_NV_transform_feedback2(); // gl
    void load_extension_GL_NV_uniform_buffer_unified_memory(); // gl
    void load_extension_GL_NV_vdpau_interop(); // gl
    void load_extension_GL_NV_vertex_array_range(); // gl
    void load_extension_GL_NV_vertex_array_range2(); // gl
    void load_extension_GL_NV_vertex_attrib_integer_64bit(); // gl
    void load_extension_GL_NV_vertex_buffer_unified_memory(); // gl
    void load_extension_GL_NV_vertex_program(); // gl
    void load_extension_GL_NV_vertex_program1_1(); // gl
    void load_extension_GL_NV_vertex_program2(); // gl
    void load_extension_GL_NV_vertex_program2_option(); // gl
    void load_extension_GL_NV_vertex_program3(); // gl
    void load_extension_GL_NV_vertex_program4(); // gl
    void load_extension_GL_NV_video_capture(); // gl
    void load_extension_GL_NV_viewport_array(); // gles2
    void load_extension_GL_NV_viewport_array2(); // gl|glcore|gles2
    void load_extension_GL_NV_viewport_swizzle(); // gl|glcore|gles2
    void load_extension_GL_OES_EGL_image(); // gles1|gles2
    void load_extension_GL_OES_EGL_image_external(); // gles1|gles2
    void load_extension_GL_OES_EGL_image_external_essl3(); // gles2
    void load_extension_GL_OES_blend_equation_separate(); // gles1
    void load_extension_GL_OES_blend_func_separate(); // gles1
    void load_extension_GL_OES_blend_subtract(); // gles1
    void load_extension_GL_OES_byte_coordinates(); // gl|gles1
    void load_extension_GL_OES_compressed_ETC1_RGB8_sub_texture(); // gles1|gles2
    void load_extension_GL_OES_compressed_ETC1_RGB8_texture(); // gles1|gles2
    void load_extension_GL_OES_compressed_paletted_texture(); // gl|gles1|gles2
    void load_extension_GL_OES_copy_image(); // gles2
    void load_extension_GL_OES_depth24(); // gles1|gles2|glsc2
    void load_extension_GL_OES_depth32(); // gles1|gles2|glsc2
    void load_extension_GL_OES_depth_texture(); // gles2
    void load_extension_GL_OES_draw_buffers_indexed(); // gles2
    void load_extension_GL_OES_draw_elements_base_vertex(); // gles2
    void load_extension_GL_OES_draw_texture(); // gles1
    void load_extension_GL_OES_element_index_uint(); // gles1|gles2
    void load_extension_GL_OES_extended_matrix_palette(); // gles1
    void load_extension_GL_OES_fbo_render_mipmap(); // gles1|gles2
    void load_extension_GL_OES_fixed_point(); // gl|gles1
    void load_extension_GL_OES_fragment_precision_high(); // gles2
    void load_extension_GL_OES_framebuffer_object(); // gles1
    void load_extension_GL_OES_geometry_point_size(); // gles2
    void load_extension_GL_OES_geometry_shader(); // gles2
    void load_extension_GL_OES_get_program_binary(); // gles2
    void load_extension_GL_OES_gpu_shader5(); // gles2
    void load_extension_GL_OES_mapbuffer(); // gles1|gles2
    void load_extension_GL_OES_matrix_get(); // gles1
    void load_extension_GL_OES_matrix_palette(); // gles1
    void load_extension_GL_OES_packed_depth_stencil(); // gles1|gles2
    void load_extension_GL_OES_point_size_array(); // gles1
    void load_extension_GL_OES_point_sprite(); // gles1
    void load_extension_GL_OES_primitive_bounding_box(); // gles2
    void load_extension_GL_OES_query_matrix(); // gl|gles1
    void load_extension_GL_OES_read_format(); // gl|gles1
    void load_extension_GL_OES_required_internalformat(); // gles1|gles2
    void load_extension_GL_OES_rgb8_rgba8(); // gles1|gles2|glsc2
    void load_extension_GL_OES_sample_shading(); // gles2
    void load_extension_GL_OES_sample_variables(); // gles2
    void load_extension_GL_OES_shader_image_atomic(); // gles2
    void load_extension_GL_OES_shader_io_blocks(); // gles2
    void load_extension_GL_OES_shader_multisample_interpolation(); // gles2
    void load_extension_GL_OES_single_precision(); // gl|gles1
    void load_extension_GL_OES_standard_derivatives(); // gles2|glsc2
    void load_extension_GL_OES_stencil1(); // gles1|gles2
    void load_extension_GL_OES_stencil4(); // gles1|gles2
    void load_extension_GL_OES_stencil8(); // gles1
    void load_extension_GL_OES_stencil_wrap(); // gles1
    void load_extension_GL_OES_surfaceless_context(); // gles2
    void load_extension_GL_OES_tessellation_point_size(); // gles2
    void load_extension_GL_OES_tessellation_shader(); // gles2
    void load_extension_GL_OES_texture_3D(); // gles2
    void load_extension_GL_OES_texture_border_clamp(); // gles2
    void load_extension_GL_OES_texture_buffer(); // gles2
    void load_extension_GL_OES_texture_compression_astc(); // gles2
    void load_extension_GL_OES_texture_cube_map(); // gles1
    void load_extension_GL_OES_texture_cube_map_array(); // gles2
    void load_extension_GL_OES_texture_env_crossbar(); // gles1
    void load_extension_GL_OES_texture_float(); // gles2
    void load_extension_GL_OES_texture_float_linear(); // gles2
    void load_extension_GL_OES_texture_half_float(); // gles2
    void load_extension_GL_OES_texture_half_float_linear(); // gles2
    void load_extension_GL_OES_texture_mirrored_repeat(); // gles1
    void load_extension_GL_OES_texture_npot(); // gles2
    void load_extension_GL_OES_texture_stencil8(); // gles2
    void load_extension_GL_OES_texture_storage_multisample_2d_array(); // gles2
    void load_extension_GL_OES_texture_view(); // gles2
    void load_extension_GL_OES_vertex_array_object(); // gles1|gles2
    void load_extension_GL_OES_vertex_half_float(); // gles2
    void load_extension_GL_OES_vertex_type_10_10_10_2(); // gles2
    void load_extension_GL_OES_viewport_array(); // gles2
    void load_extension_GL_OML_interlace(); // gl
    void load_extension_GL_OML_resample(); // gl
    void load_extension_GL_OML_subsample(); // gl
    void load_extension_GL_OVR_multiview(); // gl|glcore|gles2
    void load_extension_GL_OVR_multiview2(); // gl|glcore|gles2
    void load_extension_GL_OVR_multiview_multisampled_render_to_texture(); // gles2
    void load_extension_GL_PGI_misc_hints(); // gl
    void load_extension_GL_PGI_vertex_hints(); // gl
    void load_extension_GL_QCOM_alpha_test(); // gles2
    void load_extension_GL_QCOM_binning_control(); // gles2
    void load_extension_GL_QCOM_driver_control(); // gles1|gles2
    void load_extension_GL_QCOM_extended_get(); // gles1|gles2
    void load_extension_GL_QCOM_extended_get2(); // gles1|gles2
    void load_extension_GL_QCOM_perfmon_global_mode(); // gles1|gles2
    void load_extension_GL_QCOM_tiled_rendering(); // gles1|gles2
    void load_extension_GL_QCOM_writeonly_rendering(); // gles1|gles2
    void load_extension_GL_REND_screen_coordinates(); // gl
    void load_extension_GL_S3_s3tc(); // gl
    void load_extension_GL_SGIS_detail_texture(); // gl
    void load_extension_GL_SGIS_fog_function(); // gl
    void load_extension_GL_SGIS_generate_mipmap(); // gl
    void load_extension_GL_SGIS_multisample(); // gl
    void load_extension_GL_SGIS_pixel_texture(); // gl
    void load_extension_GL_SGIS_point_line_texgen(); // gl
    void load_extension_GL_SGIS_point_parameters(); // gl
    void load_extension_GL_SGIS_sharpen_texture(); // gl
    void load_extension_GL_SGIS_texture4D(); // gl
    void load_extension_GL_SGIS_texture_border_clamp(); // gl
    void load_extension_GL_SGIS_texture_color_mask(); // gl
    void load_extension_GL_SGIS_texture_edge_clamp(); // gl
    void load_extension_GL_SGIS_texture_filter4(); // gl
    void load_extension_GL_SGIS_texture_lod(); // gl
    void load_extension_GL_SGIS_texture_select(); // gl
    void load_extension_GL_SGIX_async(); // gl
    void load_extension_GL_SGIX_async_histogram(); // gl
    void load_extension_GL_SGIX_async_pixel(); // gl
    void load_extension_GL_SGIX_blend_alpha_minmax(); // gl
    void load_extension_GL_SGIX_calligraphic_fragment(); // gl
    void load_extension_GL_SGIX_clipmap(); // gl
    void load_extension_GL_SGIX_convolution_accuracy(); // gl
    void load_extension_GL_SGIX_depth_pass_instrument(); // gl
    void load_extension_GL_SGIX_depth_texture(); // gl
    void load_extension_GL_SGIX_flush_raster(); // gl
    void load_extension_GL_SGIX_fog_offset(); // gl
    void load_extension_GL_SGIX_fragment_lighting(); // gl
    void load_extension_GL_SGIX_framezoom(); // gl
    void load_extension_GL_SGIX_igloo_interface(); // gl
    void load_extension_GL_SGIX_instruments(); // gl
    void load_extension_GL_SGIX_interlace(); // gl
    void load_extension_GL_SGIX_ir_instrument1(); // gl
    void load_extension_GL_SGIX_list_priority(); // gl
    void load_extension_GL_SGIX_pixel_texture(); // gl
    void load_extension_GL_SGIX_pixel_tiles(); // gl
    void load_extension_GL_SGIX_polynomial_ffd(); // gl
    void load_extension_GL_SGIX_reference_plane(); // gl
    void load_extension_GL_SGIX_resample(); // gl
    void load_extension_GL_SGIX_scalebias_hint(); // gl
    void load_extension_GL_SGIX_shadow(); // gl
    void load_extension_GL_SGIX_shadow_ambient(); // gl
    void load_extension_GL_SGIX_sprite(); // gl
    void load_extension_GL_SGIX_subsample(); // gl
    void load_extension_GL_SGIX_tag_sample_buffer(); // gl
    void load_extension_GL_SGIX_texture_add_env(); // gl
    void load_extension_GL_SGIX_texture_coordinate_clamp(); // gl
    void load_extension_GL_SGIX_texture_lod_bias(); // gl
    void load_extension_GL_SGIX_texture_multi_buffer(); // gl
    void load_extension_GL_SGIX_texture_scale_bias(); // gl
    void load_extension_GL_SGIX_vertex_preclip(); // gl
    void load_extension_GL_SGIX_ycrcb(); // gl
    void load_extension_GL_SGIX_ycrcb_subsample(); // gl
    void load_extension_GL_SGIX_ycrcba(); // gl
    void load_extension_GL_SGI_color_matrix(); // gl
    void load_extension_GL_SGI_color_table(); // gl
    void load_extension_GL_SGI_texture_color_table(); // gl
    void load_extension_GL_SUNX_constant_data(); // gl
    void load_extension_GL_SUN_convolution_border_modes(); // gl
    void load_extension_GL_SUN_global_alpha(); // gl
    void load_extension_GL_SUN_mesh_array(); // gl
    void load_extension_GL_SUN_slice_accum(); // gl
    void load_extension_GL_SUN_triangle_list(); // gl
    void load_extension_GL_SUN_vertex(); // gl
    void load_extension_GL_VIV_shader_binary(); // gles2
    void load_extension_GL_WIN_phong_shading(); // gl
    void load_extension_GL_WIN_specular_fog(); // gl
}

#define GL_CURRENT_BIT                                            0x00000001
#define GL_POINT_BIT                                              0x00000002
#define GL_LINE_BIT                                               0x00000004
#define GL_POLYGON_BIT                                            0x00000008
#define GL_POLYGON_STIPPLE_BIT                                    0x00000010
#define GL_PIXEL_MODE_BIT                                         0x00000020
#define GL_LIGHTING_BIT                                           0x00000040
#define GL_FOG_BIT                                                0x00000080
#define GL_DEPTH_BUFFER_BIT                                       0x00000100
#define GL_ACCUM_BUFFER_BIT                                       0x00000200
#define GL_STENCIL_BUFFER_BIT                                     0x00000400
#define GL_VIEWPORT_BIT                                           0x00000800
#define GL_TRANSFORM_BIT                                          0x00001000
#define GL_ENABLE_BIT                                             0x00002000
#define GL_COLOR_BUFFER_BIT                                       0x00004000
#define GL_HINT_BIT                                               0x00008000
#define GL_EVAL_BIT                                               0x00010000
#define GL_LIST_BIT                                               0x00020000
#define GL_TEXTURE_BIT                                            0x00040000
#define GL_SCISSOR_BIT                                            0x00080000
#define GL_MULTISAMPLE_BIT                                        0x20000000
#define GL_MULTISAMPLE_BIT_ARB                                    0x20000000
#define GL_MULTISAMPLE_BIT_EXT                                    0x20000000
#define GL_MULTISAMPLE_BIT_3DFX                                   0x20000000
#define GL_ALL_ATTRIB_BITS                                        0xFFFFFFFF
#define GL_COVERAGE_BUFFER_BIT_NV                                 0x00008000
#define GL_CLIENT_PIXEL_STORE_BIT                                 0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT                                0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS                                 0xFFFFFFFF
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT                    0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT                                 0x00000002
#define GL_CONTEXT_FLAG_DEBUG_BIT_KHR                             0x00000002
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT                         0x00000004
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB                     0x00000004
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR                          0x00000008
#define GL_CONTEXT_FLAG_PROTECTED_CONTENT_BIT_EXT                 0x00000010
#define GL_CONTEXT_CORE_PROFILE_BIT                               0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT                      0x00000002
#define GL_MAP_READ_BIT                                           0x0001
#define GL_MAP_READ_BIT_EXT                                       0x0001
#define GL_MAP_WRITE_BIT                                          0x0002
#define GL_MAP_WRITE_BIT_EXT                                      0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                               0x0004
#define GL_MAP_INVALIDATE_RANGE_BIT_EXT                           0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                              0x0008
#define GL_MAP_INVALIDATE_BUFFER_BIT_EXT                          0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                                 0x0010
#define GL_MAP_FLUSH_EXPLICIT_BIT_EXT                             0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                                 0x0020
#define GL_MAP_UNSYNCHRONIZED_BIT_EXT                             0x0020
#define GL_MAP_PERSISTENT_BIT                                     0x0040
#define GL_MAP_PERSISTENT_BIT_EXT                                 0x0040
#define GL_MAP_COHERENT_BIT                                       0x0080
#define GL_MAP_COHERENT_BIT_EXT                                   0x0080
#define GL_DYNAMIC_STORAGE_BIT                                    0x0100
#define GL_DYNAMIC_STORAGE_BIT_EXT                                0x0100
#define GL_CLIENT_STORAGE_BIT                                     0x0200
#define GL_CLIENT_STORAGE_BIT_EXT                                 0x0200
#define GL_SPARSE_STORAGE_BIT_ARB                                 0x0400
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT                        0x00000001
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT                    0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT                              0x00000002
#define GL_ELEMENT_ARRAY_BARRIER_BIT_EXT                          0x00000002
#define GL_UNIFORM_BARRIER_BIT                                    0x00000004
#define GL_UNIFORM_BARRIER_BIT_EXT                                0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT                              0x00000008
#define GL_TEXTURE_FETCH_BARRIER_BIT_EXT                          0x00000008
#define GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV                    0x00000010
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT                        0x00000020
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT                    0x00000020
#define GL_COMMAND_BARRIER_BIT                                    0x00000040
#define GL_COMMAND_BARRIER_BIT_EXT                                0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT                               0x00000080
#define GL_PIXEL_BUFFER_BARRIER_BIT_EXT                           0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT                             0x00000100
#define GL_TEXTURE_UPDATE_BARRIER_BIT_EXT                         0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT                              0x00000200
#define GL_BUFFER_UPDATE_BARRIER_BIT_EXT                          0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT                                0x00000400
#define GL_FRAMEBUFFER_BARRIER_BIT_EXT                            0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT                         0x00000800
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT                     0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT                             0x00001000
#define GL_ATOMIC_COUNTER_BARRIER_BIT_EXT                         0x00001000
#define GL_SHADER_STORAGE_BARRIER_BIT                             0x00002000
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT                       0x00004000
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT                   0x00004000
#define GL_QUERY_BUFFER_BARRIER_BIT                               0x00008000
#define GL_ALL_BARRIER_BITS                                       0xFFFFFFFF
#define GL_ALL_BARRIER_BITS_EXT                                   0xFFFFFFFF
#define GL_QUERY_DEPTH_PASS_EVENT_BIT_AMD                         0x00000001
#define GL_QUERY_DEPTH_FAIL_EVENT_BIT_AMD                         0x00000002
#define GL_QUERY_STENCIL_FAIL_EVENT_BIT_AMD                       0x00000004
#define GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD                  0x00000008
#define GL_QUERY_ALL_EVENT_BITS_AMD                               0xFFFFFFFF
#define GL_SYNC_FLUSH_COMMANDS_BIT                                0x00000001
#define GL_SYNC_FLUSH_COMMANDS_BIT_APPLE                          0x00000001
#define GL_VERTEX_SHADER_BIT                                      0x00000001
#define GL_VERTEX_SHADER_BIT_EXT                                  0x00000001
#define GL_FRAGMENT_SHADER_BIT                                    0x00000002
#define GL_FRAGMENT_SHADER_BIT_EXT                                0x00000002
#define GL_GEOMETRY_SHADER_BIT                                    0x00000004
#define GL_GEOMETRY_SHADER_BIT_EXT                                0x00000004
#define GL_GEOMETRY_SHADER_BIT_OES                                0x00000004
#define GL_TESS_CONTROL_SHADER_BIT                                0x00000008
#define GL_TESS_CONTROL_SHADER_BIT_EXT                            0x00000008
#define GL_TESS_CONTROL_SHADER_BIT_OES                            0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT                             0x00000010
#define GL_TESS_EVALUATION_SHADER_BIT_EXT                         0x00000010
#define GL_TESS_EVALUATION_SHADER_BIT_OES                         0x00000010
#define GL_COMPUTE_SHADER_BIT                                     0x00000020
#define GL_ALL_SHADER_BITS                                        0xFFFFFFFF
#define GL_ALL_SHADER_BITS_EXT                                    0xFFFFFFFF
#define GL_TEXTURE_STORAGE_SPARSE_BIT_AMD                         0x00000001
#define GL_RED_BIT_ATI                                            0x00000001
#define GL_GREEN_BIT_ATI                                          0x00000002
#define GL_BLUE_BIT_ATI                                           0x00000004
#define GL_2X_BIT_ATI                                             0x00000001
#define GL_4X_BIT_ATI                                             0x00000002
#define GL_8X_BIT_ATI                                             0x00000004
#define GL_HALF_BIT_ATI                                           0x00000008
#define GL_QUARTER_BIT_ATI                                        0x00000010
#define GL_EIGHTH_BIT_ATI                                         0x00000020
#define GL_SATURATE_BIT_ATI                                       0x00000040
#define GL_COMP_BIT_ATI                                           0x00000002
#define GL_NEGATE_BIT_ATI                                         0x00000004
#define GL_BIAS_BIT_ATI                                           0x00000008
#define GL_TRACE_OPERATIONS_BIT_MESA                              0x0001
#define GL_TRACE_PRIMITIVES_BIT_MESA                              0x0002
#define GL_TRACE_ARRAYS_BIT_MESA                                  0x0004
#define GL_TRACE_TEXTURES_BIT_MESA                                0x0008
#define GL_TRACE_PIXELS_BIT_MESA                                  0x0010
#define GL_TRACE_ERRORS_BIT_MESA                                  0x0020
#define GL_TRACE_ALL_BITS_MESA                                    0xFFFF
#define GL_BOLD_BIT_NV                                            0x01
#define GL_ITALIC_BIT_NV                                          0x02
#define GL_GLYPH_WIDTH_BIT_NV                                     0x01
#define GL_GLYPH_HEIGHT_BIT_NV                                    0x02
#define GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV                      0x04
#define GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV                      0x08
#define GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV                0x10
#define GL_GLYPH_VERTICAL_BEARING_X_BIT_NV                        0x20
#define GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV                        0x40
#define GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV                  0x80
#define GL_GLYPH_HAS_KERNING_BIT_NV                               0x100
#define GL_FONT_X_MIN_BOUNDS_BIT_NV                               0x00010000
#define GL_FONT_Y_MIN_BOUNDS_BIT_NV                               0x00020000
#define GL_FONT_X_MAX_BOUNDS_BIT_NV                               0x00040000
#define GL_FONT_Y_MAX_BOUNDS_BIT_NV                               0x00080000
#define GL_FONT_UNITS_PER_EM_BIT_NV                               0x00100000
#define GL_FONT_ASCENDER_BIT_NV                                   0x00200000
#define GL_FONT_DESCENDER_BIT_NV                                  0x00400000
#define GL_FONT_HEIGHT_BIT_NV                                     0x00800000
#define GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV                          0x01000000
#define GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV                         0x02000000
#define GL_FONT_UNDERLINE_POSITION_BIT_NV                         0x04000000
#define GL_FONT_UNDERLINE_THICKNESS_BIT_NV                        0x08000000
#define GL_FONT_HAS_KERNING_BIT_NV                                0x10000000
#define GL_FONT_NUM_GLYPH_INDICES_BIT_NV                          0x20000000
#define GL_PERFQUERY_SINGLE_CONTEXT_INTEL                         0x00000000
#define GL_PERFQUERY_GLOBAL_CONTEXT_INTEL                         0x00000001
#define GL_VERTEX23_BIT_PGI                                       0x00000004
#define GL_VERTEX4_BIT_PGI                                        0x00000008
#define GL_COLOR3_BIT_PGI                                         0x00010000
#define GL_COLOR4_BIT_PGI                                         0x00020000
#define GL_EDGEFLAG_BIT_PGI                                       0x00040000
#define GL_INDEX_BIT_PGI                                          0x00080000
#define GL_MAT_AMBIENT_BIT_PGI                                    0x00100000
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI                        0x00200000
#define GL_MAT_DIFFUSE_BIT_PGI                                    0x00400000
#define GL_MAT_EMISSION_BIT_PGI                                   0x00800000
#define GL_MAT_COLOR_INDEXES_BIT_PGI                              0x01000000
#define GL_MAT_SHININESS_BIT_PGI                                  0x02000000
#define GL_MAT_SPECULAR_BIT_PGI                                   0x04000000
#define GL_NORMAL_BIT_PGI                                         0x08000000
#define GL_TEXCOORD1_BIT_PGI                                      0x10000000
#define GL_TEXCOORD2_BIT_PGI                                      0x20000000
#define GL_TEXCOORD3_BIT_PGI                                      0x40000000
#define GL_TEXCOORD4_BIT_PGI                                      0x80000000
#define GL_COLOR_BUFFER_BIT0_QCOM                                 0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM                                 0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM                                 0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM                                 0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM                                 0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM                                 0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM                                 0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM                                 0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM                                 0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM                                 0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM                                 0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM                                 0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM                                 0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM                                 0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM                                 0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM                                 0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM                               0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM                               0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM                               0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM                               0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM                               0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM                               0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM                               0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM                               0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM                           0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM                           0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM                           0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM                           0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM                           0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM                           0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM                           0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM                           0x80000000
#define GL_TEXTURE_DEFORMATION_BIT_SGIX                           0x00000001
#define GL_GEOMETRY_DEFORMATION_BIT_SGIX                          0x00000002
#define GL_TERMINATE_SEQUENCE_COMMAND_NV                          0x0000
#define GL_NOP_COMMAND_NV                                         0x0001
#define GL_DRAW_ELEMENTS_COMMAND_NV                               0x0002
#define GL_DRAW_ARRAYS_COMMAND_NV                                 0x0003
#define GL_DRAW_ELEMENTS_STRIP_COMMAND_NV                         0x0004
#define GL_DRAW_ARRAYS_STRIP_COMMAND_NV                           0x0005
#define GL_DRAW_ELEMENTS_INSTANCED_COMMAND_NV                     0x0006
#define GL_DRAW_ARRAYS_INSTANCED_COMMAND_NV                       0x0007
#define GL_ELEMENT_ADDRESS_COMMAND_NV                             0x0008
#define GL_ATTRIBUTE_ADDRESS_COMMAND_NV                           0x0009
#define GL_UNIFORM_ADDRESS_COMMAND_NV                             0x000A
#define GL_BLEND_COLOR_COMMAND_NV                                 0x000B
#define GL_STENCIL_REF_COMMAND_NV                                 0x000C
#define GL_LINE_WIDTH_COMMAND_NV                                  0x000D
#define GL_POLYGON_OFFSET_COMMAND_NV                              0x000E
#define GL_ALPHA_REF_COMMAND_NV                                   0x000F
#define GL_VIEWPORT_COMMAND_NV                                    0x0010
#define GL_SCISSOR_COMMAND_NV                                     0x0011
#define GL_FRONT_FACE_COMMAND_NV                                  0x0012
#define GL_LAYOUT_DEFAULT_INTEL                                   0
#define GL_LAYOUT_LINEAR_INTEL                                    1
#define GL_LAYOUT_LINEAR_CPU_CACHED_INTEL                         2
#define GL_CLOSE_PATH_NV                                          0x00
#define GL_MOVE_TO_NV                                             0x02
#define GL_RELATIVE_MOVE_TO_NV                                    0x03
#define GL_LINE_TO_NV                                             0x04
#define GL_RELATIVE_LINE_TO_NV                                    0x05
#define GL_HORIZONTAL_LINE_TO_NV                                  0x06
#define GL_RELATIVE_HORIZONTAL_LINE_TO_NV                         0x07
#define GL_VERTICAL_LINE_TO_NV                                    0x08
#define GL_RELATIVE_VERTICAL_LINE_TO_NV                           0x09
#define GL_QUADRATIC_CURVE_TO_NV                                  0x0A
#define GL_RELATIVE_QUADRATIC_CURVE_TO_NV                         0x0B
#define GL_CUBIC_CURVE_TO_NV                                      0x0C
#define GL_RELATIVE_CUBIC_CURVE_TO_NV                             0x0D
#define GL_SMOOTH_QUADRATIC_CURVE_TO_NV                           0x0E
#define GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV                  0x0F
#define GL_SMOOTH_CUBIC_CURVE_TO_NV                               0x10
#define GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV                      0x11
#define GL_SMALL_CCW_ARC_TO_NV                                    0x12
#define GL_RELATIVE_SMALL_CCW_ARC_TO_NV                           0x13
#define GL_SMALL_CW_ARC_TO_NV                                     0x14
#define GL_RELATIVE_SMALL_CW_ARC_TO_NV                            0x15
#define GL_LARGE_CCW_ARC_TO_NV                                    0x16
#define GL_RELATIVE_LARGE_CCW_ARC_TO_NV                           0x17
#define GL_LARGE_CW_ARC_TO_NV                                     0x18
#define GL_RELATIVE_LARGE_CW_ARC_TO_NV                            0x19
#define GL_CONIC_CURVE_TO_NV                                      0x1A
#define GL_RELATIVE_CONIC_CURVE_TO_NV                             0x1B
#define GL_SHARED_EDGE_NV                                         0xC0
#define GL_ROUNDED_RECT_NV                                        0xE8
#define GL_RELATIVE_ROUNDED_RECT_NV                               0xE9
#define GL_ROUNDED_RECT2_NV                                       0xEA
#define GL_RELATIVE_ROUNDED_RECT2_NV                              0xEB
#define GL_ROUNDED_RECT4_NV                                       0xEC
#define GL_RELATIVE_ROUNDED_RECT4_NV                              0xED
#define GL_ROUNDED_RECT8_NV                                       0xEE
#define GL_RELATIVE_ROUNDED_RECT8_NV                              0xEF
#define GL_RESTART_PATH_NV                                        0xF0
#define GL_DUP_FIRST_CUBIC_CURVE_TO_NV                            0xF2
#define GL_DUP_LAST_CUBIC_CURVE_TO_NV                             0xF4
#define GL_RECT_NV                                                0xF6
#define GL_RELATIVE_RECT_NV                                       0xF7
#define GL_CIRCULAR_CCW_ARC_TO_NV                                 0xF8
#define GL_CIRCULAR_CW_ARC_TO_NV                                  0xFA
#define GL_CIRCULAR_TANGENT_ARC_TO_NV                             0xFC
#define GL_ARC_TO_NV                                              0xFE
#define GL_RELATIVE_ARC_TO_NV                                     0xFF
#define GL_NEXT_BUFFER_NV                                         -2
#define GL_SKIP_COMPONENTS4_NV                                    -3
#define GL_SKIP_COMPONENTS3_NV                                    -4
#define GL_SKIP_COMPONENTS2_NV                                    -5
#define GL_SKIP_COMPONENTS1_NV                                    -6
#define GL_RESTART_SUN                                            0x0001
#define GL_REPLACE_MIDDLE_SUN                                     0x0002
#define GL_REPLACE_OLDEST_SUN                                     0x0003
#define GL_FALSE                                                  0
#define GL_NO_ERROR                                               0
#define GL_ZERO                                                   0
#define GL_NONE                                                   0
#define GL_NONE_OES                                               0
#define GL_TRUE                                                   1
#define GL_ONE                                                    1
#define GL_INVALID_INDEX                                          0xFFFFFFFF
#define GL_ALL_PIXELS_AMD                                         0xFFFFFFFF
#define GL_TIMEOUT_IGNORED                                        0xFFFFFFFFFFFFFFFF
#define GL_TIMEOUT_IGNORED_APPLE                                  0xFFFFFFFFFFFFFFFF
#define GL_VERSION_ES_CL_1_0                                      1
#define GL_VERSION_ES_CM_1_1                                      1
#define GL_VERSION_ES_CL_1_1                                      1
#define GL_POINTS                                                 0x0000
#define GL_LINES                                                  0x0001
#define GL_LINE_LOOP                                              0x0002
#define GL_LINE_STRIP                                             0x0003
#define GL_TRIANGLES                                              0x0004
#define GL_TRIANGLE_STRIP                                         0x0005
#define GL_TRIANGLE_FAN                                           0x0006
#define GL_QUADS                                                  0x0007
#define GL_QUADS_EXT                                              0x0007
#define GL_QUADS_OES                                              0x0007
#define GL_QUAD_STRIP                                             0x0008
#define GL_POLYGON                                                0x0009
#define GL_LINES_ADJACENCY                                        0x000A
#define GL_LINES_ADJACENCY_ARB                                    0x000A
#define GL_LINES_ADJACENCY_EXT                                    0x000A
#define GL_LINES_ADJACENCY_OES                                    0x000A
#define GL_LINE_STRIP_ADJACENCY                                   0x000B
#define GL_LINE_STRIP_ADJACENCY_ARB                               0x000B
#define GL_LINE_STRIP_ADJACENCY_EXT                               0x000B
#define GL_LINE_STRIP_ADJACENCY_OES                               0x000B
#define GL_TRIANGLES_ADJACENCY                                    0x000C
#define GL_TRIANGLES_ADJACENCY_ARB                                0x000C
#define GL_TRIANGLES_ADJACENCY_EXT                                0x000C
#define GL_TRIANGLES_ADJACENCY_OES                                0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY                               0x000D
#define GL_TRIANGLE_STRIP_ADJACENCY_ARB                           0x000D
#define GL_TRIANGLE_STRIP_ADJACENCY_EXT                           0x000D
#define GL_TRIANGLE_STRIP_ADJACENCY_OES                           0x000D
#define GL_PATCHES                                                0x000E
#define GL_PATCHES_EXT                                            0x000E
#define GL_PATCHES_OES                                            0x000E
#define GL_ACCUM                                                  0x0100
#define GL_LOAD                                                   0x0101
#define GL_RETURN                                                 0x0102
#define GL_MULT                                                   0x0103
#define GL_ADD                                                    0x0104
#define GL_NEVER                                                  0x0200
#define GL_LESS                                                   0x0201
#define GL_EQUAL                                                  0x0202
#define GL_LEQUAL                                                 0x0203
#define GL_GREATER                                                0x0204
#define GL_NOTEQUAL                                               0x0205
#define GL_GEQUAL                                                 0x0206
#define GL_ALWAYS                                                 0x0207
#define GL_SRC_COLOR                                              0x0300
#define GL_ONE_MINUS_SRC_COLOR                                    0x0301
#define GL_SRC_ALPHA                                              0x0302
#define GL_ONE_MINUS_SRC_ALPHA                                    0x0303
#define GL_DST_ALPHA                                              0x0304
#define GL_ONE_MINUS_DST_ALPHA                                    0x0305
#define GL_DST_COLOR                                              0x0306
#define GL_ONE_MINUS_DST_COLOR                                    0x0307
#define GL_SRC_ALPHA_SATURATE                                     0x0308
#define GL_SRC_ALPHA_SATURATE_EXT                                 0x0308
#define GL_FRONT_LEFT                                             0x0400
#define GL_FRONT_RIGHT                                            0x0401
#define GL_BACK_LEFT                                              0x0402
#define GL_BACK_RIGHT                                             0x0403
#define GL_FRONT                                                  0x0404
#define GL_BACK                                                   0x0405
#define GL_LEFT                                                   0x0406
#define GL_RIGHT                                                  0x0407
#define GL_FRONT_AND_BACK                                         0x0408
#define GL_AUX0                                                   0x0409
#define GL_AUX1                                                   0x040A
#define GL_AUX2                                                   0x040B
#define GL_AUX3                                                   0x040C
#define GL_INVALID_ENUM                                           0x0500
#define GL_INVALID_VALUE                                          0x0501
#define GL_INVALID_OPERATION                                      0x0502
#define GL_STACK_OVERFLOW                                         0x0503
#define GL_STACK_OVERFLOW_KHR                                     0x0503
#define GL_STACK_UNDERFLOW                                        0x0504
#define GL_STACK_UNDERFLOW_KHR                                    0x0504
#define GL_OUT_OF_MEMORY                                          0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION                          0x0506
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                      0x0506
#define GL_INVALID_FRAMEBUFFER_OPERATION_OES                      0x0506
#define GL_CONTEXT_LOST                                           0x0507
#define GL_CONTEXT_LOST_KHR                                       0x0507
#define GL_2D                                                     0x0600
#define GL_3D                                                     0x0601
#define GL_3D_COLOR                                               0x0602
#define GL_3D_COLOR_TEXTURE                                       0x0603
#define GL_4D_COLOR_TEXTURE                                       0x0604
#define GL_PASS_THROUGH_TOKEN                                     0x0700
#define GL_POINT_TOKEN                                            0x0701
#define GL_LINE_TOKEN                                             0x0702
#define GL_POLYGON_TOKEN                                          0x0703
#define GL_BITMAP_TOKEN                                           0x0704
#define GL_DRAW_PIXEL_TOKEN                                       0x0705
#define GL_COPY_PIXEL_TOKEN                                       0x0706
#define GL_LINE_RESET_TOKEN                                       0x0707
#define GL_EXP                                                    0x0800
#define GL_EXP2                                                   0x0801
#define GL_CW                                                     0x0900
#define GL_CCW                                                    0x0901
#define GL_COEFF                                                  0x0A00
#define GL_ORDER                                                  0x0A01
#define GL_DOMAIN                                                 0x0A02
#define GL_CURRENT_COLOR                                          0x0B00
#define GL_CURRENT_INDEX                                          0x0B01
#define GL_CURRENT_NORMAL                                         0x0B02
#define GL_CURRENT_TEXTURE_COORDS                                 0x0B03
#define GL_CURRENT_RASTER_COLOR                                   0x0B04
#define GL_CURRENT_RASTER_INDEX                                   0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS                          0x0B06
#define GL_CURRENT_RASTER_POSITION                                0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID                          0x0B08
#define GL_CURRENT_RASTER_DISTANCE                                0x0B09
#define GL_POINT_SMOOTH                                           0x0B10
#define GL_POINT_SIZE                                             0x0B11
#define GL_POINT_SIZE_RANGE                                       0x0B12
#define GL_SMOOTH_POINT_SIZE_RANGE                                0x0B12
#define GL_POINT_SIZE_GRANULARITY                                 0x0B13
#define GL_SMOOTH_POINT_SIZE_GRANULARITY                          0x0B13
#define GL_LINE_SMOOTH                                            0x0B20
#define GL_LINE_WIDTH                                             0x0B21
#define GL_LINE_WIDTH_RANGE                                       0x0B22
#define GL_SMOOTH_LINE_WIDTH_RANGE                                0x0B22
#define GL_LINE_WIDTH_GRANULARITY                                 0x0B23
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY                          0x0B23
#define GL_LINE_STIPPLE                                           0x0B24
#define GL_LINE_STIPPLE_PATTERN                                   0x0B25
#define GL_LINE_STIPPLE_REPEAT                                    0x0B26
#define GL_LIST_MODE                                              0x0B30
#define GL_MAX_LIST_NESTING                                       0x0B31
#define GL_LIST_BASE                                              0x0B32
#define GL_LIST_INDEX                                             0x0B33
#define GL_POLYGON_MODE                                           0x0B40
#define GL_POLYGON_MODE_NV                                        0x0B40
#define GL_POLYGON_SMOOTH                                         0x0B41
#define GL_POLYGON_STIPPLE                                        0x0B42
#define GL_EDGE_FLAG                                              0x0B43
#define GL_CULL_FACE                                              0x0B44
#define GL_CULL_FACE_MODE                                         0x0B45
#define GL_FRONT_FACE                                             0x0B46
#define GL_LIGHTING                                               0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER                               0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE                                   0x0B52
#define GL_LIGHT_MODEL_AMBIENT                                    0x0B53
#define GL_SHADE_MODEL                                            0x0B54
#define GL_COLOR_MATERIAL_FACE                                    0x0B55
#define GL_COLOR_MATERIAL_PARAMETER                               0x0B56
#define GL_COLOR_MATERIAL                                         0x0B57
#define GL_FOG                                                    0x0B60
#define GL_FOG_INDEX                                              0x0B61
#define GL_FOG_DENSITY                                            0x0B62
#define GL_FOG_START                                              0x0B63
#define GL_FOG_END                                                0x0B64
#define GL_FOG_MODE                                               0x0B65
#define GL_FOG_COLOR                                              0x0B66
#define GL_DEPTH_RANGE                                            0x0B70
#define GL_DEPTH_TEST                                             0x0B71
#define GL_DEPTH_WRITEMASK                                        0x0B72
#define GL_DEPTH_CLEAR_VALUE                                      0x0B73
#define GL_DEPTH_FUNC                                             0x0B74
#define GL_ACCUM_CLEAR_VALUE                                      0x0B80
#define GL_STENCIL_TEST                                           0x0B90
#define GL_STENCIL_CLEAR_VALUE                                    0x0B91
#define GL_STENCIL_FUNC                                           0x0B92
#define GL_STENCIL_VALUE_MASK                                     0x0B93
#define GL_STENCIL_FAIL                                           0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL                                0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS                                0x0B96
#define GL_STENCIL_REF                                            0x0B97
#define GL_STENCIL_WRITEMASK                                      0x0B98
#define GL_MATRIX_MODE                                            0x0BA0
#define GL_NORMALIZE                                              0x0BA1
#define GL_VIEWPORT                                               0x0BA2
#define GL_MODELVIEW_STACK_DEPTH                                  0x0BA3
#define GL_MODELVIEW0_STACK_DEPTH_EXT                             0x0BA3
#define GL_PATH_MODELVIEW_STACK_DEPTH_NV                          0x0BA3
#define GL_PROJECTION_STACK_DEPTH                                 0x0BA4
#define GL_PATH_PROJECTION_STACK_DEPTH_NV                         0x0BA4
#define GL_TEXTURE_STACK_DEPTH                                    0x0BA5
#define GL_MODELVIEW_MATRIX                                       0x0BA6
#define GL_MODELVIEW0_MATRIX_EXT                                  0x0BA6
#define GL_PATH_MODELVIEW_MATRIX_NV                               0x0BA6
#define GL_PROJECTION_MATRIX                                      0x0BA7
#define GL_PATH_PROJECTION_MATRIX_NV                              0x0BA7
#define GL_TEXTURE_MATRIX                                         0x0BA8
#define GL_ATTRIB_STACK_DEPTH                                     0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH                              0x0BB1
#define GL_ALPHA_TEST                                             0x0BC0
#define GL_ALPHA_TEST_QCOM                                        0x0BC0
#define GL_ALPHA_TEST_FUNC                                        0x0BC1
#define GL_ALPHA_TEST_FUNC_QCOM                                   0x0BC1
#define GL_ALPHA_TEST_REF                                         0x0BC2
#define GL_ALPHA_TEST_REF_QCOM                                    0x0BC2
#define GL_DITHER                                                 0x0BD0
#define GL_BLEND_DST                                              0x0BE0
#define GL_BLEND_SRC                                              0x0BE1
#define GL_BLEND                                                  0x0BE2
#define GL_LOGIC_OP_MODE                                          0x0BF0
#define GL_INDEX_LOGIC_OP                                         0x0BF1
#define GL_LOGIC_OP                                               0x0BF1
#define GL_COLOR_LOGIC_OP                                         0x0BF2
#define GL_AUX_BUFFERS                                            0x0C00
#define GL_DRAW_BUFFER                                            0x0C01
#define GL_DRAW_BUFFER_EXT                                        0x0C01
#define GL_READ_BUFFER                                            0x0C02
#define GL_READ_BUFFER_EXT                                        0x0C02
#define GL_READ_BUFFER_NV                                         0x0C02
#define GL_SCISSOR_BOX                                            0x0C10
#define GL_SCISSOR_TEST                                           0x0C11
#define GL_INDEX_CLEAR_VALUE                                      0x0C20
#define GL_INDEX_WRITEMASK                                        0x0C21
#define GL_COLOR_CLEAR_VALUE                                      0x0C22
#define GL_COLOR_WRITEMASK                                        0x0C23
#define GL_INDEX_MODE                                             0x0C30
#define GL_RGBA_MODE                                              0x0C31
#define GL_DOUBLEBUFFER                                           0x0C32
#define GL_STEREO                                                 0x0C33
#define GL_RENDER_MODE                                            0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT                            0x0C50
#define GL_POINT_SMOOTH_HINT                                      0x0C51
#define GL_LINE_SMOOTH_HINT                                       0x0C52
#define GL_POLYGON_SMOOTH_HINT                                    0x0C53
#define GL_FOG_HINT                                               0x0C54
#define GL_TEXTURE_GEN_S                                          0x0C60
#define GL_TEXTURE_GEN_T                                          0x0C61
#define GL_TEXTURE_GEN_R                                          0x0C62
#define GL_TEXTURE_GEN_Q                                          0x0C63
#define GL_PIXEL_MAP_I_TO_I                                       0x0C70
#define GL_PIXEL_MAP_S_TO_S                                       0x0C71
#define GL_PIXEL_MAP_I_TO_R                                       0x0C72
#define GL_PIXEL_MAP_I_TO_G                                       0x0C73
#define GL_PIXEL_MAP_I_TO_B                                       0x0C74
#define GL_PIXEL_MAP_I_TO_A                                       0x0C75
#define GL_PIXEL_MAP_R_TO_R                                       0x0C76
#define GL_PIXEL_MAP_G_TO_G                                       0x0C77
#define GL_PIXEL_MAP_B_TO_B                                       0x0C78
#define GL_PIXEL_MAP_A_TO_A                                       0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE                                  0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE                                  0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE                                  0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE                                  0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE                                  0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE                                  0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE                                  0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE                                  0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE                                  0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE                                  0x0CB9
#define GL_UNPACK_SWAP_BYTES                                      0x0CF0
#define GL_UNPACK_LSB_FIRST                                       0x0CF1
#define GL_UNPACK_ROW_LENGTH                                      0x0CF2
#define GL_UNPACK_ROW_LENGTH_EXT                                  0x0CF2
#define GL_UNPACK_SKIP_ROWS                                       0x0CF3
#define GL_UNPACK_SKIP_ROWS_EXT                                   0x0CF3
#define GL_UNPACK_SKIP_PIXELS                                     0x0CF4
#define GL_UNPACK_SKIP_PIXELS_EXT                                 0x0CF4
#define GL_UNPACK_ALIGNMENT                                       0x0CF5
#define GL_PACK_SWAP_BYTES                                        0x0D00
#define GL_PACK_LSB_FIRST                                         0x0D01
#define GL_PACK_ROW_LENGTH                                        0x0D02
#define GL_PACK_SKIP_ROWS                                         0x0D03
#define GL_PACK_SKIP_PIXELS                                       0x0D04
#define GL_PACK_ALIGNMENT                                         0x0D05
#define GL_MAP_COLOR                                              0x0D10
#define GL_MAP_STENCIL                                            0x0D11
#define GL_INDEX_SHIFT                                            0x0D12
#define GL_INDEX_OFFSET                                           0x0D13
#define GL_RED_SCALE                                              0x0D14
#define GL_RED_BIAS                                               0x0D15
#define GL_ZOOM_X                                                 0x0D16
#define GL_ZOOM_Y                                                 0x0D17
#define GL_GREEN_SCALE                                            0x0D18
#define GL_GREEN_BIAS                                             0x0D19
#define GL_BLUE_SCALE                                             0x0D1A
#define GL_BLUE_BIAS                                              0x0D1B
#define GL_ALPHA_SCALE                                            0x0D1C
#define GL_ALPHA_BIAS                                             0x0D1D
#define GL_DEPTH_SCALE                                            0x0D1E
#define GL_DEPTH_BIAS                                             0x0D1F
#define GL_MAX_EVAL_ORDER                                         0x0D30
#define GL_MAX_LIGHTS                                             0x0D31
#define GL_MAX_CLIP_PLANES                                        0x0D32
#define GL_MAX_CLIP_PLANES_IMG                                    0x0D32
#define GL_MAX_CLIP_DISTANCES                                     0x0D32
#define GL_MAX_CLIP_DISTANCES_EXT                                 0x0D32
#define GL_MAX_CLIP_DISTANCES_APPLE                               0x0D32
#define GL_MAX_TEXTURE_SIZE                                       0x0D33
#define GL_MAX_PIXEL_MAP_TABLE                                    0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH                                 0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH                              0x0D36
#define GL_PATH_MAX_MODELVIEW_STACK_DEPTH_NV                      0x0D36
#define GL_MAX_NAME_STACK_DEPTH                                   0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH                             0x0D38
#define GL_PATH_MAX_PROJECTION_STACK_DEPTH_NV                     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH                                0x0D39
#define GL_MAX_VIEWPORT_DIMS                                      0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH                          0x0D3B
#define GL_SUBPIXEL_BITS                                          0x0D50
#define GL_INDEX_BITS                                             0x0D51
#define GL_RED_BITS                                               0x0D52
#define GL_GREEN_BITS                                             0x0D53
#define GL_BLUE_BITS                                              0x0D54
#define GL_ALPHA_BITS                                             0x0D55
#define GL_DEPTH_BITS                                             0x0D56
#define GL_STENCIL_BITS                                           0x0D57
#define GL_ACCUM_RED_BITS                                         0x0D58
#define GL_ACCUM_GREEN_BITS                                       0x0D59
#define GL_ACCUM_BLUE_BITS                                        0x0D5A
#define GL_ACCUM_ALPHA_BITS                                       0x0D5B
#define GL_NAME_STACK_DEPTH                                       0x0D70
#define GL_AUTO_NORMAL                                            0x0D80
#define GL_MAP1_COLOR_4                                           0x0D90
#define GL_MAP1_INDEX                                             0x0D91
#define GL_MAP1_NORMAL                                            0x0D92
#define GL_MAP1_TEXTURE_COORD_1                                   0x0D93
#define GL_MAP1_TEXTURE_COORD_2                                   0x0D94
#define GL_MAP1_TEXTURE_COORD_3                                   0x0D95
#define GL_MAP1_TEXTURE_COORD_4                                   0x0D96
#define GL_MAP1_VERTEX_3                                          0x0D97
#define GL_MAP1_VERTEX_4                                          0x0D98
#define GL_MAP2_COLOR_4                                           0x0DB0
#define GL_MAP2_INDEX                                             0x0DB1
#define GL_MAP2_NORMAL                                            0x0DB2
#define GL_MAP2_TEXTURE_COORD_1                                   0x0DB3
#define GL_MAP2_TEXTURE_COORD_2                                   0x0DB4
#define GL_MAP2_TEXTURE_COORD_3                                   0x0DB5
#define GL_MAP2_TEXTURE_COORD_4                                   0x0DB6
#define GL_MAP2_VERTEX_3                                          0x0DB7
#define GL_MAP2_VERTEX_4                                          0x0DB8
#define GL_MAP1_GRID_DOMAIN                                       0x0DD0
#define GL_MAP1_GRID_SEGMENTS                                     0x0DD1
#define GL_MAP2_GRID_DOMAIN                                       0x0DD2
#define GL_MAP2_GRID_SEGMENTS                                     0x0DD3
#define GL_TEXTURE_1D                                             0x0DE0
#define GL_TEXTURE_2D                                             0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER                                0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE                                   0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE                                   0x0DF2
#define GL_SELECTION_BUFFER_POINTER                               0x0DF3
#define GL_SELECTION_BUFFER_SIZE                                  0x0DF4
#define GL_TEXTURE_WIDTH                                          0x1000
#define GL_TEXTURE_HEIGHT                                         0x1001
#define GL_TEXTURE_INTERNAL_FORMAT                                0x1003
#define GL_TEXTURE_COMPONENTS                                     0x1003
#define GL_TEXTURE_BORDER_COLOR                                   0x1004
#define GL_TEXTURE_BORDER_COLOR_EXT                               0x1004
#define GL_TEXTURE_BORDER_COLOR_NV                                0x1004
#define GL_TEXTURE_BORDER_COLOR_OES                               0x1004
#define GL_TEXTURE_BORDER                                         0x1005
#define GL_TEXTURE_TARGET                                         0x1006
#define GL_DONT_CARE                                              0x1100
#define GL_FASTEST                                                0x1101
#define GL_NICEST                                                 0x1102
#define GL_AMBIENT                                                0x1200
#define GL_DIFFUSE                                                0x1201
#define GL_SPECULAR                                               0x1202
#define GL_POSITION                                               0x1203
#define GL_SPOT_DIRECTION                                         0x1204
#define GL_SPOT_EXPONENT                                          0x1205
#define GL_SPOT_CUTOFF                                            0x1206
#define GL_CONSTANT_ATTENUATION                                   0x1207
#define GL_LINEAR_ATTENUATION                                     0x1208
#define GL_QUADRATIC_ATTENUATION                                  0x1209
#define GL_COMPILE                                                0x1300
#define GL_COMPILE_AND_EXECUTE                                    0x1301
#define GL_BYTE                                                   0x1400
#define GL_UNSIGNED_BYTE                                          0x1401
#define GL_SHORT                                                  0x1402
#define GL_UNSIGNED_SHORT                                         0x1403
#define GL_INT                                                    0x1404
#define GL_UNSIGNED_INT                                           0x1405
#define GL_FLOAT                                                  0x1406
#define GL_2_BYTES                                                0x1407
#define GL_2_BYTES_NV                                             0x1407
#define GL_3_BYTES                                                0x1408
#define GL_3_BYTES_NV                                             0x1408
#define GL_4_BYTES                                                0x1409
#define GL_4_BYTES_NV                                             0x1409
#define GL_DOUBLE                                                 0x140A
#define GL_DOUBLE_EXT                                             0x140A
#define GL_HALF_FLOAT                                             0x140B
#define GL_HALF_FLOAT_ARB                                         0x140B
#define GL_HALF_FLOAT_NV                                          0x140B
#define GL_HALF_APPLE                                             0x140B
#define GL_FIXED                                                  0x140C
#define GL_FIXED_OES                                              0x140C
#define GL_INT64_ARB                                              0x140E
#define GL_INT64_NV                                               0x140E
#define GL_UNSIGNED_INT64_ARB                                     0x140F
#define GL_UNSIGNED_INT64_NV                                      0x140F
#define GL_CLEAR                                                  0x1500
#define GL_AND                                                    0x1501
#define GL_AND_REVERSE                                            0x1502
#define GL_COPY                                                   0x1503
#define GL_AND_INVERTED                                           0x1504
#define GL_NOOP                                                   0x1505
#define GL_XOR                                                    0x1506
#define GL_XOR_NV                                                 0x1506
#define GL_OR                                                     0x1507
#define GL_NOR                                                    0x1508
#define GL_EQUIV                                                  0x1509
#define GL_INVERT                                                 0x150A
#define GL_OR_REVERSE                                             0x150B
#define GL_COPY_INVERTED                                          0x150C
#define GL_OR_INVERTED                                            0x150D
#define GL_NAND                                                   0x150E
#define GL_SET                                                    0x150F
#define GL_EMISSION                                               0x1600
#define GL_SHININESS                                              0x1601
#define GL_AMBIENT_AND_DIFFUSE                                    0x1602
#define GL_COLOR_INDEXES                                          0x1603
#define GL_MODELVIEW                                              0x1700
#define GL_MODELVIEW0_ARB                                         0x1700
#define GL_MODELVIEW0_EXT                                         0x1700
#define GL_PATH_MODELVIEW_NV                                      0x1700
#define GL_PROJECTION                                             0x1701
#define GL_PATH_PROJECTION_NV                                     0x1701
#define GL_TEXTURE                                                0x1702
#define GL_COLOR                                                  0x1800
#define GL_COLOR_EXT                                              0x1800
#define GL_DEPTH                                                  0x1801
#define GL_DEPTH_EXT                                              0x1801
#define GL_STENCIL                                                0x1802
#define GL_STENCIL_EXT                                            0x1802
#define GL_COLOR_INDEX                                            0x1900
#define GL_STENCIL_INDEX                                          0x1901
#define GL_STENCIL_INDEX_OES                                      0x1901
#define GL_DEPTH_COMPONENT                                        0x1902
#define GL_RED                                                    0x1903
#define GL_RED_EXT                                                0x1903
#define GL_RED_NV                                                 0x1903
#define GL_GREEN                                                  0x1904
#define GL_GREEN_NV                                               0x1904
#define GL_BLUE                                                   0x1905
#define GL_BLUE_NV                                                0x1905
#define GL_ALPHA                                                  0x1906
#define GL_RGB                                                    0x1907
#define GL_RGBA                                                   0x1908
#define GL_LUMINANCE                                              0x1909
#define GL_LUMINANCE_ALPHA                                        0x190A
#define GL_BITMAP                                                 0x1A00
#define GL_POINT                                                  0x1B00
#define GL_POINT_NV                                               0x1B00
#define GL_LINE                                                   0x1B01
#define GL_LINE_NV                                                0x1B01
#define GL_FILL                                                   0x1B02
#define GL_FILL_NV                                                0x1B02
#define GL_RENDER                                                 0x1C00
#define GL_FEEDBACK                                               0x1C01
#define GL_SELECT                                                 0x1C02
#define GL_FLAT                                                   0x1D00
#define GL_SMOOTH                                                 0x1D01
#define GL_KEEP                                                   0x1E00
#define GL_REPLACE                                                0x1E01
#define GL_INCR                                                   0x1E02
#define GL_DECR                                                   0x1E03
#define GL_VENDOR                                                 0x1F00
#define GL_RENDERER                                               0x1F01
#define GL_VERSION                                                0x1F02
#define GL_EXTENSIONS                                             0x1F03
#define GL_S                                                      0x2000
#define GL_T                                                      0x2001
#define GL_R                                                      0x2002
#define GL_Q                                                      0x2003
#define GL_MODULATE                                               0x2100
#define GL_DECAL                                                  0x2101
#define GL_TEXTURE_ENV_MODE                                       0x2200
#define GL_TEXTURE_ENV_COLOR                                      0x2201
#define GL_TEXTURE_ENV                                            0x2300
#define GL_EYE_LINEAR                                             0x2400
#define GL_EYE_LINEAR_NV                                          0x2400
#define GL_OBJECT_LINEAR                                          0x2401
#define GL_OBJECT_LINEAR_NV                                       0x2401
#define GL_SPHERE_MAP                                             0x2402
#define GL_TEXTURE_GEN_MODE                                       0x2500
#define GL_TEXTURE_GEN_MODE_OES                                   0x2500
#define GL_OBJECT_PLANE                                           0x2501
#define GL_EYE_PLANE                                              0x2502
#define GL_NEAREST                                                0x2600
#define GL_LINEAR                                                 0x2601
#define GL_NEAREST_MIPMAP_NEAREST                                 0x2700
#define GL_LINEAR_MIPMAP_NEAREST                                  0x2701
#define GL_NEAREST_MIPMAP_LINEAR                                  0x2702
#define GL_LINEAR_MIPMAP_LINEAR                                   0x2703
#define GL_TEXTURE_MAG_FILTER                                     0x2800
#define GL_TEXTURE_MIN_FILTER                                     0x2801
#define GL_TEXTURE_WRAP_S                                         0x2802
#define GL_TEXTURE_WRAP_T                                         0x2803
#define GL_CLAMP                                                  0x2900
#define GL_REPEAT                                                 0x2901
#define GL_POLYGON_OFFSET_UNITS                                   0x2A00
#define GL_POLYGON_OFFSET_POINT                                   0x2A01
#define GL_POLYGON_OFFSET_POINT_NV                                0x2A01
#define GL_POLYGON_OFFSET_LINE                                    0x2A02
#define GL_POLYGON_OFFSET_LINE_NV                                 0x2A02
#define GL_R3_G3_B2                                               0x2A10
#define GL_V2F                                                    0x2A20
#define GL_V3F                                                    0x2A21
#define GL_C4UB_V2F                                               0x2A22
#define GL_C4UB_V3F                                               0x2A23
#define GL_C3F_V3F                                                0x2A24
#define GL_N3F_V3F                                                0x2A25
#define GL_C4F_N3F_V3F                                            0x2A26
#define GL_T2F_V3F                                                0x2A27
#define GL_T4F_V4F                                                0x2A28
#define GL_T2F_C4UB_V3F                                           0x2A29
#define GL_T2F_C3F_V3F                                            0x2A2A
#define GL_T2F_N3F_V3F                                            0x2A2B
#define GL_T2F_C4F_N3F_V3F                                        0x2A2C
#define GL_T4F_C4F_N3F_V4F                                        0x2A2D
#define GL_CLIP_PLANE0                                            0x3000
#define GL_CLIP_PLANE0_IMG                                        0x3000
#define GL_CLIP_DISTANCE0                                         0x3000
#define GL_CLIP_DISTANCE0_EXT                                     0x3000
#define GL_CLIP_DISTANCE0_APPLE                                   0x3000
#define GL_CLIP_PLANE1                                            0x3001
#define GL_CLIP_PLANE1_IMG                                        0x3001
#define GL_CLIP_DISTANCE1                                         0x3001
#define GL_CLIP_DISTANCE1_EXT                                     0x3001
#define GL_CLIP_DISTANCE1_APPLE                                   0x3001
#define GL_CLIP_PLANE2                                            0x3002
#define GL_CLIP_PLANE2_IMG                                        0x3002
#define GL_CLIP_DISTANCE2                                         0x3002
#define GL_CLIP_DISTANCE2_EXT                                     0x3002
#define GL_CLIP_DISTANCE2_APPLE                                   0x3002
#define GL_CLIP_PLANE3                                            0x3003
#define GL_CLIP_PLANE3_IMG                                        0x3003
#define GL_CLIP_DISTANCE3                                         0x3003
#define GL_CLIP_DISTANCE3_EXT                                     0x3003
#define GL_CLIP_DISTANCE3_APPLE                                   0x3003
#define GL_CLIP_PLANE4                                            0x3004
#define GL_CLIP_PLANE4_IMG                                        0x3004
#define GL_CLIP_DISTANCE4                                         0x3004
#define GL_CLIP_DISTANCE4_EXT                                     0x3004
#define GL_CLIP_DISTANCE4_APPLE                                   0x3004
#define GL_CLIP_PLANE5                                            0x3005
#define GL_CLIP_PLANE5_IMG                                        0x3005
#define GL_CLIP_DISTANCE5                                         0x3005
#define GL_CLIP_DISTANCE5_EXT                                     0x3005
#define GL_CLIP_DISTANCE5_APPLE                                   0x3005
#define GL_CLIP_DISTANCE6                                         0x3006
#define GL_CLIP_DISTANCE6_EXT                                     0x3006
#define GL_CLIP_DISTANCE6_APPLE                                   0x3006
#define GL_CLIP_DISTANCE7                                         0x3007
#define GL_CLIP_DISTANCE7_EXT                                     0x3007
#define GL_CLIP_DISTANCE7_APPLE                                   0x3007
#define GL_LIGHT0                                                 0x4000
#define GL_LIGHT1                                                 0x4001
#define GL_LIGHT2                                                 0x4002
#define GL_LIGHT3                                                 0x4003
#define GL_LIGHT4                                                 0x4004
#define GL_LIGHT5                                                 0x4005
#define GL_LIGHT6                                                 0x4006
#define GL_LIGHT7                                                 0x4007
#define GL_ABGR_EXT                                               0x8000
#define GL_CONSTANT_COLOR                                         0x8001
#define GL_CONSTANT_COLOR_EXT                                     0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR                               0x8002
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT                           0x8002
#define GL_CONSTANT_ALPHA                                         0x8003
#define GL_CONSTANT_ALPHA_EXT                                     0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA                               0x8004
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT                           0x8004
#define GL_BLEND_COLOR                                            0x8005
#define GL_BLEND_COLOR_EXT                                        0x8005
#define GL_FUNC_ADD                                               0x8006
#define GL_FUNC_ADD_EXT                                           0x8006
#define GL_FUNC_ADD_OES                                           0x8006
#define GL_MIN                                                    0x8007
#define GL_MIN_EXT                                                0x8007
#define GL_MAX                                                    0x8008
#define GL_MAX_EXT                                                0x8008
#define GL_BLEND_EQUATION                                         0x8009
#define GL_BLEND_EQUATION_EXT                                     0x8009
#define GL_BLEND_EQUATION_OES                                     0x8009
#define GL_BLEND_EQUATION_RGB                                     0x8009
#define GL_BLEND_EQUATION_RGB_EXT                                 0x8009
#define GL_BLEND_EQUATION_RGB_OES                                 0x8009
#define GL_FUNC_SUBTRACT                                          0x800A
#define GL_FUNC_SUBTRACT_EXT                                      0x800A
#define GL_FUNC_SUBTRACT_OES                                      0x800A
#define GL_FUNC_REVERSE_SUBTRACT                                  0x800B
#define GL_FUNC_REVERSE_SUBTRACT_EXT                              0x800B
#define GL_FUNC_REVERSE_SUBTRACT_OES                              0x800B
#define GL_CMYK_EXT                                               0x800C
#define GL_CMYKA_EXT                                              0x800D
#define GL_PACK_CMYK_HINT_EXT                                     0x800E
#define GL_UNPACK_CMYK_HINT_EXT                                   0x800F
#define GL_CONVOLUTION_1D                                         0x8010
#define GL_CONVOLUTION_1D_EXT                                     0x8010
#define GL_CONVOLUTION_2D                                         0x8011
#define GL_CONVOLUTION_2D_EXT                                     0x8011
#define GL_SEPARABLE_2D                                           0x8012
#define GL_SEPARABLE_2D_EXT                                       0x8012
#define GL_CONVOLUTION_BORDER_MODE                                0x8013
#define GL_CONVOLUTION_BORDER_MODE_EXT                            0x8013
#define GL_CONVOLUTION_FILTER_SCALE                               0x8014
#define GL_CONVOLUTION_FILTER_SCALE_EXT                           0x8014
#define GL_CONVOLUTION_FILTER_BIAS                                0x8015
#define GL_CONVOLUTION_FILTER_BIAS_EXT                            0x8015
#define GL_REDUCE                                                 0x8016
#define GL_REDUCE_EXT                                             0x8016
#define GL_CONVOLUTION_FORMAT                                     0x8017
#define GL_CONVOLUTION_FORMAT_EXT                                 0x8017
#define GL_CONVOLUTION_WIDTH                                      0x8018
#define GL_CONVOLUTION_WIDTH_EXT                                  0x8018
#define GL_CONVOLUTION_HEIGHT                                     0x8019
#define GL_CONVOLUTION_HEIGHT_EXT                                 0x8019
#define GL_MAX_CONVOLUTION_WIDTH                                  0x801A
#define GL_MAX_CONVOLUTION_WIDTH_EXT                              0x801A
#define GL_MAX_CONVOLUTION_HEIGHT                                 0x801B
#define GL_MAX_CONVOLUTION_HEIGHT_EXT                             0x801B
#define GL_POST_CONVOLUTION_RED_SCALE                             0x801C
#define GL_POST_CONVOLUTION_RED_SCALE_EXT                         0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE                           0x801D
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT                       0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE                            0x801E
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT                        0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE                           0x801F
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT                       0x801F
#define GL_POST_CONVOLUTION_RED_BIAS                              0x8020
#define GL_POST_CONVOLUTION_RED_BIAS_EXT                          0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS                            0x8021
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT                        0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS                             0x8022
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT                         0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS                            0x8023
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT                        0x8023
#define GL_HISTOGRAM                                              0x8024
#define GL_HISTOGRAM_EXT                                          0x8024
#define GL_PROXY_HISTOGRAM                                        0x8025
#define GL_PROXY_HISTOGRAM_EXT                                    0x8025
#define GL_HISTOGRAM_WIDTH                                        0x8026
#define GL_HISTOGRAM_WIDTH_EXT                                    0x8026
#define GL_HISTOGRAM_FORMAT                                       0x8027
#define GL_HISTOGRAM_FORMAT_EXT                                   0x8027
#define GL_HISTOGRAM_RED_SIZE                                     0x8028
#define GL_HISTOGRAM_RED_SIZE_EXT                                 0x8028
#define GL_HISTOGRAM_GREEN_SIZE                                   0x8029
#define GL_HISTOGRAM_GREEN_SIZE_EXT                               0x8029
#define GL_HISTOGRAM_BLUE_SIZE                                    0x802A
#define GL_HISTOGRAM_BLUE_SIZE_EXT                                0x802A
#define GL_HISTOGRAM_ALPHA_SIZE                                   0x802B
#define GL_HISTOGRAM_ALPHA_SIZE_EXT                               0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE                               0x802C
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT                           0x802C
#define GL_HISTOGRAM_SINK                                         0x802D
#define GL_HISTOGRAM_SINK_EXT                                     0x802D
#define GL_MINMAX                                                 0x802E
#define GL_MINMAX_EXT                                             0x802E
#define GL_MINMAX_FORMAT                                          0x802F
#define GL_MINMAX_FORMAT_EXT                                      0x802F
#define GL_MINMAX_SINK                                            0x8030
#define GL_MINMAX_SINK_EXT                                        0x8030
#define GL_TABLE_TOO_LARGE_EXT                                    0x8031
#define GL_TABLE_TOO_LARGE                                        0x8031
#define GL_UNSIGNED_BYTE_3_3_2                                    0x8032
#define GL_UNSIGNED_BYTE_3_3_2_EXT                                0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4                                 0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT                             0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1                                 0x8034
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT                             0x8034
#define GL_UNSIGNED_INT_8_8_8_8                                   0x8035
#define GL_UNSIGNED_INT_8_8_8_8_EXT                               0x8035
#define GL_UNSIGNED_INT_10_10_10_2                                0x8036
#define GL_UNSIGNED_INT_10_10_10_2_EXT                            0x8036
#define GL_POLYGON_OFFSET_EXT                                     0x8037
#define GL_POLYGON_OFFSET_FILL                                    0x8037
#define GL_POLYGON_OFFSET_FACTOR                                  0x8038
#define GL_POLYGON_OFFSET_FACTOR_EXT                              0x8038
#define GL_POLYGON_OFFSET_BIAS_EXT                                0x8039
#define GL_RESCALE_NORMAL                                         0x803A
#define GL_RESCALE_NORMAL_EXT                                     0x803A
#define GL_ALPHA4                                                 0x803B
#define GL_ALPHA4_EXT                                             0x803B
#define GL_ALPHA8                                                 0x803C
#define GL_ALPHA8_EXT                                             0x803C
#define GL_ALPHA8_OES                                             0x803C
#define GL_ALPHA12                                                0x803D
#define GL_ALPHA12_EXT                                            0x803D
#define GL_ALPHA16                                                0x803E
#define GL_ALPHA16_EXT                                            0x803E
#define GL_LUMINANCE4                                             0x803F
#define GL_LUMINANCE4_EXT                                         0x803F
#define GL_LUMINANCE8                                             0x8040
#define GL_LUMINANCE8_EXT                                         0x8040
#define GL_LUMINANCE8_OES                                         0x8040
#define GL_LUMINANCE12                                            0x8041
#define GL_LUMINANCE12_EXT                                        0x8041
#define GL_LUMINANCE16                                            0x8042
#define GL_LUMINANCE16_EXT                                        0x8042
#define GL_LUMINANCE4_ALPHA4                                      0x8043
#define GL_LUMINANCE4_ALPHA4_EXT                                  0x8043
#define GL_LUMINANCE4_ALPHA4_OES                                  0x8043
#define GL_LUMINANCE6_ALPHA2                                      0x8044
#define GL_LUMINANCE6_ALPHA2_EXT                                  0x8044
#define GL_LUMINANCE8_ALPHA8                                      0x8045
#define GL_LUMINANCE8_ALPHA8_EXT                                  0x8045
#define GL_LUMINANCE8_ALPHA8_OES                                  0x8045
#define GL_LUMINANCE12_ALPHA4                                     0x8046
#define GL_LUMINANCE12_ALPHA4_EXT                                 0x8046
#define GL_LUMINANCE12_ALPHA12                                    0x8047
#define GL_LUMINANCE12_ALPHA12_EXT                                0x8047
#define GL_LUMINANCE16_ALPHA16                                    0x8048
#define GL_LUMINANCE16_ALPHA16_EXT                                0x8048
#define GL_INTENSITY                                              0x8049
#define GL_INTENSITY_EXT                                          0x8049
#define GL_INTENSITY4                                             0x804A
#define GL_INTENSITY4_EXT                                         0x804A
#define GL_INTENSITY8                                             0x804B
#define GL_INTENSITY8_EXT                                         0x804B
#define GL_INTENSITY12                                            0x804C
#define GL_INTENSITY12_EXT                                        0x804C
#define GL_INTENSITY16                                            0x804D
#define GL_INTENSITY16_EXT                                        0x804D
#define GL_RGB2_EXT                                               0x804E
#define GL_RGB4                                                   0x804F
#define GL_RGB4_EXT                                               0x804F
#define GL_RGB5                                                   0x8050
#define GL_RGB5_EXT                                               0x8050
#define GL_RGB8                                                   0x8051
#define GL_RGB8_EXT                                               0x8051
#define GL_RGB8_OES                                               0x8051
#define GL_RGB10                                                  0x8052
#define GL_RGB10_EXT                                              0x8052
#define GL_RGB12                                                  0x8053
#define GL_RGB12_EXT                                              0x8053
#define GL_RGB16                                                  0x8054
#define GL_RGB16_EXT                                              0x8054
#define GL_RGBA2                                                  0x8055
#define GL_RGBA2_EXT                                              0x8055
#define GL_RGBA4                                                  0x8056
#define GL_RGBA4_EXT                                              0x8056
#define GL_RGBA4_OES                                              0x8056
#define GL_RGB5_A1                                                0x8057
#define GL_RGB5_A1_EXT                                            0x8057
#define GL_RGB5_A1_OES                                            0x8057
#define GL_RGBA8                                                  0x8058
#define GL_RGBA8_EXT                                              0x8058
#define GL_RGBA8_OES                                              0x8058
#define GL_RGB10_A2                                               0x8059
#define GL_RGB10_A2_EXT                                           0x8059
#define GL_RGBA12                                                 0x805A
#define GL_RGBA12_EXT                                             0x805A
#define GL_RGBA16                                                 0x805B
#define GL_RGBA16_EXT                                             0x805B
#define GL_TEXTURE_RED_SIZE                                       0x805C
#define GL_TEXTURE_RED_SIZE_EXT                                   0x805C
#define GL_TEXTURE_GREEN_SIZE                                     0x805D
#define GL_TEXTURE_GREEN_SIZE_EXT                                 0x805D
#define GL_TEXTURE_BLUE_SIZE                                      0x805E
#define GL_TEXTURE_BLUE_SIZE_EXT                                  0x805E
#define GL_TEXTURE_ALPHA_SIZE                                     0x805F
#define GL_TEXTURE_ALPHA_SIZE_EXT                                 0x805F
#define GL_TEXTURE_LUMINANCE_SIZE                                 0x8060
#define GL_TEXTURE_LUMINANCE_SIZE_EXT                             0x8060
#define GL_TEXTURE_INTENSITY_SIZE                                 0x8061
#define GL_TEXTURE_INTENSITY_SIZE_EXT                             0x8061
#define GL_REPLACE_EXT                                            0x8062
#define GL_PROXY_TEXTURE_1D                                       0x8063
#define GL_PROXY_TEXTURE_1D_EXT                                   0x8063
#define GL_PROXY_TEXTURE_2D                                       0x8064
#define GL_PROXY_TEXTURE_2D_EXT                                   0x8064
#define GL_TEXTURE_TOO_LARGE_EXT                                  0x8065
#define GL_TEXTURE_PRIORITY                                       0x8066
#define GL_TEXTURE_PRIORITY_EXT                                   0x8066
#define GL_TEXTURE_RESIDENT                                       0x8067
#define GL_TEXTURE_RESIDENT_EXT                                   0x8067
#define GL_TEXTURE_1D_BINDING_EXT                                 0x8068
#define GL_TEXTURE_BINDING_1D                                     0x8068
#define GL_TEXTURE_2D_BINDING_EXT                                 0x8069
#define GL_TEXTURE_BINDING_2D                                     0x8069
#define GL_TEXTURE_3D_BINDING_EXT                                 0x806A
#define GL_TEXTURE_3D_BINDING_OES                                 0x806A
#define GL_TEXTURE_BINDING_3D                                     0x806A
#define GL_TEXTURE_BINDING_3D_OES                                 0x806A
#define GL_PACK_SKIP_IMAGES                                       0x806B
#define GL_PACK_SKIP_IMAGES_EXT                                   0x806B
#define GL_PACK_IMAGE_HEIGHT                                      0x806C
#define GL_PACK_IMAGE_HEIGHT_EXT                                  0x806C
#define GL_UNPACK_SKIP_IMAGES                                     0x806D
#define GL_UNPACK_SKIP_IMAGES_EXT                                 0x806D
#define GL_UNPACK_IMAGE_HEIGHT                                    0x806E
#define GL_UNPACK_IMAGE_HEIGHT_EXT                                0x806E
#define GL_TEXTURE_3D                                             0x806F
#define GL_TEXTURE_3D_EXT                                         0x806F
#define GL_TEXTURE_3D_OES                                         0x806F
#define GL_PROXY_TEXTURE_3D                                       0x8070
#define GL_PROXY_TEXTURE_3D_EXT                                   0x8070
#define GL_TEXTURE_DEPTH                                          0x8071
#define GL_TEXTURE_DEPTH_EXT                                      0x8071
#define GL_TEXTURE_WRAP_R                                         0x8072
#define GL_TEXTURE_WRAP_R_EXT                                     0x8072
#define GL_TEXTURE_WRAP_R_OES                                     0x8072
#define GL_MAX_3D_TEXTURE_SIZE                                    0x8073
#define GL_MAX_3D_TEXTURE_SIZE_EXT                                0x8073
#define GL_MAX_3D_TEXTURE_SIZE_OES                                0x8073
#define GL_VERTEX_ARRAY                                           0x8074
#define GL_VERTEX_ARRAY_EXT                                       0x8074
#define GL_VERTEX_ARRAY_KHR                                       0x8074
#define GL_NORMAL_ARRAY                                           0x8075
#define GL_NORMAL_ARRAY_EXT                                       0x8075
#define GL_COLOR_ARRAY                                            0x8076
#define GL_COLOR_ARRAY_EXT                                        0x8076
#define GL_INDEX_ARRAY                                            0x8077
#define GL_INDEX_ARRAY_EXT                                        0x8077
#define GL_TEXTURE_COORD_ARRAY                                    0x8078
#define GL_TEXTURE_COORD_ARRAY_EXT                                0x8078
#define GL_EDGE_FLAG_ARRAY                                        0x8079
#define GL_EDGE_FLAG_ARRAY_EXT                                    0x8079
#define GL_VERTEX_ARRAY_SIZE                                      0x807A
#define GL_VERTEX_ARRAY_SIZE_EXT                                  0x807A
#define GL_VERTEX_ARRAY_TYPE                                      0x807B
#define GL_VERTEX_ARRAY_TYPE_EXT                                  0x807B
#define GL_VERTEX_ARRAY_STRIDE                                    0x807C
#define GL_VERTEX_ARRAY_STRIDE_EXT                                0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT                                 0x807D
#define GL_NORMAL_ARRAY_TYPE                                      0x807E
#define GL_NORMAL_ARRAY_TYPE_EXT                                  0x807E
#define GL_NORMAL_ARRAY_STRIDE                                    0x807F
#define GL_NORMAL_ARRAY_STRIDE_EXT                                0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT                                 0x8080
#define GL_COLOR_ARRAY_SIZE                                       0x8081
#define GL_COLOR_ARRAY_SIZE_EXT                                   0x8081
#define GL_COLOR_ARRAY_TYPE                                       0x8082
#define GL_COLOR_ARRAY_TYPE_EXT                                   0x8082
#define GL_COLOR_ARRAY_STRIDE                                     0x8083
#define GL_COLOR_ARRAY_STRIDE_EXT                                 0x8083
#define GL_COLOR_ARRAY_COUNT_EXT                                  0x8084
#define GL_INDEX_ARRAY_TYPE                                       0x8085
#define GL_INDEX_ARRAY_TYPE_EXT                                   0x8085
#define GL_INDEX_ARRAY_STRIDE                                     0x8086
#define GL_INDEX_ARRAY_STRIDE_EXT                                 0x8086
#define GL_INDEX_ARRAY_COUNT_EXT                                  0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE                               0x8088
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT                           0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE                               0x8089
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT                           0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE                             0x808A
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT                         0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT                          0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE                                 0x808C
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT                             0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT                              0x808D
#define GL_VERTEX_ARRAY_POINTER                                   0x808E
#define GL_VERTEX_ARRAY_POINTER_EXT                               0x808E
#define GL_NORMAL_ARRAY_POINTER                                   0x808F
#define GL_NORMAL_ARRAY_POINTER_EXT                               0x808F
#define GL_COLOR_ARRAY_POINTER                                    0x8090
#define GL_COLOR_ARRAY_POINTER_EXT                                0x8090
#define GL_INDEX_ARRAY_POINTER                                    0x8091
#define GL_INDEX_ARRAY_POINTER_EXT                                0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER                            0x8092
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT                        0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER                                0x8093
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT                            0x8093
#define GL_INTERLACE_SGIX                                         0x8094
#define GL_DETAIL_TEXTURE_2D_SGIS                                 0x8095
#define GL_DETAIL_TEXTURE_2D_BINDING_SGIS                         0x8096
#define GL_LINEAR_DETAIL_SGIS                                     0x8097
#define GL_LINEAR_DETAIL_ALPHA_SGIS                               0x8098
#define GL_LINEAR_DETAIL_COLOR_SGIS                               0x8099
#define GL_DETAIL_TEXTURE_LEVEL_SGIS                              0x809A
#define GL_DETAIL_TEXTURE_MODE_SGIS                               0x809B
#define GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS                        0x809C
#define GL_MULTISAMPLE                                            0x809D
#define GL_MULTISAMPLE_ARB                                        0x809D
#define GL_MULTISAMPLE_EXT                                        0x809D
#define GL_MULTISAMPLE_SGIS                                       0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE                               0x809E
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB                           0x809E
#define GL_SAMPLE_ALPHA_TO_MASK_EXT                               0x809E
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS                              0x809E
#define GL_SAMPLE_ALPHA_TO_ONE                                    0x809F
#define GL_SAMPLE_ALPHA_TO_ONE_ARB                                0x809F
#define GL_SAMPLE_ALPHA_TO_ONE_EXT                                0x809F
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS                               0x809F
#define GL_SAMPLE_COVERAGE                                        0x80A0
#define GL_SAMPLE_COVERAGE_ARB                                    0x80A0
#define GL_SAMPLE_MASK_EXT                                        0x80A0
#define GL_SAMPLE_MASK_SGIS                                       0x80A0
#define GL_1PASS_EXT                                              0x80A1
#define GL_1PASS_SGIS                                             0x80A1
#define GL_2PASS_0_EXT                                            0x80A2
#define GL_2PASS_0_SGIS                                           0x80A2
#define GL_2PASS_1_EXT                                            0x80A3
#define GL_2PASS_1_SGIS                                           0x80A3
#define GL_4PASS_0_EXT                                            0x80A4
#define GL_4PASS_0_SGIS                                           0x80A4
#define GL_4PASS_1_EXT                                            0x80A5
#define GL_4PASS_1_SGIS                                           0x80A5
#define GL_4PASS_2_EXT                                            0x80A6
#define GL_4PASS_2_SGIS                                           0x80A6
#define GL_4PASS_3_EXT                                            0x80A7
#define GL_4PASS_3_SGIS                                           0x80A7
#define GL_SAMPLE_BUFFERS                                         0x80A8
#define GL_SAMPLE_BUFFERS_ARB                                     0x80A8
#define GL_SAMPLE_BUFFERS_EXT                                     0x80A8
#define GL_SAMPLE_BUFFERS_SGIS                                    0x80A8
#define GL_SAMPLES                                                0x80A9
#define GL_SAMPLES_ARB                                            0x80A9
#define GL_SAMPLES_EXT                                            0x80A9
#define GL_SAMPLES_SGIS                                           0x80A9
#define GL_SAMPLE_COVERAGE_VALUE                                  0x80AA
#define GL_SAMPLE_COVERAGE_VALUE_ARB                              0x80AA
#define GL_SAMPLE_MASK_VALUE_EXT                                  0x80AA
#define GL_SAMPLE_MASK_VALUE_SGIS                                 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT                                 0x80AB
#define GL_SAMPLE_COVERAGE_INVERT_ARB                             0x80AB
#define GL_SAMPLE_MASK_INVERT_EXT                                 0x80AB
#define GL_SAMPLE_MASK_INVERT_SGIS                                0x80AB
#define GL_SAMPLE_PATTERN_EXT                                     0x80AC
#define GL_SAMPLE_PATTERN_SGIS                                    0x80AC
#define GL_LINEAR_SHARPEN_SGIS                                    0x80AD
#define GL_LINEAR_SHARPEN_ALPHA_SGIS                              0x80AE
#define GL_LINEAR_SHARPEN_COLOR_SGIS                              0x80AF
#define GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS                       0x80B0
#define GL_COLOR_MATRIX                                           0x80B1
#define GL_COLOR_MATRIX_SGI                                       0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH                               0x80B2
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI                           0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH                           0x80B3
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI                       0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE                            0x80B4
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI                        0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE                          0x80B5
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI                      0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE                           0x80B6
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI                       0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE                          0x80B7
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI                      0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS                             0x80B8
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI                         0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS                           0x80B9
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI                       0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS                            0x80BA
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI                        0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS                           0x80BB
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI                       0x80BB
#define GL_TEXTURE_COLOR_TABLE_SGI                                0x80BC
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI                          0x80BD
#define GL_TEXTURE_ENV_BIAS_SGIX                                  0x80BE
#define GL_SHADOW_AMBIENT_SGIX                                    0x80BF
#define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB                         0x80BF
#define GL_BLEND_DST_RGB                                          0x80C8
#define GL_BLEND_DST_RGB_EXT                                      0x80C8
#define GL_BLEND_DST_RGB_OES                                      0x80C8
#define GL_BLEND_SRC_RGB                                          0x80C9
#define GL_BLEND_SRC_RGB_EXT                                      0x80C9
#define GL_BLEND_SRC_RGB_OES                                      0x80C9
#define GL_BLEND_DST_ALPHA                                        0x80CA
#define GL_BLEND_DST_ALPHA_EXT                                    0x80CA
#define GL_BLEND_DST_ALPHA_OES                                    0x80CA
#define GL_BLEND_SRC_ALPHA                                        0x80CB
#define GL_BLEND_SRC_ALPHA_EXT                                    0x80CB
#define GL_BLEND_SRC_ALPHA_OES                                    0x80CB
#define GL_422_EXT                                                0x80CC
#define GL_422_REV_EXT                                            0x80CD
#define GL_422_AVERAGE_EXT                                        0x80CE
#define GL_422_REV_AVERAGE_EXT                                    0x80CF
#define GL_COLOR_TABLE                                            0x80D0
#define GL_COLOR_TABLE_SGI                                        0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE                           0x80D1
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI                       0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE                          0x80D2
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI                      0x80D2
#define GL_PROXY_COLOR_TABLE                                      0x80D3
#define GL_PROXY_COLOR_TABLE_SGI                                  0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE                     0x80D4
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI                 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE                    0x80D5
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI                0x80D5
#define GL_COLOR_TABLE_SCALE                                      0x80D6
#define GL_COLOR_TABLE_SCALE_SGI                                  0x80D6
#define GL_COLOR_TABLE_BIAS                                       0x80D7
#define GL_COLOR_TABLE_BIAS_SGI                                   0x80D7
#define GL_COLOR_TABLE_FORMAT                                     0x80D8
#define GL_COLOR_TABLE_FORMAT_SGI                                 0x80D8
#define GL_COLOR_TABLE_WIDTH                                      0x80D9
#define GL_COLOR_TABLE_WIDTH_SGI                                  0x80D9
#define GL_COLOR_TABLE_RED_SIZE                                   0x80DA
#define GL_COLOR_TABLE_RED_SIZE_SGI                               0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE                                 0x80DB
#define GL_COLOR_TABLE_GREEN_SIZE_SGI                             0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE                                  0x80DC
#define GL_COLOR_TABLE_BLUE_SIZE_SGI                              0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE                                 0x80DD
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI                             0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE                             0x80DE
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI                         0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE                             0x80DF
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI                         0x80DF
#define GL_BGR                                                    0x80E0
#define GL_BGR_EXT                                                0x80E0
#define GL_BGRA                                                   0x80E1
#define GL_BGRA_EXT                                               0x80E1
#define GL_BGRA_IMG                                               0x80E1
#define GL_COLOR_INDEX1_EXT                                       0x80E2
#define GL_COLOR_INDEX2_EXT                                       0x80E3
#define GL_COLOR_INDEX4_EXT                                       0x80E4
#define GL_COLOR_INDEX8_EXT                                       0x80E5
#define GL_COLOR_INDEX12_EXT                                      0x80E6
#define GL_COLOR_INDEX16_EXT                                      0x80E7
#define GL_MAX_ELEMENTS_VERTICES                                  0x80E8
#define GL_MAX_ELEMENTS_VERTICES_EXT                              0x80E8
#define GL_MAX_ELEMENTS_INDICES                                   0x80E9
#define GL_MAX_ELEMENTS_INDICES_EXT                               0x80E9
#define GL_PHONG_WIN                                              0x80EA
#define GL_PHONG_HINT_WIN                                         0x80EB
#define GL_FOG_SPECULAR_TEXTURE_WIN                               0x80EC
#define GL_TEXTURE_INDEX_SIZE_EXT                                 0x80ED
#define GL_PARAMETER_BUFFER_ARB                                   0x80EE
#define GL_PARAMETER_BUFFER_BINDING_ARB                           0x80EF
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT                          0x80F0
#define GL_DUAL_ALPHA4_SGIS                                       0x8110
#define GL_DUAL_ALPHA8_SGIS                                       0x8111
#define GL_DUAL_ALPHA12_SGIS                                      0x8112
#define GL_DUAL_ALPHA16_SGIS                                      0x8113
#define GL_DUAL_LUMINANCE4_SGIS                                   0x8114
#define GL_DUAL_LUMINANCE8_SGIS                                   0x8115
#define GL_DUAL_LUMINANCE12_SGIS                                  0x8116
#define GL_DUAL_LUMINANCE16_SGIS                                  0x8117
#define GL_DUAL_INTENSITY4_SGIS                                   0x8118
#define GL_DUAL_INTENSITY8_SGIS                                   0x8119
#define GL_DUAL_INTENSITY12_SGIS                                  0x811A
#define GL_DUAL_INTENSITY16_SGIS                                  0x811B
#define GL_DUAL_LUMINANCE_ALPHA4_SGIS                             0x811C
#define GL_DUAL_LUMINANCE_ALPHA8_SGIS                             0x811D
#define GL_QUAD_ALPHA4_SGIS                                       0x811E
#define GL_QUAD_ALPHA8_SGIS                                       0x811F
#define GL_QUAD_LUMINANCE4_SGIS                                   0x8120
#define GL_QUAD_LUMINANCE8_SGIS                                   0x8121
#define GL_QUAD_INTENSITY4_SGIS                                   0x8122
#define GL_QUAD_INTENSITY8_SGIS                                   0x8123
#define GL_DUAL_TEXTURE_SELECT_SGIS                               0x8124
#define GL_QUAD_TEXTURE_SELECT_SGIS                               0x8125
#define GL_POINT_SIZE_MIN                                         0x8126
#define GL_POINT_SIZE_MIN_ARB                                     0x8126
#define GL_POINT_SIZE_MIN_EXT                                     0x8126
#define GL_POINT_SIZE_MIN_SGIS                                    0x8126
#define GL_POINT_SIZE_MAX                                         0x8127
#define GL_POINT_SIZE_MAX_ARB                                     0x8127
#define GL_POINT_SIZE_MAX_EXT                                     0x8127
#define GL_POINT_SIZE_MAX_SGIS                                    0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE                              0x8128
#define GL_POINT_FADE_THRESHOLD_SIZE_ARB                          0x8128
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT                          0x8128
#define GL_POINT_FADE_THRESHOLD_SIZE_SGIS                         0x8128
#define GL_DISTANCE_ATTENUATION_EXT                               0x8129
#define GL_DISTANCE_ATTENUATION_SGIS                              0x8129
#define GL_POINT_DISTANCE_ATTENUATION                             0x8129
#define GL_POINT_DISTANCE_ATTENUATION_ARB                         0x8129
#define GL_FOG_FUNC_SGIS                                          0x812A
#define GL_FOG_FUNC_POINTS_SGIS                                   0x812B
#define GL_MAX_FOG_FUNC_POINTS_SGIS                               0x812C
#define GL_CLAMP_TO_BORDER                                        0x812D
#define GL_CLAMP_TO_BORDER_ARB                                    0x812D
#define GL_CLAMP_TO_BORDER_EXT                                    0x812D
#define GL_CLAMP_TO_BORDER_NV                                     0x812D
#define GL_CLAMP_TO_BORDER_SGIS                                   0x812D
#define GL_CLAMP_TO_BORDER_OES                                    0x812D
#define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX                         0x812E
#define GL_CLAMP_TO_EDGE                                          0x812F
#define GL_CLAMP_TO_EDGE_SGIS                                     0x812F
#define GL_PACK_SKIP_VOLUMES_SGIS                                 0x8130
#define GL_PACK_IMAGE_DEPTH_SGIS                                  0x8131
#define GL_UNPACK_SKIP_VOLUMES_SGIS                               0x8132
#define GL_UNPACK_IMAGE_DEPTH_SGIS                                0x8133
#define GL_TEXTURE_4D_SGIS                                        0x8134
#define GL_PROXY_TEXTURE_4D_SGIS                                  0x8135
#define GL_TEXTURE_4DSIZE_SGIS                                    0x8136
#define GL_TEXTURE_WRAP_Q_SGIS                                    0x8137
#define GL_MAX_4D_TEXTURE_SIZE_SGIS                               0x8138
#define GL_PIXEL_TEX_GEN_SGIX                                     0x8139
#define GL_TEXTURE_MIN_LOD                                        0x813A
#define GL_TEXTURE_MIN_LOD_SGIS                                   0x813A
#define GL_TEXTURE_MAX_LOD                                        0x813B
#define GL_TEXTURE_MAX_LOD_SGIS                                   0x813B
#define GL_TEXTURE_BASE_LEVEL                                     0x813C
#define GL_TEXTURE_BASE_LEVEL_SGIS                                0x813C
#define GL_TEXTURE_MAX_LEVEL                                      0x813D
#define GL_TEXTURE_MAX_LEVEL_APPLE                                0x813D
#define GL_TEXTURE_MAX_LEVEL_SGIS                                 0x813D
#define GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX                         0x813E
#define GL_PIXEL_TILE_CACHE_INCREMENT_SGIX                        0x813F
#define GL_PIXEL_TILE_WIDTH_SGIX                                  0x8140
#define GL_PIXEL_TILE_HEIGHT_SGIX                                 0x8141
#define GL_PIXEL_TILE_GRID_WIDTH_SGIX                             0x8142
#define GL_PIXEL_TILE_GRID_HEIGHT_SGIX                            0x8143
#define GL_PIXEL_TILE_GRID_DEPTH_SGIX                             0x8144
#define GL_PIXEL_TILE_CACHE_SIZE_SGIX                             0x8145
#define GL_FILTER4_SGIS                                           0x8146
#define GL_TEXTURE_FILTER4_SIZE_SGIS                              0x8147
#define GL_SPRITE_SGIX                                            0x8148
#define GL_SPRITE_MODE_SGIX                                       0x8149
#define GL_SPRITE_AXIS_SGIX                                       0x814A
#define GL_SPRITE_TRANSLATION_SGIX                                0x814B
#define GL_SPRITE_AXIAL_SGIX                                      0x814C
#define GL_SPRITE_OBJECT_ALIGNED_SGIX                             0x814D
#define GL_SPRITE_EYE_ALIGNED_SGIX                                0x814E
#define GL_TEXTURE_4D_BINDING_SGIS                                0x814F
#define GL_IGNORE_BORDER_HP                                       0x8150
#define GL_CONSTANT_BORDER                                        0x8151
#define GL_CONSTANT_BORDER_HP                                     0x8151
#define GL_REPLICATE_BORDER                                       0x8153
#define GL_REPLICATE_BORDER_HP                                    0x8153
#define GL_CONVOLUTION_BORDER_COLOR                               0x8154
#define GL_CONVOLUTION_BORDER_COLOR_HP                            0x8154
#define GL_IMAGE_SCALE_X_HP                                       0x8155
#define GL_IMAGE_SCALE_Y_HP                                       0x8156
#define GL_IMAGE_TRANSLATE_X_HP                                   0x8157
#define GL_IMAGE_TRANSLATE_Y_HP                                   0x8158
#define GL_IMAGE_ROTATE_ANGLE_HP                                  0x8159
#define GL_IMAGE_ROTATE_ORIGIN_X_HP                               0x815A
#define GL_IMAGE_ROTATE_ORIGIN_Y_HP                               0x815B
#define GL_IMAGE_MAG_FILTER_HP                                    0x815C
#define GL_IMAGE_MIN_FILTER_HP                                    0x815D
#define GL_IMAGE_CUBIC_WEIGHT_HP                                  0x815E
#define GL_CUBIC_HP                                               0x815F
#define GL_AVERAGE_HP                                             0x8160
#define GL_IMAGE_TRANSFORM_2D_HP                                  0x8161
#define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP                    0x8162
#define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP              0x8163
#define GL_OCCLUSION_TEST_HP                                      0x8165
#define GL_OCCLUSION_TEST_RESULT_HP                               0x8166
#define GL_TEXTURE_LIGHTING_MODE_HP                               0x8167
#define GL_TEXTURE_POST_SPECULAR_HP                               0x8168
#define GL_TEXTURE_PRE_SPECULAR_HP                                0x8169
#define GL_LINEAR_CLIPMAP_LINEAR_SGIX                             0x8170
#define GL_TEXTURE_CLIPMAP_CENTER_SGIX                            0x8171
#define GL_TEXTURE_CLIPMAP_FRAME_SGIX                             0x8172
#define GL_TEXTURE_CLIPMAP_OFFSET_SGIX                            0x8173
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX                     0x8174
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX                        0x8175
#define GL_TEXTURE_CLIPMAP_DEPTH_SGIX                             0x8176
#define GL_MAX_CLIPMAP_DEPTH_SGIX                                 0x8177
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX                         0x8178
#define GL_POST_TEXTURE_FILTER_BIAS_SGIX                          0x8179
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX                         0x817A
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX                    0x817B
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX                   0x817C
#define GL_REFERENCE_PLANE_SGIX                                   0x817D
#define GL_REFERENCE_PLANE_EQUATION_SGIX                          0x817E
#define GL_IR_INSTRUMENT1_SGIX                                    0x817F
#define GL_INSTRUMENT_BUFFER_POINTER_SGIX                         0x8180
#define GL_INSTRUMENT_MEASUREMENTS_SGIX                           0x8181
#define GL_LIST_PRIORITY_SGIX                                     0x8182
#define GL_CALLIGRAPHIC_FRAGMENT_SGIX                             0x8183
#define GL_PIXEL_TEX_GEN_Q_CEILING_SGIX                           0x8184
#define GL_PIXEL_TEX_GEN_Q_ROUND_SGIX                             0x8185
#define GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX                             0x8186
#define GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX                       0x8187
#define GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX                    0x8188
#define GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX                            0x8189
#define GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX                            0x818A
#define GL_FRAMEZOOM_SGIX                                         0x818B
#define GL_FRAMEZOOM_FACTOR_SGIX                                  0x818C
#define GL_MAX_FRAMEZOOM_FACTOR_SGIX                              0x818D
#define GL_TEXTURE_LOD_BIAS_S_SGIX                                0x818E
#define GL_TEXTURE_LOD_BIAS_T_SGIX                                0x818F
#define GL_TEXTURE_LOD_BIAS_R_SGIX                                0x8190
#define GL_GENERATE_MIPMAP                                        0x8191
#define GL_GENERATE_MIPMAP_SGIS                                   0x8191
#define GL_GENERATE_MIPMAP_HINT                                   0x8192
#define GL_GENERATE_MIPMAP_HINT_SGIS                              0x8192
#define GL_GEOMETRY_DEFORMATION_SGIX                              0x8194
#define GL_TEXTURE_DEFORMATION_SGIX                               0x8195
#define GL_DEFORMATIONS_MASK_SGIX                                 0x8196
#define GL_MAX_DEFORMATION_ORDER_SGIX                             0x8197
#define GL_FOG_OFFSET_SGIX                                        0x8198
#define GL_FOG_OFFSET_VALUE_SGIX                                  0x8199
#define GL_TEXTURE_COMPARE_SGIX                                   0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX                          0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX                                  0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX                                  0x819D
#define GL_DEPTH_COMPONENT16                                      0x81A5
#define GL_DEPTH_COMPONENT16_ARB                                  0x81A5
#define GL_DEPTH_COMPONENT16_OES                                  0x81A5
#define GL_DEPTH_COMPONENT16_SGIX                                 0x81A5
#define GL_DEPTH_COMPONENT24                                      0x81A6
#define GL_DEPTH_COMPONENT24_ARB                                  0x81A6
#define GL_DEPTH_COMPONENT24_OES                                  0x81A6
#define GL_DEPTH_COMPONENT24_SGIX                                 0x81A6
#define GL_DEPTH_COMPONENT32                                      0x81A7
#define GL_DEPTH_COMPONENT32_ARB                                  0x81A7
#define GL_DEPTH_COMPONENT32_OES                                  0x81A7
#define GL_DEPTH_COMPONENT32_SGIX                                 0x81A7
#define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT                           0x81A8
#define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT                           0x81A9
#define GL_CULL_VERTEX_EXT                                        0x81AA
#define GL_CULL_VERTEX_EYE_POSITION_EXT                           0x81AB
#define GL_CULL_VERTEX_OBJECT_POSITION_EXT                        0x81AC
#define GL_IUI_V2F_EXT                                            0x81AD
#define GL_IUI_V3F_EXT                                            0x81AE
#define GL_IUI_N3F_V2F_EXT                                        0x81AF
#define GL_IUI_N3F_V3F_EXT                                        0x81B0
#define GL_T2F_IUI_V2F_EXT                                        0x81B1
#define GL_T2F_IUI_V3F_EXT                                        0x81B2
#define GL_T2F_IUI_N3F_V2F_EXT                                    0x81B3
#define GL_T2F_IUI_N3F_V3F_EXT                                    0x81B4
#define GL_INDEX_TEST_EXT                                         0x81B5
#define GL_INDEX_TEST_FUNC_EXT                                    0x81B6
#define GL_INDEX_TEST_REF_EXT                                     0x81B7
#define GL_INDEX_MATERIAL_EXT                                     0x81B8
#define GL_INDEX_MATERIAL_PARAMETER_EXT                           0x81B9
#define GL_INDEX_MATERIAL_FACE_EXT                                0x81BA
#define GL_YCRCB_422_SGIX                                         0x81BB
#define GL_YCRCB_444_SGIX                                         0x81BC
#define GL_WRAP_BORDER_SUN                                        0x81D4
#define GL_UNPACK_CONSTANT_DATA_SUNX                              0x81D5
#define GL_TEXTURE_CONSTANT_DATA_SUNX                             0x81D6
#define GL_TRIANGLE_LIST_SUN                                      0x81D7
#define GL_REPLACEMENT_CODE_SUN                                   0x81D8
#define GL_GLOBAL_ALPHA_SUN                                       0x81D9
#define GL_GLOBAL_ALPHA_FACTOR_SUN                                0x81DA
#define GL_TEXTURE_COLOR_WRITEMASK_SGIS                           0x81EF
#define GL_EYE_DISTANCE_TO_POINT_SGIS                             0x81F0
#define GL_OBJECT_DISTANCE_TO_POINT_SGIS                          0x81F1
#define GL_EYE_DISTANCE_TO_LINE_SGIS                              0x81F2
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS                           0x81F3
#define GL_EYE_POINT_SGIS                                         0x81F4
#define GL_OBJECT_POINT_SGIS                                      0x81F5
#define GL_EYE_LINE_SGIS                                          0x81F6
#define GL_OBJECT_LINE_SGIS                                       0x81F7
#define GL_LIGHT_MODEL_COLOR_CONTROL                              0x81F8
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT                          0x81F8
#define GL_SINGLE_COLOR                                           0x81F9
#define GL_SINGLE_COLOR_EXT                                       0x81F9
#define GL_SEPARATE_SPECULAR_COLOR                                0x81FA
#define GL_SEPARATE_SPECULAR_COLOR_EXT                            0x81FA
#define GL_SHARED_TEXTURE_PALETTE_EXT                             0x81FB
#define GL_TEXT_FRAGMENT_SHADER_ATI                               0x8200
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING                  0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT              0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE                  0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT              0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE                        0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE                      0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE                       0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE                      0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE                      0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE                    0x8217
#define GL_FRAMEBUFFER_DEFAULT                                    0x8218
#define GL_FRAMEBUFFER_UNDEFINED                                  0x8219
#define GL_FRAMEBUFFER_UNDEFINED_OES                              0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT                               0x821A
#define GL_MAJOR_VERSION                                          0x821B
#define GL_MINOR_VERSION                                          0x821C
#define GL_NUM_EXTENSIONS                                         0x821D
#define GL_CONTEXT_FLAGS                                          0x821E
#define GL_BUFFER_IMMUTABLE_STORAGE                               0x821F
#define GL_BUFFER_IMMUTABLE_STORAGE_EXT                           0x821F
#define GL_BUFFER_STORAGE_FLAGS                                   0x8220
#define GL_BUFFER_STORAGE_FLAGS_EXT                               0x8220
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED                0x8221
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED_OES            0x8221
#define GL_INDEX                                                  0x8222
#define GL_COMPRESSED_RED                                         0x8225
#define GL_COMPRESSED_RG                                          0x8226
#define GL_RG                                                     0x8227
#define GL_RG_EXT                                                 0x8227
#define GL_RG_INTEGER                                             0x8228
#define GL_R8                                                     0x8229
#define GL_R8_EXT                                                 0x8229
#define GL_R16                                                    0x822A
#define GL_R16_EXT                                                0x822A
#define GL_RG8                                                    0x822B
#define GL_RG8_EXT                                                0x822B
#define GL_RG16                                                   0x822C
#define GL_RG16_EXT                                               0x822C
#define GL_R16F                                                   0x822D
#define GL_R16F_EXT                                               0x822D
#define GL_R32F                                                   0x822E
#define GL_R32F_EXT                                               0x822E
#define GL_RG16F                                                  0x822F
#define GL_RG16F_EXT                                              0x822F
#define GL_RG32F                                                  0x8230
#define GL_RG32F_EXT                                              0x8230
#define GL_R8I                                                    0x8231
#define GL_R8UI                                                   0x8232
#define GL_R16I                                                   0x8233
#define GL_R16UI                                                  0x8234
#define GL_R32I                                                   0x8235
#define GL_R32UI                                                  0x8236
#define GL_RG8I                                                   0x8237
#define GL_RG8UI                                                  0x8238
#define GL_RG16I                                                  0x8239
#define GL_RG16UI                                                 0x823A
#define GL_RG32I                                                  0x823B
#define GL_RG32UI                                                 0x823C
#define GL_SYNC_CL_EVENT_ARB                                      0x8240
#define GL_SYNC_CL_EVENT_COMPLETE_ARB                             0x8241
#define GL_DEBUG_OUTPUT_SYNCHRONOUS                               0x8242
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB                           0x8242
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR                           0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH                       0x8243
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB                   0x8243
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR                   0x8243
#define GL_DEBUG_CALLBACK_FUNCTION                                0x8244
#define GL_DEBUG_CALLBACK_FUNCTION_ARB                            0x8244
#define GL_DEBUG_CALLBACK_FUNCTION_KHR                            0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM                              0x8245
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB                          0x8245
#define GL_DEBUG_CALLBACK_USER_PARAM_KHR                          0x8245
#define GL_DEBUG_SOURCE_API                                       0x8246
#define GL_DEBUG_SOURCE_API_ARB                                   0x8246
#define GL_DEBUG_SOURCE_API_KHR                                   0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM                             0x8247
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB                         0x8247
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR                         0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER                           0x8248
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB                       0x8248
#define GL_DEBUG_SOURCE_SHADER_COMPILER_KHR                       0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY                               0x8249
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB                           0x8249
#define GL_DEBUG_SOURCE_THIRD_PARTY_KHR                           0x8249
#define GL_DEBUG_SOURCE_APPLICATION                               0x824A
#define GL_DEBUG_SOURCE_APPLICATION_ARB                           0x824A
#define GL_DEBUG_SOURCE_APPLICATION_KHR                           0x824A
#define GL_DEBUG_SOURCE_OTHER                                     0x824B
#define GL_DEBUG_SOURCE_OTHER_ARB                                 0x824B
#define GL_DEBUG_SOURCE_OTHER_KHR                                 0x824B
#define GL_DEBUG_TYPE_ERROR                                       0x824C
#define GL_DEBUG_TYPE_ERROR_ARB                                   0x824C
#define GL_DEBUG_TYPE_ERROR_KHR                                   0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                         0x824D
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB                     0x824D
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR                     0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                          0x824E
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB                      0x824E
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR                      0x824E
#define GL_DEBUG_TYPE_PORTABILITY                                 0x824F
#define GL_DEBUG_TYPE_PORTABILITY_ARB                             0x824F
#define GL_DEBUG_TYPE_PORTABILITY_KHR                             0x824F
#define GL_DEBUG_TYPE_PERFORMANCE                                 0x8250
#define GL_DEBUG_TYPE_PERFORMANCE_ARB                             0x8250
#define GL_DEBUG_TYPE_PERFORMANCE_KHR                             0x8250
#define GL_DEBUG_TYPE_OTHER                                       0x8251
#define GL_DEBUG_TYPE_OTHER_ARB                                   0x8251
#define GL_DEBUG_TYPE_OTHER_KHR                                   0x8251
#define GL_LOSE_CONTEXT_ON_RESET                                  0x8252
#define GL_LOSE_CONTEXT_ON_RESET_ARB                              0x8252
#define GL_LOSE_CONTEXT_ON_RESET_EXT                              0x8252
#define GL_LOSE_CONTEXT_ON_RESET_KHR                              0x8252
#define GL_GUILTY_CONTEXT_RESET                                   0x8253
#define GL_GUILTY_CONTEXT_RESET_ARB                               0x8253
#define GL_GUILTY_CONTEXT_RESET_EXT                               0x8253
#define GL_GUILTY_CONTEXT_RESET_KHR                               0x8253
#define GL_INNOCENT_CONTEXT_RESET                                 0x8254
#define GL_INNOCENT_CONTEXT_RESET_ARB                             0x8254
#define GL_INNOCENT_CONTEXT_RESET_EXT                             0x8254
#define GL_INNOCENT_CONTEXT_RESET_KHR                             0x8254
#define GL_UNKNOWN_CONTEXT_RESET                                  0x8255
#define GL_UNKNOWN_CONTEXT_RESET_ARB                              0x8255
#define GL_UNKNOWN_CONTEXT_RESET_EXT                              0x8255
#define GL_UNKNOWN_CONTEXT_RESET_KHR                              0x8255
#define GL_RESET_NOTIFICATION_STRATEGY                            0x8256
#define GL_RESET_NOTIFICATION_STRATEGY_ARB                        0x8256
#define GL_RESET_NOTIFICATION_STRATEGY_EXT                        0x8256
#define GL_RESET_NOTIFICATION_STRATEGY_KHR                        0x8256
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT                        0x8257
#define GL_PROGRAM_SEPARABLE                                      0x8258
#define GL_PROGRAM_SEPARABLE_EXT                                  0x8258
#define GL_ACTIVE_PROGRAM                                         0x8259
#define GL_PROGRAM_PIPELINE_BINDING                               0x825A
#define GL_PROGRAM_PIPELINE_BINDING_EXT                           0x825A
#define GL_MAX_VIEWPORTS                                          0x825B
#define GL_MAX_VIEWPORTS_NV                                       0x825B
#define GL_MAX_VIEWPORTS_OES                                      0x825B
#define GL_VIEWPORT_SUBPIXEL_BITS                                 0x825C
#define GL_VIEWPORT_SUBPIXEL_BITS_EXT                             0x825C
#define GL_VIEWPORT_SUBPIXEL_BITS_NV                              0x825C
#define GL_VIEWPORT_SUBPIXEL_BITS_OES                             0x825C
#define GL_VIEWPORT_BOUNDS_RANGE                                  0x825D
#define GL_VIEWPORT_BOUNDS_RANGE_EXT                              0x825D
#define GL_VIEWPORT_BOUNDS_RANGE_NV                               0x825D
#define GL_VIEWPORT_BOUNDS_RANGE_OES                              0x825D
#define GL_LAYER_PROVOKING_VERTEX                                 0x825E
#define GL_LAYER_PROVOKING_VERTEX_EXT                             0x825E
#define GL_LAYER_PROVOKING_VERTEX_OES                             0x825E
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX                        0x825F
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX_EXT                    0x825F
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX_NV                     0x825F
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX_OES                    0x825F
#define GL_UNDEFINED_VERTEX                                       0x8260
#define GL_UNDEFINED_VERTEX_EXT                                   0x8260
#define GL_UNDEFINED_VERTEX_OES                                   0x8260
#define GL_NO_RESET_NOTIFICATION                                  0x8261
#define GL_NO_RESET_NOTIFICATION_ARB                              0x8261
#define GL_NO_RESET_NOTIFICATION_EXT                              0x8261
#define GL_NO_RESET_NOTIFICATION_KHR                              0x8261
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                         0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS                         0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS                     0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS                            0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS                0x8266
#define GL_COMPUTE_WORK_GROUP_SIZE                                0x8267
#define GL_DEBUG_TYPE_MARKER                                      0x8268
#define GL_DEBUG_TYPE_MARKER_KHR                                  0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP                                  0x8269
#define GL_DEBUG_TYPE_PUSH_GROUP_KHR                              0x8269
#define GL_DEBUG_TYPE_POP_GROUP                                   0x826A
#define GL_DEBUG_TYPE_POP_GROUP_KHR                               0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION                            0x826B
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR                        0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH                            0x826C
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR                        0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH                                0x826D
#define GL_DEBUG_GROUP_STACK_DEPTH_KHR                            0x826D
#define GL_MAX_UNIFORM_LOCATIONS                                  0x826E
#define GL_INTERNALFORMAT_SUPPORTED                               0x826F
#define GL_INTERNALFORMAT_PREFERRED                               0x8270
#define GL_INTERNALFORMAT_RED_SIZE                                0x8271
#define GL_INTERNALFORMAT_GREEN_SIZE                              0x8272
#define GL_INTERNALFORMAT_BLUE_SIZE                               0x8273
#define GL_INTERNALFORMAT_ALPHA_SIZE                              0x8274
#define GL_INTERNALFORMAT_DEPTH_SIZE                              0x8275
#define GL_INTERNALFORMAT_STENCIL_SIZE                            0x8276
#define GL_INTERNALFORMAT_SHARED_SIZE                             0x8277
#define GL_INTERNALFORMAT_RED_TYPE                                0x8278
#define GL_INTERNALFORMAT_GREEN_TYPE                              0x8279
#define GL_INTERNALFORMAT_BLUE_TYPE                               0x827A
#define GL_INTERNALFORMAT_ALPHA_TYPE                              0x827B
#define GL_INTERNALFORMAT_DEPTH_TYPE                              0x827C
#define GL_INTERNALFORMAT_STENCIL_TYPE                            0x827D
#define GL_MAX_WIDTH                                              0x827E
#define GL_MAX_HEIGHT                                             0x827F
#define GL_MAX_DEPTH                                              0x8280
#define GL_MAX_LAYERS                                             0x8281
#define GL_MAX_COMBINED_DIMENSIONS                                0x8282
#define GL_COLOR_COMPONENTS                                       0x8283
#define GL_DEPTH_COMPONENTS                                       0x8284
#define GL_STENCIL_COMPONENTS                                     0x8285
#define GL_COLOR_RENDERABLE                                       0x8286
#define GL_DEPTH_RENDERABLE                                       0x8287
#define GL_STENCIL_RENDERABLE                                     0x8288
#define GL_FRAMEBUFFER_RENDERABLE                                 0x8289
#define GL_FRAMEBUFFER_RENDERABLE_LAYERED                         0x828A
#define GL_FRAMEBUFFER_BLEND                                      0x828B
#define GL_READ_PIXELS                                            0x828C
#define GL_READ_PIXELS_FORMAT                                     0x828D
#define GL_READ_PIXELS_TYPE                                       0x828E
#define GL_TEXTURE_IMAGE_FORMAT                                   0x828F
#define GL_TEXTURE_IMAGE_TYPE                                     0x8290
#define GL_GET_TEXTURE_IMAGE_FORMAT                               0x8291
#define GL_GET_TEXTURE_IMAGE_TYPE                                 0x8292
#define GL_MIPMAP                                                 0x8293
#define GL_MANUAL_GENERATE_MIPMAP                                 0x8294
#define GL_AUTO_GENERATE_MIPMAP                                   0x8295
#define GL_COLOR_ENCODING                                         0x8296
#define GL_SRGB_READ                                              0x8297
#define GL_SRGB_WRITE                                             0x8298
#define GL_SRGB_DECODE_ARB                                        0x8299
#define GL_FILTER                                                 0x829A
#define GL_VERTEX_TEXTURE                                         0x829B
#define GL_TESS_CONTROL_TEXTURE                                   0x829C
#define GL_TESS_EVALUATION_TEXTURE                                0x829D
#define GL_GEOMETRY_TEXTURE                                       0x829E
#define GL_FRAGMENT_TEXTURE                                       0x829F
#define GL_COMPUTE_TEXTURE                                        0x82A0
#define GL_TEXTURE_SHADOW                                         0x82A1
#define GL_TEXTURE_GATHER                                         0x82A2
#define GL_TEXTURE_GATHER_SHADOW                                  0x82A3
#define GL_SHADER_IMAGE_LOAD                                      0x82A4
#define GL_SHADER_IMAGE_STORE                                     0x82A5
#define GL_SHADER_IMAGE_ATOMIC                                    0x82A6
#define GL_IMAGE_TEXEL_SIZE                                       0x82A7
#define GL_IMAGE_COMPATIBILITY_CLASS                              0x82A8
#define GL_IMAGE_PIXEL_FORMAT                                     0x82A9
#define GL_IMAGE_PIXEL_TYPE                                       0x82AA
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST                    0x82AC
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST                  0x82AD
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE                   0x82AE
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE                 0x82AF
#define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH                         0x82B1
#define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT                        0x82B2
#define GL_TEXTURE_COMPRESSED_BLOCK_SIZE                          0x82B3
#define GL_CLEAR_BUFFER                                           0x82B4
#define GL_TEXTURE_VIEW                                           0x82B5
#define GL_VIEW_COMPATIBILITY_CLASS                               0x82B6
#define GL_FULL_SUPPORT                                           0x82B7
#define GL_CAVEAT_SUPPORT                                         0x82B8
#define GL_IMAGE_CLASS_4_X_32                                     0x82B9
#define GL_IMAGE_CLASS_2_X_32                                     0x82BA
#define GL_IMAGE_CLASS_1_X_32                                     0x82BB
#define GL_IMAGE_CLASS_4_X_16                                     0x82BC
#define GL_IMAGE_CLASS_2_X_16                                     0x82BD
#define GL_IMAGE_CLASS_1_X_16                                     0x82BE
#define GL_IMAGE_CLASS_4_X_8                                      0x82BF
#define GL_IMAGE_CLASS_2_X_8                                      0x82C0
#define GL_IMAGE_CLASS_1_X_8                                      0x82C1
#define GL_IMAGE_CLASS_11_11_10                                   0x82C2
#define GL_IMAGE_CLASS_10_10_10_2                                 0x82C3
#define GL_VIEW_CLASS_128_BITS                                    0x82C4
#define GL_VIEW_CLASS_96_BITS                                     0x82C5
#define GL_VIEW_CLASS_64_BITS                                     0x82C6
#define GL_VIEW_CLASS_48_BITS                                     0x82C7
#define GL_VIEW_CLASS_32_BITS                                     0x82C8
#define GL_VIEW_CLASS_24_BITS                                     0x82C9
#define GL_VIEW_CLASS_16_BITS                                     0x82CA
#define GL_VIEW_CLASS_8_BITS                                      0x82CB
#define GL_VIEW_CLASS_S3TC_DXT1_RGB                               0x82CC
#define GL_VIEW_CLASS_S3TC_DXT1_RGBA                              0x82CD
#define GL_VIEW_CLASS_S3TC_DXT3_RGBA                              0x82CE
#define GL_VIEW_CLASS_S3TC_DXT5_RGBA                              0x82CF
#define GL_VIEW_CLASS_RGTC1_RED                                   0x82D0
#define GL_VIEW_CLASS_RGTC2_RG                                    0x82D1
#define GL_VIEW_CLASS_BPTC_UNORM                                  0x82D2
#define GL_VIEW_CLASS_BPTC_FLOAT                                  0x82D3
#define GL_VERTEX_ATTRIB_BINDING                                  0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET                          0x82D5
#define GL_VERTEX_BINDING_DIVISOR                                 0x82D6
#define GL_VERTEX_BINDING_OFFSET                                  0x82D7
#define GL_VERTEX_BINDING_STRIDE                                  0x82D8
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET                      0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS                             0x82DA
#define GL_TEXTURE_VIEW_MIN_LEVEL                                 0x82DB
#define GL_TEXTURE_VIEW_MIN_LEVEL_EXT                             0x82DB
#define GL_TEXTURE_VIEW_MIN_LEVEL_OES                             0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS                                0x82DC
#define GL_TEXTURE_VIEW_NUM_LEVELS_EXT                            0x82DC
#define GL_TEXTURE_VIEW_NUM_LEVELS_OES                            0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER                                 0x82DD
#define GL_TEXTURE_VIEW_MIN_LAYER_EXT                             0x82DD
#define GL_TEXTURE_VIEW_MIN_LAYER_OES                             0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS                                0x82DE
#define GL_TEXTURE_VIEW_NUM_LAYERS_EXT                            0x82DE
#define GL_TEXTURE_VIEW_NUM_LAYERS_OES                            0x82DE
#define GL_TEXTURE_IMMUTABLE_LEVELS                               0x82DF
#define GL_BUFFER                                                 0x82E0
#define GL_BUFFER_KHR                                             0x82E0
#define GL_SHADER                                                 0x82E1
#define GL_SHADER_KHR                                             0x82E1
#define GL_PROGRAM                                                0x82E2
#define GL_PROGRAM_KHR                                            0x82E2
#define GL_QUERY                                                  0x82E3
#define GL_QUERY_KHR                                              0x82E3
#define GL_PROGRAM_PIPELINE                                       0x82E4
#define GL_PROGRAM_PIPELINE_KHR                                   0x82E4
#define GL_MAX_VERTEX_ATTRIB_STRIDE                               0x82E5
#define GL_SAMPLER                                                0x82E6
#define GL_SAMPLER_KHR                                            0x82E6
#define GL_DISPLAY_LIST                                           0x82E7
#define GL_MAX_LABEL_LENGTH                                       0x82E8
#define GL_MAX_LABEL_LENGTH_KHR                                   0x82E8
#define GL_NUM_SHADING_LANGUAGE_VERSIONS                          0x82E9
#define GL_QUERY_TARGET                                           0x82EA
#define GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB                        0x82EC
#define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB                 0x82ED
#define GL_VERTICES_SUBMITTED_ARB                                 0x82EE
#define GL_PRIMITIVES_SUBMITTED_ARB                               0x82EF
#define GL_VERTEX_SHADER_INVOCATIONS_ARB                          0x82F0
#define GL_TESS_CONTROL_SHADER_PATCHES_ARB                        0x82F1
#define GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB                 0x82F2
#define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB                 0x82F3
#define GL_FRAGMENT_SHADER_INVOCATIONS_ARB                        0x82F4
#define GL_COMPUTE_SHADER_INVOCATIONS_ARB                         0x82F5
#define GL_CLIPPING_INPUT_PRIMITIVES_ARB                          0x82F6
#define GL_CLIPPING_OUTPUT_PRIMITIVES_ARB                         0x82F7
#define GL_SPARSE_BUFFER_PAGE_SIZE_ARB                            0x82F8
#define GL_MAX_CULL_DISTANCES                                     0x82F9
#define GL_MAX_CULL_DISTANCES_EXT                                 0x82F9
#define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES                   0x82FA
#define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT               0x82FA
#define GL_CONTEXT_RELEASE_BEHAVIOR                               0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_KHR                           0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH                         0x82FC
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR                     0x82FC
#define GL_DEPTH_PASS_INSTRUMENT_SGIX                             0x8310
#define GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX                    0x8311
#define GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX                         0x8312
#define GL_FRAGMENTS_INSTRUMENT_SGIX                              0x8313
#define GL_FRAGMENTS_INSTRUMENT_COUNTERS_SGIX                     0x8314
#define GL_FRAGMENTS_INSTRUMENT_MAX_SGIX                          0x8315
#define GL_CONVOLUTION_HINT_SGIX                                  0x8316
#define GL_YCRCB_SGIX                                             0x8318
#define GL_YCRCBA_SGIX                                            0x8319
#define GL_UNPACK_COMPRESSED_SIZE_SGIX                            0x831A
#define GL_PACK_MAX_COMPRESSED_SIZE_SGIX                          0x831B
#define GL_PACK_COMPRESSED_SIZE_SGIX                              0x831C
#define GL_SLIM8U_SGIX                                            0x831D
#define GL_SLIM10U_SGIX                                           0x831E
#define GL_SLIM12S_SGIX                                           0x831F
#define GL_ALPHA_MIN_SGIX                                         0x8320
#define GL_ALPHA_MAX_SGIX                                         0x8321
#define GL_SCALEBIAS_HINT_SGIX                                    0x8322
#define GL_ASYNC_MARKER_SGIX                                      0x8329
#define GL_PIXEL_TEX_GEN_MODE_SGIX                                0x832B
#define GL_ASYNC_HISTOGRAM_SGIX                                   0x832C
#define GL_MAX_ASYNC_HISTOGRAM_SGIX                               0x832D
#define GL_PIXEL_TRANSFORM_2D_EXT                                 0x8330
#define GL_PIXEL_MAG_FILTER_EXT                                   0x8331
#define GL_PIXEL_MIN_FILTER_EXT                                   0x8332
#define GL_PIXEL_CUBIC_WEIGHT_EXT                                 0x8333
#define GL_CUBIC_EXT                                              0x8334
#define GL_AVERAGE_EXT                                            0x8335
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT                     0x8336
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT                 0x8337
#define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT                          0x8338
#define GL_FRAGMENT_MATERIAL_EXT                                  0x8349
#define GL_FRAGMENT_NORMAL_EXT                                    0x834A
#define GL_FRAGMENT_COLOR_EXT                                     0x834C
#define GL_ATTENUATION_EXT                                        0x834D
#define GL_SHADOW_ATTENUATION_EXT                                 0x834E
#define GL_TEXTURE_APPLICATION_MODE_EXT                           0x834F
#define GL_TEXTURE_LIGHT_EXT                                      0x8350
#define GL_TEXTURE_MATERIAL_FACE_EXT                              0x8351
#define GL_TEXTURE_MATERIAL_PARAMETER_EXT                         0x8352
#define GL_PIXEL_TEXTURE_SGIS                                     0x8353
#define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS                         0x8354
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS                       0x8355
#define GL_PIXEL_GROUP_COLOR_SGIS                                 0x8356
#define GL_LINE_QUALITY_HINT_SGIX                                 0x835B
#define GL_ASYNC_TEX_IMAGE_SGIX                                   0x835C
#define GL_ASYNC_DRAW_PIXELS_SGIX                                 0x835D
#define GL_ASYNC_READ_PIXELS_SGIX                                 0x835E
#define GL_MAX_ASYNC_TEX_IMAGE_SGIX                               0x835F
#define GL_MAX_ASYNC_DRAW_PIXELS_SGIX                             0x8360
#define GL_MAX_ASYNC_READ_PIXELS_SGIX                             0x8361
#define GL_UNSIGNED_BYTE_2_3_3_REV                                0x8362
#define GL_UNSIGNED_BYTE_2_3_3_REV_EXT                            0x8362
#define GL_UNSIGNED_SHORT_5_6_5                                   0x8363
#define GL_UNSIGNED_SHORT_5_6_5_EXT                               0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV                               0x8364
#define GL_UNSIGNED_SHORT_5_6_5_REV_EXT                           0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV                             0x8365
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                         0x8365
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG                         0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                             0x8366
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                         0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV                               0x8367
#define GL_UNSIGNED_INT_8_8_8_8_REV_EXT                           0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV                            0x8368
#define GL_UNSIGNED_INT_2_10_10_10_REV_EXT                        0x8368
#define GL_TEXTURE_MAX_CLAMP_S_SGIX                               0x8369
#define GL_TEXTURE_MAX_CLAMP_T_SGIX                               0x836A
#define GL_TEXTURE_MAX_CLAMP_R_SGIX                               0x836B
#define GL_MIRRORED_REPEAT                                        0x8370
#define GL_MIRRORED_REPEAT_ARB                                    0x8370
#define GL_MIRRORED_REPEAT_IBM                                    0x8370
#define GL_MIRRORED_REPEAT_OES                                    0x8370
#define GL_RGB_S3TC                                               0x83A0
#define GL_RGB4_S3TC                                              0x83A1
#define GL_RGBA_S3TC                                              0x83A2
#define GL_RGBA4_S3TC                                             0x83A3
#define GL_RGBA_DXT5_S3TC                                         0x83A4
#define GL_RGBA4_DXT5_S3TC                                        0x83A5
#define GL_VERTEX_PRECLIP_SGIX                                    0x83EE
#define GL_VERTEX_PRECLIP_HINT_SGIX                               0x83EF
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                           0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                          0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE                        0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                          0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE                        0x83F3
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                          0x83F3
#define GL_PARALLEL_ARRAYS_INTEL                                  0x83F4
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL                   0x83F5
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL                   0x83F6
#define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL                    0x83F7
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL            0x83F8
#define GL_PERFQUERY_DONOT_FLUSH_INTEL                            0x83F9
#define GL_PERFQUERY_FLUSH_INTEL                                  0x83FA
#define GL_PERFQUERY_WAIT_INTEL                                   0x83FB
#define GL_CONSERVATIVE_RASTERIZATION_INTEL                       0x83FE
#define GL_TEXTURE_MEMORY_LAYOUT_INTEL                            0x83FF
#define GL_FRAGMENT_LIGHTING_SGIX                                 0x8400
#define GL_FRAGMENT_COLOR_MATERIAL_SGIX                           0x8401
#define GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX                      0x8402
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX                 0x8403
#define GL_MAX_FRAGMENT_LIGHTS_SGIX                               0x8404
#define GL_MAX_ACTIVE_LIGHTS_SGIX                                 0x8405
#define GL_CURRENT_RASTER_NORMAL_SGIX                             0x8406
#define GL_LIGHT_ENV_MODE_SGIX                                    0x8407
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX                 0x8408
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX                     0x8409
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX                      0x840A
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX         0x840B
#define GL_FRAGMENT_LIGHT0_SGIX                                   0x840C
#define GL_FRAGMENT_LIGHT1_SGIX                                   0x840D
#define GL_FRAGMENT_LIGHT2_SGIX                                   0x840E
#define GL_FRAGMENT_LIGHT3_SGIX                                   0x840F
#define GL_FRAGMENT_LIGHT4_SGIX                                   0x8410
#define GL_FRAGMENT_LIGHT5_SGIX                                   0x8411
#define GL_FRAGMENT_LIGHT6_SGIX                                   0x8412
#define GL_FRAGMENT_LIGHT7_SGIX                                   0x8413
#define GL_PACK_RESAMPLE_SGIX                                     0x842E
#define GL_UNPACK_RESAMPLE_SGIX                                   0x842F
#define GL_RESAMPLE_DECIMATE_SGIX                                 0x8430
#define GL_RESAMPLE_REPLICATE_SGIX                                0x8433
#define GL_RESAMPLE_ZERO_FILL_SGIX                                0x8434
#define GL_TANGENT_ARRAY_EXT                                      0x8439
#define GL_BINORMAL_ARRAY_EXT                                     0x843A
#define GL_CURRENT_TANGENT_EXT                                    0x843B
#define GL_CURRENT_BINORMAL_EXT                                   0x843C
#define GL_TANGENT_ARRAY_TYPE_EXT                                 0x843E
#define GL_TANGENT_ARRAY_STRIDE_EXT                               0x843F
#define GL_BINORMAL_ARRAY_TYPE_EXT                                0x8440
#define GL_BINORMAL_ARRAY_STRIDE_EXT                              0x8441
#define GL_TANGENT_ARRAY_POINTER_EXT                              0x8442
#define GL_BINORMAL_ARRAY_POINTER_EXT                             0x8443
#define GL_MAP1_TANGENT_EXT                                       0x8444
#define GL_MAP2_TANGENT_EXT                                       0x8445
#define GL_MAP1_BINORMAL_EXT                                      0x8446
#define GL_MAP2_BINORMAL_EXT                                      0x8447
#define GL_NEAREST_CLIPMAP_NEAREST_SGIX                           0x844D
#define GL_NEAREST_CLIPMAP_LINEAR_SGIX                            0x844E
#define GL_LINEAR_CLIPMAP_NEAREST_SGIX                            0x844F
#define GL_FOG_COORDINATE_SOURCE                                  0x8450
#define GL_FOG_COORDINATE_SOURCE_EXT                              0x8450
#define GL_FOG_COORD_SRC                                          0x8450
#define GL_FOG_COORDINATE                                         0x8451
#define GL_FOG_COORD                                              0x8451
#define GL_FOG_COORDINATE_EXT                                     0x8451
#define GL_FRAGMENT_DEPTH                                         0x8452
#define GL_FRAGMENT_DEPTH_EXT                                     0x8452
#define GL_CURRENT_FOG_COORDINATE                                 0x8453
#define GL_CURRENT_FOG_COORD                                      0x8453
#define GL_CURRENT_FOG_COORDINATE_EXT                             0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE                              0x8454
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT                          0x8454
#define GL_FOG_COORD_ARRAY_TYPE                                   0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE                            0x8455
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT                        0x8455
#define GL_FOG_COORD_ARRAY_STRIDE                                 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER                           0x8456
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT                       0x8456
#define GL_FOG_COORD_ARRAY_POINTER                                0x8456
#define GL_FOG_COORDINATE_ARRAY                                   0x8457
#define GL_FOG_COORDINATE_ARRAY_EXT                               0x8457
#define GL_FOG_COORD_ARRAY                                        0x8457
#define GL_COLOR_SUM                                              0x8458
#define GL_COLOR_SUM_ARB                                          0x8458
#define GL_COLOR_SUM_EXT                                          0x8458
#define GL_CURRENT_SECONDARY_COLOR                                0x8459
#define GL_CURRENT_SECONDARY_COLOR_EXT                            0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE                             0x845A
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT                         0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE                             0x845B
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT                         0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE                           0x845C
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT                       0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER                          0x845D
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT                      0x845D
#define GL_SECONDARY_COLOR_ARRAY                                  0x845E
#define GL_SECONDARY_COLOR_ARRAY_EXT                              0x845E
#define GL_CURRENT_RASTER_SECONDARY_COLOR                         0x845F
#define GL_ALIASED_POINT_SIZE_RANGE                               0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE                               0x846E
#define GL_SCREEN_COORDINATES_REND                                0x8490
#define GL_INVERTED_SCREEN_W_REND                                 0x8491
#define GL_TEXTURE0                                               0x84C0
#define GL_TEXTURE0_ARB                                           0x84C0
#define GL_TEXTURE1                                               0x84C1
#define GL_TEXTURE1_ARB                                           0x84C1
#define GL_TEXTURE2                                               0x84C2
#define GL_TEXTURE2_ARB                                           0x84C2
#define GL_TEXTURE3                                               0x84C3
#define GL_TEXTURE3_ARB                                           0x84C3
#define GL_TEXTURE4                                               0x84C4
#define GL_TEXTURE4_ARB                                           0x84C4
#define GL_TEXTURE5                                               0x84C5
#define GL_TEXTURE5_ARB                                           0x84C5
#define GL_TEXTURE6                                               0x84C6
#define GL_TEXTURE6_ARB                                           0x84C6
#define GL_TEXTURE7                                               0x84C7
#define GL_TEXTURE7_ARB                                           0x84C7
#define GL_TEXTURE8                                               0x84C8
#define GL_TEXTURE8_ARB                                           0x84C8
#define GL_TEXTURE9                                               0x84C9
#define GL_TEXTURE9_ARB                                           0x84C9
#define GL_TEXTURE10                                              0x84CA
#define GL_TEXTURE10_ARB                                          0x84CA
#define GL_TEXTURE11                                              0x84CB
#define GL_TEXTURE11_ARB                                          0x84CB
#define GL_TEXTURE12                                              0x84CC
#define GL_TEXTURE12_ARB                                          0x84CC
#define GL_TEXTURE13                                              0x84CD
#define GL_TEXTURE13_ARB                                          0x84CD
#define GL_TEXTURE14                                              0x84CE
#define GL_TEXTURE14_ARB                                          0x84CE
#define GL_TEXTURE15                                              0x84CF
#define GL_TEXTURE15_ARB                                          0x84CF
#define GL_TEXTURE16                                              0x84D0
#define GL_TEXTURE16_ARB                                          0x84D0
#define GL_TEXTURE17                                              0x84D1
#define GL_TEXTURE17_ARB                                          0x84D1
#define GL_TEXTURE18                                              0x84D2
#define GL_TEXTURE18_ARB                                          0x84D2
#define GL_TEXTURE19                                              0x84D3
#define GL_TEXTURE19_ARB                                          0x84D3
#define GL_TEXTURE20                                              0x84D4
#define GL_TEXTURE20_ARB                                          0x84D4
#define GL_TEXTURE21                                              0x84D5
#define GL_TEXTURE21_ARB                                          0x84D5
#define GL_TEXTURE22                                              0x84D6
#define GL_TEXTURE22_ARB                                          0x84D6
#define GL_TEXTURE23                                              0x84D7
#define GL_TEXTURE23_ARB                                          0x84D7
#define GL_TEXTURE24                                              0x84D8
#define GL_TEXTURE24_ARB                                          0x84D8
#define GL_TEXTURE25                                              0x84D9
#define GL_TEXTURE25_ARB                                          0x84D9
#define GL_TEXTURE26                                              0x84DA
#define GL_TEXTURE26_ARB                                          0x84DA
#define GL_TEXTURE27                                              0x84DB
#define GL_TEXTURE27_ARB                                          0x84DB
#define GL_TEXTURE28                                              0x84DC
#define GL_TEXTURE28_ARB                                          0x84DC
#define GL_TEXTURE29                                              0x84DD
#define GL_TEXTURE29_ARB                                          0x84DD
#define GL_TEXTURE30                                              0x84DE
#define GL_TEXTURE30_ARB                                          0x84DE
#define GL_TEXTURE31                                              0x84DF
#define GL_TEXTURE31_ARB                                          0x84DF
#define GL_ACTIVE_TEXTURE                                         0x84E0
#define GL_ACTIVE_TEXTURE_ARB                                     0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE                                  0x84E1
#define GL_CLIENT_ACTIVE_TEXTURE_ARB                              0x84E1
#define GL_MAX_TEXTURE_UNITS                                      0x84E2
#define GL_MAX_TEXTURE_UNITS_ARB                                  0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX                             0x84E3
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB                         0x84E3
#define GL_PATH_TRANSPOSE_MODELVIEW_MATRIX_NV                     0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX                            0x84E4
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB                        0x84E4
#define GL_PATH_TRANSPOSE_PROJECTION_MATRIX_NV                    0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX                               0x84E5
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB                           0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX                                 0x84E6
#define GL_TRANSPOSE_COLOR_MATRIX_ARB                             0x84E6
#define GL_SUBTRACT                                               0x84E7
#define GL_SUBTRACT_ARB                                           0x84E7
#define GL_MAX_RENDERBUFFER_SIZE                                  0x84E8
#define GL_MAX_RENDERBUFFER_SIZE_EXT                              0x84E8
#define GL_MAX_RENDERBUFFER_SIZE_OES                              0x84E8
#define GL_COMPRESSED_ALPHA                                       0x84E9
#define GL_COMPRESSED_ALPHA_ARB                                   0x84E9
#define GL_COMPRESSED_LUMINANCE                                   0x84EA
#define GL_COMPRESSED_LUMINANCE_ARB                               0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA                             0x84EB
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB                         0x84EB
#define GL_COMPRESSED_INTENSITY                                   0x84EC
#define GL_COMPRESSED_INTENSITY_ARB                               0x84EC
#define GL_COMPRESSED_RGB                                         0x84ED
#define GL_COMPRESSED_RGB_ARB                                     0x84ED
#define GL_COMPRESSED_RGBA                                        0x84EE
#define GL_COMPRESSED_RGBA_ARB                                    0x84EE
#define GL_TEXTURE_COMPRESSION_HINT                               0x84EF
#define GL_TEXTURE_COMPRESSION_HINT_ARB                           0x84EF
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER        0x84F0
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER     0x84F1
#define GL_ALL_COMPLETED_NV                                       0x84F2
#define GL_FENCE_STATUS_NV                                        0x84F3
#define GL_FENCE_CONDITION_NV                                     0x84F4
#define GL_TEXTURE_RECTANGLE                                      0x84F5
#define GL_TEXTURE_RECTANGLE_ARB                                  0x84F5
#define GL_TEXTURE_RECTANGLE_NV                                   0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE                              0x84F6
#define GL_TEXTURE_BINDING_RECTANGLE_ARB                          0x84F6
#define GL_TEXTURE_BINDING_RECTANGLE_NV                           0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE                                0x84F7
#define GL_PROXY_TEXTURE_RECTANGLE_ARB                            0x84F7
#define GL_PROXY_TEXTURE_RECTANGLE_NV                             0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE                             0x84F8
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB                         0x84F8
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV                          0x84F8
#define GL_DEPTH_STENCIL                                          0x84F9
#define GL_DEPTH_STENCIL_EXT                                      0x84F9
#define GL_DEPTH_STENCIL_NV                                       0x84F9
#define GL_DEPTH_STENCIL_OES                                      0x84F9
#define GL_UNSIGNED_INT_24_8                                      0x84FA
#define GL_UNSIGNED_INT_24_8_EXT                                  0x84FA
#define GL_UNSIGNED_INT_24_8_NV                                   0x84FA
#define GL_UNSIGNED_INT_24_8_OES                                  0x84FA
#define GL_MAX_TEXTURE_LOD_BIAS                                   0x84FD
#define GL_MAX_TEXTURE_LOD_BIAS_EXT                               0x84FD
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                             0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                         0x84FF
#define GL_TEXTURE_FILTER_CONTROL                                 0x8500
#define GL_TEXTURE_FILTER_CONTROL_EXT                             0x8500
#define GL_TEXTURE_LOD_BIAS                                       0x8501
#define GL_TEXTURE_LOD_BIAS_EXT                                   0x8501
#define GL_MODELVIEW1_STACK_DEPTH_EXT                             0x8502
#define GL_COMBINE4_NV                                            0x8503
#define GL_MAX_SHININESS_NV                                       0x8504
#define GL_MAX_SPOT_EXPONENT_NV                                   0x8505
#define GL_MODELVIEW1_MATRIX_EXT                                  0x8506
#define GL_INCR_WRAP                                              0x8507
#define GL_INCR_WRAP_EXT                                          0x8507
#define GL_INCR_WRAP_OES                                          0x8507
#define GL_DECR_WRAP                                              0x8508
#define GL_DECR_WRAP_EXT                                          0x8508
#define GL_DECR_WRAP_OES                                          0x8508
#define GL_VERTEX_WEIGHTING_EXT                                   0x8509
#define GL_MODELVIEW1_ARB                                         0x850A
#define GL_MODELVIEW1_EXT                                         0x850A
#define GL_CURRENT_VERTEX_WEIGHT_EXT                              0x850B
#define GL_VERTEX_WEIGHT_ARRAY_EXT                                0x850C
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT                           0x850D
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT                           0x850E
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT                         0x850F
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT                        0x8510
#define GL_NORMAL_MAP                                             0x8511
#define GL_NORMAL_MAP_ARB                                         0x8511
#define GL_NORMAL_MAP_EXT                                         0x8511
#define GL_NORMAL_MAP_NV                                          0x8511
#define GL_NORMAL_MAP_OES                                         0x8511
#define GL_REFLECTION_MAP                                         0x8512
#define GL_REFLECTION_MAP_ARB                                     0x8512
#define GL_REFLECTION_MAP_EXT                                     0x8512
#define GL_REFLECTION_MAP_NV                                      0x8512
#define GL_REFLECTION_MAP_OES                                     0x8512
#define GL_TEXTURE_CUBE_MAP                                       0x8513
#define GL_TEXTURE_CUBE_MAP_ARB                                   0x8513
#define GL_TEXTURE_CUBE_MAP_EXT                                   0x8513
#define GL_TEXTURE_CUBE_MAP_OES                                   0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP                               0x8514
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB                           0x8514
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT                           0x8514
#define GL_TEXTURE_BINDING_CUBE_MAP_OES                           0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                            0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB                        0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT                        0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES                        0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                            0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB                        0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT                        0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES                        0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                            0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB                        0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT                        0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES                        0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                            0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB                        0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT                        0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES                        0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                            0x8519
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB                        0x8519
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT                        0x8519
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES                        0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                            0x851A
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB                        0x851A
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT                        0x851A
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES                        0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP                                 0x851B
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB                             0x851B
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT                             0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                              0x851C
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB                          0x851C
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT                          0x851C
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_OES                          0x851C
#define GL_VERTEX_ARRAY_RANGE_APPLE                               0x851D
#define GL_VERTEX_ARRAY_RANGE_NV                                  0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE                        0x851E
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV                           0x851E
#define GL_VERTEX_ARRAY_RANGE_VALID_NV                            0x851F
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE                        0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV                      0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE                       0x8521
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV                          0x8521
#define GL_REGISTER_COMBINERS_NV                                  0x8522
#define GL_VARIABLE_A_NV                                          0x8523
#define GL_VARIABLE_B_NV                                          0x8524
#define GL_VARIABLE_C_NV                                          0x8525
#define GL_VARIABLE_D_NV                                          0x8526
#define GL_VARIABLE_E_NV                                          0x8527
#define GL_VARIABLE_F_NV                                          0x8528
#define GL_VARIABLE_G_NV                                          0x8529
#define GL_CONSTANT_COLOR0_NV                                     0x852A
#define GL_CONSTANT_COLOR1_NV                                     0x852B
#define GL_PRIMARY_COLOR_NV                                       0x852C
#define GL_SECONDARY_COLOR_NV                                     0x852D
#define GL_SPARE0_NV                                              0x852E
#define GL_SPARE1_NV                                              0x852F
#define GL_DISCARD_NV                                             0x8530
#define GL_E_TIMES_F_NV                                           0x8531
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV                         0x8532
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV                    0x8533
#define GL_MULTISAMPLE_FILTER_HINT_NV                             0x8534
#define GL_PER_STAGE_CONSTANTS_NV                                 0x8535
#define GL_UNSIGNED_IDENTITY_NV                                   0x8536
#define GL_UNSIGNED_INVERT_NV                                     0x8537
#define GL_EXPAND_NORMAL_NV                                       0x8538
#define GL_EXPAND_NEGATE_NV                                       0x8539
#define GL_HALF_BIAS_NORMAL_NV                                    0x853A
#define GL_HALF_BIAS_NEGATE_NV                                    0x853B
#define GL_SIGNED_IDENTITY_NV                                     0x853C
#define GL_SIGNED_NEGATE_NV                                       0x853D
#define GL_SCALE_BY_TWO_NV                                        0x853E
#define GL_SCALE_BY_FOUR_NV                                       0x853F
#define GL_SCALE_BY_ONE_HALF_NV                                   0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV                           0x8541
#define GL_COMBINER_INPUT_NV                                      0x8542
#define GL_COMBINER_MAPPING_NV                                    0x8543
#define GL_COMBINER_COMPONENT_USAGE_NV                            0x8544
#define GL_COMBINER_AB_DOT_PRODUCT_NV                             0x8545
#define GL_COMBINER_CD_DOT_PRODUCT_NV                             0x8546
#define GL_COMBINER_MUX_SUM_NV                                    0x8547
#define GL_COMBINER_SCALE_NV                                      0x8548
#define GL_COMBINER_BIAS_NV                                       0x8549
#define GL_COMBINER_AB_OUTPUT_NV                                  0x854A
#define GL_COMBINER_CD_OUTPUT_NV                                  0x854B
#define GL_COMBINER_SUM_OUTPUT_NV                                 0x854C
#define GL_MAX_GENERAL_COMBINERS_NV                               0x854D
#define GL_NUM_GENERAL_COMBINERS_NV                               0x854E
#define GL_COLOR_SUM_CLAMP_NV                                     0x854F
#define GL_COMBINER0_NV                                           0x8550
#define GL_COMBINER1_NV                                           0x8551
#define GL_COMBINER2_NV                                           0x8552
#define GL_COMBINER3_NV                                           0x8553
#define GL_COMBINER4_NV                                           0x8554
#define GL_COMBINER5_NV                                           0x8555
#define GL_COMBINER6_NV                                           0x8556
#define GL_COMBINER7_NV                                           0x8557
#define GL_PRIMITIVE_RESTART_NV                                   0x8558
#define GL_PRIMITIVE_RESTART_INDEX_NV                             0x8559
#define GL_FOG_DISTANCE_MODE_NV                                   0x855A
#define GL_EYE_RADIAL_NV                                          0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV                                  0x855C
#define GL_EMBOSS_LIGHT_NV                                        0x855D
#define GL_EMBOSS_CONSTANT_NV                                     0x855E
#define GL_EMBOSS_MAP_NV                                          0x855F
#define GL_RED_MIN_CLAMP_INGR                                     0x8560
#define GL_GREEN_MIN_CLAMP_INGR                                   0x8561
#define GL_BLUE_MIN_CLAMP_INGR                                    0x8562
#define GL_ALPHA_MIN_CLAMP_INGR                                   0x8563
#define GL_RED_MAX_CLAMP_INGR                                     0x8564
#define GL_GREEN_MAX_CLAMP_INGR                                   0x8565
#define GL_BLUE_MAX_CLAMP_INGR                                    0x8566
#define GL_ALPHA_MAX_CLAMP_INGR                                   0x8567
#define GL_INTERLACE_READ_INGR                                    0x8568
#define GL_COMBINE                                                0x8570
#define GL_COMBINE_ARB                                            0x8570
#define GL_COMBINE_EXT                                            0x8570
#define GL_COMBINE_RGB                                            0x8571
#define GL_COMBINE_RGB_ARB                                        0x8571
#define GL_COMBINE_RGB_EXT                                        0x8571
#define GL_COMBINE_ALPHA                                          0x8572
#define GL_COMBINE_ALPHA_ARB                                      0x8572
#define GL_COMBINE_ALPHA_EXT                                      0x8572
#define GL_RGB_SCALE                                              0x8573
#define GL_RGB_SCALE_ARB                                          0x8573
#define GL_RGB_SCALE_EXT                                          0x8573
#define GL_ADD_SIGNED                                             0x8574
#define GL_ADD_SIGNED_ARB                                         0x8574
#define GL_ADD_SIGNED_EXT                                         0x8574
#define GL_INTERPOLATE                                            0x8575
#define GL_INTERPOLATE_ARB                                        0x8575
#define GL_INTERPOLATE_EXT                                        0x8575
#define GL_CONSTANT                                               0x8576
#define GL_CONSTANT_ARB                                           0x8576
#define GL_CONSTANT_EXT                                           0x8576
#define GL_CONSTANT_NV                                            0x8576
#define GL_PRIMARY_COLOR                                          0x8577
#define GL_PRIMARY_COLOR_ARB                                      0x8577
#define GL_PRIMARY_COLOR_EXT                                      0x8577
#define GL_PREVIOUS                                               0x8578
#define GL_PREVIOUS_ARB                                           0x8578
#define GL_PREVIOUS_EXT                                           0x8578
#define GL_SOURCE0_RGB                                            0x8580
#define GL_SOURCE0_RGB_ARB                                        0x8580
#define GL_SOURCE0_RGB_EXT                                        0x8580
#define GL_SRC0_RGB                                               0x8580
#define GL_SOURCE1_RGB                                            0x8581
#define GL_SOURCE1_RGB_ARB                                        0x8581
#define GL_SOURCE1_RGB_EXT                                        0x8581
#define GL_SRC1_RGB                                               0x8581
#define GL_SOURCE2_RGB                                            0x8582
#define GL_SOURCE2_RGB_ARB                                        0x8582
#define GL_SOURCE2_RGB_EXT                                        0x8582
#define GL_SRC2_RGB                                               0x8582
#define GL_SOURCE3_RGB_NV                                         0x8583
#define GL_SOURCE0_ALPHA                                          0x8588
#define GL_SOURCE0_ALPHA_ARB                                      0x8588
#define GL_SOURCE0_ALPHA_EXT                                      0x8588
#define GL_SRC0_ALPHA                                             0x8588
#define GL_SOURCE1_ALPHA                                          0x8589
#define GL_SOURCE1_ALPHA_ARB                                      0x8589
#define GL_SOURCE1_ALPHA_EXT                                      0x8589
#define GL_SRC1_ALPHA                                             0x8589
#define GL_SRC1_ALPHA_EXT                                         0x8589
#define GL_SOURCE2_ALPHA                                          0x858A
#define GL_SOURCE2_ALPHA_ARB                                      0x858A
#define GL_SOURCE2_ALPHA_EXT                                      0x858A
#define GL_SRC2_ALPHA                                             0x858A
#define GL_SOURCE3_ALPHA_NV                                       0x858B
#define GL_OPERAND0_RGB                                           0x8590
#define GL_OPERAND0_RGB_ARB                                       0x8590
#define GL_OPERAND0_RGB_EXT                                       0x8590
#define GL_OPERAND1_RGB                                           0x8591
#define GL_OPERAND1_RGB_ARB                                       0x8591
#define GL_OPERAND1_RGB_EXT                                       0x8591
#define GL_OPERAND2_RGB                                           0x8592
#define GL_OPERAND2_RGB_ARB                                       0x8592
#define GL_OPERAND2_RGB_EXT                                       0x8592
#define GL_OPERAND3_RGB_NV                                        0x8593
#define GL_OPERAND0_ALPHA                                         0x8598
#define GL_OPERAND0_ALPHA_ARB                                     0x8598
#define GL_OPERAND0_ALPHA_EXT                                     0x8598
#define GL_OPERAND1_ALPHA                                         0x8599
#define GL_OPERAND1_ALPHA_ARB                                     0x8599
#define GL_OPERAND1_ALPHA_EXT                                     0x8599
#define GL_OPERAND2_ALPHA                                         0x859A
#define GL_OPERAND2_ALPHA_ARB                                     0x859A
#define GL_OPERAND2_ALPHA_EXT                                     0x859A
#define GL_OPERAND3_ALPHA_NV                                      0x859B
#define GL_PACK_SUBSAMPLE_RATE_SGIX                               0x85A0
#define GL_UNPACK_SUBSAMPLE_RATE_SGIX                             0x85A1
#define GL_PIXEL_SUBSAMPLE_4444_SGIX                              0x85A2
#define GL_PIXEL_SUBSAMPLE_2424_SGIX                              0x85A3
#define GL_PIXEL_SUBSAMPLE_4242_SGIX                              0x85A4
#define GL_PERTURB_EXT                                            0x85AE
#define GL_TEXTURE_NORMAL_EXT                                     0x85AF
#define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE                      0x85B0
#define GL_TRANSFORM_HINT_APPLE                                   0x85B1
#define GL_UNPACK_CLIENT_STORAGE_APPLE                            0x85B2
#define GL_BUFFER_OBJECT_APPLE                                    0x85B3
#define GL_STORAGE_CLIENT_APPLE                                   0x85B4
#define GL_VERTEX_ARRAY_BINDING                                   0x85B5
#define GL_VERTEX_ARRAY_BINDING_APPLE                             0x85B5
#define GL_VERTEX_ARRAY_BINDING_OES                               0x85B5
#define GL_TEXTURE_RANGE_LENGTH_APPLE                             0x85B7
#define GL_TEXTURE_RANGE_POINTER_APPLE                            0x85B8
#define GL_YCBCR_422_APPLE                                        0x85B9
#define GL_UNSIGNED_SHORT_8_8_APPLE                               0x85BA
#define GL_UNSIGNED_SHORT_8_8_MESA                                0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE                           0x85BB
#define GL_UNSIGNED_SHORT_8_8_REV_MESA                            0x85BB
#define GL_TEXTURE_STORAGE_HINT_APPLE                             0x85BC
#define GL_STORAGE_PRIVATE_APPLE                                  0x85BD
#define GL_STORAGE_CACHED_APPLE                                   0x85BE
#define GL_STORAGE_SHARED_APPLE                                   0x85BF
#define GL_REPLACEMENT_CODE_ARRAY_SUN                             0x85C0
#define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN                        0x85C1
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN                      0x85C2
#define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN                     0x85C3
#define GL_R1UI_V3F_SUN                                           0x85C4
#define GL_R1UI_C4UB_V3F_SUN                                      0x85C5
#define GL_R1UI_C3F_V3F_SUN                                       0x85C6
#define GL_R1UI_N3F_V3F_SUN                                       0x85C7
#define GL_R1UI_C4F_N3F_V3F_SUN                                   0x85C8
#define GL_R1UI_T2F_V3F_SUN                                       0x85C9
#define GL_R1UI_T2F_N3F_V3F_SUN                                   0x85CA
#define GL_R1UI_T2F_C4F_N3F_V3F_SUN                               0x85CB
#define GL_SLICE_ACCUM_SUN                                        0x85CC
#define GL_QUAD_MESH_SUN                                          0x8614
#define GL_TRIANGLE_MESH_SUN                                      0x8615
#define GL_VERTEX_PROGRAM_ARB                                     0x8620
#define GL_VERTEX_PROGRAM_NV                                      0x8620
#define GL_VERTEX_STATE_PROGRAM_NV                                0x8621
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                            0x8622
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                        0x8622
#define GL_ATTRIB_ARRAY_SIZE_NV                                   0x8623
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                               0x8623
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV                                 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                             0x8624
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV                                   0x8625
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                               0x8625
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                           0x8625
#define GL_CURRENT_ATTRIB_NV                                      0x8626
#define GL_CURRENT_VERTEX_ATTRIB                                  0x8626
#define GL_CURRENT_VERTEX_ATTRIB_ARB                              0x8626
#define GL_PROGRAM_LENGTH_ARB                                     0x8627
#define GL_PROGRAM_LENGTH_NV                                      0x8627
#define GL_PROGRAM_STRING_ARB                                     0x8628
#define GL_PROGRAM_STRING_NV                                      0x8628
#define GL_MODELVIEW_PROJECTION_NV                                0x8629
#define GL_IDENTITY_NV                                            0x862A
#define GL_INVERSE_NV                                             0x862B
#define GL_TRANSPOSE_NV                                           0x862C
#define GL_INVERSE_TRANSPOSE_NV                                   0x862D
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB                     0x862E
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV                        0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB                               0x862F
#define GL_MAX_TRACK_MATRICES_NV                                  0x862F
#define GL_MATRIX0_NV                                             0x8630
#define GL_MATRIX1_NV                                             0x8631
#define GL_MATRIX2_NV                                             0x8632
#define GL_MATRIX3_NV                                             0x8633
#define GL_MATRIX4_NV                                             0x8634
#define GL_MATRIX5_NV                                             0x8635
#define GL_MATRIX6_NV                                             0x8636
#define GL_MATRIX7_NV                                             0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                         0x8640
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV                          0x8640
#define GL_CURRENT_MATRIX_ARB                                     0x8641
#define GL_CURRENT_MATRIX_NV                                      0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE                              0x8642
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB                          0x8642
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV                           0x8642
#define GL_PROGRAM_POINT_SIZE                                     0x8642
#define GL_PROGRAM_POINT_SIZE_ARB                                 0x8642
#define GL_PROGRAM_POINT_SIZE_EXT                                 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE                                0x8643
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB                            0x8643
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV                             0x8643
#define GL_PROGRAM_PARAMETER_NV                                   0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV                                0x8645
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                            0x8645
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                        0x8645
#define GL_PROGRAM_TARGET_NV                                      0x8646
#define GL_PROGRAM_RESIDENT_NV                                    0x8647
#define GL_TRACK_MATRIX_NV                                        0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV                              0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV                              0x864A
#define GL_PROGRAM_ERROR_POSITION_ARB                             0x864B
#define GL_PROGRAM_ERROR_POSITION_NV                              0x864B
#define GL_OFFSET_TEXTURE_RECTANGLE_NV                            0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV                      0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV                       0x864E
#define GL_DEPTH_CLAMP                                            0x864F
#define GL_DEPTH_CLAMP_NV                                         0x864F
#define GL_VERTEX_ATTRIB_ARRAY0_NV                                0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV                                0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV                                0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV                                0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV                                0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV                                0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV                                0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV                                0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV                                0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV                                0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV                               0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV                               0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV                               0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV                               0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV                               0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV                               0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV                               0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV                               0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV                               0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV                               0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV                               0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV                               0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV                               0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV                               0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV                               0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV                               0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV                              0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV                              0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV                              0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV                              0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV                              0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV                              0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV                               0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV                               0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV                               0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV                               0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV                               0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV                               0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV                               0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV                               0x8677
#define GL_PROGRAM_BINDING_ARB                                    0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV                               0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV                               0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV                              0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV                              0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV                              0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV                              0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV                              0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV                              0x867F
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                          0x86A0
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB                      0x86A0
#define GL_TEXTURE_COMPRESSED                                     0x86A1
#define GL_TEXTURE_COMPRESSED_ARB                                 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                         0x86A2
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB                     0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS                             0x86A3
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB                         0x86A3
#define GL_MAX_VERTEX_UNITS_ARB                                   0x86A4
#define GL_MAX_VERTEX_UNITS_OES                                   0x86A4
#define GL_ACTIVE_VERTEX_UNITS_ARB                                0x86A5
#define GL_WEIGHT_SUM_UNITY_ARB                                   0x86A6
#define GL_VERTEX_BLEND_ARB                                       0x86A7
#define GL_CURRENT_WEIGHT_ARB                                     0x86A8
#define GL_WEIGHT_ARRAY_TYPE_ARB                                  0x86A9
#define GL_WEIGHT_ARRAY_TYPE_OES                                  0x86A9
#define GL_WEIGHT_ARRAY_STRIDE_ARB                                0x86AA
#define GL_WEIGHT_ARRAY_STRIDE_OES                                0x86AA
#define GL_WEIGHT_ARRAY_SIZE_ARB                                  0x86AB
#define GL_WEIGHT_ARRAY_SIZE_OES                                  0x86AB
#define GL_WEIGHT_ARRAY_POINTER_ARB                               0x86AC
#define GL_WEIGHT_ARRAY_POINTER_OES                               0x86AC
#define GL_WEIGHT_ARRAY_ARB                                       0x86AD
#define GL_WEIGHT_ARRAY_OES                                       0x86AD
#define GL_DOT3_RGB                                               0x86AE
#define GL_DOT3_RGB_ARB                                           0x86AE
#define GL_DOT3_RGBA                                              0x86AF
#define GL_DOT3_RGBA_ARB                                          0x86AF
#define GL_DOT3_RGBA_IMG                                          0x86AF
#define GL_COMPRESSED_RGB_FXT1_3DFX                               0x86B0
#define GL_COMPRESSED_RGBA_FXT1_3DFX                              0x86B1
#define GL_MULTISAMPLE_3DFX                                       0x86B2
#define GL_SAMPLE_BUFFERS_3DFX                                    0x86B3
#define GL_SAMPLES_3DFX                                           0x86B4
#define GL_EVAL_2D_NV                                             0x86C0
#define GL_EVAL_TRIANGULAR_2D_NV                                  0x86C1
#define GL_MAP_TESSELLATION_NV                                    0x86C2
#define GL_MAP_ATTRIB_U_ORDER_NV                                  0x86C3
#define GL_MAP_ATTRIB_V_ORDER_NV                                  0x86C4
#define GL_EVAL_FRACTIONAL_TESSELLATION_NV                        0x86C5
#define GL_EVAL_VERTEX_ATTRIB0_NV                                 0x86C6
#define GL_EVAL_VERTEX_ATTRIB1_NV                                 0x86C7
#define GL_EVAL_VERTEX_ATTRIB2_NV                                 0x86C8
#define GL_EVAL_VERTEX_ATTRIB3_NV                                 0x86C9
#define GL_EVAL_VERTEX_ATTRIB4_NV                                 0x86CA
#define GL_EVAL_VERTEX_ATTRIB5_NV                                 0x86CB
#define GL_EVAL_VERTEX_ATTRIB6_NV                                 0x86CC
#define GL_EVAL_VERTEX_ATTRIB7_NV                                 0x86CD
#define GL_EVAL_VERTEX_ATTRIB8_NV                                 0x86CE
#define GL_EVAL_VERTEX_ATTRIB9_NV                                 0x86CF
#define GL_EVAL_VERTEX_ATTRIB10_NV                                0x86D0
#define GL_EVAL_VERTEX_ATTRIB11_NV                                0x86D1
#define GL_EVAL_VERTEX_ATTRIB12_NV                                0x86D2
#define GL_EVAL_VERTEX_ATTRIB13_NV                                0x86D3
#define GL_EVAL_VERTEX_ATTRIB14_NV                                0x86D4
#define GL_EVAL_VERTEX_ATTRIB15_NV                                0x86D5
#define GL_MAX_MAP_TESSELLATION_NV                                0x86D6
#define GL_MAX_RATIONAL_EVAL_ORDER_NV                             0x86D7
#define GL_MAX_PROGRAM_PATCH_ATTRIBS_NV                           0x86D8
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV                   0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV                              0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV                          0x86DB
#define GL_DSDT_MAG_INTENSITY_NV                                  0x86DC
#define GL_SHADER_CONSISTENT_NV                                   0x86DD
#define GL_TEXTURE_SHADER_NV                                      0x86DE
#define GL_SHADER_OPERATION_NV                                    0x86DF
#define GL_CULL_MODES_NV                                          0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV                               0x86E1
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV                            0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV                                0x86E2
#define GL_OFFSET_TEXTURE_2D_SCALE_NV                             0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV                                 0x86E3
#define GL_OFFSET_TEXTURE_2D_BIAS_NV                              0x86E3
#define GL_PREVIOUS_TEXTURE_INPUT_NV                              0x86E4
#define GL_CONST_EYE_NV                                           0x86E5
#define GL_PASS_THROUGH_NV                                        0x86E6
#define GL_CULL_FRAGMENT_NV                                       0x86E7
#define GL_OFFSET_TEXTURE_2D_NV                                   0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV                             0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV                             0x86EA
#define GL_SURFACE_STATE_NV                                       0x86EB
#define GL_DOT_PRODUCT_NV                                         0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV                           0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV                              0x86EE
#define GL_DOT_PRODUCT_TEXTURE_3D_NV                              0x86EF
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV                        0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV                        0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV                        0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV              0x86F3
#define GL_HILO_NV                                                0x86F4
#define GL_DSDT_NV                                                0x86F5
#define GL_DSDT_MAG_NV                                            0x86F6
#define GL_DSDT_MAG_VIB_NV                                        0x86F7
#define GL_HILO16_NV                                              0x86F8
#define GL_SIGNED_HILO_NV                                         0x86F9
#define GL_SIGNED_HILO16_NV                                       0x86FA
#define GL_SIGNED_RGBA_NV                                         0x86FB
#define GL_SIGNED_RGBA8_NV                                        0x86FC
#define GL_SURFACE_REGISTERED_NV                                  0x86FD
#define GL_SIGNED_RGB_NV                                          0x86FE
#define GL_SIGNED_RGB8_NV                                         0x86FF
#define GL_SURFACE_MAPPED_NV                                      0x8700
#define GL_SIGNED_LUMINANCE_NV                                    0x8701
#define GL_SIGNED_LUMINANCE8_NV                                   0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV                              0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV                            0x8704
#define GL_SIGNED_ALPHA_NV                                        0x8705
#define GL_SIGNED_ALPHA8_NV                                       0x8706
#define GL_SIGNED_INTENSITY_NV                                    0x8707
#define GL_SIGNED_INTENSITY8_NV                                   0x8708
#define GL_DSDT8_NV                                               0x8709
#define GL_DSDT8_MAG8_NV                                          0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV                               0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV                           0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV                         0x870D
#define GL_HI_SCALE_NV                                            0x870E
#define GL_LO_SCALE_NV                                            0x870F
#define GL_DS_SCALE_NV                                            0x8710
#define GL_DT_SCALE_NV                                            0x8711
#define GL_MAGNITUDE_SCALE_NV                                     0x8712
#define GL_VIBRANCE_SCALE_NV                                      0x8713
#define GL_HI_BIAS_NV                                             0x8714
#define GL_LO_BIAS_NV                                             0x8715
#define GL_DS_BIAS_NV                                             0x8716
#define GL_DT_BIAS_NV                                             0x8717
#define GL_MAGNITUDE_BIAS_NV                                      0x8718
#define GL_VIBRANCE_BIAS_NV                                       0x8719
#define GL_TEXTURE_BORDER_VALUES_NV                               0x871A
#define GL_TEXTURE_HI_SIZE_NV                                     0x871B
#define GL_TEXTURE_LO_SIZE_NV                                     0x871C
#define GL_TEXTURE_DS_SIZE_NV                                     0x871D
#define GL_TEXTURE_DT_SIZE_NV                                     0x871E
#define GL_TEXTURE_MAG_SIZE_NV                                    0x871F
#define GL_MODELVIEW2_ARB                                         0x8722
#define GL_MODELVIEW3_ARB                                         0x8723
#define GL_MODELVIEW4_ARB                                         0x8724
#define GL_MODELVIEW5_ARB                                         0x8725
#define GL_MODELVIEW6_ARB                                         0x8726
#define GL_MODELVIEW7_ARB                                         0x8727
#define GL_MODELVIEW8_ARB                                         0x8728
#define GL_MODELVIEW9_ARB                                         0x8729
#define GL_MODELVIEW10_ARB                                        0x872A
#define GL_MODELVIEW11_ARB                                        0x872B
#define GL_MODELVIEW12_ARB                                        0x872C
#define GL_MODELVIEW13_ARB                                        0x872D
#define GL_MODELVIEW14_ARB                                        0x872E
#define GL_MODELVIEW15_ARB                                        0x872F
#define GL_MODELVIEW16_ARB                                        0x8730
#define GL_MODELVIEW17_ARB                                        0x8731
#define GL_MODELVIEW18_ARB                                        0x8732
#define GL_MODELVIEW19_ARB                                        0x8733
#define GL_MODELVIEW20_ARB                                        0x8734
#define GL_MODELVIEW21_ARB                                        0x8735
#define GL_MODELVIEW22_ARB                                        0x8736
#define GL_MODELVIEW23_ARB                                        0x8737
#define GL_MODELVIEW24_ARB                                        0x8738
#define GL_MODELVIEW25_ARB                                        0x8739
#define GL_MODELVIEW26_ARB                                        0x873A
#define GL_MODELVIEW27_ARB                                        0x873B
#define GL_MODELVIEW28_ARB                                        0x873C
#define GL_MODELVIEW29_ARB                                        0x873D
#define GL_MODELVIEW30_ARB                                        0x873E
#define GL_MODELVIEW31_ARB                                        0x873F
#define GL_DOT3_RGB_EXT                                           0x8740
#define GL_Z400_BINARY_AMD                                        0x8740
#define GL_DOT3_RGBA_EXT                                          0x8741
#define GL_PROGRAM_BINARY_LENGTH_OES                              0x8741
#define GL_PROGRAM_BINARY_LENGTH                                  0x8741
#define GL_MIRROR_CLAMP_ATI                                       0x8742
#define GL_MIRROR_CLAMP_EXT                                       0x8742
#define GL_MIRROR_CLAMP_TO_EDGE                                   0x8743
#define GL_MIRROR_CLAMP_TO_EDGE_ATI                               0x8743
#define GL_MIRROR_CLAMP_TO_EDGE_EXT                               0x8743
#define GL_MODULATE_ADD_ATI                                       0x8744
#define GL_MODULATE_SIGNED_ADD_ATI                                0x8745
#define GL_MODULATE_SUBTRACT_ATI                                  0x8746
#define GL_SET_AMD                                                0x874A
#define GL_REPLACE_VALUE_AMD                                      0x874B
#define GL_STENCIL_OP_VALUE_AMD                                   0x874C
#define GL_STENCIL_BACK_OP_VALUE_AMD                              0x874D
#define GL_VERTEX_ATTRIB_ARRAY_LONG                               0x874E
#define GL_OCCLUSION_QUERY_EVENT_MASK_AMD                         0x874F
#define GL_DEPTH_STENCIL_MESA                                     0x8750
#define GL_UNSIGNED_INT_24_8_MESA                                 0x8751
#define GL_UNSIGNED_INT_8_24_REV_MESA                             0x8752
#define GL_UNSIGNED_SHORT_15_1_MESA                               0x8753
#define GL_UNSIGNED_SHORT_1_15_REV_MESA                           0x8754
#define GL_TRACE_MASK_MESA                                        0x8755
#define GL_TRACE_NAME_MESA                                        0x8756
#define GL_YCBCR_MESA                                             0x8757
#define GL_PACK_INVERT_MESA                                       0x8758
#define GL_DEBUG_OBJECT_MESA                                      0x8759
#define GL_TEXTURE_1D_STACK_MESAX                                 0x8759
#define GL_DEBUG_PRINT_MESA                                       0x875A
#define GL_TEXTURE_2D_STACK_MESAX                                 0x875A
#define GL_DEBUG_ASSERT_MESA                                      0x875B
#define GL_PROXY_TEXTURE_1D_STACK_MESAX                           0x875B
#define GL_PROXY_TEXTURE_2D_STACK_MESAX                           0x875C
#define GL_TEXTURE_1D_STACK_BINDING_MESAX                         0x875D
#define GL_TEXTURE_2D_STACK_BINDING_MESAX                         0x875E
#define GL_STATIC_ATI                                             0x8760
#define GL_DYNAMIC_ATI                                            0x8761
#define GL_PRESERVE_ATI                                           0x8762
#define GL_DISCARD_ATI                                            0x8763
#define GL_BUFFER_SIZE                                            0x8764
#define GL_BUFFER_SIZE_ARB                                        0x8764
#define GL_OBJECT_BUFFER_SIZE_ATI                                 0x8764
#define GL_BUFFER_USAGE                                           0x8765
#define GL_BUFFER_USAGE_ARB                                       0x8765
#define GL_OBJECT_BUFFER_USAGE_ATI                                0x8765
#define GL_ARRAY_OBJECT_BUFFER_ATI                                0x8766
#define GL_ARRAY_OBJECT_OFFSET_ATI                                0x8767
#define GL_ELEMENT_ARRAY_ATI                                      0x8768
#define GL_ELEMENT_ARRAY_TYPE_ATI                                 0x8769
#define GL_ELEMENT_ARRAY_POINTER_ATI                              0x876A
#define GL_MAX_VERTEX_STREAMS_ATI                                 0x876B
#define GL_VERTEX_STREAM0_ATI                                     0x876C
#define GL_VERTEX_STREAM1_ATI                                     0x876D
#define GL_VERTEX_STREAM2_ATI                                     0x876E
#define GL_VERTEX_STREAM3_ATI                                     0x876F
#define GL_VERTEX_STREAM4_ATI                                     0x8770
#define GL_VERTEX_STREAM5_ATI                                     0x8771
#define GL_VERTEX_STREAM6_ATI                                     0x8772
#define GL_VERTEX_STREAM7_ATI                                     0x8773
#define GL_VERTEX_SOURCE_ATI                                      0x8774
#define GL_BUMP_ROT_MATRIX_ATI                                    0x8775
#define GL_BUMP_ROT_MATRIX_SIZE_ATI                               0x8776
#define GL_BUMP_NUM_TEX_UNITS_ATI                                 0x8777
#define GL_BUMP_TEX_UNITS_ATI                                     0x8778
#define GL_DUDV_ATI                                               0x8779
#define GL_DU8DV8_ATI                                             0x877A
#define GL_BUMP_ENVMAP_ATI                                        0x877B
#define GL_BUMP_TARGET_ATI                                        0x877C
#define GL_VERTEX_SHADER_EXT                                      0x8780
#define GL_VERTEX_SHADER_BINDING_EXT                              0x8781
#define GL_OP_INDEX_EXT                                           0x8782
#define GL_OP_NEGATE_EXT                                          0x8783
#define GL_OP_DOT3_EXT                                            0x8784
#define GL_OP_DOT4_EXT                                            0x8785
#define GL_OP_MUL_EXT                                             0x8786
#define GL_OP_ADD_EXT                                             0x8787
#define GL_OP_MADD_EXT                                            0x8788
#define GL_OP_FRAC_EXT                                            0x8789
#define GL_OP_MAX_EXT                                             0x878A
#define GL_OP_MIN_EXT                                             0x878B
#define GL_OP_SET_GE_EXT                                          0x878C
#define GL_OP_SET_LT_EXT                                          0x878D
#define GL_OP_CLAMP_EXT                                           0x878E
#define GL_OP_FLOOR_EXT                                           0x878F
#define GL_OP_ROUND_EXT                                           0x8790
#define GL_OP_EXP_BASE_2_EXT                                      0x8791
#define GL_OP_LOG_BASE_2_EXT                                      0x8792
#define GL_OP_POWER_EXT                                           0x8793
#define GL_OP_RECIP_EXT                                           0x8794
#define GL_OP_RECIP_SQRT_EXT                                      0x8795
#define GL_OP_SUB_EXT                                             0x8796
#define GL_OP_CROSS_PRODUCT_EXT                                   0x8797
#define GL_OP_MULTIPLY_MATRIX_EXT                                 0x8798
#define GL_OP_MOV_EXT                                             0x8799
#define GL_OUTPUT_VERTEX_EXT                                      0x879A
#define GL_OUTPUT_COLOR0_EXT                                      0x879B
#define GL_OUTPUT_COLOR1_EXT                                      0x879C
#define GL_OUTPUT_TEXTURE_COORD0_EXT                              0x879D
#define GL_OUTPUT_TEXTURE_COORD1_EXT                              0x879E
#define GL_OUTPUT_TEXTURE_COORD2_EXT                              0x879F
#define GL_OUTPUT_TEXTURE_COORD3_EXT                              0x87A0
#define GL_OUTPUT_TEXTURE_COORD4_EXT                              0x87A1
#define GL_OUTPUT_TEXTURE_COORD5_EXT                              0x87A2
#define GL_OUTPUT_TEXTURE_COORD6_EXT                              0x87A3
#define GL_OUTPUT_TEXTURE_COORD7_EXT                              0x87A4
#define GL_OUTPUT_TEXTURE_COORD8_EXT                              0x87A5
#define GL_OUTPUT_TEXTURE_COORD9_EXT                              0x87A6
#define GL_OUTPUT_TEXTURE_COORD10_EXT                             0x87A7
#define GL_OUTPUT_TEXTURE_COORD11_EXT                             0x87A8
#define GL_OUTPUT_TEXTURE_COORD12_EXT                             0x87A9
#define GL_OUTPUT_TEXTURE_COORD13_EXT                             0x87AA
#define GL_OUTPUT_TEXTURE_COORD14_EXT                             0x87AB
#define GL_OUTPUT_TEXTURE_COORD15_EXT                             0x87AC
#define GL_OUTPUT_TEXTURE_COORD16_EXT                             0x87AD
#define GL_OUTPUT_TEXTURE_COORD17_EXT                             0x87AE
#define GL_OUTPUT_TEXTURE_COORD18_EXT                             0x87AF
#define GL_OUTPUT_TEXTURE_COORD19_EXT                             0x87B0
#define GL_OUTPUT_TEXTURE_COORD20_EXT                             0x87B1
#define GL_OUTPUT_TEXTURE_COORD21_EXT                             0x87B2
#define GL_OUTPUT_TEXTURE_COORD22_EXT                             0x87B3
#define GL_OUTPUT_TEXTURE_COORD23_EXT                             0x87B4
#define GL_OUTPUT_TEXTURE_COORD24_EXT                             0x87B5
#define GL_OUTPUT_TEXTURE_COORD25_EXT                             0x87B6
#define GL_OUTPUT_TEXTURE_COORD26_EXT                             0x87B7
#define GL_OUTPUT_TEXTURE_COORD27_EXT                             0x87B8
#define GL_OUTPUT_TEXTURE_COORD28_EXT                             0x87B9
#define GL_OUTPUT_TEXTURE_COORD29_EXT                             0x87BA
#define GL_OUTPUT_TEXTURE_COORD30_EXT                             0x87BB
#define GL_OUTPUT_TEXTURE_COORD31_EXT                             0x87BC
#define GL_OUTPUT_FOG_EXT                                         0x87BD
#define GL_SCALAR_EXT                                             0x87BE
#define GL_VECTOR_EXT                                             0x87BF
#define GL_MATRIX_EXT                                             0x87C0
#define GL_VARIANT_EXT                                            0x87C1
#define GL_INVARIANT_EXT                                          0x87C2
#define GL_LOCAL_CONSTANT_EXT                                     0x87C3
#define GL_LOCAL_EXT                                              0x87C4
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT                     0x87C5
#define GL_MAX_VERTEX_SHADER_VARIANTS_EXT                         0x87C6
#define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                       0x87C7
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                  0x87C8
#define GL_MAX_VERTEX_SHADER_LOCALS_EXT                           0x87C9
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT           0x87CA
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT               0x87CB
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT        0x87CC
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT             0x87CD
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT                 0x87CE
#define GL_VERTEX_SHADER_INSTRUCTIONS_EXT                         0x87CF
#define GL_VERTEX_SHADER_VARIANTS_EXT                             0x87D0
#define GL_VERTEX_SHADER_INVARIANTS_EXT                           0x87D1
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                      0x87D2
#define GL_VERTEX_SHADER_LOCALS_EXT                               0x87D3
#define GL_VERTEX_SHADER_OPTIMIZED_EXT                            0x87D4
#define GL_X_EXT                                                  0x87D5
#define GL_Y_EXT                                                  0x87D6
#define GL_Z_EXT                                                  0x87D7
#define GL_W_EXT                                                  0x87D8
#define GL_NEGATIVE_X_EXT                                         0x87D9
#define GL_NEGATIVE_Y_EXT                                         0x87DA
#define GL_NEGATIVE_Z_EXT                                         0x87DB
#define GL_NEGATIVE_W_EXT                                         0x87DC
#define GL_ZERO_EXT                                               0x87DD
#define GL_ONE_EXT                                                0x87DE
#define GL_NEGATIVE_ONE_EXT                                       0x87DF
#define GL_NORMALIZED_RANGE_EXT                                   0x87E0
#define GL_FULL_RANGE_EXT                                         0x87E1
#define GL_CURRENT_VERTEX_EXT                                     0x87E2
#define GL_MVP_MATRIX_EXT                                         0x87E3
#define GL_VARIANT_VALUE_EXT                                      0x87E4
#define GL_VARIANT_DATATYPE_EXT                                   0x87E5
#define GL_VARIANT_ARRAY_STRIDE_EXT                               0x87E6
#define GL_VARIANT_ARRAY_TYPE_EXT                                 0x87E7
#define GL_VARIANT_ARRAY_EXT                                      0x87E8
#define GL_VARIANT_ARRAY_POINTER_EXT                              0x87E9
#define GL_INVARIANT_VALUE_EXT                                    0x87EA
#define GL_INVARIANT_DATATYPE_EXT                                 0x87EB
#define GL_LOCAL_CONSTANT_VALUE_EXT                               0x87EC
#define GL_LOCAL_CONSTANT_DATATYPE_EXT                            0x87ED
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                        0x87EE
#define GL_PN_TRIANGLES_ATI                                       0x87F0
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI                 0x87F1
#define GL_PN_TRIANGLES_POINT_MODE_ATI                            0x87F2
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI                           0x87F3
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI                     0x87F4
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI                     0x87F5
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI                      0x87F6
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI                    0x87F7
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI                 0x87F8
#define GL_3DC_X_AMD                                              0x87F9
#define GL_3DC_XY_AMD                                             0x87FA
#define GL_VBO_FREE_MEMORY_ATI                                    0x87FB
#define GL_TEXTURE_FREE_MEMORY_ATI                                0x87FC
#define GL_RENDERBUFFER_FREE_MEMORY_ATI                           0x87FD
#define GL_NUM_PROGRAM_BINARY_FORMATS                             0x87FE
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES                         0x87FE
#define GL_PROGRAM_BINARY_FORMATS                                 0x87FF
#define GL_PROGRAM_BINARY_FORMATS_OES                             0x87FF
#define GL_STENCIL_BACK_FUNC                                      0x8800
#define GL_STENCIL_BACK_FUNC_ATI                                  0x8800
#define GL_STENCIL_BACK_FAIL                                      0x8801
#define GL_STENCIL_BACK_FAIL_ATI                                  0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                           0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI                       0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                           0x8803
#define GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI                       0x8803
#define GL_FRAGMENT_PROGRAM_ARB                                   0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB                           0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB                           0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB                           0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                    0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                    0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                    0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB                       0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB                       0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB                       0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                0x8810
#define GL_RGBA32F                                                0x8814
#define GL_RGBA32F_ARB                                            0x8814
#define GL_RGBA32F_EXT                                            0x8814
#define GL_RGBA_FLOAT32_APPLE                                     0x8814
#define GL_RGBA_FLOAT32_ATI                                       0x8814
#define GL_RGB32F                                                 0x8815
#define GL_RGB32F_ARB                                             0x8815
#define GL_RGB32F_EXT                                             0x8815
#define GL_RGB_FLOAT32_APPLE                                      0x8815
#define GL_RGB_FLOAT32_ATI                                        0x8815
#define GL_ALPHA32F_ARB                                           0x8816
#define GL_ALPHA32F_EXT                                           0x8816
#define GL_ALPHA_FLOAT32_APPLE                                    0x8816
#define GL_ALPHA_FLOAT32_ATI                                      0x8816
#define GL_INTENSITY32F_ARB                                       0x8817
#define GL_INTENSITY_FLOAT32_APPLE                                0x8817
#define GL_INTENSITY_FLOAT32_ATI                                  0x8817
#define GL_LUMINANCE32F_ARB                                       0x8818
#define GL_LUMINANCE32F_EXT                                       0x8818
#define GL_LUMINANCE_FLOAT32_APPLE                                0x8818
#define GL_LUMINANCE_FLOAT32_ATI                                  0x8818
#define GL_LUMINANCE_ALPHA32F_ARB                                 0x8819
#define GL_LUMINANCE_ALPHA32F_EXT                                 0x8819
#define GL_LUMINANCE_ALPHA_FLOAT32_APPLE                          0x8819
#define GL_LUMINANCE_ALPHA_FLOAT32_ATI                            0x8819
#define GL_RGBA16F                                                0x881A
#define GL_RGBA16F_ARB                                            0x881A
#define GL_RGBA16F_EXT                                            0x881A
#define GL_RGBA_FLOAT16_APPLE                                     0x881A
#define GL_RGBA_FLOAT16_ATI                                       0x881A
#define GL_RGB16F                                                 0x881B
#define GL_RGB16F_ARB                                             0x881B
#define GL_RGB16F_EXT                                             0x881B
#define GL_RGB_FLOAT16_APPLE                                      0x881B
#define GL_RGB_FLOAT16_ATI                                        0x881B
#define GL_ALPHA16F_ARB                                           0x881C
#define GL_ALPHA16F_EXT                                           0x881C
#define GL_ALPHA_FLOAT16_APPLE                                    0x881C
#define GL_ALPHA_FLOAT16_ATI                                      0x881C
#define GL_INTENSITY16F_ARB                                       0x881D
#define GL_INTENSITY_FLOAT16_APPLE                                0x881D
#define GL_INTENSITY_FLOAT16_ATI                                  0x881D
#define GL_LUMINANCE16F_ARB                                       0x881E
#define GL_LUMINANCE16F_EXT                                       0x881E
#define GL_LUMINANCE_FLOAT16_APPLE                                0x881E
#define GL_LUMINANCE_FLOAT16_ATI                                  0x881E
#define GL_LUMINANCE_ALPHA16F_ARB                                 0x881F
#define GL_LUMINANCE_ALPHA16F_EXT                                 0x881F
#define GL_LUMINANCE_ALPHA_FLOAT16_APPLE                          0x881F
#define GL_LUMINANCE_ALPHA_FLOAT16_ATI                            0x881F
#define GL_RGBA_FLOAT_MODE_ARB                                    0x8820
#define GL_RGBA_FLOAT_MODE_ATI                                    0x8820
#define GL_WRITEONLY_RENDERING_QCOM                               0x8823
#define GL_MAX_DRAW_BUFFERS                                       0x8824
#define GL_MAX_DRAW_BUFFERS_ARB                                   0x8824
#define GL_MAX_DRAW_BUFFERS_ATI                                   0x8824
#define GL_MAX_DRAW_BUFFERS_EXT                                   0x8824
#define GL_MAX_DRAW_BUFFERS_NV                                    0x8824
#define GL_DRAW_BUFFER0                                           0x8825
#define GL_DRAW_BUFFER0_ARB                                       0x8825
#define GL_DRAW_BUFFER0_ATI                                       0x8825
#define GL_DRAW_BUFFER0_EXT                                       0x8825
#define GL_DRAW_BUFFER0_NV                                        0x8825
#define GL_DRAW_BUFFER1                                           0x8826
#define GL_DRAW_BUFFER1_ARB                                       0x8826
#define GL_DRAW_BUFFER1_ATI                                       0x8826
#define GL_DRAW_BUFFER1_EXT                                       0x8826
#define GL_DRAW_BUFFER1_NV                                        0x8826
#define GL_DRAW_BUFFER2                                           0x8827
#define GL_DRAW_BUFFER2_ARB                                       0x8827
#define GL_DRAW_BUFFER2_ATI                                       0x8827
#define GL_DRAW_BUFFER2_EXT                                       0x8827
#define GL_DRAW_BUFFER2_NV                                        0x8827
#define GL_DRAW_BUFFER3                                           0x8828
#define GL_DRAW_BUFFER3_ARB                                       0x8828
#define GL_DRAW_BUFFER3_ATI                                       0x8828
#define GL_DRAW_BUFFER3_EXT                                       0x8828
#define GL_DRAW_BUFFER3_NV                                        0x8828
#define GL_DRAW_BUFFER4                                           0x8829
#define GL_DRAW_BUFFER4_ARB                                       0x8829
#define GL_DRAW_BUFFER4_ATI                                       0x8829
#define GL_DRAW_BUFFER4_EXT                                       0x8829
#define GL_DRAW_BUFFER4_NV                                        0x8829
#define GL_DRAW_BUFFER5                                           0x882A
#define GL_DRAW_BUFFER5_ARB                                       0x882A
#define GL_DRAW_BUFFER5_ATI                                       0x882A
#define GL_DRAW_BUFFER5_EXT                                       0x882A
#define GL_DRAW_BUFFER5_NV                                        0x882A
#define GL_DRAW_BUFFER6                                           0x882B
#define GL_DRAW_BUFFER6_ARB                                       0x882B
#define GL_DRAW_BUFFER6_ATI                                       0x882B
#define GL_DRAW_BUFFER6_EXT                                       0x882B
#define GL_DRAW_BUFFER6_NV                                        0x882B
#define GL_DRAW_BUFFER7                                           0x882C
#define GL_DRAW_BUFFER7_ARB                                       0x882C
#define GL_DRAW_BUFFER7_ATI                                       0x882C
#define GL_DRAW_BUFFER7_EXT                                       0x882C
#define GL_DRAW_BUFFER7_NV                                        0x882C
#define GL_DRAW_BUFFER8                                           0x882D
#define GL_DRAW_BUFFER8_ARB                                       0x882D
#define GL_DRAW_BUFFER8_ATI                                       0x882D
#define GL_DRAW_BUFFER8_EXT                                       0x882D
#define GL_DRAW_BUFFER8_NV                                        0x882D
#define GL_DRAW_BUFFER9                                           0x882E
#define GL_DRAW_BUFFER9_ARB                                       0x882E
#define GL_DRAW_BUFFER9_ATI                                       0x882E
#define GL_DRAW_BUFFER9_EXT                                       0x882E
#define GL_DRAW_BUFFER9_NV                                        0x882E
#define GL_DRAW_BUFFER10                                          0x882F
#define GL_DRAW_BUFFER10_ARB                                      0x882F
#define GL_DRAW_BUFFER10_ATI                                      0x882F
#define GL_DRAW_BUFFER10_EXT                                      0x882F
#define GL_DRAW_BUFFER10_NV                                       0x882F
#define GL_DRAW_BUFFER11                                          0x8830
#define GL_DRAW_BUFFER11_ARB                                      0x8830
#define GL_DRAW_BUFFER11_ATI                                      0x8830
#define GL_DRAW_BUFFER11_EXT                                      0x8830
#define GL_DRAW_BUFFER11_NV                                       0x8830
#define GL_DRAW_BUFFER12                                          0x8831
#define GL_DRAW_BUFFER12_ARB                                      0x8831
#define GL_DRAW_BUFFER12_ATI                                      0x8831
#define GL_DRAW_BUFFER12_EXT                                      0x8831
#define GL_DRAW_BUFFER12_NV                                       0x8831
#define GL_DRAW_BUFFER13                                          0x8832
#define GL_DRAW_BUFFER13_ARB                                      0x8832
#define GL_DRAW_BUFFER13_ATI                                      0x8832
#define GL_DRAW_BUFFER13_EXT                                      0x8832
#define GL_DRAW_BUFFER13_NV                                       0x8832
#define GL_DRAW_BUFFER14                                          0x8833
#define GL_DRAW_BUFFER14_ARB                                      0x8833
#define GL_DRAW_BUFFER14_ATI                                      0x8833
#define GL_DRAW_BUFFER14_EXT                                      0x8833
#define GL_DRAW_BUFFER14_NV                                       0x8833
#define GL_DRAW_BUFFER15                                          0x8834
#define GL_DRAW_BUFFER15_ARB                                      0x8834
#define GL_DRAW_BUFFER15_ATI                                      0x8834
#define GL_DRAW_BUFFER15_EXT                                      0x8834
#define GL_DRAW_BUFFER15_NV                                       0x8834
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI                        0x8835
#define GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI                     0x8837
#define GL_BLEND_EQUATION_ALPHA                                   0x883D
#define GL_BLEND_EQUATION_ALPHA_EXT                               0x883D
#define GL_BLEND_EQUATION_ALPHA_OES                               0x883D
#define GL_SUBSAMPLE_DISTANCE_AMD                                 0x883F
#define GL_MATRIX_PALETTE_ARB                                     0x8840
#define GL_MATRIX_PALETTE_OES                                     0x8840
#define GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB                     0x8841
#define GL_MAX_PALETTE_MATRICES_ARB                               0x8842
#define GL_MAX_PALETTE_MATRICES_OES                               0x8842
#define GL_CURRENT_PALETTE_MATRIX_ARB                             0x8843
#define GL_CURRENT_PALETTE_MATRIX_OES                             0x8843
#define GL_MATRIX_INDEX_ARRAY_ARB                                 0x8844
#define GL_MATRIX_INDEX_ARRAY_OES                                 0x8844
#define GL_CURRENT_MATRIX_INDEX_ARB                               0x8845
#define GL_MATRIX_INDEX_ARRAY_SIZE_ARB                            0x8846
#define GL_MATRIX_INDEX_ARRAY_SIZE_OES                            0x8846
#define GL_MATRIX_INDEX_ARRAY_TYPE_ARB                            0x8847
#define GL_MATRIX_INDEX_ARRAY_TYPE_OES                            0x8847
#define GL_MATRIX_INDEX_ARRAY_STRIDE_ARB                          0x8848
#define GL_MATRIX_INDEX_ARRAY_STRIDE_OES                          0x8848
#define GL_MATRIX_INDEX_ARRAY_POINTER_ARB                         0x8849
#define GL_MATRIX_INDEX_ARRAY_POINTER_OES                         0x8849
#define GL_TEXTURE_DEPTH_SIZE                                     0x884A
#define GL_TEXTURE_DEPTH_SIZE_ARB                                 0x884A
#define GL_DEPTH_TEXTURE_MODE                                     0x884B
#define GL_DEPTH_TEXTURE_MODE_ARB                                 0x884B
#define GL_TEXTURE_COMPARE_MODE                                   0x884C
#define GL_TEXTURE_COMPARE_MODE_ARB                               0x884C
#define GL_TEXTURE_COMPARE_MODE_EXT                               0x884C
#define GL_TEXTURE_COMPARE_FUNC                                   0x884D
#define GL_TEXTURE_COMPARE_FUNC_ARB                               0x884D
#define GL_TEXTURE_COMPARE_FUNC_EXT                               0x884D
#define GL_COMPARE_R_TO_TEXTURE                                   0x884E
#define GL_COMPARE_R_TO_TEXTURE_ARB                               0x884E
#define GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT                       0x884E
#define GL_COMPARE_REF_TO_TEXTURE                                 0x884E
#define GL_COMPARE_REF_TO_TEXTURE_EXT                             0x884E
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                              0x884F
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV                        0x8850
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV                  0x8851
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV                 0x8852
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV           0x8853
#define GL_OFFSET_HILO_TEXTURE_2D_NV                              0x8854
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV                       0x8855
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV                   0x8856
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV            0x8857
#define GL_DEPENDENT_HILO_TEXTURE_2D_NV                           0x8858
#define GL_DEPENDENT_RGB_TEXTURE_3D_NV                            0x8859
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV                      0x885A
#define GL_DOT_PRODUCT_PASS_THROUGH_NV                            0x885B
#define GL_DOT_PRODUCT_TEXTURE_1D_NV                              0x885C
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV                    0x885D
#define GL_HILO8_NV                                               0x885E
#define GL_SIGNED_HILO8_NV                                        0x885F
#define GL_FORCE_BLUE_TO_ONE_NV                                   0x8860
#define GL_POINT_SPRITE                                           0x8861
#define GL_POINT_SPRITE_ARB                                       0x8861
#define GL_POINT_SPRITE_NV                                        0x8861
#define GL_POINT_SPRITE_OES                                       0x8861
#define GL_COORD_REPLACE                                          0x8862
#define GL_COORD_REPLACE_ARB                                      0x8862
#define GL_COORD_REPLACE_NV                                       0x8862
#define GL_COORD_REPLACE_OES                                      0x8862
#define GL_POINT_SPRITE_R_MODE_NV                                 0x8863
#define GL_PIXEL_COUNTER_BITS_NV                                  0x8864
#define GL_QUERY_COUNTER_BITS                                     0x8864
#define GL_QUERY_COUNTER_BITS_ARB                                 0x8864
#define GL_QUERY_COUNTER_BITS_EXT                                 0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV                          0x8865
#define GL_CURRENT_QUERY                                          0x8865
#define GL_CURRENT_QUERY_ARB                                      0x8865
#define GL_CURRENT_QUERY_EXT                                      0x8865
#define GL_PIXEL_COUNT_NV                                         0x8866
#define GL_QUERY_RESULT                                           0x8866
#define GL_QUERY_RESULT_ARB                                       0x8866
#define GL_QUERY_RESULT_EXT                                       0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV                               0x8867
#define GL_QUERY_RESULT_AVAILABLE                                 0x8867
#define GL_QUERY_RESULT_AVAILABLE_ARB                             0x8867
#define GL_QUERY_RESULT_AVAILABLE_EXT                             0x8867
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV               0x8868
#define GL_MAX_VERTEX_ATTRIBS                                     0x8869
#define GL_MAX_VERTEX_ATTRIBS_ARB                                 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                         0x886A
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB                     0x886A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS                      0x886C
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT                  0x886C
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_OES                  0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS                   0x886D
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT               0x886D
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_OES               0x886D
#define GL_DEPTH_STENCIL_TO_RGBA_NV                               0x886E
#define GL_DEPTH_STENCIL_TO_BGRA_NV                               0x886F
#define GL_FRAGMENT_PROGRAM_NV                                    0x8870
#define GL_MAX_TEXTURE_COORDS                                     0x8871
#define GL_MAX_TEXTURE_COORDS_ARB                                 0x8871
#define GL_MAX_TEXTURE_COORDS_NV                                  0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS                                0x8872
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB                            0x8872
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV                             0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV                            0x8873
#define GL_PROGRAM_ERROR_STRING_ARB                               0x8874
#define GL_PROGRAM_ERROR_STRING_NV                                0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB                               0x8875
#define GL_PROGRAM_FORMAT_ARB                                     0x8876
#define GL_WRITE_PIXEL_DATA_RANGE_NV                              0x8878
#define GL_READ_PIXEL_DATA_RANGE_NV                               0x8879
#define GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV                       0x887A
#define GL_READ_PIXEL_DATA_RANGE_LENGTH_NV                        0x887B
#define GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV                      0x887C
#define GL_READ_PIXEL_DATA_RANGE_POINTER_NV                       0x887D
#define GL_GEOMETRY_SHADER_INVOCATIONS                            0x887F
#define GL_GEOMETRY_SHADER_INVOCATIONS_EXT                        0x887F
#define GL_GEOMETRY_SHADER_INVOCATIONS_OES                        0x887F
#define GL_FLOAT_R_NV                                             0x8880
#define GL_FLOAT_RG_NV                                            0x8881
#define GL_FLOAT_RGB_NV                                           0x8882
#define GL_FLOAT_RGBA_NV                                          0x8883
#define GL_FLOAT_R16_NV                                           0x8884
#define GL_FLOAT_R32_NV                                           0x8885
#define GL_FLOAT_RG16_NV                                          0x8886
#define GL_FLOAT_RG32_NV                                          0x8887
#define GL_FLOAT_RGB16_NV                                         0x8888
#define GL_FLOAT_RGB32_NV                                         0x8889
#define GL_FLOAT_RGBA16_NV                                        0x888A
#define GL_FLOAT_RGBA32_NV                                        0x888B
#define GL_TEXTURE_FLOAT_COMPONENTS_NV                            0x888C
#define GL_FLOAT_CLEAR_COLOR_VALUE_NV                             0x888D
#define GL_FLOAT_RGBA_MODE_NV                                     0x888E
#define GL_TEXTURE_UNSIGNED_REMAP_MODE_NV                         0x888F
#define GL_DEPTH_BOUNDS_TEST_EXT                                  0x8890
#define GL_DEPTH_BOUNDS_EXT                                       0x8891
#define GL_ARRAY_BUFFER                                           0x8892
#define GL_ARRAY_BUFFER_ARB                                       0x8892
#define GL_ELEMENT_ARRAY_BUFFER                                   0x8893
#define GL_ELEMENT_ARRAY_BUFFER_ARB                               0x8893
#define GL_ARRAY_BUFFER_BINDING                                   0x8894
#define GL_ARRAY_BUFFER_BINDING_ARB                               0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                           0x8895
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB                       0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING                            0x8896
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB                        0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING                            0x8897
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB                        0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING                             0x8898
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB                         0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING                             0x8899
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB                         0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING                     0x889A
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB                 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING                         0x889B
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB                     0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING                   0x889C
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB               0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB                0x889D
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING                    0x889D
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING                         0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING                            0x889E
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB                        0x889E
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_OES                        0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING                     0x889F
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB                 0x889F
#define GL_PROGRAM_INSTRUCTIONS_ARB                               0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                           0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                        0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB                    0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB                                0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB                            0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                         0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB                     0x88A7
#define GL_PROGRAM_PARAMETERS_ARB                                 0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB                             0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB                          0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                      0x88AB
#define GL_PROGRAM_ATTRIBS_ARB                                    0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB                                0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB                             0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                         0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB                          0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB                      0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                   0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB               0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                       0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                         0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                        0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                           0x88B7
#define GL_READ_ONLY                                              0x88B8
#define GL_READ_ONLY_ARB                                          0x88B8
#define GL_WRITE_ONLY                                             0x88B9
#define GL_WRITE_ONLY_ARB                                         0x88B9
#define GL_WRITE_ONLY_OES                                         0x88B9
#define GL_READ_WRITE                                             0x88BA
#define GL_READ_WRITE_ARB                                         0x88BA
#define GL_BUFFER_ACCESS                                          0x88BB
#define GL_BUFFER_ACCESS_ARB                                      0x88BB
#define GL_BUFFER_ACCESS_OES                                      0x88BB
#define GL_BUFFER_MAPPED                                          0x88BC
#define GL_BUFFER_MAPPED_ARB                                      0x88BC
#define GL_BUFFER_MAPPED_OES                                      0x88BC
#define GL_BUFFER_MAP_POINTER                                     0x88BD
#define GL_BUFFER_MAP_POINTER_ARB                                 0x88BD
#define GL_BUFFER_MAP_POINTER_OES                                 0x88BD
#define GL_WRITE_DISCARD_NV                                       0x88BE
#define GL_TIME_ELAPSED                                           0x88BF
#define GL_TIME_ELAPSED_EXT                                       0x88BF
#define GL_MATRIX0_ARB                                            0x88C0
#define GL_MATRIX1_ARB                                            0x88C1
#define GL_MATRIX2_ARB                                            0x88C2
#define GL_MATRIX3_ARB                                            0x88C3
#define GL_MATRIX4_ARB                                            0x88C4
#define GL_MATRIX5_ARB                                            0x88C5
#define GL_MATRIX6_ARB                                            0x88C6
#define GL_MATRIX7_ARB                                            0x88C7
#define GL_MATRIX8_ARB                                            0x88C8
#define GL_MATRIX9_ARB                                            0x88C9
#define GL_MATRIX10_ARB                                           0x88CA
#define GL_MATRIX11_ARB                                           0x88CB
#define GL_MATRIX12_ARB                                           0x88CC
#define GL_MATRIX13_ARB                                           0x88CD
#define GL_MATRIX14_ARB                                           0x88CE
#define GL_MATRIX15_ARB                                           0x88CF
#define GL_MATRIX16_ARB                                           0x88D0
#define GL_MATRIX17_ARB                                           0x88D1
#define GL_MATRIX18_ARB                                           0x88D2
#define GL_MATRIX19_ARB                                           0x88D3
#define GL_MATRIX20_ARB                                           0x88D4
#define GL_MATRIX21_ARB                                           0x88D5
#define GL_MATRIX22_ARB                                           0x88D6
#define GL_MATRIX23_ARB                                           0x88D7
#define GL_MATRIX24_ARB                                           0x88D8
#define GL_MATRIX25_ARB                                           0x88D9
#define GL_MATRIX26_ARB                                           0x88DA
#define GL_MATRIX27_ARB                                           0x88DB
#define GL_MATRIX28_ARB                                           0x88DC
#define GL_MATRIX29_ARB                                           0x88DD
#define GL_MATRIX30_ARB                                           0x88DE
#define GL_MATRIX31_ARB                                           0x88DF
#define GL_STREAM_DRAW                                            0x88E0
#define GL_STREAM_DRAW_ARB                                        0x88E0
#define GL_STREAM_READ                                            0x88E1
#define GL_STREAM_READ_ARB                                        0x88E1
#define GL_STREAM_COPY                                            0x88E2
#define GL_STREAM_COPY_ARB                                        0x88E2
#define GL_STATIC_DRAW                                            0x88E4
#define GL_STATIC_DRAW_ARB                                        0x88E4
#define GL_STATIC_READ                                            0x88E5
#define GL_STATIC_READ_ARB                                        0x88E5
#define GL_STATIC_COPY                                            0x88E6
#define GL_STATIC_COPY_ARB                                        0x88E6
#define GL_DYNAMIC_DRAW                                           0x88E8
#define GL_DYNAMIC_DRAW_ARB                                       0x88E8
#define GL_DYNAMIC_READ                                           0x88E9
#define GL_DYNAMIC_READ_ARB                                       0x88E9
#define GL_DYNAMIC_COPY                                           0x88EA
#define GL_DYNAMIC_COPY_ARB                                       0x88EA
#define GL_PIXEL_PACK_BUFFER                                      0x88EB
#define GL_PIXEL_PACK_BUFFER_ARB                                  0x88EB
#define GL_PIXEL_PACK_BUFFER_EXT                                  0x88EB
#define GL_PIXEL_UNPACK_BUFFER                                    0x88EC
#define GL_PIXEL_UNPACK_BUFFER_ARB                                0x88EC
#define GL_PIXEL_UNPACK_BUFFER_EXT                                0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING                              0x88ED
#define GL_PIXEL_PACK_BUFFER_BINDING_ARB                          0x88ED
#define GL_PIXEL_PACK_BUFFER_BINDING_EXT                          0x88ED
#define GL_ETC1_SRGB8_NV                                          0x88EE
#define GL_PIXEL_UNPACK_BUFFER_BINDING                            0x88EF
#define GL_PIXEL_UNPACK_BUFFER_BINDING_ARB                        0x88EF
#define GL_PIXEL_UNPACK_BUFFER_BINDING_EXT                        0x88EF
#define GL_DEPTH24_STENCIL8                                       0x88F0
#define GL_DEPTH24_STENCIL8_EXT                                   0x88F0
#define GL_DEPTH24_STENCIL8_OES                                   0x88F0
#define GL_TEXTURE_STENCIL_SIZE                                   0x88F1
#define GL_TEXTURE_STENCIL_SIZE_EXT                               0x88F1
#define GL_STENCIL_TAG_BITS_EXT                                   0x88F2
#define GL_STENCIL_CLEAR_TAG_VALUE_EXT                            0x88F3
#define GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV                       0x88F4
#define GL_MAX_PROGRAM_CALL_DEPTH_NV                              0x88F5
#define GL_MAX_PROGRAM_IF_DEPTH_NV                                0x88F6
#define GL_MAX_PROGRAM_LOOP_DEPTH_NV                              0x88F7
#define GL_MAX_PROGRAM_LOOP_COUNT_NV                              0x88F8
#define GL_SRC1_COLOR                                             0x88F9
#define GL_SRC1_COLOR_EXT                                         0x88F9
#define GL_ONE_MINUS_SRC1_COLOR                                   0x88FA
#define GL_ONE_MINUS_SRC1_COLOR_EXT                               0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA                                   0x88FB
#define GL_ONE_MINUS_SRC1_ALPHA_EXT                               0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS                           0x88FC
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT                       0x88FC
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                            0x88FD
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT                        0x88FD
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV                         0x88FD
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                            0x88FE
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE                      0x88FE
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB                        0x88FE
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT                        0x88FE
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_NV                         0x88FE
#define GL_MAX_ARRAY_TEXTURE_LAYERS                               0x88FF
#define GL_MAX_ARRAY_TEXTURE_LAYERS_EXT                           0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET                               0x8904
#define GL_MIN_PROGRAM_TEXEL_OFFSET_EXT                           0x8904
#define GL_MIN_PROGRAM_TEXEL_OFFSET_NV                            0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                               0x8905
#define GL_MAX_PROGRAM_TEXEL_OFFSET_EXT                           0x8905
#define GL_MAX_PROGRAM_TEXEL_OFFSET_NV                            0x8905
#define GL_PROGRAM_ATTRIB_COMPONENTS_NV                           0x8906
#define GL_PROGRAM_RESULT_COMPONENTS_NV                           0x8907
#define GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV                       0x8908
#define GL_MAX_PROGRAM_RESULT_COMPONENTS_NV                       0x8909
#define GL_STENCIL_TEST_TWO_SIDE_EXT                              0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT                                0x8911
#define GL_MIRROR_CLAMP_TO_BORDER_EXT                             0x8912
#define GL_SAMPLES_PASSED                                         0x8914
#define GL_SAMPLES_PASSED_ARB                                     0x8914
#define GL_GEOMETRY_VERTICES_OUT                                  0x8916
#define GL_GEOMETRY_LINKED_VERTICES_OUT_EXT                       0x8916
#define GL_GEOMETRY_LINKED_VERTICES_OUT_OES                       0x8916
#define GL_GEOMETRY_INPUT_TYPE                                    0x8917
#define GL_GEOMETRY_LINKED_INPUT_TYPE_EXT                         0x8917
#define GL_GEOMETRY_LINKED_INPUT_TYPE_OES                         0x8917
#define GL_GEOMETRY_OUTPUT_TYPE                                   0x8918
#define GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT                        0x8918
#define GL_GEOMETRY_LINKED_OUTPUT_TYPE_OES                        0x8918
#define GL_SAMPLER_BINDING                                        0x8919
#define GL_CLAMP_VERTEX_COLOR                                     0x891A
#define GL_CLAMP_VERTEX_COLOR_ARB                                 0x891A
#define GL_CLAMP_FRAGMENT_COLOR                                   0x891B
#define GL_CLAMP_FRAGMENT_COLOR_ARB                               0x891B
#define GL_CLAMP_READ_COLOR                                       0x891C
#define GL_CLAMP_READ_COLOR_ARB                                   0x891C
#define GL_FIXED_ONLY                                             0x891D
#define GL_FIXED_ONLY_ARB                                         0x891D
#define GL_TESS_CONTROL_PROGRAM_NV                                0x891E
#define GL_TESS_EVALUATION_PROGRAM_NV                             0x891F
#define GL_FRAGMENT_SHADER_ATI                                    0x8920
#define GL_REG_0_ATI                                              0x8921
#define GL_REG_1_ATI                                              0x8922
#define GL_REG_2_ATI                                              0x8923
#define GL_REG_3_ATI                                              0x8924
#define GL_REG_4_ATI                                              0x8925
#define GL_REG_5_ATI                                              0x8926
#define GL_REG_6_ATI                                              0x8927
#define GL_REG_7_ATI                                              0x8928
#define GL_REG_8_ATI                                              0x8929
#define GL_REG_9_ATI                                              0x892A
#define GL_REG_10_ATI                                             0x892B
#define GL_REG_11_ATI                                             0x892C
#define GL_REG_12_ATI                                             0x892D
#define GL_REG_13_ATI                                             0x892E
#define GL_REG_14_ATI                                             0x892F
#define GL_REG_15_ATI                                             0x8930
#define GL_REG_16_ATI                                             0x8931
#define GL_REG_17_ATI                                             0x8932
#define GL_REG_18_ATI                                             0x8933
#define GL_REG_19_ATI                                             0x8934
#define GL_REG_20_ATI                                             0x8935
#define GL_REG_21_ATI                                             0x8936
#define GL_REG_22_ATI                                             0x8937
#define GL_REG_23_ATI                                             0x8938
#define GL_REG_24_ATI                                             0x8939
#define GL_REG_25_ATI                                             0x893A
#define GL_REG_26_ATI                                             0x893B
#define GL_REG_27_ATI                                             0x893C
#define GL_REG_28_ATI                                             0x893D
#define GL_REG_29_ATI                                             0x893E
#define GL_REG_30_ATI                                             0x893F
#define GL_REG_31_ATI                                             0x8940
#define GL_CON_0_ATI                                              0x8941
#define GL_CON_1_ATI                                              0x8942
#define GL_CON_2_ATI                                              0x8943
#define GL_CON_3_ATI                                              0x8944
#define GL_CON_4_ATI                                              0x8945
#define GL_CON_5_ATI                                              0x8946
#define GL_CON_6_ATI                                              0x8947
#define GL_CON_7_ATI                                              0x8948
#define GL_CON_8_ATI                                              0x8949
#define GL_CON_9_ATI                                              0x894A
#define GL_CON_10_ATI                                             0x894B
#define GL_CON_11_ATI                                             0x894C
#define GL_CON_12_ATI                                             0x894D
#define GL_CON_13_ATI                                             0x894E
#define GL_CON_14_ATI                                             0x894F
#define GL_CON_15_ATI                                             0x8950
#define GL_CON_16_ATI                                             0x8951
#define GL_CON_17_ATI                                             0x8952
#define GL_CON_18_ATI                                             0x8953
#define GL_CON_19_ATI                                             0x8954
#define GL_CON_20_ATI                                             0x8955
#define GL_CON_21_ATI                                             0x8956
#define GL_CON_22_ATI                                             0x8957
#define GL_CON_23_ATI                                             0x8958
#define GL_CON_24_ATI                                             0x8959
#define GL_CON_25_ATI                                             0x895A
#define GL_CON_26_ATI                                             0x895B
#define GL_CON_27_ATI                                             0x895C
#define GL_CON_28_ATI                                             0x895D
#define GL_CON_29_ATI                                             0x895E
#define GL_CON_30_ATI                                             0x895F
#define GL_CON_31_ATI                                             0x8960
#define GL_MOV_ATI                                                0x8961
#define GL_ADD_ATI                                                0x8963
#define GL_MUL_ATI                                                0x8964
#define GL_SUB_ATI                                                0x8965
#define GL_DOT3_ATI                                               0x8966
#define GL_DOT4_ATI                                               0x8967
#define GL_MAD_ATI                                                0x8968
#define GL_LERP_ATI                                               0x8969
#define GL_CND_ATI                                                0x896A
#define GL_CND0_ATI                                               0x896B
#define GL_DOT2_ADD_ATI                                           0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI                             0x896D
#define GL_NUM_FRAGMENT_REGISTERS_ATI                             0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI                             0x896F
#define GL_NUM_PASSES_ATI                                         0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI                          0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI                             0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI                  0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI                            0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI                                0x8975
#define GL_SWIZZLE_STR_ATI                                        0x8976
#define GL_SWIZZLE_STQ_ATI                                        0x8977
#define GL_SWIZZLE_STR_DR_ATI                                     0x8978
#define GL_SWIZZLE_STQ_DQ_ATI                                     0x8979
#define GL_SWIZZLE_STRQ_ATI                                       0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI                                    0x897B
#define GL_INTERLACE_OML                                          0x8980
#define GL_INTERLACE_READ_OML                                     0x8981
#define GL_FORMAT_SUBSAMPLE_24_24_OML                             0x8982
#define GL_FORMAT_SUBSAMPLE_244_244_OML                           0x8983
#define GL_PACK_RESAMPLE_OML                                      0x8984
#define GL_UNPACK_RESAMPLE_OML                                    0x8985
#define GL_RESAMPLE_REPLICATE_OML                                 0x8986
#define GL_RESAMPLE_ZERO_FILL_OML                                 0x8987
#define GL_RESAMPLE_AVERAGE_OML                                   0x8988
#define GL_RESAMPLE_DECIMATE_OML                                  0x8989
#define GL_POINT_SIZE_ARRAY_TYPE_OES                              0x898A
#define GL_POINT_SIZE_ARRAY_STRIDE_OES                            0x898B
#define GL_POINT_SIZE_ARRAY_POINTER_OES                           0x898C
#define GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES                 0x898D
#define GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES                0x898E
#define GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES                   0x898F
#define GL_VERTEX_ATTRIB_MAP1_APPLE                               0x8A00
#define GL_VERTEX_ATTRIB_MAP2_APPLE                               0x8A01
#define GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE                          0x8A02
#define GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE                         0x8A03
#define GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE                         0x8A04
#define GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE                        0x8A05
#define GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE                          0x8A06
#define GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE                         0x8A07
#define GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE                         0x8A08
#define GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE                        0x8A09
#define GL_DRAW_PIXELS_APPLE                                      0x8A0A
#define GL_FENCE_APPLE                                            0x8A0B
#define GL_ELEMENT_ARRAY_APPLE                                    0x8A0C
#define GL_ELEMENT_ARRAY_TYPE_APPLE                               0x8A0D
#define GL_ELEMENT_ARRAY_POINTER_APPLE                            0x8A0E
#define GL_COLOR_FLOAT_APPLE                                      0x8A0F
#define GL_UNIFORM_BUFFER                                         0x8A11
#define GL_BUFFER_SERIALIZED_MODIFY_APPLE                         0x8A12
#define GL_BUFFER_FLUSHING_UNMAP_APPLE                            0x8A13
#define GL_AUX_DEPTH_STENCIL_APPLE                                0x8A14
#define GL_PACK_ROW_BYTES_APPLE                                   0x8A15
#define GL_UNPACK_ROW_BYTES_APPLE                                 0x8A16
#define GL_RELEASED_APPLE                                         0x8A19
#define GL_VOLATILE_APPLE                                         0x8A1A
#define GL_RETAINED_APPLE                                         0x8A1B
#define GL_UNDEFINED_APPLE                                        0x8A1C
#define GL_PURGEABLE_APPLE                                        0x8A1D
#define GL_RGB_422_APPLE                                          0x8A1F
#define GL_UNIFORM_BUFFER_BINDING                                 0x8A28
#define GL_UNIFORM_BUFFER_START                                   0x8A29
#define GL_UNIFORM_BUFFER_SIZE                                    0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                              0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS                            0x8A2C
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT                        0x8A2C
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS_OES                        0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                            0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                            0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                            0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                                 0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS                 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS               0x8A32
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT           0x8A32
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_OES           0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS               0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT                        0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH                   0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                                  0x8A36
#define GL_UNIFORM_TYPE                                           0x8A37
#define GL_UNIFORM_SIZE                                           0x8A38
#define GL_UNIFORM_NAME_LENGTH                                    0x8A39
#define GL_UNIFORM_BLOCK_INDEX                                    0x8A3A
#define GL_UNIFORM_OFFSET                                         0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                                   0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                                  0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                                   0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                                  0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                                0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                              0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                          0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES                   0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER              0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER            0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER            0x8A46
#define GL_TEXTURE_SRGB_DECODE_EXT                                0x8A48
#define GL_DECODE_EXT                                             0x8A49
#define GL_SKIP_DECODE_EXT                                        0x8A4A
#define GL_PROGRAM_PIPELINE_OBJECT_EXT                            0x8A4F
#define GL_RGB_RAW_422_APPLE                                      0x8A51
#define GL_FRAGMENT_SHADER_DISCARDS_SAMPLES_EXT                   0x8A52
#define GL_SYNC_OBJECT_APPLE                                      0x8A53
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT                       0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT                       0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT                 0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT                 0x8A57
#define GL_FRAGMENT_SHADER                                        0x8B30
#define GL_FRAGMENT_SHADER_ARB                                    0x8B30
#define GL_VERTEX_SHADER                                          0x8B31
#define GL_VERTEX_SHADER_ARB                                      0x8B31
#define GL_PROGRAM_OBJECT_ARB                                     0x8B40
#define GL_PROGRAM_OBJECT_EXT                                     0x8B40
#define GL_SHADER_OBJECT_ARB                                      0x8B48
#define GL_SHADER_OBJECT_EXT                                      0x8B48
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS                        0x8B49
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB                    0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                          0x8B4A
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB                      0x8B4A
#define GL_MAX_VARYING_FLOATS                                     0x8B4B
#define GL_MAX_VARYING_COMPONENTS                                 0x8B4B
#define GL_MAX_VARYING_COMPONENTS_EXT                             0x8B4B
#define GL_MAX_VARYING_FLOATS_ARB                                 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                         0x8B4C
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB                     0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS                       0x8B4D
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB                   0x8B4D
#define GL_OBJECT_TYPE_ARB                                        0x8B4E
#define GL_SHADER_TYPE                                            0x8B4F
#define GL_OBJECT_SUBTYPE_ARB                                     0x8B4F
#define GL_FLOAT_VEC2                                             0x8B50
#define GL_FLOAT_VEC2_ARB                                         0x8B50
#define GL_FLOAT_VEC3                                             0x8B51
#define GL_FLOAT_VEC3_ARB                                         0x8B51
#define GL_FLOAT_VEC4                                             0x8B52
#define GL_FLOAT_VEC4_ARB                                         0x8B52
#define GL_INT_VEC2                                               0x8B53
#define GL_INT_VEC2_ARB                                           0x8B53
#define GL_INT_VEC3                                               0x8B54
#define GL_INT_VEC3_ARB                                           0x8B54
#define GL_INT_VEC4                                               0x8B55
#define GL_INT_VEC4_ARB                                           0x8B55
#define GL_BOOL                                                   0x8B56
#define GL_BOOL_ARB                                               0x8B56
#define GL_BOOL_VEC2                                              0x8B57
#define GL_BOOL_VEC2_ARB                                          0x8B57
#define GL_BOOL_VEC3                                              0x8B58
#define GL_BOOL_VEC3_ARB                                          0x8B58
#define GL_BOOL_VEC4                                              0x8B59
#define GL_BOOL_VEC4_ARB                                          0x8B59
#define GL_FLOAT_MAT2                                             0x8B5A
#define GL_FLOAT_MAT2_ARB                                         0x8B5A
#define GL_FLOAT_MAT3                                             0x8B5B
#define GL_FLOAT_MAT3_ARB                                         0x8B5B
#define GL_FLOAT_MAT4                                             0x8B5C
#define GL_FLOAT_MAT4_ARB                                         0x8B5C
#define GL_SAMPLER_1D                                             0x8B5D
#define GL_SAMPLER_1D_ARB                                         0x8B5D
#define GL_SAMPLER_2D                                             0x8B5E
#define GL_SAMPLER_2D_ARB                                         0x8B5E
#define GL_SAMPLER_3D                                             0x8B5F
#define GL_SAMPLER_3D_ARB                                         0x8B5F
#define GL_SAMPLER_3D_OES                                         0x8B5F
#define GL_SAMPLER_CUBE                                           0x8B60
#define GL_SAMPLER_CUBE_ARB                                       0x8B60
#define GL_SAMPLER_1D_SHADOW                                      0x8B61
#define GL_SAMPLER_1D_SHADOW_ARB                                  0x8B61
#define GL_SAMPLER_2D_SHADOW                                      0x8B62
#define GL_SAMPLER_2D_SHADOW_ARB                                  0x8B62
#define GL_SAMPLER_2D_SHADOW_EXT                                  0x8B62
#define GL_SAMPLER_2D_RECT                                        0x8B63
#define GL_SAMPLER_2D_RECT_ARB                                    0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW                                 0x8B64
#define GL_SAMPLER_2D_RECT_SHADOW_ARB                             0x8B64
#define GL_FLOAT_MAT2x3                                           0x8B65
#define GL_FLOAT_MAT2x3_NV                                        0x8B65
#define GL_FLOAT_MAT2x4                                           0x8B66
#define GL_FLOAT_MAT2x4_NV                                        0x8B66
#define GL_FLOAT_MAT3x2                                           0x8B67
#define GL_FLOAT_MAT3x2_NV                                        0x8B67
#define GL_FLOAT_MAT3x4                                           0x8B68
#define GL_FLOAT_MAT3x4_NV                                        0x8B68
#define GL_FLOAT_MAT4x2                                           0x8B69
#define GL_FLOAT_MAT4x2_NV                                        0x8B69
#define GL_FLOAT_MAT4x3                                           0x8B6A
#define GL_FLOAT_MAT4x3_NV                                        0x8B6A
#define GL_DELETE_STATUS                                          0x8B80
#define GL_OBJECT_DELETE_STATUS_ARB                               0x8B80
#define GL_COMPILE_STATUS                                         0x8B81
#define GL_OBJECT_COMPILE_STATUS_ARB                              0x8B81
#define GL_LINK_STATUS                                            0x8B82
#define GL_OBJECT_LINK_STATUS_ARB                                 0x8B82
#define GL_VALIDATE_STATUS                                        0x8B83
#define GL_OBJECT_VALIDATE_STATUS_ARB                             0x8B83
#define GL_INFO_LOG_LENGTH                                        0x8B84
#define GL_OBJECT_INFO_LOG_LENGTH_ARB                             0x8B84
#define GL_ATTACHED_SHADERS                                       0x8B85
#define GL_OBJECT_ATTACHED_OBJECTS_ARB                            0x8B85
#define GL_ACTIVE_UNIFORMS                                        0x8B86
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB                             0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                              0x8B87
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB                   0x8B87
#define GL_SHADER_SOURCE_LENGTH                                   0x8B88
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB                        0x8B88
#define GL_ACTIVE_ATTRIBUTES                                      0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB                           0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                            0x8B8A
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB                 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT                        0x8B8B
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB                    0x8B8B
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES                    0x8B8B
#define GL_SHADING_LANGUAGE_VERSION                               0x8B8C
#define GL_SHADING_LANGUAGE_VERSION_ARB                           0x8B8C
#define GL_CURRENT_PROGRAM                                        0x8B8D
#define GL_ACTIVE_PROGRAM_EXT                                     0x8B8D
#define GL_PALETTE4_RGB8_OES                                      0x8B90
#define GL_PALETTE4_RGBA8_OES                                     0x8B91
#define GL_PALETTE4_R5_G6_B5_OES                                  0x8B92
#define GL_PALETTE4_RGBA4_OES                                     0x8B93
#define GL_PALETTE4_RGB5_A1_OES                                   0x8B94
#define GL_PALETTE8_RGB8_OES                                      0x8B95
#define GL_PALETTE8_RGBA8_OES                                     0x8B96
#define GL_PALETTE8_R5_G6_B5_OES                                  0x8B97
#define GL_PALETTE8_RGBA4_OES                                     0x8B98
#define GL_PALETTE8_RGB5_A1_OES                                   0x8B99
#define GL_IMPLEMENTATION_COLOR_READ_TYPE                         0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_TYPE_OES                     0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT                       0x8B9B
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES                   0x8B9B
#define GL_POINT_SIZE_ARRAY_OES                                   0x8B9C
#define GL_TEXTURE_CROP_RECT_OES                                  0x8B9D
#define GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES                  0x8B9E
#define GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES                    0x8B9F
#define GL_FRAGMENT_PROGRAM_POSITION_MESA                         0x8BB0
#define GL_FRAGMENT_PROGRAM_CALLBACK_MESA                         0x8BB1
#define GL_FRAGMENT_PROGRAM_CALLBACK_FUNC_MESA                    0x8BB2
#define GL_FRAGMENT_PROGRAM_CALLBACK_DATA_MESA                    0x8BB3
#define GL_VERTEX_PROGRAM_POSITION_MESA                           0x8BB4
#define GL_VERTEX_PROGRAM_CALLBACK_MESA                           0x8BB5
#define GL_VERTEX_PROGRAM_CALLBACK_FUNC_MESA                      0x8BB6
#define GL_VERTEX_PROGRAM_CALLBACK_DATA_MESA                      0x8BB7
#define GL_COUNTER_TYPE_AMD                                       0x8BC0
#define GL_COUNTER_RANGE_AMD                                      0x8BC1
#define GL_UNSIGNED_INT64_AMD                                     0x8BC2
#define GL_PERCENTAGE_AMD                                         0x8BC3
#define GL_PERFMON_RESULT_AVAILABLE_AMD                           0x8BC4
#define GL_PERFMON_RESULT_SIZE_AMD                                0x8BC5
#define GL_PERFMON_RESULT_AMD                                     0x8BC6
#define GL_TEXTURE_WIDTH_QCOM                                     0x8BD2
#define GL_TEXTURE_HEIGHT_QCOM                                    0x8BD3
#define GL_TEXTURE_DEPTH_QCOM                                     0x8BD4
#define GL_TEXTURE_INTERNAL_FORMAT_QCOM                           0x8BD5
#define GL_TEXTURE_FORMAT_QCOM                                    0x8BD6
#define GL_TEXTURE_TYPE_QCOM                                      0x8BD7
#define GL_TEXTURE_IMAGE_VALID_QCOM                               0x8BD8
#define GL_TEXTURE_NUM_LEVELS_QCOM                                0x8BD9
#define GL_TEXTURE_TARGET_QCOM                                    0x8BDA
#define GL_TEXTURE_OBJECT_VALID_QCOM                              0x8BDB
#define GL_STATE_RESTORE                                          0x8BDC
#define GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT                            0x8BE7
#define GL_TEXTURE_PROTECTED_EXT                                  0x8BFA
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                        0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                        0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                       0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                       0x8C03
#define GL_MODULATE_COLOR_IMG                                     0x8C04
#define GL_RECIP_ADD_SIGNED_ALPHA_IMG                             0x8C05
#define GL_TEXTURE_ALPHA_MODULATE_IMG                             0x8C06
#define GL_FACTOR_ALPHA_MODULATE_IMG                              0x8C07
#define GL_FRAGMENT_ALPHA_MODULATE_IMG                            0x8C08
#define GL_ADD_BLEND_IMG                                          0x8C09
#define GL_SGX_BINARY_IMG                                         0x8C0A
#define GL_TEXTURE_RED_TYPE                                       0x8C10
#define GL_TEXTURE_RED_TYPE_ARB                                   0x8C10
#define GL_TEXTURE_GREEN_TYPE                                     0x8C11
#define GL_TEXTURE_GREEN_TYPE_ARB                                 0x8C11
#define GL_TEXTURE_BLUE_TYPE                                      0x8C12
#define GL_TEXTURE_BLUE_TYPE_ARB                                  0x8C12
#define GL_TEXTURE_ALPHA_TYPE                                     0x8C13
#define GL_TEXTURE_ALPHA_TYPE_ARB                                 0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE                                 0x8C14
#define GL_TEXTURE_LUMINANCE_TYPE_ARB                             0x8C14
#define GL_TEXTURE_INTENSITY_TYPE                                 0x8C15
#define GL_TEXTURE_INTENSITY_TYPE_ARB                             0x8C15
#define GL_TEXTURE_DEPTH_TYPE                                     0x8C16
#define GL_TEXTURE_DEPTH_TYPE_ARB                                 0x8C16
#define GL_UNSIGNED_NORMALIZED                                    0x8C17
#define GL_UNSIGNED_NORMALIZED_ARB                                0x8C17
#define GL_UNSIGNED_NORMALIZED_EXT                                0x8C17
#define GL_TEXTURE_1D_ARRAY                                       0x8C18
#define GL_TEXTURE_1D_ARRAY_EXT                                   0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY                                 0x8C19
#define GL_PROXY_TEXTURE_1D_ARRAY_EXT                             0x8C19
#define GL_TEXTURE_2D_ARRAY                                       0x8C1A
#define GL_TEXTURE_2D_ARRAY_EXT                                   0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY                                 0x8C1B
#define GL_PROXY_TEXTURE_2D_ARRAY_EXT                             0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY                               0x8C1C
#define GL_TEXTURE_BINDING_1D_ARRAY_EXT                           0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY                               0x8C1D
#define GL_TEXTURE_BINDING_2D_ARRAY_EXT                           0x8C1D
#define GL_GEOMETRY_PROGRAM_NV                                    0x8C26
#define GL_MAX_PROGRAM_OUTPUT_VERTICES_NV                         0x8C27
#define GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV                 0x8C28
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS                       0x8C29
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB                   0x8C29
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT                   0x8C29
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_OES                   0x8C29
#define GL_TEXTURE_BUFFER                                         0x8C2A
#define GL_TEXTURE_BUFFER_ARB                                     0x8C2A
#define GL_TEXTURE_BUFFER_EXT                                     0x8C2A
#define GL_TEXTURE_BUFFER_OES                                     0x8C2A
#define GL_TEXTURE_BUFFER_BINDING                                 0x8C2A
#define GL_TEXTURE_BUFFER_BINDING_EXT                             0x8C2A
#define GL_TEXTURE_BUFFER_BINDING_OES                             0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE                                0x8C2B
#define GL_MAX_TEXTURE_BUFFER_SIZE_ARB                            0x8C2B
#define GL_MAX_TEXTURE_BUFFER_SIZE_EXT                            0x8C2B
#define GL_MAX_TEXTURE_BUFFER_SIZE_OES                            0x8C2B
#define GL_TEXTURE_BINDING_BUFFER                                 0x8C2C
#define GL_TEXTURE_BINDING_BUFFER_ARB                             0x8C2C
#define GL_TEXTURE_BINDING_BUFFER_EXT                             0x8C2C
#define GL_TEXTURE_BINDING_BUFFER_OES                             0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING                      0x8C2D
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB                  0x8C2D
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT                  0x8C2D
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_OES                  0x8C2D
#define GL_TEXTURE_BUFFER_FORMAT_ARB                              0x8C2E
#define GL_TEXTURE_BUFFER_FORMAT_EXT                              0x8C2E
#define GL_ANY_SAMPLES_PASSED                                     0x8C2F
#define GL_ANY_SAMPLES_PASSED_EXT                                 0x8C2F
#define GL_SAMPLE_SHADING                                         0x8C36
#define GL_SAMPLE_SHADING_ARB                                     0x8C36
#define GL_SAMPLE_SHADING_OES                                     0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE                               0x8C37
#define GL_MIN_SAMPLE_SHADING_VALUE_ARB                           0x8C37
#define GL_MIN_SAMPLE_SHADING_VALUE_OES                           0x8C37
#define GL_R11F_G11F_B10F                                         0x8C3A
#define GL_R11F_G11F_B10F_APPLE                                   0x8C3A
#define GL_R11F_G11F_B10F_EXT                                     0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV                           0x8C3B
#define GL_UNSIGNED_INT_10F_11F_11F_REV_APPLE                     0x8C3B
#define GL_UNSIGNED_INT_10F_11F_11F_REV_EXT                       0x8C3B
#define GL_RGBA_SIGNED_COMPONENTS_EXT                             0x8C3C
#define GL_RGB9_E5                                                0x8C3D
#define GL_RGB9_E5_APPLE                                          0x8C3D
#define GL_RGB9_E5_EXT                                            0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV                               0x8C3E
#define GL_UNSIGNED_INT_5_9_9_9_REV_APPLE                         0x8C3E
#define GL_UNSIGNED_INT_5_9_9_9_REV_EXT                           0x8C3E
#define GL_TEXTURE_SHARED_SIZE                                    0x8C3F
#define GL_TEXTURE_SHARED_SIZE_EXT                                0x8C3F
#define GL_SRGB                                                   0x8C40
#define GL_SRGB_EXT                                               0x8C40
#define GL_SRGB8                                                  0x8C41
#define GL_SRGB8_EXT                                              0x8C41
#define GL_SRGB8_NV                                               0x8C41
#define GL_SRGB_ALPHA                                             0x8C42
#define GL_SRGB_ALPHA_EXT                                         0x8C42
#define GL_SRGB8_ALPHA8                                           0x8C43
#define GL_SRGB8_ALPHA8_EXT                                       0x8C43
#define GL_SLUMINANCE_ALPHA                                       0x8C44
#define GL_SLUMINANCE_ALPHA_EXT                                   0x8C44
#define GL_SLUMINANCE_ALPHA_NV                                    0x8C44
#define GL_SLUMINANCE8_ALPHA8                                     0x8C45
#define GL_SLUMINANCE8_ALPHA8_EXT                                 0x8C45
#define GL_SLUMINANCE8_ALPHA8_NV                                  0x8C45
#define GL_SLUMINANCE                                             0x8C46
#define GL_SLUMINANCE_EXT                                         0x8C46
#define GL_SLUMINANCE_NV                                          0x8C46
#define GL_SLUMINANCE8                                            0x8C47
#define GL_SLUMINANCE8_EXT                                        0x8C47
#define GL_SLUMINANCE8_NV                                         0x8C47
#define GL_COMPRESSED_SRGB                                        0x8C48
#define GL_COMPRESSED_SRGB_EXT                                    0x8C48
#define GL_COMPRESSED_SRGB_ALPHA                                  0x8C49
#define GL_COMPRESSED_SRGB_ALPHA_EXT                              0x8C49
#define GL_COMPRESSED_SLUMINANCE                                  0x8C4A
#define GL_COMPRESSED_SLUMINANCE_EXT                              0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA                            0x8C4B
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT                        0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                          0x8C4C
#define GL_COMPRESSED_SRGB_S3TC_DXT1_NV                           0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT                    0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV                     0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT                    0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV                     0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT                    0x8C4F
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV                     0x8C4F
#define GL_COMPRESSED_LUMINANCE_LATC1_EXT                         0x8C70
#define GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT                  0x8C71
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT                   0x8C72
#define GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT            0x8C73
#define GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV               0x8C74
#define GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV            0x8C75
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH                  0x8C76
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT              0x8C76
#define GL_BACK_PRIMARY_COLOR_NV                                  0x8C77
#define GL_BACK_SECONDARY_COLOR_NV                                0x8C78
#define GL_TEXTURE_COORD_NV                                       0x8C79
#define GL_CLIP_DISTANCE_NV                                       0x8C7A
#define GL_VERTEX_ID_NV                                           0x8C7B
#define GL_PRIMITIVE_ID_NV                                        0x8C7C
#define GL_GENERIC_ATTRIB_NV                                      0x8C7D
#define GL_TRANSFORM_FEEDBACK_ATTRIBS_NV                          0x8C7E
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                         0x8C7F
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT                     0x8C7F
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV                      0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS             0x8C80
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT         0x8C80
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV          0x8C80
#define GL_ACTIVE_VARYINGS_NV                                     0x8C81
#define GL_ACTIVE_VARYING_MAX_LENGTH_NV                           0x8C82
#define GL_TRANSFORM_FEEDBACK_VARYINGS                            0x8C83
#define GL_TRANSFORM_FEEDBACK_VARYINGS_EXT                        0x8C83
#define GL_TRANSFORM_FEEDBACK_VARYINGS_NV                         0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START                        0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT                    0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_START_NV                     0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                         0x8C85
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT                     0x8C85
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV                      0x8C85
#define GL_TRANSFORM_FEEDBACK_RECORD_NV                           0x8C86
#define GL_PRIMITIVES_GENERATED                                   0x8C87
#define GL_PRIMITIVES_GENERATED_EXT                               0x8C87
#define GL_PRIMITIVES_GENERATED_NV                                0x8C87
#define GL_PRIMITIVES_GENERATED_OES                               0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN                  0x8C88
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT              0x8C88
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV               0x8C88
#define GL_RASTERIZER_DISCARD                                     0x8C89
#define GL_RASTERIZER_DISCARD_EXT                                 0x8C89
#define GL_RASTERIZER_DISCARD_NV                                  0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS          0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT      0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV       0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS                0x8C8B
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT            0x8C8B
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV             0x8C8B
#define GL_INTERLEAVED_ATTRIBS                                    0x8C8C
#define GL_INTERLEAVED_ATTRIBS_EXT                                0x8C8C
#define GL_INTERLEAVED_ATTRIBS_NV                                 0x8C8C
#define GL_SEPARATE_ATTRIBS                                       0x8C8D
#define GL_SEPARATE_ATTRIBS_EXT                                   0x8C8D
#define GL_SEPARATE_ATTRIBS_NV                                    0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER                              0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_EXT                          0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_NV                           0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING                      0x8C8F
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT                  0x8C8F
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV                   0x8C8F
#define GL_ATC_RGB_AMD                                            0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                            0x8C93
#define GL_POINT_SPRITE_COORD_ORIGIN                              0x8CA0
#define GL_LOWER_LEFT                                             0x8CA1
#define GL_UPPER_LEFT                                             0x8CA2
#define GL_STENCIL_BACK_REF                                       0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK                                0x8CA4
#define GL_STENCIL_BACK_WRITEMASK                                 0x8CA5
#define GL_DRAW_FRAMEBUFFER_BINDING                               0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING_ANGLE                         0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING_APPLE                         0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT                           0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING_NV                            0x8CA6
#define GL_FRAMEBUFFER_BINDING                                    0x8CA6
#define GL_FRAMEBUFFER_BINDING_ANGLE                              0x8CA6
#define GL_FRAMEBUFFER_BINDING_EXT                                0x8CA6
#define GL_FRAMEBUFFER_BINDING_OES                                0x8CA6
#define GL_RENDERBUFFER_BINDING                                   0x8CA7
#define GL_RENDERBUFFER_BINDING_ANGLE                             0x8CA7
#define GL_RENDERBUFFER_BINDING_EXT                               0x8CA7
#define GL_RENDERBUFFER_BINDING_OES                               0x8CA7
#define GL_READ_FRAMEBUFFER                                       0x8CA8
#define GL_READ_FRAMEBUFFER_ANGLE                                 0x8CA8
#define GL_READ_FRAMEBUFFER_APPLE                                 0x8CA8
#define GL_READ_FRAMEBUFFER_EXT                                   0x8CA8
#define GL_READ_FRAMEBUFFER_NV                                    0x8CA8
#define GL_DRAW_FRAMEBUFFER                                       0x8CA9
#define GL_DRAW_FRAMEBUFFER_ANGLE                                 0x8CA9
#define GL_DRAW_FRAMEBUFFER_APPLE                                 0x8CA9
#define GL_DRAW_FRAMEBUFFER_EXT                                   0x8CA9
#define GL_DRAW_FRAMEBUFFER_NV                                    0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING                               0x8CAA
#define GL_READ_FRAMEBUFFER_BINDING_ANGLE                         0x8CAA
#define GL_READ_FRAMEBUFFER_BINDING_APPLE                         0x8CAA
#define GL_READ_FRAMEBUFFER_BINDING_EXT                           0x8CAA
#define GL_READ_FRAMEBUFFER_BINDING_NV                            0x8CAA
#define GL_RENDERBUFFER_COVERAGE_SAMPLES_NV                       0x8CAB
#define GL_RENDERBUFFER_SAMPLES                                   0x8CAB
#define GL_RENDERBUFFER_SAMPLES_ANGLE                             0x8CAB
#define GL_RENDERBUFFER_SAMPLES_APPLE                             0x8CAB
#define GL_RENDERBUFFER_SAMPLES_EXT                               0x8CAB
#define GL_RENDERBUFFER_SAMPLES_NV                                0x8CAB
#define GL_DEPTH_COMPONENT32F                                     0x8CAC
#define GL_DEPTH32F_STENCIL8                                      0x8CAD
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE                     0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT                 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES                 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME                     0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT                 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES                 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL                   0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT               0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES               0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE           0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT       0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES       0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT          0x8CD4
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES          0x8CD4
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER                   0x8CD4
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT               0x8CD4
#define GL_FRAMEBUFFER_COMPLETE                                   0x8CD5
#define GL_FRAMEBUFFER_COMPLETE_EXT                               0x8CD5
#define GL_FRAMEBUFFER_COMPLETE_OES                               0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT                      0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT                  0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES                  0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT              0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT          0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES          0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS                      0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT                  0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES                  0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                     0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES                     0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER                     0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT                 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_OES                 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER                     0x8CDC
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT                 0x8CDC
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_OES                 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                                0x8CDD
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                            0x8CDD
#define GL_FRAMEBUFFER_UNSUPPORTED_OES                            0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS                                  0x8CDF
#define GL_MAX_COLOR_ATTACHMENTS_EXT                              0x8CDF
#define GL_MAX_COLOR_ATTACHMENTS_NV                               0x8CDF
#define GL_COLOR_ATTACHMENT0                                      0x8CE0
#define GL_COLOR_ATTACHMENT0_EXT                                  0x8CE0
#define GL_COLOR_ATTACHMENT0_NV                                   0x8CE0
#define GL_COLOR_ATTACHMENT0_OES                                  0x8CE0
#define GL_COLOR_ATTACHMENT1                                      0x8CE1
#define GL_COLOR_ATTACHMENT1_EXT                                  0x8CE1
#define GL_COLOR_ATTACHMENT1_NV                                   0x8CE1
#define GL_COLOR_ATTACHMENT2                                      0x8CE2
#define GL_COLOR_ATTACHMENT2_EXT                                  0x8CE2
#define GL_COLOR_ATTACHMENT2_NV                                   0x8CE2
#define GL_COLOR_ATTACHMENT3                                      0x8CE3
#define GL_COLOR_ATTACHMENT3_EXT                                  0x8CE3
#define GL_COLOR_ATTACHMENT3_NV                                   0x8CE3
#define GL_COLOR_ATTACHMENT4                                      0x8CE4
#define GL_COLOR_ATTACHMENT4_EXT                                  0x8CE4
#define GL_COLOR_ATTACHMENT4_NV                                   0x8CE4
#define GL_COLOR_ATTACHMENT5                                      0x8CE5
#define GL_COLOR_ATTACHMENT5_EXT                                  0x8CE5
#define GL_COLOR_ATTACHMENT5_NV                                   0x8CE5
#define GL_COLOR_ATTACHMENT6                                      0x8CE6
#define GL_COLOR_ATTACHMENT6_EXT                                  0x8CE6
#define GL_COLOR_ATTACHMENT6_NV                                   0x8CE6
#define GL_COLOR_ATTACHMENT7                                      0x8CE7
#define GL_COLOR_ATTACHMENT7_EXT                                  0x8CE7
#define GL_COLOR_ATTACHMENT7_NV                                   0x8CE7
#define GL_COLOR_ATTACHMENT8                                      0x8CE8
#define GL_COLOR_ATTACHMENT8_EXT                                  0x8CE8
#define GL_COLOR_ATTACHMENT8_NV                                   0x8CE8
#define GL_COLOR_ATTACHMENT9                                      0x8CE9
#define GL_COLOR_ATTACHMENT9_EXT                                  0x8CE9
#define GL_COLOR_ATTACHMENT9_NV                                   0x8CE9
#define GL_COLOR_ATTACHMENT10                                     0x8CEA
#define GL_COLOR_ATTACHMENT10_EXT                                 0x8CEA
#define GL_COLOR_ATTACHMENT10_NV                                  0x8CEA
#define GL_COLOR_ATTACHMENT11                                     0x8CEB
#define GL_COLOR_ATTACHMENT11_EXT                                 0x8CEB
#define GL_COLOR_ATTACHMENT11_NV                                  0x8CEB
#define GL_COLOR_ATTACHMENT12                                     0x8CEC
#define GL_COLOR_ATTACHMENT12_EXT                                 0x8CEC
#define GL_COLOR_ATTACHMENT12_NV                                  0x8CEC
#define GL_COLOR_ATTACHMENT13                                     0x8CED
#define GL_COLOR_ATTACHMENT13_EXT                                 0x8CED
#define GL_COLOR_ATTACHMENT13_NV                                  0x8CED
#define GL_COLOR_ATTACHMENT14                                     0x8CEE
#define GL_COLOR_ATTACHMENT14_EXT                                 0x8CEE
#define GL_COLOR_ATTACHMENT14_NV                                  0x8CEE
#define GL_COLOR_ATTACHMENT15                                     0x8CEF
#define GL_COLOR_ATTACHMENT15_EXT                                 0x8CEF
#define GL_COLOR_ATTACHMENT15_NV                                  0x8CEF
#define GL_COLOR_ATTACHMENT16                                     0x8CF0
#define GL_COLOR_ATTACHMENT17                                     0x8CF1
#define GL_COLOR_ATTACHMENT18                                     0x8CF2
#define GL_COLOR_ATTACHMENT19                                     0x8CF3
#define GL_COLOR_ATTACHMENT20                                     0x8CF4
#define GL_COLOR_ATTACHMENT21                                     0x8CF5
#define GL_COLOR_ATTACHMENT22                                     0x8CF6
#define GL_COLOR_ATTACHMENT23                                     0x8CF7
#define GL_COLOR_ATTACHMENT24                                     0x8CF8
#define GL_COLOR_ATTACHMENT25                                     0x8CF9
#define GL_COLOR_ATTACHMENT26                                     0x8CFA
#define GL_COLOR_ATTACHMENT27                                     0x8CFB
#define GL_COLOR_ATTACHMENT28                                     0x8CFC
#define GL_COLOR_ATTACHMENT29                                     0x8CFD
#define GL_COLOR_ATTACHMENT30                                     0x8CFE
#define GL_COLOR_ATTACHMENT31                                     0x8CFF
#define GL_DEPTH_ATTACHMENT                                       0x8D00
#define GL_DEPTH_ATTACHMENT_EXT                                   0x8D00
#define GL_DEPTH_ATTACHMENT_OES                                   0x8D00
#define GL_STENCIL_ATTACHMENT                                     0x8D20
#define GL_STENCIL_ATTACHMENT_EXT                                 0x8D20
#define GL_STENCIL_ATTACHMENT_OES                                 0x8D20
#define GL_FRAMEBUFFER                                            0x8D40
#define GL_FRAMEBUFFER_EXT                                        0x8D40
#define GL_FRAMEBUFFER_OES                                        0x8D40
#define GL_RENDERBUFFER                                           0x8D41
#define GL_RENDERBUFFER_EXT                                       0x8D41
#define GL_RENDERBUFFER_OES                                       0x8D41
#define GL_RENDERBUFFER_WIDTH                                     0x8D42
#define GL_RENDERBUFFER_WIDTH_EXT                                 0x8D42
#define GL_RENDERBUFFER_WIDTH_OES                                 0x8D42
#define GL_RENDERBUFFER_HEIGHT                                    0x8D43
#define GL_RENDERBUFFER_HEIGHT_EXT                                0x8D43
#define GL_RENDERBUFFER_HEIGHT_OES                                0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT                           0x8D44
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                       0x8D44
#define GL_RENDERBUFFER_INTERNAL_FORMAT_OES                       0x8D44
#define GL_STENCIL_INDEX1                                         0x8D46
#define GL_STENCIL_INDEX1_EXT                                     0x8D46
#define GL_STENCIL_INDEX1_OES                                     0x8D46
#define GL_STENCIL_INDEX4                                         0x8D47
#define GL_STENCIL_INDEX4_EXT                                     0x8D47
#define GL_STENCIL_INDEX4_OES                                     0x8D47
#define GL_STENCIL_INDEX8                                         0x8D48
#define GL_STENCIL_INDEX8_EXT                                     0x8D48
#define GL_STENCIL_INDEX8_OES                                     0x8D48
#define GL_STENCIL_INDEX16                                        0x8D49
#define GL_STENCIL_INDEX16_EXT                                    0x8D49
#define GL_RENDERBUFFER_RED_SIZE                                  0x8D50
#define GL_RENDERBUFFER_RED_SIZE_EXT                              0x8D50
#define GL_RENDERBUFFER_RED_SIZE_OES                              0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE                                0x8D51
#define GL_RENDERBUFFER_GREEN_SIZE_EXT                            0x8D51
#define GL_RENDERBUFFER_GREEN_SIZE_OES                            0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE                                 0x8D52
#define GL_RENDERBUFFER_BLUE_SIZE_EXT                             0x8D52
#define GL_RENDERBUFFER_BLUE_SIZE_OES                             0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE                                0x8D53
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT                            0x8D53
#define GL_RENDERBUFFER_ALPHA_SIZE_OES                            0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE                                0x8D54
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT                            0x8D54
#define GL_RENDERBUFFER_DEPTH_SIZE_OES                            0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE                              0x8D55
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT                          0x8D55
#define GL_RENDERBUFFER_STENCIL_SIZE_OES                          0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE                     0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE               0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_APPLE               0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT                 0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV                  0x8D56
#define GL_MAX_SAMPLES                                            0x8D57
#define GL_MAX_SAMPLES_ANGLE                                      0x8D57
#define GL_MAX_SAMPLES_APPLE                                      0x8D57
#define GL_MAX_SAMPLES_EXT                                        0x8D57
#define GL_MAX_SAMPLES_NV                                         0x8D57
#define GL_TEXTURE_GEN_STR_OES                                    0x8D60
#define GL_HALF_FLOAT_OES                                         0x8D61
#define GL_RGB565_OES                                             0x8D62
#define GL_RGB565                                                 0x8D62
#define GL_ETC1_RGB8_OES                                          0x8D64
#define GL_TEXTURE_EXTERNAL_OES                                   0x8D65
#define GL_SAMPLER_EXTERNAL_OES                                   0x8D66
#define GL_TEXTURE_BINDING_EXTERNAL_OES                           0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES                       0x8D68
#define GL_PRIMITIVE_RESTART_FIXED_INDEX                          0x8D69
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE                        0x8D6A
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT                    0x8D6A
#define GL_MAX_ELEMENT_INDEX                                      0x8D6B
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT             0x8D6C
#define GL_RGBA32UI                                               0x8D70
#define GL_RGBA32UI_EXT                                           0x8D70
#define GL_RGB32UI                                                0x8D71
#define GL_RGB32UI_EXT                                            0x8D71
#define GL_ALPHA32UI_EXT                                          0x8D72
#define GL_INTENSITY32UI_EXT                                      0x8D73
#define GL_LUMINANCE32UI_EXT                                      0x8D74
#define GL_LUMINANCE_ALPHA32UI_EXT                                0x8D75
#define GL_RGBA16UI                                               0x8D76
#define GL_RGBA16UI_EXT                                           0x8D76
#define GL_RGB16UI                                                0x8D77
#define GL_RGB16UI_EXT                                            0x8D77
#define GL_ALPHA16UI_EXT                                          0x8D78
#define GL_INTENSITY16UI_EXT                                      0x8D79
#define GL_LUMINANCE16UI_EXT                                      0x8D7A
#define GL_LUMINANCE_ALPHA16UI_EXT                                0x8D7B
#define GL_RGBA8UI                                                0x8D7C
#define GL_RGBA8UI_EXT                                            0x8D7C
#define GL_RGB8UI                                                 0x8D7D
#define GL_RGB8UI_EXT                                             0x8D7D
#define GL_ALPHA8UI_EXT                                           0x8D7E
#define GL_INTENSITY8UI_EXT                                       0x8D7F
#define GL_LUMINANCE8UI_EXT                                       0x8D80
#define GL_LUMINANCE_ALPHA8UI_EXT                                 0x8D81
#define GL_RGBA32I                                                0x8D82
#define GL_RGBA32I_EXT                                            0x8D82
#define GL_RGB32I                                                 0x8D83
#define GL_RGB32I_EXT                                             0x8D83
#define GL_ALPHA32I_EXT                                           0x8D84
#define GL_INTENSITY32I_EXT                                       0x8D85
#define GL_LUMINANCE32I_EXT                                       0x8D86
#define GL_LUMINANCE_ALPHA32I_EXT                                 0x8D87
#define GL_RGBA16I                                                0x8D88
#define GL_RGBA16I_EXT                                            0x8D88
#define GL_RGB16I                                                 0x8D89
#define GL_RGB16I_EXT                                             0x8D89
#define GL_ALPHA16I_EXT                                           0x8D8A
#define GL_INTENSITY16I_EXT                                       0x8D8B
#define GL_LUMINANCE16I_EXT                                       0x8D8C
#define GL_LUMINANCE_ALPHA16I_EXT                                 0x8D8D
#define GL_RGBA8I                                                 0x8D8E
#define GL_RGBA8I_EXT                                             0x8D8E
#define GL_RGB8I                                                  0x8D8F
#define GL_RGB8I_EXT                                              0x8D8F
#define GL_ALPHA8I_EXT                                            0x8D90
#define GL_INTENSITY8I_EXT                                        0x8D91
#define GL_LUMINANCE8I_EXT                                        0x8D92
#define GL_LUMINANCE_ALPHA8I_EXT                                  0x8D93
#define GL_RED_INTEGER                                            0x8D94
#define GL_RED_INTEGER_EXT                                        0x8D94
#define GL_GREEN_INTEGER                                          0x8D95
#define GL_GREEN_INTEGER_EXT                                      0x8D95
#define GL_BLUE_INTEGER                                           0x8D96
#define GL_BLUE_INTEGER_EXT                                       0x8D96
#define GL_ALPHA_INTEGER                                          0x8D97
#define GL_ALPHA_INTEGER_EXT                                      0x8D97
#define GL_RGB_INTEGER                                            0x8D98
#define GL_RGB_INTEGER_EXT                                        0x8D98
#define GL_RGBA_INTEGER                                           0x8D99
#define GL_RGBA_INTEGER_EXT                                       0x8D99
#define GL_BGR_INTEGER                                            0x8D9A
#define GL_BGR_INTEGER_EXT                                        0x8D9A
#define GL_BGRA_INTEGER                                           0x8D9B
#define GL_BGRA_INTEGER_EXT                                       0x8D9B
#define GL_LUMINANCE_INTEGER_EXT                                  0x8D9C
#define GL_LUMINANCE_ALPHA_INTEGER_EXT                            0x8D9D
#define GL_RGBA_INTEGER_MODE_EXT                                  0x8D9E
#define GL_INT_2_10_10_10_REV                                     0x8D9F
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV               0x8DA0
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV                   0x8DA1
#define GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV                     0x8DA2
#define GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV                   0x8DA3
#define GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV                   0x8DA4
#define GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV                         0x8DA5
#define GL_MAX_PROGRAM_GENERIC_RESULTS_NV                         0x8DA6
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED                         0x8DA7
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB                     0x8DA7
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT                     0x8DA7
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_OES                     0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS                   0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB               0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT               0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_OES               0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB                 0x8DA9
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT                 0x8DA9
#define GL_LAYER_NV                                               0x8DAA
#define GL_DEPTH_COMPONENT32F_NV                                  0x8DAB
#define GL_DEPTH32F_STENCIL8_NV                                   0x8DAC
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                         0x8DAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV                      0x8DAD
#define GL_SHADER_INCLUDE_ARB                                     0x8DAE
#define GL_DEPTH_BUFFER_FLOAT_MODE_NV                             0x8DAF
#define GL_FRAMEBUFFER_SRGB                                       0x8DB9
#define GL_FRAMEBUFFER_SRGB_EXT                                   0x8DB9
#define GL_FRAMEBUFFER_SRGB_CAPABLE_EXT                           0x8DBA
#define GL_COMPRESSED_RED_RGTC1                                   0x8DBB
#define GL_COMPRESSED_RED_RGTC1_EXT                               0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1                            0x8DBC
#define GL_COMPRESSED_SIGNED_RED_RGTC1_EXT                        0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2_EXT                         0x8DBD
#define GL_COMPRESSED_RG_RGTC2                                    0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT                  0x8DBE
#define GL_COMPRESSED_SIGNED_RG_RGTC2                             0x8DBE
#define GL_SAMPLER_1D_ARRAY                                       0x8DC0
#define GL_SAMPLER_1D_ARRAY_EXT                                   0x8DC0
#define GL_SAMPLER_2D_ARRAY                                       0x8DC1
#define GL_SAMPLER_2D_ARRAY_EXT                                   0x8DC1
#define GL_SAMPLER_BUFFER                                         0x8DC2
#define GL_SAMPLER_BUFFER_EXT                                     0x8DC2
#define GL_SAMPLER_BUFFER_OES                                     0x8DC2
#define GL_SAMPLER_1D_ARRAY_SHADOW                                0x8DC3
#define GL_SAMPLER_1D_ARRAY_SHADOW_EXT                            0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW                                0x8DC4
#define GL_SAMPLER_2D_ARRAY_SHADOW_EXT                            0x8DC4
#define GL_SAMPLER_2D_ARRAY_SHADOW_NV                             0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                                    0x8DC5
#define GL_SAMPLER_CUBE_SHADOW_EXT                                0x8DC5
#define GL_SAMPLER_CUBE_SHADOW_NV                                 0x8DC5
#define GL_UNSIGNED_INT_VEC2                                      0x8DC6
#define GL_UNSIGNED_INT_VEC2_EXT                                  0x8DC6
#define GL_UNSIGNED_INT_VEC3                                      0x8DC7
#define GL_UNSIGNED_INT_VEC3_EXT                                  0x8DC7
#define GL_UNSIGNED_INT_VEC4                                      0x8DC8
#define GL_UNSIGNED_INT_VEC4_EXT                                  0x8DC8
#define GL_INT_SAMPLER_1D                                         0x8DC9
#define GL_INT_SAMPLER_1D_EXT                                     0x8DC9
#define GL_INT_SAMPLER_2D                                         0x8DCA
#define GL_INT_SAMPLER_2D_EXT                                     0x8DCA
#define GL_INT_SAMPLER_3D                                         0x8DCB
#define GL_INT_SAMPLER_3D_EXT                                     0x8DCB
#define GL_INT_SAMPLER_CUBE                                       0x8DCC
#define GL_INT_SAMPLER_CUBE_EXT                                   0x8DCC
#define GL_INT_SAMPLER_2D_RECT                                    0x8DCD
#define GL_INT_SAMPLER_2D_RECT_EXT                                0x8DCD
#define GL_INT_SAMPLER_1D_ARRAY                                   0x8DCE
#define GL_INT_SAMPLER_1D_ARRAY_EXT                               0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY                                   0x8DCF
#define GL_INT_SAMPLER_2D_ARRAY_EXT                               0x8DCF
#define GL_INT_SAMPLER_BUFFER                                     0x8DD0
#define GL_INT_SAMPLER_BUFFER_EXT                                 0x8DD0
#define GL_INT_SAMPLER_BUFFER_OES                                 0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_1D                                0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_1D_EXT                            0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D                                0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_2D_EXT                            0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D                                0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_3D_EXT                            0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE                              0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_CUBE_EXT                          0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT                           0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT                       0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                          0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT                      0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                          0x8DD7
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT                      0x8DD7
#define GL_UNSIGNED_INT_SAMPLER_BUFFER                            0x8DD8
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT                        0x8DD8
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_OES                        0x8DD8
#define GL_GEOMETRY_SHADER                                        0x8DD9
#define GL_GEOMETRY_SHADER_ARB                                    0x8DD9
#define GL_GEOMETRY_SHADER_EXT                                    0x8DD9
#define GL_GEOMETRY_SHADER_OES                                    0x8DD9
#define GL_GEOMETRY_VERTICES_OUT_ARB                              0x8DDA
#define GL_GEOMETRY_VERTICES_OUT_EXT                              0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_ARB                                0x8DDB
#define GL_GEOMETRY_INPUT_TYPE_EXT                                0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_ARB                               0x8DDC
#define GL_GEOMETRY_OUTPUT_TYPE_EXT                               0x8DDC
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB                    0x8DDD
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT                    0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_ARB                      0x8DDE
#define GL_MAX_VERTEX_VARYING_COMPONENTS_EXT                      0x8DDE
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS                        0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB                    0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT                    0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_OES                    0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES                           0x8DE0
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB                       0x8DE0
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT                       0x8DE0
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_OES                       0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS                   0x8DE1
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB               0x8DE1
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT               0x8DE1
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_OES               0x8DE1
#define GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT                       0x8DE2
#define GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT                     0x8DE3
#define GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT                     0x8DE4
#define GL_ACTIVE_SUBROUTINES                                     0x8DE5
#define GL_ACTIVE_SUBROUTINE_UNIFORMS                             0x8DE6
#define GL_MAX_SUBROUTINES                                        0x8DE7
#define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS                       0x8DE8
#define GL_NAMED_STRING_LENGTH_ARB                                0x8DE9
#define GL_NAMED_STRING_TYPE_ARB                                  0x8DEA
#define GL_MAX_BINDABLE_UNIFORM_SIZE_EXT                          0x8DED
#define GL_UNIFORM_BUFFER_EXT                                     0x8DEE
#define GL_UNIFORM_BUFFER_BINDING_EXT                             0x8DEF
#define GL_LOW_FLOAT                                              0x8DF0
#define GL_MEDIUM_FLOAT                                           0x8DF1
#define GL_HIGH_FLOAT                                             0x8DF2
#define GL_LOW_INT                                                0x8DF3
#define GL_MEDIUM_INT                                             0x8DF4
#define GL_HIGH_INT                                               0x8DF5
#define GL_UNSIGNED_INT_10_10_10_2_OES                            0x8DF6
#define GL_INT_10_10_10_2_OES                                     0x8DF7
#define GL_SHADER_BINARY_FORMATS                                  0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS                              0x8DF9
#define GL_SHADER_COMPILER                                        0x8DFA
#define GL_MAX_VERTEX_UNIFORM_VECTORS                             0x8DFB
#define GL_MAX_VARYING_VECTORS                                    0x8DFC
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS                           0x8DFD
#define GL_RENDERBUFFER_COLOR_SAMPLES_NV                          0x8E10
#define GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV                      0x8E11
#define GL_MULTISAMPLE_COVERAGE_MODES_NV                          0x8E12
#define GL_QUERY_WAIT                                             0x8E13
#define GL_QUERY_WAIT_NV                                          0x8E13
#define GL_QUERY_NO_WAIT                                          0x8E14
#define GL_QUERY_NO_WAIT_NV                                       0x8E14
#define GL_QUERY_BY_REGION_WAIT                                   0x8E15
#define GL_QUERY_BY_REGION_WAIT_NV                                0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT                                0x8E16
#define GL_QUERY_BY_REGION_NO_WAIT_NV                             0x8E16
#define GL_QUERY_WAIT_INVERTED                                    0x8E17
#define GL_QUERY_NO_WAIT_INVERTED                                 0x8E18
#define GL_QUERY_BY_REGION_WAIT_INVERTED                          0x8E19
#define GL_QUERY_BY_REGION_NO_WAIT_INVERTED                       0x8E1A
#define GL_POLYGON_OFFSET_CLAMP_EXT                               0x8E1B
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS           0x8E1E
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT       0x8E1E
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_OES       0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS        0x8E1F
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT    0x8E1F
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_OES    0x8E1F
#define GL_COLOR_SAMPLES_NV                                       0x8E20
#define GL_TRANSFORM_FEEDBACK                                     0x8E22
#define GL_TRANSFORM_FEEDBACK_NV                                  0x8E22
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED                       0x8E23
#define GL_TRANSFORM_FEEDBACK_PAUSED                              0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV                    0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE                       0x8E24
#define GL_TRANSFORM_FEEDBACK_ACTIVE                              0x8E24
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV                    0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING                             0x8E25
#define GL_TRANSFORM_FEEDBACK_BINDING_NV                          0x8E25
#define GL_FRAME_NV                                               0x8E26
#define GL_FIELDS_NV                                              0x8E27
#define GL_CURRENT_TIME_NV                                        0x8E28
#define GL_TIMESTAMP                                              0x8E28
#define GL_TIMESTAMP_EXT                                          0x8E28
#define GL_NUM_FILL_STREAMS_NV                                    0x8E29
#define GL_PRESENT_TIME_NV                                        0x8E2A
#define GL_PRESENT_DURATION_NV                                    0x8E2B
#define GL_DEPTH_COMPONENT16_NONLINEAR_NV                         0x8E2C
#define GL_PROGRAM_MATRIX_EXT                                     0x8E2D
#define GL_TRANSPOSE_PROGRAM_MATRIX_EXT                           0x8E2E
#define GL_PROGRAM_MATRIX_STACK_DEPTH_EXT                         0x8E2F
#define GL_TEXTURE_SWIZZLE_R                                      0x8E42
#define GL_TEXTURE_SWIZZLE_R_EXT                                  0x8E42
#define GL_TEXTURE_SWIZZLE_G                                      0x8E43
#define GL_TEXTURE_SWIZZLE_G_EXT                                  0x8E43
#define GL_TEXTURE_SWIZZLE_B                                      0x8E44
#define GL_TEXTURE_SWIZZLE_B_EXT                                  0x8E44
#define GL_TEXTURE_SWIZZLE_A                                      0x8E45
#define GL_TEXTURE_SWIZZLE_A_EXT                                  0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA                                   0x8E46
#define GL_TEXTURE_SWIZZLE_RGBA_EXT                               0x8E46
#define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS                    0x8E47
#define GL_ACTIVE_SUBROUTINE_MAX_LENGTH                           0x8E48
#define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH                   0x8E49
#define GL_NUM_COMPATIBLE_SUBROUTINES                             0x8E4A
#define GL_COMPATIBLE_SUBROUTINES                                 0x8E4B
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION               0x8E4C
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT           0x8E4C
#define GL_FIRST_VERTEX_CONVENTION                                0x8E4D
#define GL_FIRST_VERTEX_CONVENTION_EXT                            0x8E4D
#define GL_FIRST_VERTEX_CONVENTION_OES                            0x8E4D
#define GL_LAST_VERTEX_CONVENTION                                 0x8E4E
#define GL_LAST_VERTEX_CONVENTION_EXT                             0x8E4E
#define GL_LAST_VERTEX_CONVENTION_OES                             0x8E4E
#define GL_PROVOKING_VERTEX                                       0x8E4F
#define GL_PROVOKING_VERTEX_EXT                                   0x8E4F
#define GL_SAMPLE_POSITION                                        0x8E50
#define GL_SAMPLE_POSITION_NV                                     0x8E50
#define GL_SAMPLE_LOCATION_ARB                                    0x8E50
#define GL_SAMPLE_LOCATION_NV                                     0x8E50
#define GL_SAMPLE_MASK                                            0x8E51
#define GL_SAMPLE_MASK_NV                                         0x8E51
#define GL_SAMPLE_MASK_VALUE                                      0x8E52
#define GL_SAMPLE_MASK_VALUE_NV                                   0x8E52
#define GL_TEXTURE_BINDING_RENDERBUFFER_NV                        0x8E53
#define GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV             0x8E54
#define GL_TEXTURE_RENDERBUFFER_NV                                0x8E55
#define GL_SAMPLER_RENDERBUFFER_NV                                0x8E56
#define GL_INT_SAMPLER_RENDERBUFFER_NV                            0x8E57
#define GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV                   0x8E58
#define GL_MAX_SAMPLE_MASK_WORDS                                  0x8E59
#define GL_MAX_SAMPLE_MASK_WORDS_NV                               0x8E59
#define GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV                    0x8E5A
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS                        0x8E5A
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT                    0x8E5A
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS_OES                    0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET                      0x8E5B
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES                  0x8E5B
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV                   0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET                      0x8E5C
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES                  0x8E5C
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV                   0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS                     0x8E5D
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES                 0x8E5D
#define GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV          0x8E5D
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET                      0x8E5E
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB                  0x8E5E
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV                   0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET                      0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB                  0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV                   0x8E5F
#define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS                         0x8E70
#define GL_MAX_VERTEX_STREAMS                                     0x8E71
#define GL_PATCH_VERTICES                                         0x8E72
#define GL_PATCH_VERTICES_EXT                                     0x8E72
#define GL_PATCH_VERTICES_OES                                     0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL                              0x8E73
#define GL_PATCH_DEFAULT_INNER_LEVEL_EXT                          0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL                              0x8E74
#define GL_PATCH_DEFAULT_OUTER_LEVEL_EXT                          0x8E74
#define GL_TESS_CONTROL_OUTPUT_VERTICES                           0x8E75
#define GL_TESS_CONTROL_OUTPUT_VERTICES_EXT                       0x8E75
#define GL_TESS_CONTROL_OUTPUT_VERTICES_OES                       0x8E75
#define GL_TESS_GEN_MODE                                          0x8E76
#define GL_TESS_GEN_MODE_EXT                                      0x8E76
#define GL_TESS_GEN_MODE_OES                                      0x8E76
#define GL_TESS_GEN_SPACING                                       0x8E77
#define GL_TESS_GEN_SPACING_EXT                                   0x8E77
#define GL_TESS_GEN_SPACING_OES                                   0x8E77
#define GL_TESS_GEN_VERTEX_ORDER                                  0x8E78
#define GL_TESS_GEN_VERTEX_ORDER_EXT                              0x8E78
#define GL_TESS_GEN_VERTEX_ORDER_OES                              0x8E78
#define GL_TESS_GEN_POINT_MODE                                    0x8E79
#define GL_TESS_GEN_POINT_MODE_EXT                                0x8E79
#define GL_TESS_GEN_POINT_MODE_OES                                0x8E79
#define GL_ISOLINES                                               0x8E7A
#define GL_ISOLINES_EXT                                           0x8E7A
#define GL_ISOLINES_OES                                           0x8E7A
#define GL_FRACTIONAL_ODD                                         0x8E7B
#define GL_FRACTIONAL_ODD_EXT                                     0x8E7B
#define GL_FRACTIONAL_ODD_OES                                     0x8E7B
#define GL_FRACTIONAL_EVEN                                        0x8E7C
#define GL_FRACTIONAL_EVEN_EXT                                    0x8E7C
#define GL_FRACTIONAL_EVEN_OES                                    0x8E7C
#define GL_MAX_PATCH_VERTICES                                     0x8E7D
#define GL_MAX_PATCH_VERTICES_EXT                                 0x8E7D
#define GL_MAX_PATCH_VERTICES_OES                                 0x8E7D
#define GL_MAX_TESS_GEN_LEVEL                                     0x8E7E
#define GL_MAX_TESS_GEN_LEVEL_EXT                                 0x8E7E
#define GL_MAX_TESS_GEN_LEVEL_OES                                 0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS                    0x8E7F
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT                0x8E7F
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_OES                0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS                 0x8E80
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT             0x8E80
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_OES             0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS                   0x8E81
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT               0x8E81
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_OES               0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS                0x8E82
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT            0x8E82
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_OES            0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS                     0x8E83
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT                 0x8E83
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_OES                 0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS                              0x8E84
#define GL_MAX_TESS_PATCH_COMPONENTS_EXT                          0x8E84
#define GL_MAX_TESS_PATCH_COMPONENTS_OES                          0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS               0x8E85
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT           0x8E85
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_OES           0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS                  0x8E86
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT              0x8E86
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_OES              0x8E86
#define GL_TESS_EVALUATION_SHADER                                 0x8E87
#define GL_TESS_EVALUATION_SHADER_EXT                             0x8E87
#define GL_TESS_EVALUATION_SHADER_OES                             0x8E87
#define GL_TESS_CONTROL_SHADER                                    0x8E88
#define GL_TESS_CONTROL_SHADER_EXT                                0x8E88
#define GL_TESS_CONTROL_SHADER_OES                                0x8E88
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS                        0x8E89
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT                    0x8E89
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_OES                    0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS                     0x8E8A
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT                 0x8E8A
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_OES                 0x8E8A
#define GL_COMPRESSED_RGBA_BPTC_UNORM                             0x8E8C
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB                         0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM                       0x8E8D
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB                   0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT                       0x8E8E
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB                   0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT                     0x8E8F
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB                 0x8E8F
#define GL_COVERAGE_COMPONENT_NV                                  0x8ED0
#define GL_COVERAGE_COMPONENT4_NV                                 0x8ED1
#define GL_COVERAGE_ATTACHMENT_NV                                 0x8ED2
#define GL_COVERAGE_BUFFERS_NV                                    0x8ED3
#define GL_COVERAGE_SAMPLES_NV                                    0x8ED4
#define GL_COVERAGE_ALL_FRAGMENTS_NV                              0x8ED5
#define GL_COVERAGE_EDGE_FRAGMENTS_NV                             0x8ED6
#define GL_COVERAGE_AUTOMATIC_NV                                  0x8ED7
#define GL_INCLUSIVE_EXT                                          0x8F10
#define GL_EXCLUSIVE_EXT                                          0x8F11
#define GL_WINDOW_RECTANGLE_EXT                                   0x8F12
#define GL_WINDOW_RECTANGLE_MODE_EXT                              0x8F13
#define GL_MAX_WINDOW_RECTANGLES_EXT                              0x8F14
#define GL_NUM_WINDOW_RECTANGLES_EXT                              0x8F15
#define GL_BUFFER_GPU_ADDRESS_NV                                  0x8F1D
#define GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV                         0x8F1E
#define GL_ELEMENT_ARRAY_UNIFIED_NV                               0x8F1F
#define GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV                         0x8F20
#define GL_VERTEX_ARRAY_ADDRESS_NV                                0x8F21
#define GL_NORMAL_ARRAY_ADDRESS_NV                                0x8F22
#define GL_COLOR_ARRAY_ADDRESS_NV                                 0x8F23
#define GL_INDEX_ARRAY_ADDRESS_NV                                 0x8F24
#define GL_TEXTURE_COORD_ARRAY_ADDRESS_NV                         0x8F25
#define GL_EDGE_FLAG_ARRAY_ADDRESS_NV                             0x8F26
#define GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV                       0x8F27
#define GL_FOG_COORD_ARRAY_ADDRESS_NV                             0x8F28
#define GL_ELEMENT_ARRAY_ADDRESS_NV                               0x8F29
#define GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV                          0x8F2A
#define GL_VERTEX_ARRAY_LENGTH_NV                                 0x8F2B
#define GL_NORMAL_ARRAY_LENGTH_NV                                 0x8F2C
#define GL_COLOR_ARRAY_LENGTH_NV                                  0x8F2D
#define GL_INDEX_ARRAY_LENGTH_NV                                  0x8F2E
#define GL_TEXTURE_COORD_ARRAY_LENGTH_NV                          0x8F2F
#define GL_EDGE_FLAG_ARRAY_LENGTH_NV                              0x8F30
#define GL_SECONDARY_COLOR_ARRAY_LENGTH_NV                        0x8F31
#define GL_FOG_COORD_ARRAY_LENGTH_NV                              0x8F32
#define GL_ELEMENT_ARRAY_LENGTH_NV                                0x8F33
#define GL_GPU_ADDRESS_NV                                         0x8F34
#define GL_MAX_SHADER_BUFFER_ADDRESS_NV                           0x8F35
#define GL_COPY_READ_BUFFER                                       0x8F36
#define GL_COPY_READ_BUFFER_NV                                    0x8F36
#define GL_COPY_READ_BUFFER_BINDING                               0x8F36
#define GL_COPY_WRITE_BUFFER                                      0x8F37
#define GL_COPY_WRITE_BUFFER_NV                                   0x8F37
#define GL_COPY_WRITE_BUFFER_BINDING                              0x8F37
#define GL_MAX_IMAGE_UNITS                                        0x8F38
#define GL_MAX_IMAGE_UNITS_EXT                                    0x8F38
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS          0x8F39
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT      0x8F39
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES                   0x8F39
#define GL_IMAGE_BINDING_NAME                                     0x8F3A
#define GL_IMAGE_BINDING_NAME_EXT                                 0x8F3A
#define GL_IMAGE_BINDING_LEVEL                                    0x8F3B
#define GL_IMAGE_BINDING_LEVEL_EXT                                0x8F3B
#define GL_IMAGE_BINDING_LAYERED                                  0x8F3C
#define GL_IMAGE_BINDING_LAYERED_EXT                              0x8F3C
#define GL_IMAGE_BINDING_LAYER                                    0x8F3D
#define GL_IMAGE_BINDING_LAYER_EXT                                0x8F3D
#define GL_IMAGE_BINDING_ACCESS                                   0x8F3E
#define GL_IMAGE_BINDING_ACCESS_EXT                               0x8F3E
#define GL_DRAW_INDIRECT_BUFFER                                   0x8F3F
#define GL_DRAW_INDIRECT_UNIFIED_NV                               0x8F40
#define GL_DRAW_INDIRECT_ADDRESS_NV                               0x8F41
#define GL_DRAW_INDIRECT_LENGTH_NV                                0x8F42
#define GL_DRAW_INDIRECT_BUFFER_BINDING                           0x8F43
#define GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS_NV                   0x8F44
#define GL_MAX_PROGRAM_SUBROUTINE_NUM_NV                          0x8F45
#define GL_DOUBLE_MAT2                                            0x8F46
#define GL_DOUBLE_MAT2_EXT                                        0x8F46
#define GL_DOUBLE_MAT3                                            0x8F47
#define GL_DOUBLE_MAT3_EXT                                        0x8F47
#define GL_DOUBLE_MAT4                                            0x8F48
#define GL_DOUBLE_MAT4_EXT                                        0x8F48
#define GL_DOUBLE_MAT2x3                                          0x8F49
#define GL_DOUBLE_MAT2x3_EXT                                      0x8F49
#define GL_DOUBLE_MAT2x4                                          0x8F4A
#define GL_DOUBLE_MAT2x4_EXT                                      0x8F4A
#define GL_DOUBLE_MAT3x2                                          0x8F4B
#define GL_DOUBLE_MAT3x2_EXT                                      0x8F4B
#define GL_DOUBLE_MAT3x4                                          0x8F4C
#define GL_DOUBLE_MAT3x4_EXT                                      0x8F4C
#define GL_DOUBLE_MAT4x2                                          0x8F4D
#define GL_DOUBLE_MAT4x2_EXT                                      0x8F4D
#define GL_DOUBLE_MAT4x3                                          0x8F4E
#define GL_DOUBLE_MAT4x3_EXT                                      0x8F4E
#define GL_VERTEX_BINDING_BUFFER                                  0x8F4F
#define GL_MALI_SHADER_BINARY_ARM                                 0x8F60
#define GL_MALI_PROGRAM_BINARY_ARM                                0x8F61
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT           0x8F63
#define GL_SHADER_PIXEL_LOCAL_STORAGE_EXT                         0x8F64
#define GL_FETCH_PER_SAMPLE_ARM                                   0x8F65
#define GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM              0x8F66
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT                0x8F67
#define GL_RED_SNORM                                              0x8F90
#define GL_RG_SNORM                                               0x8F91
#define GL_RGB_SNORM                                              0x8F92
#define GL_RGBA_SNORM                                             0x8F93
#define GL_R8_SNORM                                               0x8F94
#define GL_RG8_SNORM                                              0x8F95
#define GL_RGB8_SNORM                                             0x8F96
#define GL_RGBA8_SNORM                                            0x8F97
#define GL_R16_SNORM                                              0x8F98
#define GL_R16_SNORM_EXT                                          0x8F98
#define GL_RG16_SNORM                                             0x8F99
#define GL_RG16_SNORM_EXT                                         0x8F99
#define GL_RGB16_SNORM                                            0x8F9A
#define GL_RGB16_SNORM_EXT                                        0x8F9A
#define GL_RGBA16_SNORM                                           0x8F9B
#define GL_RGBA16_SNORM_EXT                                       0x8F9B
#define GL_SIGNED_NORMALIZED                                      0x8F9C
#define GL_PRIMITIVE_RESTART                                      0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX                                0x8F9E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB              0x8F9F
#define GL_PERFMON_GLOBAL_MODE_QCOM                               0x8FA0
#define GL_BINNING_CONTROL_HINT_QCOM                              0x8FB0
#define GL_CPU_OPTIMIZED_QCOM                                     0x8FB1
#define GL_GPU_OPTIMIZED_QCOM                                     0x8FB2
#define GL_RENDER_DIRECT_TO_FRAMEBUFFER_QCOM                      0x8FB3
#define GL_GPU_DISJOINT_EXT                                       0x8FBB
#define GL_SR8_EXT                                                0x8FBD
#define GL_SRG8_EXT                                               0x8FBE
#define GL_SHADER_BINARY_VIV                                      0x8FC4
#define GL_INT8_NV                                                0x8FE0
#define GL_INT8_VEC2_NV                                           0x8FE1
#define GL_INT8_VEC3_NV                                           0x8FE2
#define GL_INT8_VEC4_NV                                           0x8FE3
#define GL_INT16_NV                                               0x8FE4
#define GL_INT16_VEC2_NV                                          0x8FE5
#define GL_INT16_VEC3_NV                                          0x8FE6
#define GL_INT16_VEC4_NV                                          0x8FE7
#define GL_INT64_VEC2_ARB                                         0x8FE9
#define GL_INT64_VEC2_NV                                          0x8FE9
#define GL_INT64_VEC3_ARB                                         0x8FEA
#define GL_INT64_VEC3_NV                                          0x8FEA
#define GL_INT64_VEC4_ARB                                         0x8FEB
#define GL_INT64_VEC4_NV                                          0x8FEB
#define GL_UNSIGNED_INT8_NV                                       0x8FEC
#define GL_UNSIGNED_INT8_VEC2_NV                                  0x8FED
#define GL_UNSIGNED_INT8_VEC3_NV                                  0x8FEE
#define GL_UNSIGNED_INT8_VEC4_NV                                  0x8FEF
#define GL_UNSIGNED_INT16_NV                                      0x8FF0
#define GL_UNSIGNED_INT16_VEC2_NV                                 0x8FF1
#define GL_UNSIGNED_INT16_VEC3_NV                                 0x8FF2
#define GL_UNSIGNED_INT16_VEC4_NV                                 0x8FF3
#define GL_UNSIGNED_INT64_VEC2_ARB                                0x8FF5
#define GL_UNSIGNED_INT64_VEC2_NV                                 0x8FF5
#define GL_UNSIGNED_INT64_VEC3_ARB                                0x8FF6
#define GL_UNSIGNED_INT64_VEC3_NV                                 0x8FF6
#define GL_UNSIGNED_INT64_VEC4_ARB                                0x8FF7
#define GL_UNSIGNED_INT64_VEC4_NV                                 0x8FF7
#define GL_FLOAT16_NV                                             0x8FF8
#define GL_FLOAT16_VEC2_NV                                        0x8FF9
#define GL_FLOAT16_VEC3_NV                                        0x8FFA
#define GL_FLOAT16_VEC4_NV                                        0x8FFB
#define GL_DOUBLE_VEC2                                            0x8FFC
#define GL_DOUBLE_VEC2_EXT                                        0x8FFC
#define GL_DOUBLE_VEC3                                            0x8FFD
#define GL_DOUBLE_VEC3_EXT                                        0x8FFD
#define GL_DOUBLE_VEC4                                            0x8FFE
#define GL_DOUBLE_VEC4_EXT                                        0x8FFE
#define GL_SAMPLER_BUFFER_AMD                                     0x9001
#define GL_INT_SAMPLER_BUFFER_AMD                                 0x9002
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD                        0x9003
#define GL_TESSELLATION_MODE_AMD                                  0x9004
#define GL_TESSELLATION_FACTOR_AMD                                0x9005
#define GL_DISCRETE_AMD                                           0x9006
#define GL_CONTINUOUS_AMD                                         0x9007
#define GL_TEXTURE_CUBE_MAP_ARRAY                                 0x9009
#define GL_TEXTURE_CUBE_MAP_ARRAY_ARB                             0x9009
#define GL_TEXTURE_CUBE_MAP_ARRAY_EXT                             0x9009
#define GL_TEXTURE_CUBE_MAP_ARRAY_OES                             0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY                         0x900A
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB                     0x900A
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_EXT                     0x900A
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_OES                     0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY                           0x900B
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB                       0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY                                 0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_ARB                             0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_EXT                             0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_OES                             0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW                          0x900D
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB                      0x900D
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_EXT                      0x900D
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_OES                      0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY                             0x900E
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB                         0x900E
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_EXT                         0x900E
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_OES                         0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY                    0x900F
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB                0x900F
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_EXT                0x900F
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_OES                0x900F
#define GL_ALPHA_SNORM                                            0x9010
#define GL_LUMINANCE_SNORM                                        0x9011
#define GL_LUMINANCE_ALPHA_SNORM                                  0x9012
#define GL_INTENSITY_SNORM                                        0x9013
#define GL_ALPHA8_SNORM                                           0x9014
#define GL_LUMINANCE8_SNORM                                       0x9015
#define GL_LUMINANCE8_ALPHA8_SNORM                                0x9016
#define GL_INTENSITY8_SNORM                                       0x9017
#define GL_ALPHA16_SNORM                                          0x9018
#define GL_LUMINANCE16_SNORM                                      0x9019
#define GL_LUMINANCE16_ALPHA16_SNORM                              0x901A
#define GL_INTENSITY16_SNORM                                      0x901B
#define GL_FACTOR_MIN_AMD                                         0x901C
#define GL_FACTOR_MAX_AMD                                         0x901D
#define GL_DEPTH_CLAMP_NEAR_AMD                                   0x901E
#define GL_DEPTH_CLAMP_FAR_AMD                                    0x901F
#define GL_VIDEO_BUFFER_NV                                        0x9020
#define GL_VIDEO_BUFFER_BINDING_NV                                0x9021
#define GL_FIELD_UPPER_NV                                         0x9022
#define GL_FIELD_LOWER_NV                                         0x9023
#define GL_NUM_VIDEO_CAPTURE_STREAMS_NV                           0x9024
#define GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV                    0x9025
#define GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV                      0x9026
#define GL_LAST_VIDEO_CAPTURE_STATUS_NV                           0x9027
#define GL_VIDEO_BUFFER_PITCH_NV                                  0x9028
#define GL_VIDEO_COLOR_CONVERSION_MATRIX_NV                       0x9029
#define GL_VIDEO_COLOR_CONVERSION_MAX_NV                          0x902A
#define GL_VIDEO_COLOR_CONVERSION_MIN_NV                          0x902B
#define GL_VIDEO_COLOR_CONVERSION_OFFSET_NV                       0x902C
#define GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV                        0x902D
#define GL_PARTIAL_SUCCESS_NV                                     0x902E
#define GL_SUCCESS_NV                                             0x902F
#define GL_FAILURE_NV                                             0x9030
#define GL_YCBYCR8_422_NV                                         0x9031
#define GL_YCBAYCR8A_4224_NV                                      0x9032
#define GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV                          0x9033
#define GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV               0x9034
#define GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV                          0x9035
#define GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV               0x9036
#define GL_Z4Y12Z4CB12Z4CR12_444_NV                               0x9037
#define GL_VIDEO_CAPTURE_FRAME_WIDTH_NV                           0x9038
#define GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV                          0x9039
#define GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV                    0x903A
#define GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV                    0x903B
#define GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV                        0x903C
#define GL_TEXTURE_COVERAGE_SAMPLES_NV                            0x9045
#define GL_TEXTURE_COLOR_SAMPLES_NV                               0x9046
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX                   0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX             0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX           0x9049
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX                     0x904A
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX                     0x904B
#define GL_IMAGE_1D                                               0x904C
#define GL_IMAGE_1D_EXT                                           0x904C
#define GL_IMAGE_2D                                               0x904D
#define GL_IMAGE_2D_EXT                                           0x904D
#define GL_IMAGE_3D                                               0x904E
#define GL_IMAGE_3D_EXT                                           0x904E
#define GL_IMAGE_2D_RECT                                          0x904F
#define GL_IMAGE_2D_RECT_EXT                                      0x904F
#define GL_IMAGE_CUBE                                             0x9050
#define GL_IMAGE_CUBE_EXT                                         0x9050
#define GL_IMAGE_BUFFER                                           0x9051
#define GL_IMAGE_BUFFER_EXT                                       0x9051
#define GL_IMAGE_BUFFER_OES                                       0x9051
#define GL_IMAGE_1D_ARRAY                                         0x9052
#define GL_IMAGE_1D_ARRAY_EXT                                     0x9052
#define GL_IMAGE_2D_ARRAY                                         0x9053
#define GL_IMAGE_2D_ARRAY_EXT                                     0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY                                   0x9054
#define GL_IMAGE_CUBE_MAP_ARRAY_EXT                               0x9054
#define GL_IMAGE_CUBE_MAP_ARRAY_OES                               0x9054
#define GL_IMAGE_2D_MULTISAMPLE                                   0x9055
#define GL_IMAGE_2D_MULTISAMPLE_EXT                               0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY                             0x9056
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT                         0x9056
#define GL_INT_IMAGE_1D                                           0x9057
#define GL_INT_IMAGE_1D_EXT                                       0x9057
#define GL_INT_IMAGE_2D                                           0x9058
#define GL_INT_IMAGE_2D_EXT                                       0x9058
#define GL_INT_IMAGE_3D                                           0x9059
#define GL_INT_IMAGE_3D_EXT                                       0x9059
#define GL_INT_IMAGE_2D_RECT                                      0x905A
#define GL_INT_IMAGE_2D_RECT_EXT                                  0x905A
#define GL_INT_IMAGE_CUBE                                         0x905B
#define GL_INT_IMAGE_CUBE_EXT                                     0x905B
#define GL_INT_IMAGE_BUFFER                                       0x905C
#define GL_INT_IMAGE_BUFFER_EXT                                   0x905C
#define GL_INT_IMAGE_BUFFER_OES                                   0x905C
#define GL_INT_IMAGE_1D_ARRAY                                     0x905D
#define GL_INT_IMAGE_1D_ARRAY_EXT                                 0x905D
#define GL_INT_IMAGE_2D_ARRAY                                     0x905E
#define GL_INT_IMAGE_2D_ARRAY_EXT                                 0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY                               0x905F
#define GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT                           0x905F
#define GL_INT_IMAGE_CUBE_MAP_ARRAY_OES                           0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE                               0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_EXT                           0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY                         0x9061
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT                     0x9061
#define GL_UNSIGNED_INT_IMAGE_1D                                  0x9062
#define GL_UNSIGNED_INT_IMAGE_1D_EXT                              0x9062
#define GL_UNSIGNED_INT_IMAGE_2D                                  0x9063
#define GL_UNSIGNED_INT_IMAGE_2D_EXT                              0x9063
#define GL_UNSIGNED_INT_IMAGE_3D                                  0x9064
#define GL_UNSIGNED_INT_IMAGE_3D_EXT                              0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT                             0x9065
#define GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT                         0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE                                0x9066
#define GL_UNSIGNED_INT_IMAGE_CUBE_EXT                            0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER                              0x9067
#define GL_UNSIGNED_INT_IMAGE_BUFFER_EXT                          0x9067
#define GL_UNSIGNED_INT_IMAGE_BUFFER_OES                          0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY                            0x9068
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT                        0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY                            0x9069
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT                        0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY                      0x906A
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT                  0x906A
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_OES                  0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE                      0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT                  0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY                0x906C
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT            0x906C
#define GL_MAX_IMAGE_SAMPLES                                      0x906D
#define GL_MAX_IMAGE_SAMPLES_EXT                                  0x906D
#define GL_IMAGE_BINDING_FORMAT                                   0x906E
#define GL_IMAGE_BINDING_FORMAT_EXT                               0x906E
#define GL_RGB10_A2UI                                             0x906F
#define GL_PATH_FORMAT_SVG_NV                                     0x9070
#define GL_PATH_FORMAT_PS_NV                                      0x9071
#define GL_STANDARD_FONT_NAME_NV                                  0x9072
#define GL_SYSTEM_FONT_NAME_NV                                    0x9073
#define GL_FILE_NAME_NV                                           0x9074
#define GL_PATH_STROKE_WIDTH_NV                                   0x9075
#define GL_PATH_END_CAPS_NV                                       0x9076
#define GL_PATH_INITIAL_END_CAP_NV                                0x9077
#define GL_PATH_TERMINAL_END_CAP_NV                               0x9078
#define GL_PATH_JOIN_STYLE_NV                                     0x9079
#define GL_PATH_MITER_LIMIT_NV                                    0x907A
#define GL_PATH_DASH_CAPS_NV                                      0x907B
#define GL_PATH_INITIAL_DASH_CAP_NV                               0x907C
#define GL_PATH_TERMINAL_DASH_CAP_NV                              0x907D
#define GL_PATH_DASH_OFFSET_NV                                    0x907E
#define GL_PATH_CLIENT_LENGTH_NV                                  0x907F
#define GL_PATH_FILL_MODE_NV                                      0x9080
#define GL_PATH_FILL_MASK_NV                                      0x9081
#define GL_PATH_FILL_COVER_MODE_NV                                0x9082
#define GL_PATH_STROKE_COVER_MODE_NV                              0x9083
#define GL_PATH_STROKE_MASK_NV                                    0x9084
#define GL_COUNT_UP_NV                                            0x9088
#define GL_COUNT_DOWN_NV                                          0x9089
#define GL_PATH_OBJECT_BOUNDING_BOX_NV                            0x908A
#define GL_CONVEX_HULL_NV                                         0x908B
#define GL_BOUNDING_BOX_NV                                        0x908D
#define GL_TRANSLATE_X_NV                                         0x908E
#define GL_TRANSLATE_Y_NV                                         0x908F
#define GL_TRANSLATE_2D_NV                                        0x9090
#define GL_TRANSLATE_3D_NV                                        0x9091
#define GL_AFFINE_2D_NV                                           0x9092
#define GL_AFFINE_3D_NV                                           0x9094
#define GL_TRANSPOSE_AFFINE_2D_NV                                 0x9096
#define GL_TRANSPOSE_AFFINE_3D_NV                                 0x9098
#define GL_UTF8_NV                                                0x909A
#define GL_UTF16_NV                                               0x909B
#define GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV                      0x909C
#define GL_PATH_COMMAND_COUNT_NV                                  0x909D
#define GL_PATH_COORD_COUNT_NV                                    0x909E
#define GL_PATH_DASH_ARRAY_COUNT_NV                               0x909F
#define GL_PATH_COMPUTED_LENGTH_NV                                0x90A0
#define GL_PATH_FILL_BOUNDING_BOX_NV                              0x90A1
#define GL_PATH_STROKE_BOUNDING_BOX_NV                            0x90A2
#define GL_SQUARE_NV                                              0x90A3
#define GL_ROUND_NV                                               0x90A4
#define GL_TRIANGULAR_NV                                          0x90A5
#define GL_BEVEL_NV                                               0x90A6
#define GL_MITER_REVERT_NV                                        0x90A7
#define GL_MITER_TRUNCATE_NV                                      0x90A8
#define GL_SKIP_MISSING_GLYPH_NV                                  0x90A9
#define GL_USE_MISSING_GLYPH_NV                                   0x90AA
#define GL_PATH_ERROR_POSITION_NV                                 0x90AB
#define GL_PATH_FOG_GEN_MODE_NV                                   0x90AC
#define GL_ACCUM_ADJACENT_PAIRS_NV                                0x90AD
#define GL_ADJACENT_PAIRS_NV                                      0x90AE
#define GL_FIRST_TO_REST_NV                                       0x90AF
#define GL_PATH_GEN_MODE_NV                                       0x90B0
#define GL_PATH_GEN_COEFF_NV                                      0x90B1
#define GL_PATH_GEN_COLOR_FORMAT_NV                               0x90B2
#define GL_PATH_GEN_COMPONENTS_NV                                 0x90B3
#define GL_PATH_DASH_OFFSET_RESET_NV                              0x90B4
#define GL_MOVE_TO_RESETS_NV                                      0x90B5
#define GL_MOVE_TO_CONTINUES_NV                                   0x90B6
#define GL_PATH_STENCIL_FUNC_NV                                   0x90B7
#define GL_PATH_STENCIL_REF_NV                                    0x90B8
#define GL_PATH_STENCIL_VALUE_MASK_NV                             0x90B9
#define GL_SCALED_RESOLVE_FASTEST_EXT                             0x90BA
#define GL_SCALED_RESOLVE_NICEST_EXT                              0x90BB
#define GL_MIN_MAP_BUFFER_ALIGNMENT                               0x90BC
#define GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV                    0x90BD
#define GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV                     0x90BE
#define GL_PATH_COVER_DEPTH_FUNC_NV                               0x90BF
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE                        0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE                     0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS                    0x90C9
#define GL_MAX_VERTEX_IMAGE_UNIFORMS                              0x90CA
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS                        0x90CB
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT                    0x90CB
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_OES                    0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS                     0x90CC
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT                 0x90CC
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_OES                 0x90CC
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS                            0x90CD
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT                        0x90CD
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS_OES                        0x90CD
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS                            0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS                            0x90CF
#define GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV                    0x90D0
#define GL_MAX_DEEP_3D_TEXTURE_DEPTH_NV                           0x90D1
#define GL_SHADER_STORAGE_BUFFER                                  0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING                          0x90D3
#define GL_SHADER_STORAGE_BUFFER_START                            0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE                             0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS                       0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS                     0x90D7
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT                 0x90D7
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_OES                 0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS                 0x90D8
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT             0x90D8
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_OES             0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS              0x90D9
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT          0x90D9
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_OES          0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS                     0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS                      0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS                     0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS                     0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE                          0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT                 0x90DF
#define GL_SYNC_X11_FENCE_EXT                                     0x90E1
#define GL_DEPTH_STENCIL_TEXTURE_MODE                             0x90EA
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS                     0x90EB
#define GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB                0x90EB
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER             0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER     0x90ED
#define GL_DISPATCH_INDIRECT_BUFFER                               0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING                       0x90EF
#define GL_COLOR_ATTACHMENT_EXT                                   0x90F0
#define GL_MULTIVIEW_EXT                                          0x90F1
#define GL_MAX_MULTIVIEW_BUFFERS_EXT                              0x90F2
#define GL_CONTEXT_ROBUST_ACCESS                                  0x90F3
#define GL_CONTEXT_ROBUST_ACCESS_EXT                              0x90F3
#define GL_CONTEXT_ROBUST_ACCESS_KHR                              0x90F3
#define GL_COMPUTE_PROGRAM_NV                                     0x90FB
#define GL_COMPUTE_PROGRAM_PARAMETER_BUFFER_NV                    0x90FC
#define GL_TEXTURE_2D_MULTISAMPLE                                 0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE                           0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY                           0x9102
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES                       0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY                     0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE                         0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY                   0x9105
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY_OES               0x9105
#define GL_TEXTURE_SAMPLES                                        0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                         0x9107
#define GL_SAMPLER_2D_MULTISAMPLE                                 0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE                             0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE                    0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY                           0x910B
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY_OES                       0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY                       0x910C
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES                   0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY              0x910D
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES          0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES                              0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES                              0x910F
#define GL_MAX_INTEGER_SAMPLES                                    0x9110
#define GL_MAX_SERVER_WAIT_TIMEOUT                                0x9111
#define GL_MAX_SERVER_WAIT_TIMEOUT_APPLE                          0x9111
#define GL_OBJECT_TYPE                                            0x9112
#define GL_OBJECT_TYPE_APPLE                                      0x9112
#define GL_SYNC_CONDITION                                         0x9113
#define GL_SYNC_CONDITION_APPLE                                   0x9113
#define GL_SYNC_STATUS                                            0x9114
#define GL_SYNC_STATUS_APPLE                                      0x9114
#define GL_SYNC_FLAGS                                             0x9115
#define GL_SYNC_FLAGS_APPLE                                       0x9115
#define GL_SYNC_FENCE                                             0x9116
#define GL_SYNC_FENCE_APPLE                                       0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE                             0x9117
#define GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE                       0x9117
#define GL_UNSIGNALED                                             0x9118
#define GL_UNSIGNALED_APPLE                                       0x9118
#define GL_SIGNALED                                               0x9119
#define GL_SIGNALED_APPLE                                         0x9119
#define GL_ALREADY_SIGNALED                                       0x911A
#define GL_ALREADY_SIGNALED_APPLE                                 0x911A
#define GL_TIMEOUT_EXPIRED                                        0x911B
#define GL_TIMEOUT_EXPIRED_APPLE                                  0x911B
#define GL_CONDITION_SATISFIED                                    0x911C
#define GL_CONDITION_SATISFIED_APPLE                              0x911C
#define GL_WAIT_FAILED                                            0x911D
#define GL_WAIT_FAILED_APPLE                                      0x911D
#define GL_BUFFER_ACCESS_FLAGS                                    0x911F
#define GL_BUFFER_MAP_LENGTH                                      0x9120
#define GL_BUFFER_MAP_OFFSET                                      0x9121
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                           0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS                          0x9123
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT                      0x9123
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS_OES                      0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS                         0x9124
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT                     0x9124
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_OES                     0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                          0x9125
#define GL_CONTEXT_PROFILE_MASK                                   0x9126
#define GL_UNPACK_COMPRESSED_BLOCK_WIDTH                          0x9127
#define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT                         0x9128
#define GL_UNPACK_COMPRESSED_BLOCK_DEPTH                          0x9129
#define GL_UNPACK_COMPRESSED_BLOCK_SIZE                           0x912A
#define GL_PACK_COMPRESSED_BLOCK_WIDTH                            0x912B
#define GL_PACK_COMPRESSED_BLOCK_HEIGHT                           0x912C
#define GL_PACK_COMPRESSED_BLOCK_DEPTH                            0x912D
#define GL_PACK_COMPRESSED_BLOCK_SIZE                             0x912E
#define GL_TEXTURE_IMMUTABLE_FORMAT                               0x912F
#define GL_TEXTURE_IMMUTABLE_FORMAT_EXT                           0x912F
#define GL_SGX_PROGRAM_BINARY_IMG                                 0x9130
#define GL_RENDERBUFFER_SAMPLES_IMG                               0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG                 0x9134
#define GL_MAX_SAMPLES_IMG                                        0x9135
#define GL_TEXTURE_SAMPLES_IMG                                    0x9136
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG                       0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG                       0x9138
#define GL_CUBIC_IMG                                              0x9139
#define GL_CUBIC_MIPMAP_NEAREST_IMG                               0x913A
#define GL_CUBIC_MIPMAP_LINEAR_IMG                                0x913B
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_AND_DOWNSAMPLE_IMG  0x913C
#define GL_NUM_DOWNSAMPLE_SCALES_IMG                              0x913D
#define GL_DOWNSAMPLE_SCALES_IMG                                  0x913E
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SCALE_IMG               0x913F
#define GL_MAX_DEBUG_MESSAGE_LENGTH                               0x9143
#define GL_MAX_DEBUG_MESSAGE_LENGTH_AMD                           0x9143
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB                           0x9143
#define GL_MAX_DEBUG_MESSAGE_LENGTH_KHR                           0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES                              0x9144
#define GL_MAX_DEBUG_LOGGED_MESSAGES_AMD                          0x9144
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB                          0x9144
#define GL_MAX_DEBUG_LOGGED_MESSAGES_KHR                          0x9144
#define GL_DEBUG_LOGGED_MESSAGES                                  0x9145
#define GL_DEBUG_LOGGED_MESSAGES_AMD                              0x9145
#define GL_DEBUG_LOGGED_MESSAGES_ARB                              0x9145
#define GL_DEBUG_LOGGED_MESSAGES_KHR                              0x9145
#define GL_DEBUG_SEVERITY_HIGH                                    0x9146
#define GL_DEBUG_SEVERITY_HIGH_AMD                                0x9146
#define GL_DEBUG_SEVERITY_HIGH_ARB                                0x9146
#define GL_DEBUG_SEVERITY_HIGH_KHR                                0x9146
#define GL_DEBUG_SEVERITY_MEDIUM                                  0x9147
#define GL_DEBUG_SEVERITY_MEDIUM_AMD                              0x9147
#define GL_DEBUG_SEVERITY_MEDIUM_ARB                              0x9147
#define GL_DEBUG_SEVERITY_MEDIUM_KHR                              0x9147
#define GL_DEBUG_SEVERITY_LOW                                     0x9148
#define GL_DEBUG_SEVERITY_LOW_AMD                                 0x9148
#define GL_DEBUG_SEVERITY_LOW_ARB                                 0x9148
#define GL_DEBUG_SEVERITY_LOW_KHR                                 0x9148
#define GL_DEBUG_CATEGORY_API_ERROR_AMD                           0x9149
#define GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD                       0x914A
#define GL_DEBUG_CATEGORY_DEPRECATION_AMD                         0x914B
#define GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD                  0x914C
#define GL_DEBUG_CATEGORY_PERFORMANCE_AMD                         0x914D
#define GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD                     0x914E
#define GL_DEBUG_CATEGORY_APPLICATION_AMD                         0x914F
#define GL_DEBUG_CATEGORY_OTHER_AMD                               0x9150
#define GL_BUFFER_OBJECT_EXT                                      0x9151
#define GL_DATA_BUFFER_AMD                                        0x9151
#define GL_PERFORMANCE_MONITOR_AMD                                0x9152
#define GL_QUERY_OBJECT_AMD                                       0x9153
#define GL_QUERY_OBJECT_EXT                                       0x9153
#define GL_VERTEX_ARRAY_OBJECT_AMD                                0x9154
#define GL_VERTEX_ARRAY_OBJECT_EXT                                0x9154
#define GL_SAMPLER_OBJECT_AMD                                     0x9155
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD                     0x9160
#define GL_QUERY_BUFFER                                           0x9192
#define GL_QUERY_BUFFER_AMD                                       0x9192
#define GL_QUERY_BUFFER_BINDING                                   0x9193
#define GL_QUERY_BUFFER_BINDING_AMD                               0x9193
#define GL_QUERY_RESULT_NO_WAIT                                   0x9194
#define GL_QUERY_RESULT_NO_WAIT_AMD                               0x9194
#define GL_VIRTUAL_PAGE_SIZE_X_ARB                                0x9195
#define GL_VIRTUAL_PAGE_SIZE_X_EXT                                0x9195
#define GL_VIRTUAL_PAGE_SIZE_X_AMD                                0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_ARB                                0x9196
#define GL_VIRTUAL_PAGE_SIZE_Y_EXT                                0x9196
#define GL_VIRTUAL_PAGE_SIZE_Y_AMD                                0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_ARB                                0x9197
#define GL_VIRTUAL_PAGE_SIZE_Z_EXT                                0x9197
#define GL_VIRTUAL_PAGE_SIZE_Z_AMD                                0x9197
#define GL_MAX_SPARSE_TEXTURE_SIZE_ARB                            0x9198
#define GL_MAX_SPARSE_TEXTURE_SIZE_EXT                            0x9198
#define GL_MAX_SPARSE_TEXTURE_SIZE_AMD                            0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB                         0x9199
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_EXT                         0x9199
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_AMD                         0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS                        0x919A
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB                    0x919A
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_EXT                    0x919A
#define GL_MIN_SPARSE_LEVEL_AMD                                   0x919B
#define GL_MIN_LOD_WARNING_AMD                                    0x919C
#define GL_TEXTURE_BUFFER_OFFSET                                  0x919D
#define GL_TEXTURE_BUFFER_OFFSET_EXT                              0x919D
#define GL_TEXTURE_BUFFER_OFFSET_OES                              0x919D
#define GL_TEXTURE_BUFFER_SIZE                                    0x919E
#define GL_TEXTURE_BUFFER_SIZE_EXT                                0x919E
#define GL_TEXTURE_BUFFER_SIZE_OES                                0x919E
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT                        0x919F
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_EXT                    0x919F
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_OES                    0x919F
#define GL_STREAM_RASTERIZATION_AMD                               0x91A0
#define GL_VERTEX_ELEMENT_SWIZZLE_AMD                             0x91A4
#define GL_VERTEX_ID_SWIZZLE_AMD                                  0x91A5
#define GL_TEXTURE_SPARSE_ARB                                     0x91A6
#define GL_TEXTURE_SPARSE_EXT                                     0x91A6
#define GL_VIRTUAL_PAGE_SIZE_INDEX_ARB                            0x91A7
#define GL_VIRTUAL_PAGE_SIZE_INDEX_EXT                            0x91A7
#define GL_NUM_VIRTUAL_PAGE_SIZES_ARB                             0x91A8
#define GL_NUM_VIRTUAL_PAGE_SIZES_EXT                             0x91A8
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB             0x91A9
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_EXT             0x91A9
#define GL_NUM_SPARSE_LEVELS_ARB                                  0x91AA
#define GL_NUM_SPARSE_LEVELS_EXT                                  0x91AA
#define GL_PIXELS_PER_SAMPLE_PATTERN_X_AMD                        0x91AE
#define GL_PIXELS_PER_SAMPLE_PATTERN_Y_AMD                        0x91AF
#define GL_MAX_SHADER_COMPILER_THREADS_ARB                        0x91B0
#define GL_COMPLETION_STATUS_ARB                                  0x91B1
#define GL_COMPUTE_SHADER                                         0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS                             0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS                        0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS                             0x91BD
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT                           0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE                            0x91BF
#define GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB                       0x91BF
#define GL_FLOAT16_MAT2_AMD                                       0x91C5
#define GL_FLOAT16_MAT3_AMD                                       0x91C6
#define GL_FLOAT16_MAT4_AMD                                       0x91C7
#define GL_FLOAT16_MAT2x3_AMD                                     0x91C8
#define GL_FLOAT16_MAT2x4_AMD                                     0x91C9
#define GL_FLOAT16_MAT3x2_AMD                                     0x91CA
#define GL_FLOAT16_MAT3x4_AMD                                     0x91CB
#define GL_FLOAT16_MAT4x2_AMD                                     0x91CC
#define GL_FLOAT16_MAT4x3_AMD                                     0x91CD
#define GL_UNPACK_FLIP_Y_WEBGL                                    0x9240
#define GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL                         0x9241
#define GL_CONTEXT_LOST_WEBGL                                     0x9242
#define GL_UNPACK_COLORSPACE_CONVERSION_WEBGL                     0x9243
#define GL_BROWSER_DEFAULT_WEBGL                                  0x9244
#define GL_SHADER_BINARY_DMP                                      0x9250
#define GL_SMAPHS30_PROGRAM_BINARY_DMP                            0x9251
#define GL_SMAPHS_PROGRAM_BINARY_DMP                              0x9252
#define GL_DMP_PROGRAM_BINARY_DMP                                 0x9253
#define GL_GCCSO_SHADER_BINARY_FJ                                 0x9260
#define GL_COMPRESSED_R11_EAC                                     0x9270
#define GL_COMPRESSED_R11_EAC_OES                                 0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                              0x9271
#define GL_COMPRESSED_SIGNED_R11_EAC_OES                          0x9271
#define GL_COMPRESSED_RG11_EAC                                    0x9272
#define GL_COMPRESSED_RG11_EAC_OES                                0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                             0x9273
#define GL_COMPRESSED_SIGNED_RG11_EAC_OES                         0x9273
#define GL_COMPRESSED_RGB8_ETC2                                   0x9274
#define GL_COMPRESSED_RGB8_ETC2_OES                               0x9274
#define GL_COMPRESSED_SRGB8_ETC2                                  0x9275
#define GL_COMPRESSED_SRGB8_ETC2_OES                              0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2               0x9276
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2_OES           0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2              0x9277
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2_OES          0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                              0x9278
#define GL_COMPRESSED_RGBA8_ETC2_EAC_OES                          0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                       0x9279
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC_OES                   0x9279
#define GL_BLEND_PREMULTIPLIED_SRC_NV                             0x9280
#define GL_BLEND_OVERLAP_NV                                       0x9281
#define GL_UNCORRELATED_NV                                        0x9282
#define GL_DISJOINT_NV                                            0x9283
#define GL_CONJOINT_NV                                            0x9284
#define GL_BLEND_ADVANCED_COHERENT_KHR                            0x9285
#define GL_BLEND_ADVANCED_COHERENT_NV                             0x9285
#define GL_SRC_NV                                                 0x9286
#define GL_DST_NV                                                 0x9287
#define GL_SRC_OVER_NV                                            0x9288
#define GL_DST_OVER_NV                                            0x9289
#define GL_SRC_IN_NV                                              0x928A
#define GL_DST_IN_NV                                              0x928B
#define GL_SRC_OUT_NV                                             0x928C
#define GL_DST_OUT_NV                                             0x928D
#define GL_SRC_ATOP_NV                                            0x928E
#define GL_DST_ATOP_NV                                            0x928F
#define GL_PLUS_NV                                                0x9291
#define GL_PLUS_DARKER_NV                                         0x9292
#define GL_MULTIPLY                                               0x9294
#define GL_MULTIPLY_KHR                                           0x9294
#define GL_MULTIPLY_NV                                            0x9294
#define GL_SCREEN                                                 0x9295
#define GL_SCREEN_KHR                                             0x9295
#define GL_SCREEN_NV                                              0x9295
#define GL_OVERLAY                                                0x9296
#define GL_OVERLAY_KHR                                            0x9296
#define GL_OVERLAY_NV                                             0x9296
#define GL_DARKEN                                                 0x9297
#define GL_DARKEN_KHR                                             0x9297
#define GL_DARKEN_NV                                              0x9297
#define GL_LIGHTEN                                                0x9298
#define GL_LIGHTEN_KHR                                            0x9298
#define GL_LIGHTEN_NV                                             0x9298
#define GL_COLORDODGE                                             0x9299
#define GL_COLORDODGE_KHR                                         0x9299
#define GL_COLORDODGE_NV                                          0x9299
#define GL_COLORBURN                                              0x929A
#define GL_COLORBURN_KHR                                          0x929A
#define GL_COLORBURN_NV                                           0x929A
#define GL_HARDLIGHT                                              0x929B
#define GL_HARDLIGHT_KHR                                          0x929B
#define GL_HARDLIGHT_NV                                           0x929B
#define GL_SOFTLIGHT                                              0x929C
#define GL_SOFTLIGHT_KHR                                          0x929C
#define GL_SOFTLIGHT_NV                                           0x929C
#define GL_DIFFERENCE                                             0x929E
#define GL_DIFFERENCE_KHR                                         0x929E
#define GL_DIFFERENCE_NV                                          0x929E
#define GL_MINUS_NV                                               0x929F
#define GL_EXCLUSION                                              0x92A0
#define GL_EXCLUSION_KHR                                          0x92A0
#define GL_EXCLUSION_NV                                           0x92A0
#define GL_CONTRAST_NV                                            0x92A1
#define GL_INVERT_RGB_NV                                          0x92A3
#define GL_LINEARDODGE_NV                                         0x92A4
#define GL_LINEARBURN_NV                                          0x92A5
#define GL_VIVIDLIGHT_NV                                          0x92A6
#define GL_LINEARLIGHT_NV                                         0x92A7
#define GL_PINLIGHT_NV                                            0x92A8
#define GL_HARDMIX_NV                                             0x92A9
#define GL_HSL_HUE                                                0x92AD
#define GL_HSL_HUE_KHR                                            0x92AD
#define GL_HSL_HUE_NV                                             0x92AD
#define GL_HSL_SATURATION                                         0x92AE
#define GL_HSL_SATURATION_KHR                                     0x92AE
#define GL_HSL_SATURATION_NV                                      0x92AE
#define GL_HSL_COLOR                                              0x92AF
#define GL_HSL_COLOR_KHR                                          0x92AF
#define GL_HSL_COLOR_NV                                           0x92AF
#define GL_HSL_LUMINOSITY                                         0x92B0
#define GL_HSL_LUMINOSITY_KHR                                     0x92B0
#define GL_HSL_LUMINOSITY_NV                                      0x92B0
#define GL_PLUS_CLAMPED_NV                                        0x92B1
#define GL_PLUS_CLAMPED_ALPHA_NV                                  0x92B2
#define GL_MINUS_CLAMPED_NV                                       0x92B3
#define GL_INVERT_OVG_NV                                          0x92B4
#define GL_PURGED_CONTEXT_RESET_NV                                0x92BB
#define GL_PRIMITIVE_BOUNDING_BOX_ARB                             0x92BE
#define GL_PRIMITIVE_BOUNDING_BOX                                 0x92BE
#define GL_PRIMITIVE_BOUNDING_BOX_EXT                             0x92BE
#define GL_PRIMITIVE_BOUNDING_BOX_OES                             0x92BE
#define GL_ATOMIC_COUNTER_BUFFER                                  0x92C0
#define GL_ATOMIC_COUNTER_BUFFER_BINDING                          0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START                            0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE                             0x92C3
#define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE                        0x92C4
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS           0x92C5
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES    0x92C6
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER      0x92C7
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER    0x92CA
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER    0x92CB
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS                      0x92CC
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS                0x92CD
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT            0x92CD
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_OES            0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS             0x92CE
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT         0x92CE
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_OES         0x92CE
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS                    0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT                0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_OES                0x92CF
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS                    0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS                    0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS                             0x92D2
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS                       0x92D3
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT                   0x92D3
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_OES                   0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS                    0x92D4
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT                0x92D4
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_OES                0x92D4
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS                           0x92D5
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT                       0x92D5
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS_OES                       0x92D5
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS                           0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS                           0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                         0x92D8
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS                          0x92D9
#define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX                    0x92DA
#define GL_UNSIGNED_INT_ATOMIC_COUNTER                            0x92DB
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS                     0x92DC
#define GL_FRAGMENT_COVERAGE_TO_COLOR_NV                          0x92DD
#define GL_FRAGMENT_COVERAGE_COLOR_NV                             0x92DE
#define GL_DEBUG_OUTPUT                                           0x92E0
#define GL_DEBUG_OUTPUT_KHR                                       0x92E0
#define GL_UNIFORM                                                0x92E1
#define GL_UNIFORM_BLOCK                                          0x92E2
#define GL_PROGRAM_INPUT                                          0x92E3
#define GL_PROGRAM_OUTPUT                                         0x92E4
#define GL_BUFFER_VARIABLE                                        0x92E5
#define GL_SHADER_STORAGE_BLOCK                                   0x92E6
#define GL_IS_PER_PATCH                                           0x92E7
#define GL_IS_PER_PATCH_EXT                                       0x92E7
#define GL_IS_PER_PATCH_OES                                       0x92E7
#define GL_VERTEX_SUBROUTINE                                      0x92E8
#define GL_TESS_CONTROL_SUBROUTINE                                0x92E9
#define GL_TESS_EVALUATION_SUBROUTINE                             0x92EA
#define GL_GEOMETRY_SUBROUTINE                                    0x92EB
#define GL_FRAGMENT_SUBROUTINE                                    0x92EC
#define GL_COMPUTE_SUBROUTINE                                     0x92ED
#define GL_VERTEX_SUBROUTINE_UNIFORM                              0x92EE
#define GL_TESS_CONTROL_SUBROUTINE_UNIFORM                        0x92EF
#define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM                     0x92F0
#define GL_GEOMETRY_SUBROUTINE_UNIFORM                            0x92F1
#define GL_FRAGMENT_SUBROUTINE_UNIFORM                            0x92F2
#define GL_COMPUTE_SUBROUTINE_UNIFORM                             0x92F3
#define GL_TRANSFORM_FEEDBACK_VARYING                             0x92F4
#define GL_ACTIVE_RESOURCES                                       0x92F5
#define GL_MAX_NAME_LENGTH                                        0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES                               0x92F7
#define GL_MAX_NUM_COMPATIBLE_SUBROUTINES                         0x92F8
#define GL_NAME_LENGTH                                            0x92F9
#define GL_TYPE                                                   0x92FA
#define GL_ARRAY_SIZE                                             0x92FB
#define GL_OFFSET                                                 0x92FC
#define GL_BLOCK_INDEX                                            0x92FD
#define GL_ARRAY_STRIDE                                           0x92FE
#define GL_MATRIX_STRIDE                                          0x92FF
#define GL_IS_ROW_MAJOR                                           0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX                            0x9301
#define GL_BUFFER_BINDING                                         0x9302
#define GL_BUFFER_DATA_SIZE                                       0x9303
#define GL_NUM_ACTIVE_VARIABLES                                   0x9304
#define GL_ACTIVE_VARIABLES                                       0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER                            0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER                      0x9307
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT                  0x9307
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER_OES                  0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER                   0x9308
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT               0x9308
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER_OES               0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER                          0x9309
#define GL_REFERENCED_BY_GEOMETRY_SHADER_EXT                      0x9309
#define GL_REFERENCED_BY_GEOMETRY_SHADER_OES                      0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER                          0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER                           0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE                                   0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE                                 0x930D
#define GL_LOCATION                                               0x930E
#define GL_LOCATION_INDEX                                         0x930F
#define GL_LOCATION_INDEX_EXT                                     0x930F
#define GL_FRAMEBUFFER_DEFAULT_WIDTH                              0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT                             0x9311
#define GL_FRAMEBUFFER_DEFAULT_LAYERS                             0x9312
#define GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT                         0x9312
#define GL_FRAMEBUFFER_DEFAULT_LAYERS_OES                         0x9312
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES                            0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS             0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH                                  0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT                                 0x9316
#define GL_MAX_FRAMEBUFFER_LAYERS                                 0x9317
#define GL_MAX_FRAMEBUFFER_LAYERS_EXT                             0x9317
#define GL_MAX_FRAMEBUFFER_LAYERS_OES                             0x9317
#define GL_MAX_FRAMEBUFFER_SAMPLES                                0x9318
#define GL_RASTER_MULTISAMPLE_EXT                                 0x9327
#define GL_RASTER_SAMPLES_EXT                                     0x9328
#define GL_MAX_RASTER_SAMPLES_EXT                                 0x9329
#define GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT                      0x932A
#define GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT                  0x932B
#define GL_EFFECTIVE_RASTER_SAMPLES_EXT                           0x932C
#define GL_DEPTH_SAMPLES_NV                                       0x932D
#define GL_STENCIL_SAMPLES_NV                                     0x932E
#define GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV                       0x932F
#define GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV                     0x9330
#define GL_COVERAGE_MODULATION_TABLE_NV                           0x9331
#define GL_COVERAGE_MODULATION_NV                                 0x9332
#define GL_COVERAGE_MODULATION_TABLE_SIZE_NV                      0x9333
#define GL_WARP_SIZE_NV                                           0x9339
#define GL_WARPS_PER_SM_NV                                        0x933A
#define GL_SM_COUNT_NV                                            0x933B
#define GL_FILL_RECTANGLE_NV                                      0x933C
#define GL_SAMPLE_LOCATION_SUBPIXEL_BITS_ARB                      0x933D
#define GL_SAMPLE_LOCATION_SUBPIXEL_BITS_NV                       0x933D
#define GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_ARB                   0x933E
#define GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV                    0x933E
#define GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_ARB                  0x933F
#define GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV                   0x933F
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_ARB            0x9340
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV             0x9340
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_ARB                       0x9341
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_NV                        0x9341
#define GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_ARB          0x9342
#define GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV           0x9342
#define GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_ARB             0x9343
#define GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV              0x9343
#define GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB             0x9344
#define GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB                    0x9345
#define GL_CONSERVATIVE_RASTERIZATION_NV                          0x9346
#define GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV                      0x9347
#define GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV                      0x9348
#define GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV                    0x9349
#define GL_LOCATION_COMPONENT                                     0x934A
#define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX                        0x934B
#define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE                       0x934C
#define GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV                         0x9350
#define GL_VIEWPORT_SWIZZLE_NEGATIVE_X_NV                         0x9351
#define GL_VIEWPORT_SWIZZLE_POSITIVE_Y_NV                         0x9352
#define GL_VIEWPORT_SWIZZLE_NEGATIVE_Y_NV                         0x9353
#define GL_VIEWPORT_SWIZZLE_POSITIVE_Z_NV                         0x9354
#define GL_VIEWPORT_SWIZZLE_NEGATIVE_Z_NV                         0x9355
#define GL_VIEWPORT_SWIZZLE_POSITIVE_W_NV                         0x9356
#define GL_VIEWPORT_SWIZZLE_NEGATIVE_W_NV                         0x9357
#define GL_VIEWPORT_SWIZZLE_X_NV                                  0x9358
#define GL_VIEWPORT_SWIZZLE_Y_NV                                  0x9359
#define GL_VIEWPORT_SWIZZLE_Z_NV                                  0x935A
#define GL_VIEWPORT_SWIZZLE_W_NV                                  0x935B
#define GL_CLIP_ORIGIN                                            0x935C
#define GL_CLIP_DEPTH_MODE                                        0x935D
#define GL_NEGATIVE_ONE_TO_ONE                                    0x935E
#define GL_ZERO_TO_ONE                                            0x935F
#define GL_CLEAR_TEXTURE                                          0x9365
#define GL_TEXTURE_REDUCTION_MODE_ARB                             0x9366
#define GL_WEIGHTED_AVERAGE_ARB                                   0x9367
#define GL_FONT_GLYPHS_AVAILABLE_NV                               0x9368
#define GL_FONT_TARGET_UNAVAILABLE_NV                             0x9369
#define GL_FONT_UNAVAILABLE_NV                                    0x936A
#define GL_FONT_UNINTELLIGIBLE_NV                                 0x936B
#define GL_STANDARD_FONT_FORMAT_NV                                0x936C
#define GL_FRAGMENT_INPUT_NV                                      0x936D
#define GL_UNIFORM_BUFFER_UNIFIED_NV                              0x936E
#define GL_UNIFORM_BUFFER_ADDRESS_NV                              0x936F
#define GL_UNIFORM_BUFFER_LENGTH_NV                               0x9370
#define GL_MULTISAMPLES_NV                                        0x9371
#define GL_SUPERSAMPLE_SCALE_X_NV                                 0x9372
#define GL_SUPERSAMPLE_SCALE_Y_NV                                 0x9373
#define GL_CONFORMANT_NV                                          0x9374
#define GL_CONSERVATIVE_RASTER_DILATE_NV                          0x9379
#define GL_CONSERVATIVE_RASTER_DILATE_RANGE_NV                    0x937A
#define GL_CONSERVATIVE_RASTER_DILATE_GRANULARITY_NV              0x937B
#define GL_VIEWPORT_POSITION_W_SCALE_NV                           0x937C
#define GL_VIEWPORT_POSITION_W_SCALE_X_COEFF_NV                   0x937D
#define GL_VIEWPORT_POSITION_W_SCALE_Y_COEFF_NV                   0x937E
#define GL_NUM_SAMPLE_COUNTS                                      0x9380
#define GL_MULTISAMPLE_LINE_WIDTH_RANGE_ARB                       0x9381
#define GL_MULTISAMPLE_LINE_WIDTH_RANGE                           0x9381
#define GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY_ARB                 0x9382
#define GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY                     0x9382
#define GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE                  0x93A0
#define GL_BGRA8_EXT                                              0x93A1
#define GL_TEXTURE_USAGE_ANGLE                                    0x93A2
#define GL_FRAMEBUFFER_ATTACHMENT_ANGLE                           0x93A3
#define GL_PACK_REVERSE_ROW_ORDER_ANGLE                           0x93A4
#define GL_PROGRAM_BINARY_ANGLE                                   0x93A6
#define GL_COMPRESSED_RGBA_ASTC_4x4                               0x93B0
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR                           0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4                               0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR                           0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5                               0x93B2
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR                           0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5                               0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR                           0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6                               0x93B4
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR                           0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5                               0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR                           0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6                               0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR                           0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8                               0x93B7
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR                           0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5                              0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR                          0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6                              0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR                          0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8                              0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR                          0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10                             0x93BB
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR                         0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10                             0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR                         0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12                             0x93BD
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR                         0x93BD
#define GL_COMPRESSED_RGBA_ASTC_3x3x3_OES                         0x93C0
#define GL_COMPRESSED_RGBA_ASTC_4x3x3_OES                         0x93C1
#define GL_COMPRESSED_RGBA_ASTC_4x4x3_OES                         0x93C2
#define GL_COMPRESSED_RGBA_ASTC_4x4x4_OES                         0x93C3
#define GL_COMPRESSED_RGBA_ASTC_5x4x4_OES                         0x93C4
#define GL_COMPRESSED_RGBA_ASTC_5x5x4_OES                         0x93C5
#define GL_COMPRESSED_RGBA_ASTC_5x5x5_OES                         0x93C6
#define GL_COMPRESSED_RGBA_ASTC_6x5x5_OES                         0x93C7
#define GL_COMPRESSED_RGBA_ASTC_6x6x5_OES                         0x93C8
#define GL_COMPRESSED_RGBA_ASTC_6x6x6_OES                         0x93C9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4                       0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR                   0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4                       0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR                   0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5                       0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR                   0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5                       0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR                   0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6                       0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR                   0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5                       0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR                   0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6                       0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR                   0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8                       0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR                   0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5                      0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR                  0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6                      0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR                  0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8                      0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR                  0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10                     0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR                 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10                     0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR                 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12                     0x93DD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR                 0x93DD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES                 0x93E0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES                 0x93E1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES                 0x93E2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES                 0x93E3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES                 0x93E4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES                 0x93E5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES                 0x93E6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES                 0x93E7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES                 0x93E8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES                 0x93E9
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG                 0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG                 0x93F1
#define GL_PERFQUERY_COUNTER_EVENT_INTEL                          0x94F0
#define GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL                  0x94F1
#define GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL                   0x94F2
#define GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL                     0x94F3
#define GL_PERFQUERY_COUNTER_RAW_INTEL                            0x94F4
#define GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL                      0x94F5
#define GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL                    0x94F8
#define GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL                    0x94F9
#define GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL                     0x94FA
#define GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL                    0x94FB
#define GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL                    0x94FC
#define GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL                  0x94FD
#define GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL                0x94FE
#define GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL                0x94FF
#define GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL                  0x9500
#define GL_CONSERVATIVE_RASTER_MODE_NV                            0x954D
#define GL_CONSERVATIVE_RASTER_MODE_POST_SNAP_NV                  0x954E
#define GL_CONSERVATIVE_RASTER_MODE_PRE_SNAP_TRIANGLES_NV         0x954F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR           0x9630
#define GL_MAX_VIEWS_OVR                                          0x9631
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR     0x9632
#define GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR                0x9633
#define GL_GS_SHADER_BINARY_MTK                                   0x9640
#define GL_GS_PROGRAM_BINARY_MTK                                  0x9641
#define GL_MAX_SHADER_COMBINED_LOCAL_STORAGE_FAST_SIZE_EXT        0x9650
#define GL_MAX_SHADER_COMBINED_LOCAL_STORAGE_SIZE_EXT             0x9651
#define GL_FRAMEBUFFER_INCOMPLETE_INSUFFICIENT_SHADER_COMBINED_LOCAL_STORAGE_EXT 0x9652
#define GL_RASTER_POSITION_UNCLIPPED_IBM                          0x19262
#define GL_CULL_VERTEX_IBM                                        103050
#define GL_ALL_STATIC_DATA_IBM                                    103060
#define GL_STATIC_VERTEX_ARRAY_IBM                                103061
#define GL_VERTEX_ARRAY_LIST_IBM                                  103070
#define GL_NORMAL_ARRAY_LIST_IBM                                  103071
#define GL_COLOR_ARRAY_LIST_IBM                                   103072
#define GL_INDEX_ARRAY_LIST_IBM                                   103073
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM                           103074
#define GL_EDGE_FLAG_ARRAY_LIST_IBM                               103075
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM                          103076
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM                         103077
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM                           103080
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM                           103081
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM                            103082
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM                            103083
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM                    103084
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM                        103085
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM                   103086
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM                  103087
#define GL_PREFER_DOUBLEBUFFER_HINT_PGI                           0x1A1F8
#define GL_CONSERVE_MEMORY_HINT_PGI                               0x1A1FD
#define GL_RECLAIM_MEMORY_HINT_PGI                                0x1A1FE
#define GL_NATIVE_GRAPHICS_HANDLE_PGI                             0x1A202
#define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI                         0x1A203
#define GL_NATIVE_GRAPHICS_END_HINT_PGI                           0x1A204
#define GL_ALWAYS_FAST_HINT_PGI                                   0x1A20C
#define GL_ALWAYS_SOFT_HINT_PGI                                   0x1A20D
#define GL_ALLOW_DRAW_OBJ_HINT_PGI                                0x1A20E
#define GL_ALLOW_DRAW_WIN_HINT_PGI                                0x1A20F
#define GL_ALLOW_DRAW_FRG_HINT_PGI                                0x1A210
#define GL_ALLOW_DRAW_MEM_HINT_PGI                                0x1A211
#define GL_STRICT_DEPTHFUNC_HINT_PGI                              0x1A216
#define GL_STRICT_LIGHTING_HINT_PGI                               0x1A217
#define GL_STRICT_SCISSOR_HINT_PGI                                0x1A218
#define GL_FULL_STIPPLE_HINT_PGI                                  0x1A219
#define GL_CLIP_NEAR_HINT_PGI                                     0x1A220
#define GL_CLIP_FAR_HINT_PGI                                      0x1A221
#define GL_WIDE_LINE_HINT_PGI                                     0x1A222
#define GL_BACK_NORMALS_HINT_PGI                                  0x1A223
#define GL_VERTEX_DATA_HINT_PGI                                   0x1A22A
#define GL_VERTEX_CONSISTENT_HINT_PGI                             0x1A22B
#define GL_MATERIAL_SIDE_HINT_PGI                                 0x1A22C
#define GL_MAX_VERTEX_HINT_PGI                                    0x1A22D

#ifdef GLFL_ENABLE_PROXY
#  include "glfl_proxy_proto__.h"
#else
#  include "glfl_simple_proto__.h"
#endif

#endif
