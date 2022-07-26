project(
  'video-app', 
  'c','cpp',
  version : '0.1',
  default_options : [
    'warning_level=3', 
    'cpp_std=c++17'
  ]
)

sources = [
  'src/main.cpp',
  'src/video_reader.cpp',
  
  'src/glad.c',
]

incdir = include_directories('include')

#add gfw3 to deps
CC = meson.get_compiler('cpp')

video_deps = []
video_deps += dependency('glfw3', required : true)
video_deps += dependency('glm', required : true, fallback : ['glm','glm_dep'])
video_deps += dependency('gl', required : true)
video_deps += CC.find_library('dl', required : false)

av_libs = [
    ['avcodec', '55.28.1'],
    ['avformat',  '54.0.0'],
    ['avutil', '51.0.0'],
    ['avdevice', '54.0.0'],
    ['avfilter', '3.0.0'],
    ['swscale', '2.0.0'],
    ['swresample', '0.0.0'],
    ['postproc', '52.0.0'],
]

foreach lib : av_libs
    video_deps += [
        dependency(
            'lib@0@'.format(lib[0]),
            required: true,
            #fallback : ['ffmpeg', 'lib@0@_dep'.format(lib[0])],
            version : '>=@0@'.format(lib[1]))
    ]
endforeach

executable(
  'video-app',
  sources,
  dependencies : video_deps,
  include_directories : incdir,
  install : true
)
