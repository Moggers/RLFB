project('opendom', 'c', default_options: 'c_std=c99')

cc = meson.get_compiler('c')
libm = cc.find_library('m', required : false)

flecs_proj = subproject('flecs')
flecs_dep = flecs_proj.get_variable('flecs_dep')

vulkan = dependency('vulkan')
glfw = dependency('glfw3')
assimp = dependency('assimp')

executable('main', 'src/main.c', dependencies: [vulkan, glfw, libm, assimp, flecs_dep])
